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
#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_fill.h"
#include "ambulant/gui/qt/qt_transition.h"
#include "ambulant/gui/qt/qt_image_renderer.h"
#include "ambulant/gui/qt/qt_text_renderer.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::qt;

qt_fill_renderer::~qt_fill_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("~qt_fill_renderer(0x%x)", (void *)this);
	if (m_intransition) delete m_intransition;
	if (m_outtransition) delete m_outtransition;
	if (m_trans_engine) delete m_trans_engine;
	m_lock.leave();
}
	
void
qt_fill_renderer::start(double where)
{
	m_lock.enter();
	AM_DBG logger::get_logger()->debug("qt_fill_renderer.start(0x%x)", (void *)this);
	if (m_is_showing) {
		logger::get_logger()->trace("qt_fill_renderer.start(0x%x): already started", (void*)this);
		m_lock.leave();
		return;
	}
	m_is_showing = true;
	if (!m_dest) {
		logger::get_logger()->trace("qt_fill_renderer.start(0x%x): no surface", (void *)this);
		return;
	}
	if (m_intransition) {
		m_trans_engine = qt_transition_engine(m_dest, false, m_intransition);
		m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
	}
	m_dest->show(this);
	m_lock.leave();
}

void
qt_fill_renderer::start_outtransition(lib::transition_info *info)
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
qt_fill_renderer::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("qt_fill_renderer.stop(0x%x)", (void *)this);
	if (!m_is_showing) {
		logger::get_logger()->trace("qt_fill_renderer.stop(0x%x): already stopped", (void*)this);
	} else {
		m_is_showing = false;
		if (m_dest) m_dest->renderer_done(this);
	}
	m_lock.leave();
}

void
qt_fill_renderer::redraw(const screen_rect<int> &dirty,
			 gui_window *window)
{
	m_lock.enter();
	const screen_rect<int> &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug
	  ("qt_fill_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d)", 
	   (void *)this, r.left(), r.top(), r.right(), r.bottom());
	
	ambulant_qt_window* aqw = (ambulant_qt_window*) window;
	QPixmap *surf = NULL;
	if (m_trans_engine && m_trans_engine->is_done()) {
		delete m_trans_engine;
		m_trans_engine = NULL;
	}
	// See whether we're in a transition
	if (m_trans_engine) {
		QPixmap *qpm = aqw->ambulant_pixmap();
		surf = aqw->get_ambulant_surface();
		if (surf == NULL)
			surf = aqw->new_ambulant_surface();
		if (surf != NULL) {
			aqw->set_ambulant_surface(surf);
			/*
		// Copy the background pixels
		screen_rect<int> dstrect = r;
		dstrect.translate(m_dest->get_global_topleft());
		bitBlt(surf, dstrect.left(), dstrect.top(),
		       qpm,  dstrect.left(), dstrect.top(), 
		             dstrect.width(), dstrect.height());
			*/
		AM_DBG logger::get_logger()->debug("qt_fill_renderer.redraw: drawing to transition surface");
		}
	}

	redraw_body(dirty, window);
	
	if (surf != NULL) {
		aqw->reset_ambulant_surface();
	}
	if (m_trans_engine && surf) {
		AM_DBG logger::get_logger()->debug
		  ("qt_fill_renderer.redraw: drawing to view");
		m_trans_engine->step
		  (m_event_processor->get_timer()->elapsed());
		typedef no_arg_callback<qt_fill_renderer>transition_callback;
		event *ev = new transition_callback
		  (this, &qt_fill_renderer::transition_step);
		transition_info::time_type delay
		  = m_trans_engine->next_step_delay();
//		if (delay < 33) delay = 33; // XXX band-aid
//		delay = 500;
		AM_DBG logger::get_logger()->debug
		  ("qt_fill_renderer.redraw: now=%d, schedule step for %d",
		   m_event_processor->get_timer()->elapsed(), 
		   m_event_processor->get_timer()->elapsed()+delay);
		m_event_processor->add_event(ev, delay, event_processor::med);
	}
	m_lock.leave();
}


void
qt_fill_renderer::transition_step()
{
	if (m_dest) m_dest->need_redraw();
}

void 
qt_fill_renderer::user_event(const point &where, int what)
{
	if (what == user_event_click) m_context->clicked(m_cookie, 0);
	else if (what == user_event_mouse_over) m_context->pointed(m_cookie, 0);
	else assert(0);
}

void
qt_fill_renderer::redraw_body(const lib::screen_rect<int> &dirty,
				     common::gui_window *window) {
	const common::region_info *info = m_dest->get_info();
	const lib::screen_rect<int> &r = m_dest->get_rect();
	ambulant_qt_window* aqw = (ambulant_qt_window*) window;
	QPainter paint;
	paint.begin(aqw->ambulant_pixmap());
	// <brush> drawing
	// First find our whole area to be cleared to <brush> color
	lib::screen_rect<int> dstrect_whole = r;
	dstrect_whole.translate(m_dest->get_global_topleft());
	int	L = dstrect_whole.left(), 
		T = dstrect_whole.top(),
		W = dstrect_whole.width(), 
		H = dstrect_whole.height();
	// Fill with  color
	const char *color_attr = m_node->get_attribute("color");
	if (!color_attr) {
		lib::logger::get_logger()->trace("<brush> element without color attribute");
		return;
	}
	// Fill with <brush> color
	color_t color = lib::to_color(color_attr);
	//	lib::color_t bgcolor = info->get_bgcolor();
	AM_DBG lib::logger::get_logger()->debug
		("qt_fill_renderer.redraw_body: clearing to 0x%x", 
		 (long)color);
	QColor bgc = QColor(lib::redc(color),
			    lib::greenc(color),
			    lib::bluec(color));
	AM_DBG lib::logger::get_logger()->debug
	  ("qt_fill_renderer.redraw_body(0x%x, local_ltrb=(%d,%d,%d,%d)",
	   (void *)this, L,T,W,H);
	paint.setBrush(bgc);
	paint.drawRect(L,T,W,H);
	paint.flush();
	paint.end();
}

void
qt_background_renderer::redraw(const lib::screen_rect<int> &dirty,
			       common::gui_window *window)
{	
	const lib::screen_rect<int> &r = m_dst->get_rect();
	AM_DBG lib::logger::get_logger()->debug
		("qt_bg_renderer::drawbackground(0x%x)", (void *)this);
	if (m_src && !m_src->get_transparent()) {
	// First find our whole area to be cleared to background color
		ambulant_qt_window* aqw = (ambulant_qt_window*) window;
		QPainter paint;
		paint.begin(aqw->ambulant_pixmap());
		lib::screen_rect<int> dstrect_whole = r;
		dstrect_whole.translate(m_dst->get_global_topleft());
		int L = dstrect_whole.left(),
		    T = dstrect_whole.top(),
		    W = dstrect_whole.width(),
		    H = dstrect_whole.height();
		// XXXX Fill with background color
		lib::color_t bgcolor = m_src->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug(
			"qt_background_renderer::drawbackground:"
			 " %s0x%x,%s(%d,%d,%d,%d)",
			" clearing to ", (long)bgcolor, 
			" local_ltwh=",L,T,W,H);
		QColor bgc = QColor(lib::redc(bgcolor),
				    lib::greenc(bgcolor),
				    lib::bluec(bgcolor));
		paint.setBrush(bgc);
		paint.drawRect(L,T,W,H);
		paint.flush();
		paint.end();
	}
}
