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

// Define NONE_PLAYER to skip all cg support but use the dummy
// none_window and none_playable in stead.
//#define NONE_PLAYER

//#include "MyAmbulantView.h"
#include <iostream>
#include <ApplicationServices/ApplicationServices.h>
#include "cg_mainloop.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/node.h"
#include "ambulant/gui/cg/cg_gui.h"
#ifdef WITH_SDL
#include "ambulant/gui/SDL/sdl_factory.h"
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
#include "ambulant/common/preferences.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

#if 1
#define set_statusline(a1, a2) /* nothing */
#endif

cg_mainloop::cg_mainloop(const char *urlstr, void *view,
	bool use_mms, ambulant::common::embedder *app)
:	common::gui_player(),
	m_view(view),
	m_gui_screen(NULL)
{
	common::preferences *p = common::preferences::get_preferences();
	p->m_log_level = 0;
	set_embedder(app);
	AM_DBG lib::logger::get_logger()->debug("cg_mainloop::cg_mainloop(%p): created", (void*)this);
	init_factories();
	AM_DBG lib::logger::get_logger()->debug("m_node_factory now %p", get_node_factory());
	init_plugins();

	ambulant::net::url url = ambulant::net::url::from_url(urlstr);
	AM_DBG lib::logger::get_logger()->debug("m_node_factory now %p", get_node_factory());
	m_doc = create_document(url);
	if (!m_doc) {
		lib::logger::get_logger()->error(gettext("%s: Cannot build DOM tree"), urlstr);
		return;
	}
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
cg_mainloop::init_playable_factory()
{
	common::global_playable_factory *pf = common::get_global_playable_factory();
	set_playable_factory(pf);
#ifndef NONE_PLAYER
	pf->add_factory(gui::cg::create_cg_fill_playable_factory(this, NULL));
	pf->add_factory(gui::cg::create_cg_dsvideo_playable_factory(this, NULL));
	pf->add_factory(gui::cg::create_cg_image_playable_factory(this, NULL));
//	pf->add_factory(gui::cg::create_cg_ink_playable_factory(this, NULL));
	pf->add_factory(gui::cg::create_cg_smiltext_playable_factory(this, NULL));
	pf->add_factory(gui::cg::create_cg_text_playable_factory(this, NULL));
//	pf->add_factory(gui::cg::create_cg_video_playable_factory(this, NULL));
#endif // NONE_PLAYER
#ifdef WITH_SDL
	AM_DBG lib::logger::get_logger()->debug("cg_mainloop::cg_mainloop: add factory for SDL");
	pf->add_factory(gui::sdl::create_sdl_playable_factory(this));
#endif // WITH_SDL
}

void
cg_mainloop::init_window_factory()
{
#ifdef NONE_PLAYER
	// Replace the real window factory with a none_window_factory instance.
	set_window_factory(gui::none::create_none_window_factory());
#else
	set_window_factory(gui::cg::create_cg_window_factory(m_view));
#endif // NONE_PLAYER
}

void
cg_mainloop::init_datasource_factory()
{
	net::datasource_factory *df = new net::datasource_factory();
	set_datasource_factory(df);
#ifdef WITH_FFMPEG
	AM_DBG lib::logger::get_logger()->debug("cg_mainloop::cg_mainloop: add ffmpeg_video_datasource_factory");
	df->add_video_factory(net::get_ffmpeg_video_datasource_factory());
	AM_DBG lib::logger::get_logger()->debug("cg_mainloop::cg_mainloop: add ffmpeg_audio_datasource_factory");
	df->add_audio_factory(net::get_ffmpeg_audio_datasource_factory());
	AM_DBG lib::logger::get_logger()->debug("cg_mainloop::cg_mainloop: add ffmpeg_audio_decoder_finder");
	df->add_audio_decoder_finder(net::get_ffmpeg_audio_decoder_finder());
#ifdef WITH_RESAMPLE_DATASOURCE
	AM_DBG lib::logger::get_logger()->debug("cg_mainloop::cg_mainloop: add ffmpeg_audio_filter_finder");
	df->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
#endif// WITH_RESAMPLE_DATASOURCE
	AM_DBG lib::logger::get_logger()->debug("cg_mainloop::cg_mainloop: add ffmpeg_raw_datasource_factory");
	df->add_raw_factory(net::get_ffmpeg_raw_datasource_factory());
#endif // WITH_FFMPEG
#ifdef WITH_STDIO_DATASOURCE
	// This is for debugging only, really: the posix datasource
	// should always perform better, and is always available on OSX.
	// If you define WITH_STDIO_DATASOURCE we prefer to use the stdio datasource,
	// however.
	AM_DBG lib::logger::get_logger()->debug("cg_mainloop::cg_mainloop: add stdio_datasource_factory");
	df->add_raw_factory(net::create_stdio_datasource_factory());
#endif
	AM_DBG lib::logger::get_logger()->debug("cg_mainloop::cg_mainloop: add posix_datasource_factory");
	df->add_raw_factory(net::create_posix_datasource_factory());
}

void
cg_mainloop::init_parser_factory()
{
	set_parser_factory(lib::global_parser_factory::get_parser_factory());
}

cg_mainloop::~cg_mainloop()
{
	// We need to delete gui_player::m_player before deleting m_doc, because the
	// timenode graph in the player has referrences to the node graph in m_doc.
	m_player->terminate();
	m_player->release();
	m_player = NULL;
	delete m_doc;
	m_doc = NULL;
	delete m_gui_screen;
	// delete m_window_factory;
}

void
cg_mainloop::restart(bool reparse)
{
	bool playing = is_play_active();
	bool pausing = is_pause_active();
	stop();

	delete m_player;
	m_player = 0;
	if (reparse) {
		m_doc = create_document(m_url);
		if(!m_doc) {
			lib::logger::get_logger()->show(gettext("Failed to parse document %s"), m_url.get_url().c_str());
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
cg_mainloop::get_gui_screen()
{
#ifndef NONE_PLAYER
	if (!m_gui_screen) m_gui_screen = new gui::cg::cg_gui_screen(m_view);
#endif
	return m_gui_screen;
}

void
cg_mainloop::node_focussed(const lib::node *n)
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
//	set_statusline(m_view, "???");
}


void
cg_mainloop::update(CGContextRef ctx)
{
	AM_DBG lib::logger::get_logger()->debug("update(%p): ctx=%p", this , ctx);
	// draw red Rect
	CGContextRef context = ctx;
 	CGColorRef redColor = CGColorCreateGenericRGB(1.0,0.0,0.0,0.5);
	CGContextSetFillColorWithColor(context, redColor);
	CGContextFillRect(context, CGContextGetClipBoundingBox(ctx));
}

const CGSize
cg_mainloop::get_size_from_doc()
{
	CGSize rv = CGSizeMake(200,200);
	return rv;
}
