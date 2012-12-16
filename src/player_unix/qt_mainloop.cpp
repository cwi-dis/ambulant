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

#ifdef WITH_SDL
#include "ambulant/gui/SDL/sdl_factory.h"
#endif
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
#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_fill.h"
#include "ambulant/gui/qt/qt_html_renderer.h"
#include "ambulant/gui/qt/qt_image_renderer.h"
#include "ambulant/gui/qt/qt_smiltext.h"
#include "ambulant/gui/qt/qt_text_renderer.h"
#include "ambulant/gui/qt/qt_video_renderer.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/lib/parser_factory.h"
#ifdef WITH_XERCES_BUILTIN
#include "ambulant/lib/xerces_parser.h"
#endif

//#include "ambulant/lib/expat_parser.h"


//#include "ambulant/lib/tree_builder.h"

#include "qt_mainloop.h"
#include <qapplication.h>
#include <qevent.h>

using namespace ambulant;
using namespace gui::qt;

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif


void
open_web_browser(const std::string &href) {
	// The only standard I could find (sigh): use the $BROWSER variable.
	// This code is a big hack, because we assume it'll be replaced soon. haha! :-)
	char *browserlist = getenv("BROWSER");
	if (browserlist == NULL) {
		lib::logger::get_logger()->error(gettext("$BROWSER not set: cannot open webpage <%s>"), href.c_str());
		return;
	}
	char *colon = index(browserlist, ':');
	if (colon) *colon = 0; // Brrrr...
	char cmdbuf[2048];
	snprintf(cmdbuf, sizeof(cmdbuf), "%s %s &", browserlist, href.c_str());
	AM_DBG lib::logger::get_logger()->debug("Starting command: %s", cmdbuf);
	int rv = ::system(cmdbuf);
	if (rv) {
		lib::logger::get_logger()->error(gettext("Attempt to start browser returned status %d. Command: %s"), rv, cmdbuf);
	}
}

qt_mainloop::qt_mainloop(qt_gui* gui, int mbheight)
:	m_gui(gui)
{
	m_logger = lib::logger::get_logger();
	set_embedder(this);
	smil2::test_attrs::set_default_tests_attrs();
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("Standalone"), true);
	common::window_factory *wf = create_qt_window_factory(m_gui, mbheight, this);
	assert(wf);
	set_window_factory(wf);
	init_factories();
	init_plugins();

	const char *filename = m_gui->filename();
	net::url url = net::url::from_filename(filename);
	m_doc = create_document(url);
	if (!m_doc) {
		return;
	}
	common::preferences *prefs = common::preferences::get_preferences();
	m_logger->debug(" creating smil2 player %s", prefs->repr().c_str());
	m_player = create_player(filename);
}

qt_mainloop::~qt_mainloop() {
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

ambulant::common::player*
qt_mainloop::create_player(const char* filename) {
	ambulant::common::player* player;
	player = create_smil2_player(m_doc, this, m_embedder);

	player->initialize();

	return player;
}

void
qt_mainloop::init_playable_factory() {
	common::global_playable_factory *pf = common::get_global_playable_factory();
	set_playable_factory(pf);

	AM_DBG m_logger->debug("qt_mainloop: adding Qt playable factories");
	pf->add_factory(create_qt_fill_playable_factory(this, NULL));
#ifdef	WITH_QT_HTML_WIDGET
	pf->add_factory(create_qt_html_playable_factory(this, NULL));
#endif
	pf->add_factory(create_qt_image_playable_factory(this, NULL));
	pf->add_factory(create_qt_smiltext_playable_factory(this, NULL));
	pf->add_factory(create_qt_text_playable_factory(this, NULL));
	pf->add_factory(create_qt_video_playable_factory(this, NULL));

#ifdef WITH_SDL
	AM_DBG lib::logger::get_logger()->debug("qt_mainloop: add factory for SDL");
	pf->add_factory(gui::sdl::create_sdl_playable_factory(this));
#endif // WITH_SDL
}

void
qt_mainloop::init_datasource_factory() {
	net::datasource_factory *df = new net::datasource_factory();
	set_datasource_factory(df);
#ifndef NONE_PLAYER
#ifdef WITH_FFMPEG
	AM_DBG lib::logger::get_logger()->debug("qt_mainloop: add ffmpeg_video_datasource_factory");
	df->add_video_factory(net::get_ffmpeg_video_datasource_factory());
	AM_DBG lib::logger::get_logger()->debug("qt_mainloop: add ffmpeg_audio_datasource_factory");
	df->add_audio_factory(net::get_ffmpeg_audio_datasource_factory());
	AM_DBG lib::logger::get_logger()->debug("qt_mainloop: add ffmpeg_audio_decoder_finder");
	df->add_audio_decoder_finder(net::get_ffmpeg_audio_decoder_finder());
	AM_DBG lib::logger::get_logger()->debug("qt_mainloop: add ffmpeg_audio_filter_finder");
	df->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
	AM_DBG lib::logger::get_logger()->debug("qt_mainloop: add ffmpeg_raw_datasource_factory");
	df->add_raw_factory(net::get_ffmpeg_raw_datasource_factory());
#endif // WITH_FFMPEG
#endif // NONE_PLAYER
#ifdef WITH_STDIO_DATASOURCE
	// This is for debugging only, really: the posix datasource
	// should always perform better, and is always available on OSX.
	// If you define WITH_STDIO_DATASOURCE we prefer to use the stdio datasource,
	// however.
	AM_DBG lib::logger::get_logger()->debug("qt_mainloop: add stdio_datasource_factory");
	df->add_raw_factory(net::create_stdio_datasource_factory());
#endif
	AM_DBG lib::logger::get_logger()->debug("qt_mainloop: add posix_datasource_factory");
	df->add_raw_factory(net::create_posix_datasource_factory());
}

void
qt_mainloop::init_parser_factory() {
	lib::global_parser_factory *pf = lib::global_parser_factory::get_parser_factory();
	set_parser_factory(pf);
#ifdef WITH_XERCES_BUILTIN
	pf->add_factory(new lib::xerces_factory());
	AM_DBG m_logger->debug("mainloop::mainloop: add xerces_factory");
#endif
}

void
qt_mainloop::show_file(const net::url &url) {
	open_web_browser(url.get_url());
}

void
qt_mainloop::done(common::player *p) {
	AM_DBG m_logger->debug("qt_mainloop: implementing: done()");
	m_gui->player_done();
}

// return true when the last player is done
bool
qt_mainloop::player_done() {
	AM_DBG m_logger->debug("qt_mainloop: implementing: player_done");
//TBD	m_timer->pause();
//TBD	m_update_event = 0;
//TBD	clear_transitions();
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
	return m_player->is_done();
}

void
qt_mainloop::close(common::player *p) {
	AM_DBG m_logger->trace("qt_mainloop: implementing: close document");
	stop();
}

void
qt_mainloop::open(net::url newdoc, bool start, common::player *old) {
	AM_DBG m_logger->trace("qt_mainloop::open \"%s\"",newdoc.get_url().c_str());
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
	m_gui->internal_message(qt_logger::CUSTOM_NEW_DOCUMENT,
				strdup(msg.c_str()));
}

void
qt_mainloop::player_start(QString document_name, bool start, bool old) {
	AM_DBG m_logger->debug("player_start(%s,%d,%d)",document_name.ascii(),start,old);
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
	AM_DBG m_logger->debug("Creating player instance for: %s", document_name.ascii());
	m_player = create_player(document_name);
	if(start) {
		m_player->start();
	}
}
