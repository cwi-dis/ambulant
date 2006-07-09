// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

#define WITH_FFMPEG_VIDEO
// Define NONE_PLAYER to skip all cocoa support but use the dummy
// none_window and none_playable in stead.
//#define NONE_PLAYER

#include "MyAmbulantView.h"
#include <iostream>
#include <ApplicationServices/ApplicationServices.h>
#include "mainloop.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/timer.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#ifdef WITH_SDL
#include "ambulant/gui/SDL/sdl_gui.h"
#endif
#ifdef NONE_PLAYER
#include "ambulant/gui/none/none_gui.h"
#endif
#include "ambulant/net/datasource.h"
#include "ambulant/net/posix_datasource.h"
//#define WITH_STDIO_DATASOURCE
#ifdef WITH_STDIO_DATASOURCE
#include "ambulant/net/stdio_datasource.h"
#endif
#ifdef WITH_FFMPEG
#include "ambulant/net/ffmpeg_factory.h"
#endif
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/common/plugin_engine.h"
#ifdef WITH_LIVE
#include "ambulant/net/rtsp_factory.h"
#endif

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

mainloop::mainloop(const char *urlstr, void *view,
	bool use_mms, ambulant::common::embedder *app)
:   common::gui_player(),
	m_view(view),
	m_gui_screen(NULL)
{
	set_embedder(app);
	AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop(0x%x): created", (void*)this);
	init_factories();
	
	init_plugins();
	
	ambulant::net::url url = ambulant::net::url::from_url(urlstr);
	m_doc = create_document(url);
	if (!m_doc) {
		lib::logger::get_logger()->error(gettext("%s: Cannot build DOM tree"), urlstr);
		return;
	}
	if (use_mms)
		m_player = common::create_mms_player(m_doc, this);
	else
		m_player = common::create_smil2_player(m_doc, this, m_embedder);

	m_player->set_feedback(this);
	m_player->initialize();

	const std::string& id = url.get_ref();
	if (id != "") {
		const ambulant::lib::node *node = m_doc->get_node(id);
		if (!node)
			lib::logger::get_logger()->warn(gettext("%s: node ID not found"), id.c_str());
		goto_node(node);
	}
}

void
mainloop::init_playable_factory()
{
	m_playable_factory = common::get_global_playable_factory();
#ifndef NONE_PLAYER
	m_playable_factory->add_factory(new gui::cocoa::cocoa_renderer_factory(this));
#ifdef WITH_SDL
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add factory for SDL");
	m_playable_factory->add_factory( new gui::sdl::sdl_renderer_factory(this) );      
#endif // WITH_SDL
#endif // NONE_PLAYER
}

void
mainloop::init_window_factory()
{
#ifdef NONE_PLAYER
	// Replace the real window factory with a none_window_factory instance.
	m_window_factory = new gui::none::none_window_factory();
#else
	m_window_factory = new gui::cocoa::cocoa_window_factory(m_view);
#endif // NONE_PLAYER
}

void
mainloop::init_datasource_factory()
{
	m_datasource_factory = new net::datasource_factory();
#ifndef NONE_PLAYER
#ifdef WITH_LIVE	
	AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add live_audio_datasource_factory");
	m_datasource_factory->add_video_factory(new net::live_video_datasource_factory());
	m_datasource_factory->add_audio_factory(new net::live_audio_datasource_factory()); 
#endif
#ifdef WITH_FFMPEG
#ifdef WITH_FFMPEG_VIDEO
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_video_datasource_factory");
	m_datasource_factory->add_video_factory(net::get_ffmpeg_video_datasource_factory());
#endif // WITH_FFMPEG_VIDEO
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_datasource_factory");
	m_datasource_factory->add_audio_factory(net::get_ffmpeg_audio_datasource_factory());
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_parser_finder");
	m_datasource_factory->add_audio_parser_finder(net::get_ffmpeg_audio_parser_finder());
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_filter_finder");
	m_datasource_factory->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_raw_datasource_factory");
	m_datasource_factory->add_raw_factory(net::get_ffmpeg_raw_datasource_factory());
#endif // WITH_FFMPEG
#endif // NONE_PLAYER
#ifdef WITH_STDIO_DATASOURCE
	// This is for debugging only, really: the posix datasource
	// should always perform better, and is always available on OSX.
	// If you define WITH_STDIO_DATASOURCE we prefer to use the stdio datasource,
	// however.
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add stdio_datasource_factory");
	m_datasource_factory->add_raw_factory(net::get_stdio_datasource_factory());
#endif
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add posix_datasource_factory");
	m_datasource_factory->add_raw_factory(net::get_posix_datasource_factory());
}

void
mainloop::init_parser_factory()
{
	m_parser_factory = lib::global_parser_factory::get_parser_factory();	
}

mainloop::~mainloop()
{
	delete m_doc;
	m_doc = NULL;
	delete m_gui_screen;
	delete m_window_factory;
}

void
mainloop::restart(bool reparse)
{
	bool playing = is_play_active();
	bool pausing = is_pause_active();
	stop();
	
	delete m_player;
	m_player = 0;
	if (reparse) {
		m_doc = create_document(m_url);
		if(!m_doc) {
			lib::logger::get_logger()->show("Failed to parse document %s", m_url.get_url().c_str());
			return;
		}
	}
	AM_DBG lib::logger::get_logger()->debug("Creating player instance for: %s", m_url.get_url().c_str());
	// XXXX
	m_player = common::create_smil2_player(m_doc, this, m_embedder);

	m_player->set_feedback(this);
	m_player->initialize();

	if (playing || pausing) play();
	if (pausing) pause();
}

common::gui_screen *
mainloop::get_gui_screen()
{
	if (!m_gui_screen) m_gui_screen = new gui::cocoa::cocoa_gui_screen(m_view);
	return m_gui_screen;
}

void
mainloop::node_focussed(const lib::node *n)
{
	if (n == NULL) {
		AM_DBG lib::logger::get_logger()->debug("node_focussed(0)");
		set_statusline(m_view, "");
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("node_focussed(%s)", n->get_sig().c_str());
	const char *alt = n->get_attribute("alt");
	if (alt) {
		AM_DBG lib::logger::get_logger()->debug("node_focussed: alt=%s", alt);
		set_statusline(m_view, alt);
		return;
	}
	const char *href = n->get_attribute("href");
	if (href) {
		AM_DBG lib::logger::get_logger()->debug("node_focussed: href=%s", href);
		std::string msg = "Go to ";
		msg += href;
		set_statusline(m_view, msg.c_str());
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("node_focussed: nothing to show");
	set_statusline(m_view, "???");
}
			
