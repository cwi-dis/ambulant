// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
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

#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/gui/cocoa/cocoa_renderer.h"
#include "ambulant/gui/cocoa/cocoa_transition.h"
//#include "ambulant/common/region_info.h"
//#include "ambulant/common/smil_alignment.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

cocoa_transition_renderer::~cocoa_transition_renderer()
{
	stop();
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~cocoa_transition_renderer(0x%x)", (void *)this);
	m_intransition = NULL;
	m_outtransition = NULL;
	m_lock.leave();
}

void
cocoa_transition_renderer::set_surface(common::surface *dest)
{
	m_transition_dest = dest;
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
}

void
cocoa_transition_renderer::set_intransition(const lib::transition_info *info) {
	m_intransition = info;
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
}

void
cocoa_transition_renderer::start(double where)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("cocoa_transition_renderer.start(%f)", where);
	if (m_intransition && m_transition_dest) {
		AM_DBG logger::get_logger()->debug("cocoa_transition_renderer.start: with intransition");
		m_trans_engine = cocoa_transition_engine(m_transition_dest, false, m_intransition);
		if (m_trans_engine) {
			gui_window *window = m_transition_dest->get_gui_window();
			cocoa_window *cwindow = (cocoa_window *)window;
			AmbulantView *view = (AmbulantView *)cwindow->view();
			[view incrementTransitionCount];
			m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
			m_fullscreen = m_intransition->m_scope == scope_screen;
			if (m_fullscreen) {
				[view startScreenTransition];
			}
		}
	}
	m_lock.leave();
}

void
cocoa_transition_renderer::start_outtransition(const lib::transition_info *info)
{
	if (m_trans_engine) stop();
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("cocoa_transition_renderer.start_outtransition(0x%x)", (void *)this);
	m_outtransition = info;
	m_trans_engine = cocoa_transition_engine(m_transition_dest, true, m_outtransition);
	if (m_transition_dest && m_trans_engine) {
		gui_window *window = m_transition_dest->get_gui_window();
		cocoa_window *cwindow = (cocoa_window *)window;
		AmbulantView *view = (AmbulantView *)cwindow->view();
		[view incrementTransitionCount];
		m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
		m_fullscreen = m_outtransition->m_scope == scope_screen;
		if (m_fullscreen) {
			[view startScreenTransition];
		}
	}
	m_lock.leave();
	if (m_transition_dest) m_transition_dest->need_redraw();
}

void
cocoa_transition_renderer::stop()
{
	m_lock.enter();
	if (m_trans_engine == NULL) {
		m_lock.leave();
		return;
	}
	delete m_trans_engine;
	m_trans_engine = NULL;
	gui_window *window = m_transition_dest->get_gui_window();
	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	[view decrementTransitionCount];
	if (m_fullscreen) {
		[view endScreenTransition];
	}
	m_lock.leave();
	if (m_transition_dest) m_transition_dest->transition_done();
}

void
cocoa_transition_renderer::redraw_pre(gui_window *window)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("cocoa_transition_renderer.redraw(0x%x)", (void *)this);

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();

	// See whether we're in a transition
	NSImage *surf = NULL;
	if (m_trans_engine) {
		surf = [view getTransitionSurface];
		if (surf && [surf isValid]) {
			[surf lockFocus];
			AM_DBG logger::get_logger()->debug("cocoa_transition_renderer.redraw: drawing to transition surface");
		} else {
			lib::logger::get_logger()->trace("cocoa_transition_renderer.redraw: cannot lockFocus for transition");
			surf = NULL;
		}
	}
	m_lock.leave();
}

void
cocoa_transition_renderer::redraw_post(gui_window *window)
{
	m_lock.enter();
	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	NSImage *surf = NULL;
	if (m_trans_engine) {
		surf = [view getTransitionSurface];
		if (![surf isValid]) surf = NULL;
	}
	if (surf) {
		[surf unlockFocus];
		AM_DBG logger::get_logger()->debug("cocoa_transition_renderer.redraw: drawing to view");
		if (m_fullscreen)
			[view screenTransitionStep: m_trans_engine
				elapsed: m_event_processor->get_timer()->elapsed()];
		else
			m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef lib::no_arg_callback<cocoa_transition_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &cocoa_transition_renderer::transition_step);
		lib::transition_info::time_type delay = m_trans_engine->next_step_delay();
		if (delay < 33) delay = 33; // XXX band-aid
		AM_DBG lib::logger::get_logger()->debug("cocoa_transition_renderer.redraw: now=%d, schedule step for %d", m_event_processor->get_timer()->elapsed(), m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay, lib::ep_med);
	}

	// Finally, if the transition is done clean it up and signal that freeze_transition
	// can end for our peer renderers.
	// Note that we have to do this through an event because of locking issues.
	if (m_trans_engine && m_trans_engine->is_done()) {
		typedef lib::no_arg_callback<cocoa_transition_renderer> stop_transition_callback;
		lib::event *ev = new stop_transition_callback(this, &cocoa_transition_renderer::stop);
		m_event_processor->add_event(ev, 0, lib::ep_med);
		if (m_fullscreen)
			[view screenTransitionStep: NULL elapsed: 0];
	}
	m_lock.leave();
}

void
cocoa_transition_renderer::transition_step()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cocoa_transition_renderer.transition_step: now=%d", m_event_processor->get_timer()->elapsed());
	if (m_transition_dest) m_transition_dest->need_redraw();
	m_lock.leave();
}


} // namespace cocoa

} // namespace gui

} //namespace ambulant

