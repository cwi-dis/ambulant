// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
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
//TBD	AM_DBG m_logger->debug("sdl_gui_player::~sdl_gui_player() m_player=0x%x", m_player);
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
//	delete m_window_factory;
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
	AM_DBG m_logger->debug("sdl_gui_player::sdl_gui_player: add ffmpeg_audio_filter_finder");
	df->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
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
		ambulant_sdl_window* sdl_window = m_sdl_ambulant_window->get_ambulant_sdl_window();
		if (sdl_window != NULL) {
			sdl_window->redraw(m_rect);
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
//TBD	pf->add_factory(create_sdl_video_playable_factory(this, NULL));

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
#ifdef JNK
	static SDL_Renderer* s_renderer = NULL; //XXXX member !
	static SDL_Texture* s_texture = NULL; //XXXX member !
	static SDL_Window* s_window = NULL; //XXXX member, embed  !
	if (s_texture == NULL) {
		s_window = SDL_CreateWindow(basename(filename), 0,0,width,height,0); //XXXX consider SDL_CreateWindowFrom(XwinID) !
		assert (s_window);
		s_renderer = SDL_CreateRenderer(s_window, -1, SDL_RENDERER_ACCELERATED);
		if (s_renderer == NULL) {
			AM_DBG lib::logger::get_logger()->trace("sdl_video_renderer.redraw(0x%x): trying software renderer", this);
			s_renderer = SDL_CreateRenderer(s_window, -1, SDL_RENDERER_SOFTWARE);
			if (s_renderer == NULL) {
				lib::logger::get_logger()->warn("Cannot open: %s", "SDL video renderer");
				return;
			}
		}
		assert(s_renderer);
		s_texture = SDL_CreateTexture(s_renderer, SDL_PIXELFORMAT, SDL_TEXTUREACCESS_STREAMING, width, height);
	}
	assert(s_texture);
#endif//JNK
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

bool
sdl_gui_player::user_event(const point& p, int what) {
	bool rv = false;
	if (m_ambulant_sdl_window != NULL) {
		rv = m_ambulant_sdl_window->user_event(p, what);
	}
	return rv;
}

bool
sdl_gui_player::user_event(SDL_Point& p, int what) {
	point am_p(p.x, p.y);
	return user_event(am_p, what);
}

void
sdl_gui_player::show_file(const net::url &url)
{
	open_web_browser(url.get_url());
}


#ifdef JNK

ambulant::common::player*
sdl_gui_player::create_player(const char* filename) {
	ambulant::common::player* player;
	player = create_smil2_player(m_doc, this, get_embedder());

	player->initialize();

	return player;
}

void
sdl_gui_player::done(common::player *p)
{
	AM_DBG m_logger->debug("sdl_gui_player: implementing: done()");
	//m_gui->player_done();
}

bool
sdl_gui_player::player_done()
  // return true when the last player is done
{
	AM_DBG m_logger->debug("sdl_gui_player: implementing: player_done");
#if 0
	if(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_gui = pf->windows;
		m_player = pf->player;
		delete pf;
		m_player->resume();
//TBD		m_player->need_redraw();
		return false;
	}
#endif
	return true;
}

void
sdl_gui_player::close(common::player *p)
{
	AM_DBG m_logger->trace("sdl_gui_player: implementing: close document");
	stop();
}

void
sdl_gui_player::open(net::url newdoc, bool start, common::player *old)
{
	AM_DBG m_logger->trace("sdl_gui_player::open \"%s\"",newdoc.get_url().c_str());
	// Parse the provided URL.
	m_doc = create_document(newdoc);
	if(!m_doc) {
		m_logger->error(gettext("%s: Cannot build DOM tree"),
				newdoc.get_url().c_str());
		return;
	}
	// send msg to gui thread
	std::string msg("");
	msg += start ? "S-" : "	 ";
	msg += old	 ? "O-" : "	 ";
	msg += newdoc.get_url();
	m_gui->internal_message(sdl_logger::CUSTOM_NEW_DOCUMENT,
				strdup(msg.c_str()));
}

void
sdl_gui_player::player_start(gchar* document_name, bool start, bool old)
{
	AM_DBG m_logger->debug("player_start(%s,%d,%d)",document_name,start,old); //m_logger->debug("player_start(%s,%d,%d)",document_name.ascii(),start,old);
	if (old) {
		m_player->stop();
		delete m_player;
		m_player = create_player(document_name);
		if (start)
			m_player->start();
		return;
	}
#if 0
	if(m_player) {
	// Push the old frame on the stack
		m_player->pause();
		frame *pf = new frame();
		pf->windows = m_gui;
		pf->player = m_player;
//TBD		m_gui->erase();
		m_player = NULL;
		m_frames.push(pf);
	}
#endif
	// Create a player instance
	AM_DBG m_logger->debug("Creating player instance for: %s", document_name);
	m_player = create_player(document_name);
	if(start) {
		m_player->start();
	}
}

/*
char* sdl_gui_player::convert_data_to_image(const guchar* data, gsize size){
	GdkPixbufLoader *loader =  gdk_pixbuf_loader_new ();
	char* filename = 0;
	GError *error = NULL;
	GdkPixbuf *pixbuf;
	if (gdk_pixbuf_loader_write(loader, (const guchar*) data, (gsize) size, 0))
	{
		pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
	}else{
		g_message("Could not get Loader working\n");
		return NULL;
	}
	if (!pixbuf) {
		g_message ("Could not create the pixbuf\n");
		return NULL;
	}
	filename = "mytest.jpeg";
	gdk_pixbuf_save (pixbuf, filename, "jpeg", &error, "quality", "100", NULL);
	return filename;
}
*/

ambulant::common::gui_screen*
sdl_gui_player::get_gui_screen(){
	return m_sdl_ambulant_window;
}
#endif//JNK

#endif//WITH_SDL2
