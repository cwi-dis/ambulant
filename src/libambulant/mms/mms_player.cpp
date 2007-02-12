// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

common::player*
common::create_mms_player(lib::document *doc, common::factories* factory)
{
	return new mms_player(doc, factory);
}

mms_player::mms_player(lib::document *doc, common::factories* factory)
:	m_doc(doc),
	m_tree(doc->get_root()),
	m_timer(new lib::timer_control_impl(lib::realtime_timer_factory(), 0.0)),
	m_event_processor(lib::event_processor_factory(m_timer)),
	m_factory(factory)
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
	common::layout_manager *layoutmgr = new mms_layout_manager(m_factory->get_window_factory(), m_doc);
	if (ptl) {
#ifndef AMBULANT_NO_IOSTREAMS
		AM_DBG std::cout << "------------ mms_player: passive_timeline:" << std::endl;
		AM_DBG ptl->dump(std::cout);
#endif
		active_timeline *atl = ptl->activate(m_event_processor, m_factory->get_playable_factory(), layoutmgr);
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
	timeline_builder builder = timeline_builder(m_factory->get_window_factory(), *playroot);
	return builder.build();
}
