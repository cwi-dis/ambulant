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

#include "qt_mainloop.h"
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
#endif
#include "ambulant/gui/none/none_factory.h"
#include "ambulant/gui/qt/qt_factory.h"
qt_mainloop::qt_mainloop(qt_gui* parent) :
	m_df(NULL),
	m_doc(NULL),
	m_parent(parent),
	m_player(NULL),
 	m_refcount(1),
	m_rf(NULL),
 	m_running(false),
	m_speed(1.0),
	m_wf(NULL)
{
	// First create the datasource factory and populate it too.
	m_df = new net::datasource_factory();
#ifdef WITH_STDIO_DATASOURCE
	// This is for debugging only, really: the posix datasource
	// should always perform better, and is always available on OSX.
	// If you define WITH_STDIO_DATASOURCE we prefer to use the stdio datasource,
	// however.
    AM_DBG lib::logger::get_logger()->trace("qt_mainloop::qt_mainloop: add stdio_datasource_factory");
	m_df->add_raw_factory(new net::stdio_datasource_factory());
#endif
    AM_DBG lib::logger::get_logger()->trace("qt_mainloop::qt_mainloop: add posix_datasource_factory");
	m_df->add_raw_factory(new net::posix_datasource_factory());
	
#ifdef WITH_FFMPEG
    AM_DBG lib::logger::get_logger()->trace("qt_mainloop::qt_mainloop: add ffmpeg_audio_parser_finder");
	m_df->add_audio_parser_finder(new net::ffmpeg_audio_parser_finder());
    AM_DBG lib::logger::get_logger()->trace("qt_mainloop::qt_mainloop: add ffmpeg_audio_filter_finder");
	m_df->add_audio_filter_finder(new net::ffmpeg_audio_filter_finder());

#endif

	// Next create the playable factory and populate it.
	common::global_playable_factory *m_rf =
		new common::global_playable_factory(); 
#ifdef WITH_SDL
	AM_DBG logger::get_logger()->trace("add factory for SDL");
	m_rf->add_factory( new sdl::sdl_renderer_factory(m_df) );
AM_DBG logger::get_logger()->trace("add factory for SDL done");
#endif
#ifdef WITH_ARTS
	m_rf->add_factory(new arts::arts_renderer_factory(m_df));
#endif 
	m_rf->add_factory(new qt_renderer_factory(m_df));
	AM_DBG lib::logger::get_logger()->trace("mainloop::mainloop: add raw_video_factory");
 	m_rf->add_factory(new none::none_video_factory(m_df));
	
 	m_rf->add_factory(new qt::qt_video_factory(m_df));

	m_wf = new qt_window_factory(parent, 
				     parent->get_o_x(),
				     parent->get_o_y());

	const char *filename = parent->filename();
	m_doc = document::create_from_file(filename);
	if (!m_doc) {
		logger::get_logger()->error("Could not build tree for file: %s", filename);
		return;
	}
	bool is_mms = strcmp(".mms", filename + strlen(filename) - 4) == 0;
	if (is_mms) {
		m_player = create_mms_player(m_doc, m_wf, m_rf);
	} else {
		m_player = create_smil2_player(m_doc, m_wf, m_rf, this);
	}

}
void*
qt_mainloop::run(void* ml)
{

	net::datasource_factory*df;
	if (ml == NULL)
		return (void*) 0;

        qt_mainloop*		mainloop = (qt_mainloop*) ml;
	qt_gui*			qt_view = mainloop->m_parent;
	qt_window_factory*	wf;

	AM_DBG logger::get_logger()->trace(
		"qt_mainloop::run(qt_mainloop=0x%x)",
		mainloop);

	player** player = &mainloop->m_player;
	if (player == NULL || *player == NULL)
		return (void*) 0;
	
	(*player)->start();

	while(!(*player)->is_done())
		sleep(1);

	// XXXX Should we call a callback in the parent?

	return (void*) 1;
}

qt_mainloop::~qt_mainloop()
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
qt_mainloop::play()
{
	m_running = true;
	m_player->start();
	AM_DBG ambulant::lib::logger::get_logger()->trace("qt_mainloop::run(): returning");
}

void
qt_mainloop::stop()
{
	m_player->stop();
	AM_DBG ambulant::lib::logger::get_logger()->trace("qt_mainloop::run(): returning");
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

void
qt_mainloop::set_preferences(std::string &url)
{
//	ambulant::smil2::test_attrs::load_test_attrs(url);
}

void
qt_mainloop::show_file(const std::string &href)
{
	lib::logger::get_logger()->error("This implementation cannot open <%s> in a webbrowser yet", href.c_str());
}
