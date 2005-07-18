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

#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_transition.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

namespace gui {

using namespace common;
using namespace lib;

namespace qt {

qt_transition_renderer::~qt_transition_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("~qt_renderer(0x%x)", (void *)this);
	m_intransition = NULL;
	m_outtransition = NULL;
	if (m_trans_engine) delete m_trans_engine;
	m_trans_engine = NULL;
	m_lock.leave();
}
	
void
qt_transition_renderer::set_surface(common::surface *dest)
{ 
	m_transition_dest = dest;
#ifdef USE_SMIL21
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
#endif
}
	
void
qt_transition_renderer::set_intransition(const lib::transition_info *info)
{
	m_intransition = info;
#ifdef USE_SMIL21
	if (m_transition_dest && m_intransition && m_intransition->m_scope == scope_screen)
		m_transition_dest = m_transition_dest->get_top_surface();
#endif
}

void
qt_transition_renderer::start(double where)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("qt_renderer.start(0x%x)", (void *)this);
	if (m_intransition && m_transition_dest) {
		m_view = m_transition_dest->get_gui_window();
		m_trans_engine = qt_transition_engine(m_transition_dest, false, m_intransition);
		if (m_trans_engine) {
			m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
#ifdef USE_SMIL21
			m_fullscreen = m_intransition->m_scope == scope_screen;
			if (m_fullscreen) {
				((ambulant_qt_window*)m_view)->startScreenTransition();
			}
#endif
		}
	}
	m_lock.leave();
}

void
qt_transition_renderer::start_outtransition(const lib::transition_info *info)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("qt_renderer.start_outtransition(0x%x)", (void *)this);
	if (m_trans_engine) stop();
	m_outtransition = info;
	m_trans_engine = qt_transition_engine(m_transition_dest, true, m_outtransition);
	if (m_transition_dest && m_trans_engine) {
		m_view = m_transition_dest->get_gui_window();
		m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
#ifdef USE_SMIL21
		m_fullscreen = m_outtransition->m_scope == scope_screen;
		if (m_fullscreen) {
			((ambulant_qt_window*)m_view)->startScreenTransition();
		}
#endif
	}
	m_lock.leave();
	if (m_transition_dest) m_transition_dest->need_redraw();
}

void
qt_transition_renderer::stop()
{
	m_lock.enter();
	if (!m_trans_engine) return;
	delete m_trans_engine;
	m_trans_engine = NULL;
#ifdef USE_SMIL21
	if (m_fullscreen && m_view) {
		((ambulant_qt_window*)m_view)->endScreenTransition();
	}
#endif
	m_lock.leave();
	if (m_transition_dest) m_transition_dest->transition_done();
	m_view = NULL;
}

void
qt_transition_renderer::redraw_pre(gui_window *window)
{
	m_lock.enter();
	const screen_rect<int> &r = m_transition_dest->get_rect();
	ambulant_qt_window* aqw = (ambulant_qt_window*) window;
	AM_DBG logger::get_logger()->debug("qt_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d) gui_window=0x%x qpm=0x%x",(void*)this,r.left(),r.top(),r.right(),r.bottom(),window,aqw->get_ambulant_pixmap());

	QPixmap *surf = NULL;
	// See whether we're in a transition
	if (m_trans_engine
#ifdef USE_SMIL21
	    && !m_fullscreen
#endif
	    ) {
		QPixmap *qpm = aqw->get_ambulant_pixmap();
		surf = aqw->get_ambulant_surface();
		if (surf == NULL)
			surf = aqw->new_ambulant_surface();
		if (surf != NULL) {
			// Copy the background pixels
			screen_rect<int> dstrect = r;
			dstrect.translate(m_transition_dest->get_global_topleft());
			AM_DBG logger::get_logger()->debug("qt_renderer.redraw: bitBlt to=0x%x (%d,%d) from=0x%x (%d,%d,%d,%d)",surf, dstrect.left(), dstrect.top(), qpm,dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height());
			bitBlt(surf, dstrect.left(),dstrect.top(),
			       qpm,dstrect.left(),dstrect.top(),dstrect.width(),dstrect.height());
			AM_DBG logger::get_logger()->debug("qt_renderer.redraw: drawing to transition surface");
			aqw->set_ambulant_surface(surf);
		}
	}
	m_lock.leave();
}

void
qt_transition_renderer::redraw_post(gui_window *window)
{
	m_lock.enter();
	ambulant_qt_window* aqw = (ambulant_qt_window*) window;
	QPixmap *surf = aqw->get_ambulant_surface();
	
	if (surf != NULL) {
		aqw->reset_ambulant_surface();
	}
	if(m_trans_engine) {
		lib::transition_info::time_type now = m_event_processor->get_timer()->elapsed();
		if (m_trans_engine->is_done()) {
#ifdef USE_SMIL21
			if (m_fullscreen)
				aqw->screenTransitionStep(NULL, 0);
			else
				m_trans_engine->step(now);
#else
			m_trans_engine->step(now);
#endif
			typedef lib::no_arg_callback<qt_transition_renderer> stop_transition_callback;
			lib::event *ev = new stop_transition_callback(this, &qt_transition_renderer::stop);
			m_event_processor->add_event(ev, 0, lib::event_processor::med);
		} else {
			if ( 1 /* XXX was: surf */) {
				AM_DBG logger::get_logger()->debug("qt_renderer.redraw: drawing to view");
#ifdef USE_SMIL21
				if (m_fullscreen) {
					aqw->screenTransitionStep (m_trans_engine, now);
				} else {
					m_trans_engine->step(now);
				}
#else
				m_trans_engine->step(now);
#endif
				typedef no_arg_callback<qt_transition_renderer>transition_callback;
				event *ev = new transition_callback (this, &qt_transition_renderer::transition_step);
				transition_info::time_type delay = m_trans_engine->next_step_delay();
				if (delay < 33) delay = 33; // XXX band-aid
	//				delay = 1000;
				AM_DBG logger::get_logger()->debug("qt_transition_renderer.redraw: now=%d, schedule step for %d",m_event_processor->get_timer()->elapsed(),m_event_processor->get_timer()->elapsed()+delay);
				m_event_processor->add_event(ev, delay, event_processor::low);
			}
		}
	}
	m_lock.leave();
}

void
qt_transition_renderer::transition_step()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("qt_renderer.transition_step: now=%d",m_event_processor->get_timer()->elapsed());
	if (m_transition_dest) m_transition_dest->need_redraw();
	m_lock.leave();
}

} // namespace qt

} // namespace gui

} // namespace ambulant
