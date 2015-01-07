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

#ifdef WITH_GSTREAMER
#include "ambulant/gui/gstreamer/gstreamer_renderer_factory.h"
#endif
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
#include "ambulant/gui/gtk/gtk_factory.h"
#include "ambulant/gui/gtk/gtk_fill.h"
#ifdef	WITH_GTK_HTML_WIDGET
#include "ambulant/gui/gtk/gtk_html_renderer.h"
#endif
#include "ambulant/gui/gtk/gtk_image_renderer.h"
#include "ambulant/gui/gtk/gtk_smiltext.h"
#include "ambulant/gui/gtk/gtk_text_renderer.h"
#include "ambulant/gui/gtk/gtk_video_renderer.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/lib/parser_factory.h"
#ifdef WITH_XERCES_BUILTIN
#include "ambulant/lib/xerces_parser.h"
#endif

//#include "ambulant/lib/expat_parser.h"


//#include "ambulant/lib/tree_builder.h"

#include "gtk_mainloop.h"

using namespace ambulant;
using namespace gui::gtk;

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

gtk_mainloop::gtk_mainloop(gtk_gui* gui)
:	m_gui(gui),
	m_running(false),
	m_gtk_widget(NULL)
{
	gui_player();
	m_logger = lib::logger::get_logger();
	set_embedder(this);
	smil2::test_attrs::set_default_tests_attrs();
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("Standalone"), true);
	init_factories();
	init_plugins();

	const char *filename = "";
	if (m_gui) filename = m_gui->filename();
	net::url url = net::url::from_filename(filename);
	m_doc = create_document(url);
	if (!m_doc) {
		return;
	}
	common::preferences *prefs = common::preferences::get_preferences();
	m_logger->debug(" creating smil2 player %s", prefs->repr().c_str());
	m_player = create_player(filename);
}

gtk_mainloop::~gtk_mainloop()
{
	AM_DBG m_logger->debug("gtk_mainloop::~gtk_mainloop() m_player=0x%x", m_player);
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
	delete m_gtk_widget;
	//delete m_window_factory;
}

void
gtk_mainloop::init_window_factory()
{
	m_gtk_widget = new gtk_ambulant_widget(m_gui->get_document_container());
	common::window_factory* gtk_wf = create_gtk_window_factory(m_gtk_widget, this);
	set_window_factory(gtk_wf);
}


void
gtk_mainloop::init_playable_factory()
{
	common::global_playable_factory *pf = common::get_global_playable_factory();
	set_playable_factory(pf);

	AM_DBG m_logger->debug("gtk_mainloop: adding QGtk playable factories");
	pf->add_factory(create_gtk_fill_playable_factory(this, NULL));
#ifdef	WITH_GTK_HTML_WIDGET
	pf->add_factory(create_gtk_html_playable_factory(this, NULL));
#endif
	pf->add_factory(create_gtk_image_playable_factory(this, NULL));
	pf->add_factory(create_gtk_smiltext_playable_factory(this, NULL));
	pf->add_factory(create_gtk_text_playable_factory(this, NULL));
	pf->add_factory(create_gtk_video_playable_factory(this, NULL));

#ifdef WITH_SDL
	AM_DBG lib::logger::get_logger()->debug("gtk_mainloop: add factory for SDL");
	pf->add_factory(gui::sdl::create_sdl_playable_factory(this));
#endif // WITH_SDL
#ifdef WITH_GSTREAMER
	AM_DBG logger::get_logger()->debug("add factory for GStreamer");
	pf->add_factory(gui::gstreamer::create_gstreamer_renderer_factory(this));
	AM_DBG logger::get_logger()->debug("add factory for GStreamer done");
#endif
}

void
gtk_mainloop::init_datasource_factory()
{
	net::datasource_factory *df = new net::datasource_factory();
	set_datasource_factory(df);
#ifdef WITH_FFMPEG
	AM_DBG m_logger->debug("mainloop::mainloop: add ffmpeg_audio_datasource_factory");
	df->add_audio_factory(net::get_ffmpeg_audio_datasource_factory());
	AM_DBG m_logger->debug("gtk_mainloop::gtk_mainloop: add ffmpeg_audio_decoder_finder");
	df->add_audio_decoder_finder(net::get_ffmpeg_audio_decoder_finder());
#ifdef WITH_RESAMPLE_DATASOURCE
	AM_DBG m_logger->debug("gtk_mainloop::gtk_mainloop: add ffmpeg_audio_filter_finder");
	df->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
#endif
	AM_DBG m_logger->debug("mainloop::mainloop: add ffmpeg_video_datasource_factory");
	df->add_video_factory(net::get_ffmpeg_video_datasource_factory());
	AM_DBG m_logger->debug("mainloop::mainloop: add ffmpeg_raw_datasource_factory");
	df->add_raw_factory(net::get_ffmpeg_raw_datasource_factory());
#endif

#ifdef WITH_STDIO_DATASOURCE
	// This is for debugging only, really: the posix datasource
	// should always perform better, and is always available on OSX.
	// If you define WITH_STDIO_DATASOURCE we prefer to use the stdio datasource,
	// however.

	AM_DBG m_logger->debug("gtk_mainloop::gtk_mainloop: add stdio_datasource_factory");
	df->add_raw_factory(net::create_stdio_datasource_factory());
#endif
	AM_DBG m_logger->debug("gtk_mainloop::gtk_mainloop: add posix_datasource_factory");
	df->add_raw_factory(net::create_posix_datasource_factory());
}

void
gtk_mainloop::init_parser_factory()
{
	lib::global_parser_factory *pf = lib::global_parser_factory::get_parser_factory();
	set_parser_factory(pf);
#ifdef WITH_XERCES_BUILTIN
	pf->add_factory(new lib::xerces_factory());
	AM_DBG m_logger->debug("mainloop::mainloop: add xerces_factory");
#endif
}


ambulant::common::player*
gtk_mainloop::create_player(const char* filename) {
	ambulant::common::player* player;
	player = create_smil2_player(m_doc, this, get_embedder());

	player->initialize();

	return player;
}

void
gtk_mainloop::show_file(const net::url &url)
{
	open_web_browser(url.get_url());
}

void
gtk_mainloop::done(common::player *p)
{
	AM_DBG m_logger->debug("gtk_mainloop: implementing: done()");
	//m_gui->player_done();
}

bool
gtk_mainloop::player_done()
  // return true when the last player is done
{
	AM_DBG m_logger->debug("gtk_mainloop: implementing: player_done");
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
gtk_mainloop::close(common::player *p)
{
	AM_DBG m_logger->trace("gtk_mainloop: implementing: close document");
	stop();
}

void
gtk_mainloop::open(net::url newdoc, bool start, common::player *old)
{
	AM_DBG m_logger->trace("gtk_mainloop::open \"%s\"",newdoc.get_url().c_str());
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
	m_gui->internal_message(gtk_logger::CUSTOM_NEW_DOCUMENT,
				strdup(msg.c_str()));
}

void
gtk_mainloop::player_start(gchar* document_name, bool start, bool old)
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
char* gtk_mainloop::convert_data_to_image(const guchar* data, gsize size){
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
gtk_mainloop::get_gui_screen(){
	return m_gtk_widget;
}
