/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

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
#include "ambulant/net/ffmpeg_datasource.h"
#include "ambulant/net/ffmpeg_rawdatasource.h"
#endif
#include "ambulant/gui/none/none_factory.h"
#include "ambulant/gui/qt/qt_factory.h"
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

#define AM_DBG
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
	//	lib::logger::get_logger()->error(gettext("$BROWSER not set: cannot open webpage <%s>"), href.c_str());
	// return;
		// currently common available browsers on Linux
		browserlist = "firefox:mozilla:opera:netscape";
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

qt_mainloop::qt_mainloop(qt_gui* gui) :
	m_factory(NULL),
	m_doc(NULL),
	m_gui(gui),
	m_player(NULL),
 	m_refcount(1),
 	m_running(false),
	m_speed(1.0)
{
 	m_logger = lib::logger::get_logger();
	m_factory = new common::factories;
	// First create the parser factory and populate it;
	
	m_factory->pf = lib::global_parser_factory::get_parser_factory();	
//	m_factory->pf->add_factory(new lib::expat_factory());
#ifdef WITH_XERCES_BUILTIN
	m_factory->pf->add_factory(new lib::xerces_factory());
#endif
	// First create the datasource factory and populate it too.
	m_factory->df = new net::datasource_factory();
	
#ifdef WITH_FFMPEG
    AM_DBG m_logger->debug("mainloop::mainloop: add ffmpeg_audio_datasource_factory");
	m_factory->df->add_audio_factory(new net::ffmpeg_audio_datasource_factory());
    AM_DBG m_logger->debug("qt_mainloop::qt_mainloop: add ffmpeg_audio_parser_finder");
	m_factory->df->add_audio_parser_finder(new net::ffmpeg_audio_parser_finder());
    AM_DBG m_logger->debug("qt_mainloop::qt_mainloop: add ffmpeg_audio_filter_finder");
	m_factory->df->add_audio_filter_finder(new net::ffmpeg_audio_filter_finder());
	AM_DBG m_logger->debug("mainloop::mainloop: add ffmpeg_video_datasource_factory");
	m_factory->df->add_video_factory(new net::ffmpeg_video_datasource_factory());
    AM_DBG m_logger->debug("mainloop::mainloop: add ffmpeg_raw_datasource_factory");
	m_factory->df->add_raw_factory(new net::ffmpeg_raw_datasource_factory());
#endif

#ifdef WITH_STDIO_DATASOURCE
	// This is for debugging only, really: the posix datasource
	// should always perform better, and is always available on OSX.
	// If you define WITH_STDIO_DATASOURCE we prefer to use the stdio datasource,
	// however.
    AM_DBG m_logger->debug("qt_mainloop::qt_mainloop: add stdio_datasource_factory");
	m_factory->df->add_raw_factory(new net::stdio_datasource_factory());
#endif
    AM_DBG m_logger->debug("qt_mainloop::qt_mainloop: add posix_datasource_factory");
	m_factory->df->add_raw_factory(new net::posix_datasource_factory());

	// Next create the playable factory and populate it.
	m_factory->rf = new common::global_playable_factory();
		
	AM_DBG m_logger->debug("qt_mainloop::qt_mainloop: Starting the plugin engine");
	common::plugin_engine *pf = common::plugin_engine::get_plugin_engine();
	pf->add_plugins(m_factory);
#ifdef WITH_SDL
	AM_DBG logger::get_logger()->debug("add factory for SDL");
	m_factory->rf->add_factory( new sdl::sdl_renderer_factory(m_factory) );
AM_DBG logger::get_logger()->debug("add factory for SDL done");
#endif

#ifdef WITH_ARTS
	m_factory->rf->add_factory(new arts::arts_renderer_factory(m_factory));
#endif 

	m_factory->rf->add_factory(new qt_renderer_factory(m_factory));
	
	AM_DBG m_logger->debug("mainloop::mainloop: added qt_video_factory");		
 	m_factory->rf->add_factory(new qt_video_factory(m_factory));
		AM_DBG m_logger->debug("mainloop::mainloop: added none_video_factory");		

	m_factory->rf->add_factory(new none::none_video_factory(m_factory));

	
	m_factory->wf = new qt_window_factory(m_gui, 
					      m_gui->get_o_x(),
					      m_gui->get_o_y());

	const char *filename = m_gui->filename();
	m_doc = create_document(filename);
	if (!m_doc) {
		return;
	}
	m_player = create_player(filename);
}

ambulant::common::player*
qt_mainloop::create_player(const char* filename) {
	bool is_mms = strcmp(".mms", filename+strlen(filename)-4) == 0;
	if (is_mms) {
		m_player = create_mms_player(m_doc, m_factory);
	} else {
		m_player = create_smil2_player(m_doc, m_factory, this);
	}
}

lib::document *
qt_mainloop::create_document(const char *filename)
{
	char *data;
	AM_DBG m_logger->debug("qt_mainloop::create_document(\"%s\")", filename);
	net::url url(filename);
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
		net::url cwd_url(cwdbuf);
#endif
		url = url.join_to_base(cwd_url);
		AM_DBG m_logger->debug("mainloop::create_document: URL is now \"%s\"", url.get_url().c_str());
	}
	int size = net::read_data_from_url(url, m_factory->df, &data);
	if (size < 0) {
		m_logger->error("Cannot open %s", filename);
		return NULL;
	}
	std::string docdata(data, size);
	free(data);
	lib::document *rv = lib::document::create_from_string(m_factory,docdata);
	if (rv) rv->set_src_url(url);
	return rv;
}	

qt_mainloop::~qt_mainloop()
{
//  m_doc will be cleaned up by the smil_player.
//	if (m_doc) delete m_doc;
//	m_doc = NULL;
	AM_DBG m_logger->debug("qt_mainloop::~qt_mainloop() m_player=0x%x", m_player);
	if (m_player) {
		delete m_player;
	}
	m_player = NULL;
	if (m_factory->rf) delete m_factory->rf;
	m_factory->rf = NULL;
	
	if (m_factory->df) delete m_factory->df;
	m_factory->df = NULL;
	// m_wf Window factory is owned by caller
	m_factory->wf = NULL;
	delete m_factory;
//	m_wf = NULL;
}

void
qt_mainloop::play()
{
	m_running = true;
	m_player->start();
	AM_DBG m_logger->debug("qt_mainloop::run(): returning");
}

void
qt_mainloop::stop()
{
	m_player->stop();
	AM_DBG m_logger->debug("qt_mainloop::stop(): returning");
}

void
qt_mainloop::set_speed(double speed)
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
qt_mainloop::is_running() const
{
	if (!m_running || !m_player) return false;
	return !m_player->is_done();
}

bool
qt_mainloop::is_open() const
{
	return m_doc && m_player;
}

void
qt_mainloop::show_file(const net::url &url)
{
	open_web_browser(url.get_url());
}

void
qt_mainloop::done(common::player *p)
{
	AM_DBG m_logger->debug("qt_mainloop: implementing: done()");
	m_gui->player_done();
}

bool
qt_mainloop::player_done()
  // return true when the last player is done
{
	AM_DBG m_logger->debug("qt_mainloop: implementing: player_done");
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
qt_mainloop::close(common::player *p)
{
	AM_DBG m_logger->trace("qt_mainloop: implementing: close document");
	stop();
}

void
qt_mainloop::open(const net::url newdoc, bool start, common::player *old)
{
  //X	QString document_name(newdoc.get_url().c_str());
  //X	AM_DBG m_logger->trace("qt_mainloop::open \"%s\"",document_name.ascii());
	AM_DBG m_logger->trace("qt_mainloop::open(\"%s\")",
			       newdoc.get_url().c_str());
 	// Parse the provided URL. 
  //X	m_doc = create_document(document_name);
 	m_doc = create_document(newdoc.get_url().c_str());
	if(!m_doc) {
		m_logger->error(gettext("%s: Cannot build DOM tree"), 
  //X				document_name.ascii());
				newdoc.get_url().c_str());
		return;
	}
	// send msg to gui thread
	std::string msg("");
	msg += start ? "S-" : "  ";
	msg += old   ? "O-" : "  ";
  //X	msg += document_name.ascii();
	msg += newdoc;
	m_gui->internal_message(qt_logger::CUSTOM_NEW_DOCUMENT,
				strdup(msg.c_str()));
}

void
qt_mainloop::player_start(QString document_name, bool start, bool old)
{
	AM_DBG m_logger->debug("player_start(%s,%d,%d)",document_name.ascii(),start,old);
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
			       document_name.ascii());
	m_player = create_player(document_name);
	if(start) {
		m_player->start();
	}
}
