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

#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/gui/cocoa/cocoa_fill.h"
#include "ambulant/gui/cocoa/cocoa_transition.h"
#include "ambulant/common/region_info.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

cocoa_fill_renderer::~cocoa_fill_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("~cocoa_fill_renderer(0x%x)", (void *)this);
	if (m_intransition) delete m_intransition;
	m_intransition = NULL;
	if (m_outtransition) delete m_outtransition;
	m_outtransition = NULL;
	if (m_trans_engine) delete m_trans_engine;
	m_trans_engine = NULL;
	m_lock.leave();
}
	
void
cocoa_fill_renderer::start(double where)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("cocoa_fill_renderer.start(0x%x)", (void *)this);
	if (m_is_showing) {
		logger::get_logger()->trace("cocoa_fill_renderer.start(0x%x): already started", (void*)this);
		m_lock.leave();
		return;
	}
	m_is_showing = true;
	if (!m_dest) {
		logger::get_logger()->trace("cocoa_fill_renderer.start(0x%x): no surface", (void *)this);
		return;
	}
	if (m_intransition) {
		m_trans_engine = cocoa_transition_engine(m_dest, false, m_intransition);
		m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
	}
	m_dest->show(this);
	m_lock.leave();
}

void
cocoa_fill_renderer::start_outtransition(const lib::transition_info *info)
{
	m_lock.enter();
	m_outtransition = info;
	if (m_outtransition) {
		// XXX Schedule beginning of out transition
		//lib::event *ev = new transition_callback(this, &transition_outbegin);
		//m_event_processor->add_event(ev, XXXX);
	}
	m_lock.leave();
}

void
cocoa_fill_renderer::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("cocoa_fill_renderer.stop(0x%x)", (void *)this);
	if (!m_is_showing) {
		logger::get_logger()->trace("cocoa_fill_renderer.stop(0x%x): already stopped", (void*)this);
	} else {
		m_is_showing = false;
		if (m_dest) m_dest->renderer_done(this);
	}
	m_lock.leave();
}

void
cocoa_fill_renderer::redraw(const screen_rect<int> &dirty, gui_window *window)
{
	m_lock.enter();
	const screen_rect<int> &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_fill_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	
	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();

	// See whether we're in a transition
	NSImage *surf = NULL;
	if (m_trans_engine && m_trans_engine->is_done()) {
		delete m_trans_engine;
		m_trans_engine = NULL;
	}
	if (m_trans_engine) {
		surf = [view getTransitionSurface];
		[surf lockFocus];
		AM_DBG logger::get_logger()->debug("cocoa_fill_renderer.redraw: drawing to transition surface");
	}
	// First find our whole area (which we have to clear to background color)
	screen_rect<int> dstrect_whole = r;
	dstrect_whole.translate(m_dest->get_global_topleft());
	NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
	// Fill with  color
	const char *color_attr = m_node->get_attribute("color");
	if (!color_attr) {
		lib::logger::get_logger()->trace("<brush> element without color attribute");
		if (surf) {
			AM_DBG logger::get_logger()->debug("cocoa_fill_renderer.redraw: drawing to view");
			[surf unlockFocus];
		}
		m_lock.leave();
		return;
	}
	color_t color = lib::to_color(color_attr);
	AM_DBG lib::logger::get_logger()->debug("cocoa_fill_renderer.redraw: clearing to 0x%x", (long)color);
	NSColor *cocoa_bgcolor = [NSColor colorWithCalibratedRed:redf(color)
				green:greenf(color)
				blue:bluef(color)
				alpha:1.0];
	[cocoa_bgcolor set];
	NSRectFill(cocoa_dstrect_whole);
	if (surf) [surf unlockFocus];
	if (m_trans_engine) {
		assert(surf);
		AM_DBG logger::get_logger()->debug("cocoa_fill_renderer.redraw: drawing to view");
		m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef lib::no_arg_callback<cocoa_fill_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &cocoa_fill_renderer::transition_step);
		m_event_processor->add_event(ev, m_trans_engine->next_step_delay(), lib::event_processor::med);
	}
	m_lock.leave();
}

void
cocoa_fill_renderer::transition_step()
{
	if (m_dest) m_dest->need_redraw();
}

void 
cocoa_fill_renderer::user_event(const point &where, int what)
{
	if (what == user_event_click) m_context->clicked(m_cookie, 0);
	else if (what == user_event_mouse_over) m_context->pointed(m_cookie, 0);
	else assert(0);
}

void
cocoa_background_renderer::redraw(const lib::screen_rect<int> &dirty, common::gui_window *window)
{
	const screen_rect<int> &r =  m_dst->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_bg_renderer::drawbackground(0x%x, local_ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	
	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	AM_DBG lib::logger::get_logger()->debug("cocoa_bg_renderer::drawbackground: %d clearing to 0x%x", !m_src->get_transparent(), (long)m_src->get_bgcolor());
	if (m_src && !m_src->get_transparent()) {
		// First find our whole area (which we have to clear to background color)
		screen_rect<int> dstrect_whole = r;
		dstrect_whole.translate(m_dst->get_global_topleft());
		NSRect cocoa_dstrect_whole = [view NSRectForAmbulantRect: &dstrect_whole];
		// XXXX Fill with background color
		color_t bgcolor = m_src->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("cocoa_bg_renderer::drawbackground: clearing to 0x%x", (long)bgcolor);
		NSColor *cocoa_bgcolor = [NSColor colorWithCalibratedRed:redf(bgcolor)
					green:greenf(bgcolor)
					blue:bluef(bgcolor)
					alpha:1.0];
		[cocoa_bgcolor set];
		NSRectFill(cocoa_dstrect_whole);
	}	
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

