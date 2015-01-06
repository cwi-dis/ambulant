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

#ifdef  WITH_SDL_IMAGE // TBD

#include "ambulant/gui/SDL/sdl_renderer.h"
#include "ambulant/gui/SDL/sdl_transition.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

namespace gui {

using namespace common;
using namespace lib;

namespace sdl {

sdl_transition_renderer::~sdl_transition_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~sdl_renderer(%p)", (void *)this);
	m_intransition = NULL;
	m_outtransition = NULL;
	if (m_trans_engine) delete m_trans_engine;
	m_trans_engine = NULL;
	m_lock.leave();
}

void
sdl_transition_renderer::set_surface(common::surface *dest)
{
	m_transition_dest = dest;
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
}

void
sdl_transition_renderer::set_intransition(const lib::transition_info *info)
{
	m_intransition = info;
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
}

void
sdl_transition_renderer::start(double where)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("sdl_renderer.start(%p)", (void *)this);
	if (m_intransition && m_transition_dest) {
		m_view = m_transition_dest->get_gui_window();
		m_trans_engine = sdl_transition_engine(m_transition_dest, false, m_intransition);
		if (m_trans_engine) {
			m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
			m_fullscreen = m_intransition->m_scope == scope_screen;
			if (m_fullscreen) {
				ambulant_sdl_window* asw = (ambulant_sdl_window*)m_view;
				sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
				saw->startScreenTransition();
			}
		}
	}
	m_lock.leave();
}

void
sdl_transition_renderer::start_outtransition(const lib::transition_info *info)
{
	if (m_trans_engine) stop();
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("sdl_renderer.start_outtransition(%p)", (void *)this);
	m_outtransition = info;
	m_trans_engine = sdl_transition_engine(m_transition_dest, true, m_outtransition);
	if (m_transition_dest && m_trans_engine) {
		m_view = m_transition_dest->get_gui_window();
		m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
		m_fullscreen = m_outtransition->m_scope == scope_screen;
		if (m_fullscreen) {
			ambulant_sdl_window* asw = (ambulant_sdl_window*)m_view;
			sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
			saw->startScreenTransition();
		}
	}
	m_lock.leave();
	if (m_transition_dest) m_transition_dest->need_redraw();
}

void
sdl_transition_renderer::stop()
{
	m_lock.enter();
	if (!m_trans_engine) {
		m_lock.leave();
		return;
	}
	delete m_trans_engine;
	m_trans_engine = NULL;
	if (m_fullscreen && m_view) {
		ambulant_sdl_window* asw = (ambulant_sdl_window*)m_view;
		sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
		saw->endScreenTransition();
	}
	m_lock.leave();
	if (m_transition_dest) m_transition_dest->transition_done();
	m_view = NULL;
}

void
sdl_transition_renderer::redraw_pre(gui_window *window)
{
	m_lock.enter();
	const rect &tdr = m_transition_dest->get_rect();
	rect r = tdr;
	ambulant_sdl_window* asw = (ambulant_sdl_window*) window;
	sdl_ambulant_window* saw = asw->get_sdl_ambulant_window();
	AM_DBG logger::get_logger()->debug("sdl_renderer.redraw_pre(%p, local_ltrb=(%d,%d,%d,%d) gui_window=%p surface=%p",(void*)this,r.left(),r.top(),r.right(),r.bottom(),window,saw->get_sdl_surface());
//	saw->dump_sdl_surface(saw->get_sdl_surface(), "pre");

	// See whether we're in a transition and setup the correct surface so that
	// redraw_body() will renderer the pixels where we want them.
	if (m_trans_engine && !m_fullscreen) {
     		SDL_Surface* surf = saw->get_sdl_surface();
		if (surf != NULL) {			
			// Push (and copy)  the background pixels
			saw->push_sdl_surface (surf);
			// clear the transition surface and make it current for subsequent redraw_body()
			r.translate(m_transition_dest->get_global_topleft());
			saw->set_sdl_surface (saw->get_transition_surface());
			AM_DBG logger::get_logger()->debug("sdl_renderer.redraw: transition surface pushed");
		}
	}
	m_lock.leave();
}

void
sdl_transition_renderer::redraw_post(gui_window *window)
{
	m_lock.enter();

	sdl_ambulant_window* saw = ((ambulant_sdl_window*) window)->get_sdl_ambulant_window();

	if(m_trans_engine) {
        // If we are in a transition we need to do something: pick up the pixels
        // deposited by redraw_body() and composite them onto the screen with the
        // right "other source".
		lib::transition_info::time_type now = m_event_processor->get_timer()->elapsed();
		if (m_trans_engine->is_done()) {
			if (m_fullscreen) {
				saw->screenTransitionStep(NULL, 0);
			} else {
				m_trans_engine->step(now);
				SDL_Surface* old_surface = saw->pop_sdl_surface();
				saw->delete_transition_surface();
				saw->set_sdl_surface (old_surface);
			}
			typedef lib::no_arg_callback<sdl_transition_renderer> stop_transition_callback;
			lib::event *ev = new stop_transition_callback(this, &sdl_transition_renderer::stop);
			m_event_processor->add_event(ev, 0, lib::ep_high);
		} else {
			AM_DBG logger::get_logger()->debug("sdl_renderer.redraw: drawing to view");
			if (m_fullscreen) {
				saw->screenTransitionStep (m_trans_engine, now);
			} else {
				m_trans_engine->step(now);
				SDL_Surface* s = saw->pop_sdl_surface();
				saw->set_sdl_surface(s);
			}
			typedef no_arg_callback<sdl_transition_renderer>transition_callback;
			event *ev = new transition_callback (this, &sdl_transition_renderer::transition_step);
			transition_info::time_type delay = m_trans_engine->next_step_delay();
			if (delay < 33) delay = 33; // limit redraw frequency to 30 redraws per second
//			delay = 1000; // suitable for debug
			AM_DBG logger::get_logger()->debug("sdl_transition_renderer.redraw: now=%d, schedule step for %d",m_event_processor->get_timer()->elapsed(),m_event_processor->get_timer()->elapsed()+delay);
			m_event_processor->add_event(ev, delay, lib::ep_med);
		}
	}
	m_lock.leave();
}

void
sdl_transition_renderer::transition_step()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("sdl_renderer.transition_step: now=%d",m_event_processor->get_timer()->elapsed());
	if (m_transition_dest) m_transition_dest->need_redraw();
	m_lock.leave();
}

} // namespace sdl

} // namespace gui

} // namespace ambulant

#endif//WITH_SDL2
