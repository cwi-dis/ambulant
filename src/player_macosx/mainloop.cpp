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
#include "mainloop.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/document.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"

void
usage()
{
	std::cerr << "Usage: demoplayer file" << std::endl;
	std::cerr << "Options: --version (-v) prints version info" << std::endl;
	exit(1);
}

void
mainloop::run(const char *filename, ambulant::lib::window_factory *wf)
{
	using namespace ambulant;
	
	lib::global_renderer_factory *rf = new lib::global_renderer_factory();
	rf->add_factory(new ambulant::gui::cocoa::cocoa_renderer_factory());
#ifdef WITH_MMS_PLAYER
	lib::passive_player *p = new lib::passive_player(filename);
	if (!p) return;
	m_active_player = p->activate(wf, rf);
	if (!m_active_player) return;
	lib::timer *our_timer = new lib::timer(lib::realtime_timer_factory());
	lib::event_processor *processor = lib::event_processor_factory(our_timer);

	typedef lib::no_arg_callback<mainloop> callback;
	lib::event *ev = new callback(this, &mainloop::player_done_callback);

	m_running = true;
	m_active_player->start(processor, ev);

	while (m_running)
		sleep(1);
//	delete m_active_player;
	m_active_player = NULL;
#else
	lib::document *doc = lib::document::create_from_file(filename);
	if (!doc) {
		lib::logger::get_logger()->error("Could not build tree for file: %s", filename);
		return;
	}
	m_smil_player = new lib::smil_player(doc, wf, rf);
	m_smil_player->start();
	while (!m_smil_player->is_done())
		sleep(1);
#endif
}

void
mainloop::set_speed(double speed)
{
	m_speed = speed;
#ifdef WITH_MMS_PLAYER
	if (m_active_player)
		m_active_player->set_speed(speed);
#else
	if (m_smil_player) {
		if (speed == 0.0)
			m_smil_player->pause();
		else
			m_smil_player->resume();
	}
#endif
}
