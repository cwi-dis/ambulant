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

#include "ambulant/lib/document.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/mms/timeline_builder.h"
#include "ambulant/mms/mms_player.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace mms;

mms_player::mms_player(lib::document *doc, common::window_factory *wf, common::playable_factory *rf)
:	m_doc(doc),
	m_tree(doc->get_root()),
	m_timer(new lib::timer(lib::realtime_timer_factory(), 0.0)),
	m_event_processor(lib::event_processor_factory(m_timer)),
	m_window_factory(wf),
	m_playable_factory(rf)
{
}

mms_player::~mms_player()
{
	delete m_event_processor;
	delete m_timer;
}

void
mms_player::start()
{
	m_done = false;
	passive_timeline *ptl = build_timeline();
	common::layout_manager *layoutmgr = new mms_layout_manager(m_window_factory, m_doc);
	if (ptl) {
#ifndef AMBULANT_NO_IOSTREAMS
		AM_DBG std::cout << "------------ mms_player: passive_timeline:" << std::endl;
		AM_DBG ptl->dump(std::cout);
#endif
		active_timeline *atl = ptl->activate(m_event_processor, m_playable_factory, layoutmgr);
#ifndef AMBULANT_NO_IOSTREAMS
		AM_DBG std::cout << "------------ mms_player: active_timeline:" << std::endl;
		AM_DBG atl->dump(std::cout);
#endif
		m_active_timelines.push_back(atl);
	
		typedef lib::no_arg_callback<mms_player> callback;
		lib::event *ev = new callback(this, 
			&mms_player::timeline_done_callback);
	
		// run it
		atl->preroll();
		m_timer->set_speed(1.0);
		atl->start(ev);
	}
}
void
mms_player::stop()
{
	std::vector<active_timeline *>::iterator it;
	for(it = m_active_timelines.begin(); it != m_active_timelines.end(); it++) {
		(*it)->stop();
	}
	//lib::logger::get_logger()->error("mms_player::stop(): Not yet implemented");
}

void 
mms_player::pause() {
	if(get_speed() == 0.0) return;
	std::vector<active_timeline *>::iterator it;
	for(it = m_active_timelines.begin(); it != m_active_timelines.end(); it++)
		(*it)->pause();
	m_pause_speed = get_speed();
	set_speed(0);
}

void 
mms_player::resume() {
	if(get_speed() > 0.0) return;
	std::vector<active_timeline *>::iterator it;
	for(it = m_active_timelines.begin(); it != m_active_timelines.end(); it++)
		(*it)->resume();
	set_speed(m_pause_speed);
}

void
mms_player::set_speed(double speed)
{
	m_timer->set_speed(speed);
}

double
mms_player::get_speed() const
{
	return m_timer->get_realtime_speed();
}

passive_timeline *
mms_player::build_timeline()
{
	lib::node *playroot = m_tree->get_first_child("body");
	if (playroot == NULL)
		playroot = m_tree;
	timeline_builder builder = timeline_builder(m_window_factory, *playroot);
	return builder.build();
}

