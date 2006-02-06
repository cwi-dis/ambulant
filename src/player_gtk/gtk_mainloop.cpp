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

#ifdef WITH_ARTS
#include "ambulant/gui/arts/arts.h"
#endif
#ifdef WITH_SDL
#include "ambulant/gui/SDL/sdl_gui.h"
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
#include "ambulant/common/plugin_engine.h"
#include "ambulant/lib/parser_factory.h"
#ifdef WITH_XERCES_BUILTIN
#include "ambulant/lib/xerces_parser.h"
#endif
#ifdef WITH_LIVE
#include "ambulant/net/rtsp_factory.h"
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

gtk_mainloop::gtk_mainloop(gtk_gui* gui) :
	
	m_factory(NULL),
	m_doc(NULL),
	m_gui(gui),
	m_player(NULL),
 	m_refcount(1),
 	m_running(false),
	m_speed(1.0)
{
	m_logger = lib::logger::get_logger();
	common::global_playable_factory *rf = common::get_global_playable_factory();
	gtk_window_factory *wf = new gtk_window_factory(m_gui->get_gui_container(), 
					      m_gui->get_o_x(),
					      m_gui->get_o_y());
	net::datasource_factory *df = new net::datasource_factory();
	lib::global_parser_factory *pf = lib::global_parser_factory::get_parser_factory();	
	m_factory = new common::factories(rf, wf, df, pf);
	// First create the parser factory and populate it;
	
//	m_factory->pf->add_factory(new lib::expat_factory());
#ifdef WITH_XERCES_BUILTIN
	pf->add_factory(new lib::xerces_factory());
	AM_DBG m_logger->debug("mainloop::mainloop: add xerces_factory");
#endif
#ifdef WITH_LIVE	
	AM_DBG m_logger->debug("mainloop::mainloop: add live_audio_datasource_factory");
	df->add_video_factory(new net::live_video_datasource_factory());
	df->add_audio_factory(new net::live_audio_datasource_factory()); 
#endif
#ifdef WITH_FFMPEG
    	AM_DBG m_logger->debug("mainloop::mainloop: add ffmpeg_audio_datasource_factory");
	df->add_audio_factory(net::get_ffmpeg_audio_datasource_factory());
    	AM_DBG m_logger->debug("gtk_mainloop::gtk_mainloop: add ffmpeg_audio_parser_finder");
	df->add_audio_parser_finder(net::get_ffmpeg_audio_parser_finder());
    	AM_DBG m_logger->debug("gtk_mainloop::gtk_mainloop: add ffmpeg_audio_filter_finder");
	df->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
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
	df->add_raw_factory(new net::stdio_datasource_factory());
#endif
	AM_DBG m_logger->debug("gtk_mainloop::gtk_mainloop: add posix_datasource_factory");
	df->add_raw_factory(new net::posix_datasource_factory());

  
	AM_DBG m_logger->debug("gtk_mainloop::gtk_mainloop: Starting the plugin engine");
	common::plugin_engine *plf = common::plugin_engine::get_plugin_engine();
	plf->add_plugins(m_factory);
#ifdef WITH_SDL
	AM_DBG logger::get_logger()->debug("add factory for SDL");
	rf->add_factory( new sdl::sdl_renderer_factory(m_factory) );
	AM_DBG logger::get_logger()->debug("add factory for SDL done");
#endif

#ifdef WITH_ARTS
	rf->add_factory(new arts::arts_renderer_factory(m_factory));
#endif 
	
	rf->add_factory(new gtk_renderer_factory(m_factory));
	rf->add_factory(new gtk_video_factory(m_factory));
	AM_DBG m_logger->debug("mainloop::mainloop: added gtk_video_factory");			
	//rf->add_factory(new none::none_video_factory(m_factory));	
	AM_DBG m_logger->debug("mainloop::mainloop: added none_video_factory");
	
	
	const char *filename = m_gui->filename();
	net::url url = net::url::from_filename(filename);
	m_doc = create_document(url);
	if (!m_doc) {
		return;
	}
	m_player = create_player(filename);
}

ambulant::common::player*
gtk_mainloop::create_player(const char* filename) {
	bool is_mms = strcmp(".mms", filename+strlen(filename)-4) == 0;
	ambulant::common::player* player;
	if (is_mms) {
//		player = create_mms_player(m_doc, m_factory);
	} else {
		player = create_smil2_player(m_doc, m_factory, this);
	}
#ifdef USE_SMIL21
	player->initialize();
#endif
	return player;
}

lib::document *
gtk_mainloop::create_document(net::url& url)
{
	char *data;
	AM_DBG m_logger->debug("qt_mainloop::create_document(\"%s\")", url.get_url().c_str());
	// Correct for relative pathnames for local files
	if (url.is_local_file() && !url.is_absolute()) {
#if 0
		// Not implemented yet for posix
		net::url cwd_url(lib::filesys::getcwd());
#else
		char cwdbuf[1024];
		if (getcwd(cwdbuf, sizeof cwdbuf-2) < 0)
			strcpy(cwdbuf, ".");
		strcat(cwdbuf, "/");
		net::url cwd_url = net::url::from_filename(cwdbuf);
#endif
		url = url.join_to_base(cwd_url);
		AM_DBG m_logger->debug("mainloop::create_document: URL is now \"%s\"", url.get_url().c_str());
	}
	size_t size;
	bool result = net::read_data_from_url(url, m_factory->get_datasource_factory(), &data, &size);
	if (!result)	{
		// No error message needed, has been done by passive_datasoure::activate()
		//		m_logger->error("Cannot open %s", filename);
		return NULL;
	}
	std::string docdata(data, size);
	free(data);
	lib::document *rv = lib::document::create_from_string(m_factory,docdata, url.get_url().c_str());
	if (rv) rv->set_src_url(url);
	return rv;
}
gtk_mainloop::~gtk_mainloop()
{
//  m_doc will be cleaned up by the smil_player.
//	if (m_doc) delete m_doc;
//	m_doc = NULL;
	AM_DBG m_logger->debug("gtk_mainloop::~gtk_mainloop() m_player=0x%x", m_player);
	if (m_player) {
		delete m_player;
	}
	m_player = NULL;
	delete m_factory;
//	m_wf = NULL;
}

void
gtk_mainloop::play()
{
	m_running = true;
	m_player->start();
	AM_DBG m_logger->debug("gtk_mainloop::run(): returning");
}

void
gtk_mainloop::stop()
{
	if (m_player)
		m_player->stop();
	AM_DBG m_logger->debug("gtk_mainloop::stop(): returning");
}

void
gtk_mainloop::set_speed(double speed)
{
	m_speed = speed;
	if (m_player) {
		if (speed == 0.0)
			m_player->pause();
		else
			m_player->resume();
	}
}

bool
gtk_mainloop::is_running() const
{
	if (!m_running || !m_player) return false;
	return !m_player->is_done();
}

bool
gtk_mainloop::is_open() const
{
	return m_doc && m_player;
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
//TBD	m_timer->pause();
//TBD	m_update_event = 0;
//TBD	clear_transitions();
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
	msg += start ? "S-" : "  ";
	msg += old   ? "O-" : "  ";
	msg += newdoc;
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
	
	// Create a player instance
	AM_DBG m_logger->debug("Creating player instance for: %s",
			       document_name);
	m_player = create_player(document_name);
	if(start) {
		m_player->start();
	}
}
