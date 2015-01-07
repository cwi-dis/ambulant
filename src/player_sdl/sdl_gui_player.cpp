// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* 
 * sdl_gui_player.cpp - SDL2 player for Ambulant
 *               this code is copied from gtk_mainloop.cpp
 */

#ifdef WITH_SDL2
#include "ambulant/config/config.h"
#include "ambulant/gui/SDL/sdl_factory.h"

#include "ambulant/lib/document.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/posix_datasource.h"
//#define WITH_STDIO_DATASOURCE
#ifdef WITH_STDIO_DATASOURCE
#include "ambulant/net/stdio_datasource.h"
#endif
#ifdef WITH_FFMPEG
#include "ambulant/net/ffmpeg_factory.h"
#endif
#include "ambulant/gui/none/none_factory.h"
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_fill.h"
#include "ambulant/gui/SDL/sdl_image_renderer.h"
#include "ambulant/gui/SDL/sdl_pango_smiltext.h"
#include "ambulant/gui/SDL/sdl_ttf_smiltext.h"
//#include "ambulant/gui/SDL/sdl_text_renderer.h"
#include "ambulant/gui/SDL/sdl_video.h"
#include "ambulant/gui/SDL/sdl_window.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/lib/parser_factory.h"
#ifdef WITH_XERCES_BUILTIN
#include "ambulant/lib/xerces_parser.h"
#endif

#include "sdl_gui_player.h"
#include "sdl_gui.h"
#include <unistd.h>

using namespace ambulant;
using namespace gui::sdl;

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

void
open_web_browser(const std::string &href)
{
	// The only standard I could find (sigh): use the $BROWSER variable.
	// This code is a big hack, because we assume it'll be replaced soon. haha! :-)
	char cmdbuf[2048];
	char *browserlist = getenv("BROWSER");
	int rv;

	if (browserlist == NULL) {
		lib::logger::get_logger()->error(gettext("$BROWSER not set: cannot open webpage <%s>"), href.c_str());
		return;
	}
	char *colon = index(browserlist, ':');
	if (colon) *colon = 0; // Brrrr...
	snprintf(cmdbuf, sizeof(cmdbuf), "%s %s &", browserlist, href.c_str());
	AM_DBG lib::logger::get_logger()->debug("Starting command: %s", cmdbuf);
	rv = ::system(cmdbuf);
	if (rv) {
		lib::logger::get_logger()->error(gettext("Attempt to start browser returned status %d. Command: %s"), rv, cmdbuf);
	}
}

sdl_gui_player::sdl_gui_player(sdl_gui* gui)
:	m_gui(gui),
	m_logger(NULL),
	m_ambulant_sdl_window(NULL),
	m_sdl_ambulant_window(NULL),
	m_running(false)
{
	gui_player();
	m_logger = lib::logger::get_logger();
	set_embedder(this);
	smil2::test_attrs::set_default_tests_attrs();
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("Standalone"), true);
	init_factories();
	init_plugins();

	const char *filename = NULL;
	if (m_gui) filename = m_gui->filename();
	if (filename == NULL) {
        	return; // no argument given
	}
	net::url url = net::url::from_filename(filename);
	m_doc = create_document(url);
	if (!m_doc) {
		return;
	}
	create_top_window(filename);
	common::preferences *prefs = common::preferences::get_preferences();
	m_logger->debug(" creating smil2 player %s", prefs->repr().c_str());
	m_player = create_player(filename);
}

sdl_gui_player::~sdl_gui_player()
{
	AM_DBG m_logger->debug("sdl_gui_player::~sdl_gui_player() m_player=0x%x", m_player);
//	delete m_gui_screen;
	// We need to delete gui_player::m_player before deleting m_doc, because the
	// timenode graph in the player has referrences to the node graph in m_doc.
	if (m_player) {
		m_player->terminate();
		m_player->release();
		m_player = NULL;
	}
	if (m_embedder) {
		set_embedder(NULL);
	}
 	if (m_doc) {
		delete m_doc;
		m_doc = NULL;
	}
}

void
sdl_gui_player::init_datasource_factory()
{
	net::datasource_factory *df = new net::datasource_factory();
	set_datasource_factory(df);
#ifdef WITH_FFMPEG
	AM_DBG m_logger->debug("player::player: add ffmpeg_audio_datasource_factory");
	df->add_audio_factory(net::get_ffmpeg_audio_datasource_factory());
	AM_DBG m_logger->debug("sdl_gui_player::sdl_gui_player: add ffmpeg_audio_decoder_finder");
	df->add_audio_decoder_finder(net::get_ffmpeg_audio_decoder_finder());
#ifdef WITH_RESAMPLE_DATASOURCE
	AM_DBG m_logger->debug("sdl_gui_player::sdl_gui_player: add ffmpeg_audio_filter_finder");
	df->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
#endif
	AM_DBG m_logger->debug("player::player: add ffmpeg_video_datasource_factory");
	df->add_video_factory(net::get_ffmpeg_video_datasource_factory());
	AM_DBG m_logger->debug("player::player: add ffmpeg_raw_datasource_factory");
	df->add_raw_factory(net::get_ffmpeg_raw_datasource_factory());
#endif

#ifdef WITH_STDIO_DATASOURCE
	// This is for debugging only, really: the posix datasource
	// should always perform better, and is always available on OSX.
	// If you define WITH_STDIO_DATASOURCE we prefer to use the stdio datasource,
	// however.

	AM_DBG m_logger->debug("sdl_gui_player::sdl_gui_player: add stdio_datasource_factory");
	df->add_raw_factory(net::create_stdio_datasource_factory());
#endif
	AM_DBG m_logger->debug("sdl_gui_player::sdl_gui_player: add posix_datasource_factory");
	df->add_raw_factory(net::create_posix_datasource_factory());
}


void
sdl_gui_player::redraw() {
	if (m_sdl_ambulant_window != NULL) {
		ambulant_sdl_window* asw = m_sdl_ambulant_window->get_ambulant_sdl_window();
		if (asw != NULL) {
			asw->redraw(asw->get_bounds());
		}
	}
}

void
sdl_gui_player::redraw(void* winp, void* rp) {
	if (winp != NULL) {
		ambulant_sdl_window* asw = (ambulant_sdl_window*) winp;
		if (m_ambulant_sdl_window == NULL) {
			m_ambulant_sdl_window = asw;
			m_sdl_ambulant_window = asw->get_sdl_ambulant_window();
		}
		rect r = *(rect*) rp;
		if (asw != NULL) {
			asw->redraw(r);
		}
	}
}

void
sdl_gui_player::init_playable_factory()
{
	common::global_playable_factory *pf = common::get_global_playable_factory();
	set_playable_factory(pf);

	AM_DBG m_logger->debug("sdl_gui_player: adding sdl playable factories");
	pf->add_factory(create_sdl_fill_playable_factory(this, NULL));
#ifdef WITH_SDL_IMAGE
	pf->add_factory(create_sdl_image_playable_factory(this, NULL));
#endif
#if defined(WITH_SDL_PANGO) || defined(WITH_SDL_TTF)
	pf->add_factory(create_sdl_smiltext_playable_factory(this, NULL));
#endif
#if defined(WITH_SDL_PANGO) || defined(WITH_SDL_TTF)
	pf->add_factory(create_sdl_text_playable_factory(this, NULL));
#endif
	pf->add_factory(create_sdl_video_playable_factory(this, NULL));

//#ifdef WITH_SDL
	AM_DBG lib::logger::get_logger()->debug("sdl_gui_player: add factory for SDL");
	pf->add_factory(gui::sdl::create_sdl_playable_factory(this));
//#endif // WITH_SDL
}


ambulant::common::player*
sdl_gui_player::create_player(const char* filename) {
	ambulant::common::player* player;
	player = create_smil2_player(m_doc, this, get_embedder());

	player->initialize();

	return player;
}

void
sdl_gui_player::init_window_factory()
{
//X	m_sdl_ambulant_window = new sdl_ambulant_window(m_gui->get_document_container()); // delayed for correct size
	common::window_factory* sdl_wf = gui::sdl::create_sdl_window_factory(m_sdl_ambulant_window, this);
	set_window_factory(sdl_wf);
}

void
sdl_gui_player::create_top_window (const char *filename) {
	m_size = get_window_factory()->get_default_size();
	int width = m_size.w;
	int height = m_size.h;
	m_origin = lib::point(0,0);
	m_rect = lib::rect(m_origin, m_size);
	AM_DBG lib::logger::get_logger()->debug("sdl_gui_player::create_top_window(0x%x): width = %d, height = %d",(void *)this, width, height);
}

void
sdl_gui_player::init_parser_factory()
{
	lib::global_parser_factory *pf = lib::global_parser_factory::get_parser_factory();
	set_parser_factory(pf);
#ifdef WITH_XERCES_BUILTIN
	pf->add_factory(new lib::xerces_factory());
	AM_DBG m_logger->debug("player::player: add xerces_factory");
#endif
}

void
sdl_gui_player::resize_window(int w, int h) {
	if (m_ambulant_sdl_window != NULL) {
		m_ambulant_sdl_window->resize_window (w, h);
	}
}

bool
sdl_gui_player::user_event(SDL_Point& p, int what) {
	if (m_sdl_ambulant_window == NULL || m_ambulant_sdl_window == NULL) {
		return false;
	}
	return m_ambulant_sdl_window->user_event(m_sdl_ambulant_window->transform(p), what);
}

void
sdl_gui_player::show_file(const net::url &url)
{
	open_web_browser(url.get_url());
}



#endif//WITH_SDL2
