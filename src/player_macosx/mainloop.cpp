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

/* 
 * @$Id$ 
 */


// Environment for testing design classes

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
#undef WITH_NONE_VIDEO
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
#endif
#include "ambulant/smil2/test_attrs.h"

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

mainloop::mainloop(const char *filename, ambulant::common::window_factory *wf, bool use_mms)
:   m_running(false),
	m_speed(1.0),
	m_doc(NULL),
	m_player(NULL),
	m_rf(NULL),
	m_wf(wf),
	m_df(NULL)
{
	using namespace ambulant;
	
	// First create the datasource factory and populate it too.
	m_df = new net::datasource_factory();
#ifdef WITH_STDIO_DATASOURCE
	// This is for debugging only, really: the posix datasource
	// should always perform better, and is always available on OSX.
	// If you define WITH_STDIO_DATASOURCE we prefer to use the stdio datasource,
	// however.
    AM_DBG lib::logger::get_logger()->trace("mainloop::mainloop: add stdio_datasource_factory");
	m_df->add_raw_factory(new net::stdio_datasource_factory());
#endif
    AM_DBG lib::logger::get_logger()->trace("mainloop::mainloop: add posix_datasource_factory");
	m_df->add_raw_factory(new net::posix_datasource_factory());
	
#ifdef WITH_FFMPEG
    AM_DBG lib::logger::get_logger()->trace("mainloop::mainloop: add ffmpeg_audio_datasource_factory");
	m_df->add_audio_factory(new net::ffmpeg_audio_datasource_factory());
    AM_DBG lib::logger::get_logger()->trace("mainloop::mainloop: add ffmpeg_audio_parser_finder");
	m_df->add_audio_parser_finder(new net::ffmpeg_audio_parser_finder());
    AM_DBG lib::logger::get_logger()->trace("mainloop::mainloop: add ffmpeg_audio_filter_finder");
	m_df->add_audio_filter_finder(new net::ffmpeg_audio_filter_finder());
#endif
	
	// Next create the playable factory and populate it.
	m_rf = new common::global_playable_factory();
#ifdef WITH_NONE_VIDEO
    AM_DBG lib::logger::get_logger()->trace("mainloop::mainloop: add factory for none_video");
	m_rf->add_factory( new gui::none::none_video_factory(m_df) );      
#endif
	m_rf->add_factory(new gui::cocoa::cocoa_renderer_factory(m_df));
#ifdef WITH_SDL
    AM_DBG lib::logger::get_logger()->trace("mainloop::mainloop: add factory for SDL");
	m_rf->add_factory( new gui::sdl::sdl_renderer_factory(m_df) );      
#endif

	m_doc = lib::document::create_from_file(filename);
	if (!m_doc) {
		lib::logger::get_logger()->error("Could not build tree for file: %s", filename);
		return;
	}
	if (use_mms)
		m_player = common::create_mms_player(m_doc, m_wf, m_rf);
	else
		m_player = common::create_smil2_player(m_doc, m_wf, m_rf, this);
}

mainloop::~mainloop()
{
//  m_doc will be cleaned up by the smil_player.
//	if (m_doc) delete m_doc;
//	m_doc = NULL;
	if (m_player) delete m_player;
	m_player = NULL;
	if (m_rf) delete m_rf;
	m_rf = NULL;
	// m_wf Window factory is owned by caller
}

void
mainloop::play()
{
	m_running = true;
	m_player->start();
	AM_DBG ambulant::lib::logger::get_logger()->trace("mainloop::run(): returning");
}

void
mainloop::stop()
{
	m_player->stop();
	AM_DBG ambulant::lib::logger::get_logger()->trace("mainloop::run(): returning");
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
	if (!m_running || !m_player) return false;
	return !m_player->is_done();
}

void
mainloop::set_preferences(std::string &url)
{
	ambulant::smil2::test_attrs::load_test_attrs(url);
}

void
mainloop::show_file(const std::string& href)
{
	CFStringRef cfhref = CFStringCreateWithCString(NULL, href.c_str(), kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithString(NULL, cfhref, NULL);
	OSErr status;
	
	if ((status=LSOpenCFURLRef(url, NULL)) != 0) {
		ambulant::lib::logger::get_logger()->error("Cannot open URL <%s>: LSOpenCFURLRef error %d", href.c_str(), status);
	}
}
