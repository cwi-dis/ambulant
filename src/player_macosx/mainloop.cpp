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


//#define WITH_NONE_VIDEO
//#define WITH_FFMPEG_VIDEO
//#define TEST_PLAYBACK_FEEDBACK

#include <iostream>
#include <ApplicationServices/ApplicationServices.h>
#include "mainloop.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/document.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#ifdef WITH_SDL
#include "ambulant/gui/SDL/sdl_gui.h"
#endif
#ifdef WITH_NONE_VIDEO
#include "ambulant/gui/none/none_factory.h"
#endif
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
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/common/plugin_engine.h"
#ifdef WITH_XERCES_BUILTIN
#include "ambulant/lib/xerces_parser.h"
#endif

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

void
usage()
{
	std::cerr << "Usage: demoplayer file" << std::endl;
	std::cerr << "Options: --version (-v) prints version info" << std::endl;
	exit(1);
}

#ifdef TEST_PLAYBACK_FEEDBACK
#include "ambulant/common/player.h"
#include "ambulant/lib/node.h"

class pbfeedback : public ambulant::common::player_feedback {
  public:
	void node_started(ambulant::lib::node *n) {
		ambulant::lib::logger::get_logger()->trace("%s started", n->get_sig().c_str());
	}

	void node_stopped(ambulant::lib::node *n) {
		ambulant::lib::logger::get_logger()->trace("%s stopped", n->get_sig().c_str());
	}
};

class pbfeedback pbfeedback;
#endif

mainloop::mainloop(const char *filename, ambulant::common::window_factory *wf,
	bool use_mms, ambulant::common::embedder *app)
:   m_running(false),
	m_speed(1.0),
	m_doc(NULL),
	m_player(NULL),
	m_factory(NULL),
	m_embedder(app),
	m_goto_node(NULL)
{
	using namespace ambulant;
	m_factory = new common::factories;
	m_factory->wf = wf;
	AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop(0x%x): created", (void*)this);
	// Populate the parser factory
	m_factory->pf = lib::global_parser_factory::get_parser_factory();	
#ifdef WITH_XERCES_BUILTIN
	m_factory->pf->add_factory(new lib::xerces_factory());
#endif

	// Next create the datasource factory and populate it too.
	m_factory->df = new net::datasource_factory();
	
#ifdef WITH_FFMPEG
#ifdef WITH_FFMPEG_VIDEO
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_video_datasource_factory");
	m_factory->df->add_video_factory(new net::ffmpeg_video_datasource_factory());
#endif
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_datasource_factory");
	m_factory->df->add_audio_factory(new net::ffmpeg_audio_datasource_factory());
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_parser_finder");
	m_factory->df->add_audio_parser_finder(new net::ffmpeg_audio_parser_finder());
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_audio_filter_finder");
	m_factory->df->add_audio_filter_finder(new net::ffmpeg_audio_filter_finder());
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add ffmpeg_raw_datasource_factory");
	m_factory->df->add_raw_factory(new net::ffmpeg_raw_datasource_factory());
#endif
#ifdef WITH_STDIO_DATASOURCE
	// This is for debugging only, really: the posix datasource
	// should always perform better, and is always available on OSX.
	// If you define WITH_STDIO_DATASOURCE we prefer to use the stdio datasource,
	// however.
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add stdio_datasource_factory");
	m_factory->df->add_raw_factory(new net::stdio_datasource_factory());
#endif
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add posix_datasource_factory");
	m_factory->df->add_raw_factory(new net::posix_datasource_factory());
	
	// Next create the playable factory and populate it.
	common::global_playable_factory *rf = common::get_global_playable_factory();
	m_factory->rf = rf;
#ifdef WITH_NONE_VIDEO
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add factory for none_video");
	rf->add_factory( new gui::none::none_video_factory(m_factory) );      
#endif
	rf->add_factory(new gui::cocoa::cocoa_renderer_factory(m_factory));
#ifdef WITH_SDL
    AM_DBG lib::logger::get_logger()->debug("mainloop::mainloop: add factory for SDL");
	rf->add_factory( new gui::sdl::sdl_renderer_factory(m_factory) );      
#endif

	AM_DBG lib::logger::get_logger()->debug("qt_mainloop::qt_mainloop: Starting the plugin engine");

	common::plugin_engine *pf = common::plugin_engine::get_plugin_engine();
	pf->add_plugins(m_factory);

	ambulant::net::url url(filename);
	m_doc = create_document(url);
	if (!m_doc) {
		lib::logger::get_logger()->error(gettext("%s: Cannot build DOM tree"), filename);
		return;
	}
	if (use_mms)
		m_player = common::create_mms_player(m_doc, m_factory);
	else
		m_player = common::create_smil2_player(m_doc, m_factory, m_embedder);
#ifdef USE_SMIL21
	m_player->initialize();
#endif
#ifdef TEST_PLAYBACK_FEEDBACK
	m_player->set_feedback(&pbfeedback);
#endif
	const std::string& id = url.get_ref();
	if (id != "") {
		const ambulant::lib::node *node = m_doc->get_node(id);
		if (!node) {
			lib::logger::get_logger()->warn(gettext("%s: node ID not found"), id.c_str());
		} else {
			m_goto_node = node;
		}
	}
}

ambulant::lib::document *
mainloop::create_document(ambulant::net::url& url)
{
	char *data;
	// Correct for relative pathnames for local files
	if (url.is_local_file() && !url.is_absolute()) {
#if 0
		// Not implemented yet for posix
		ambulant::net::url cwd_url(lib::filesys::getcwd());
#else
		char cwdbuf[1024];
		if (getcwd(cwdbuf, sizeof cwdbuf-2) < 0)
			strcpy(cwdbuf, ".");
		strcat(cwdbuf, "/");
		ambulant::net::url cwd_url(cwdbuf);
#endif
		url = url.join_to_base(cwd_url);
		AM_DBG ambulant::lib::logger::get_logger()->debug("mainloop::create_document: URL is now \"%s\"", url.get_url().c_str());
	}
	int size = ambulant::net::read_data_from_url(url, m_factory->df, &data);
	if (size < 0) {
		ambulant::lib::logger::get_logger()->error(gettext("%s: Cannot open"), url.get_url().c_str());
		return NULL;
	}
	std::string docdata(data, size);
	free(data);
	ambulant::lib::logger::get_logger()->trace("%s: Parsing document...", url.get_url().c_str());
	ambulant::lib::document *rv = ambulant::lib::document::create_from_string(m_factory, docdata, url.get_url());
	if (rv) {
		ambulant::lib::logger::get_logger()->trace("%s: Parser done", url.get_url().c_str());
		rv->set_src_url(url);
	} else {
		ambulant::lib::logger::get_logger()->trace("%s: Failed to parse document ", url.get_url().c_str());
	}
	return rv;
}	

mainloop::~mainloop()
{
//  m_doc will be cleaned up by the smil_player.
//	if (m_doc) delete m_doc;
//	m_doc = NULL;
	AM_DBG ambulant::lib::logger::get_logger()->debug("mainloop::~mainloop(0x%x)", (void*)this);
	if (m_player) delete m_player;
	m_player = NULL;
	if (m_factory->rf) delete m_factory->rf;
	m_factory->rf = NULL;
	if (m_factory->df) delete m_factory->df;
	m_factory->df = NULL;
	// wf Window factory is owned by caller
	m_factory->wf = NULL;
	delete m_factory;
}

void
mainloop::play()
{
	if (!m_player) {
		ambulant::lib::logger::get_logger()->error(gettext("Cannot play document: no player"));
		return;
	}
	m_running = true;
	m_speed = 1.0;
	m_player->start();
	if (m_goto_node) {
		bool ok = m_player->goto_node(m_goto_node);
		if (!ok)
			ambulant::lib::logger::get_logger()->trace("mainloop::run: goto_node failed");
	} 
	AM_DBG ambulant::lib::logger::get_logger()->debug("mainloop::run(): returning");
}

void
mainloop::stop()
{
	if (m_player) m_player->stop();
	m_speed = 1.0;
	AM_DBG ambulant::lib::logger::get_logger()->debug("mainloop::run(): returning");
}

void
mainloop::set_speed(double speed)
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
mainloop::is_running() const
{
	AM_DBG ambulant::lib::logger::get_logger()->debug("mainloop::is_running(0x%x)", (void*)this);
	if (!m_running || !m_player) return false;
	return !m_player->is_done();
}

void
mainloop::set_preferences(std::string &url)
{
	ambulant::smil2::test_attrs::load_test_attrs(url);
}
