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

#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_image_renderer.h"
#include "ambulant/gui/qt/qt_transition.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::qt;

qt_active_image_renderer::~qt_active_image_renderer() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->trace(
		"qt_active_image_renderer::~qt_active_image_renderer(0x%x)",this);
	if (m_trans_engine) delete m_trans_engine;
	m_lock.leave();
}
	
void
qt_active_image_renderer::start(double where)
{
	AM_DBG logger::get_logger()->trace("qt_active_image_renderer.start(0x%x) m_dest=0x%x", (void *)this, (void*)m_dest);
	if (m_intransition) {
		m_trans_engine = qt_transition_engine(m_dest, false, m_intransition);
		if (m_trans_engine)
			m_trans_engine->begin(m_event_processor->get_timer()->elapsed());
	}
	if (m_outtransition) {
		// XXX Schedule beginning of out transition
		//lib::event *ev = new transition_callback(this, &transition_outbegin);
		//m_event_processor->add_event(ev, XXXX);
	}
	common::renderer_playable_dsall::start(where);
}

void
qt_active_image_renderer::start_outtransition(lib::transition_info *info)
{
}

void
qt_active_image_renderer::redraw(const lib::screen_rect<int> &dirty,
				 common::gui_window* w) {
	m_lock.enter();
	const lib::point p = m_dest->get_global_topleft();
	const lib::screen_rect<int> &r = m_dest->get_rect();
	AM_DBG lib::logger::get_logger()->trace
		("qt_active_image_renderer.redraw(0x%x):"
		" ltrb=(%d,%d,%d,%d), p=(%d,%d)",
		(void *)this,
		r.left(), r.top(), r.right(), r.bottom(),
		p.x,p.y);
	if (m_data && !m_image_loaded) {
		m_image_loaded = m_image.loadFromData(
			(const uchar*)m_data, m_data_size);
	}
	ambulant_qt_window* aqw = (ambulant_qt_window*) w;
	QPixmap *surf = NULL;
	if (m_trans_engine && m_trans_engine->is_done()) {
		delete m_trans_engine;
		m_trans_engine = NULL;
	}
	// See whether we're in a transition
	if (m_trans_engine) {
		surf = aqw->get_ambulant_surface();
		if (surf == NULL)
			surf = aqw->new_ambulant_surface();
		if (surf != NULL) {
			aqw->set_ambulant_surface(surf);	
		AM_DBG logger::get_logger()->trace("qt_active_image_renderer.redraw: drawing to transition surface");
		}
	}
	// XXXX WRONG! This is the info for the region, not for the node!
	const common::region_info *info = m_dest->get_info();
	AM_DBG lib::logger::get_logger()->trace(
		"qt_active_image_renderer.redraw: info=0x%x", info);
	QPainter paint;
	paint.begin(aqw->ambulant_pixmap());
	// background drawing
	if (info && !info->get_transparent()) {
	// First find our whole area (which we have to clear to 
	// background color)
		lib::screen_rect<int> dstrect_whole = r;
		dstrect_whole.translate(m_dest->get_global_topleft());
		int L = dstrect_whole.left(),
		    T = dstrect_whole.top(),
		    W = dstrect_whole.width(),
		    H = dstrect_whole.height();
		// XXXX Fill with background color
		lib::color_t bgcolor = info->get_bgcolor();
		AM_DBG lib::logger::get_logger()->trace(
			"qt_active_image_renderer.redraw:"
			" clearing to 0x%x", (long)bgcolor);
		QColor bgc = QColor(lib::redc(bgcolor),
				    lib::greenc(bgcolor),
				    lib::bluec(bgcolor));
		paint.setBrush(bgc);
		paint.drawRect(L,T,W,H);
	}
	if (m_image_loaded) {
		QSize qsize = aqw->ambulant_pixmap()->size();
		lib::size srcsize = lib::size(qsize.width(), qsize.height());
		lib::rect srcrect = lib::rect(lib::size(0,0));
		lib::screen_rect<int> dstrect = m_dest->get_fit_rect(
			srcsize, &srcrect, m_alignment);
		dstrect.translate(m_dest->get_global_topleft());
		int L = dstrect.left(), 
		    T = dstrect.top(),
		    W = dstrect.width(),
		    H = dstrect.height();
		AM_DBG lib::logger::get_logger()->trace(
			"qt_active_image_renderer.redraw(0x%x):"
			" drawImage at (L=%d,T=%d,W=%d,H=%d)",
			(void *)this,L,T,W,H);
		paint.drawImage(L,T,m_image,0,0,W,H);
	}
	else {
		AM_DBG lib::logger::get_logger()->error(
			"qt_active_image_renderer.redraw(0x%x):"
			" no m_image",
			(void *)this
		);
	}
#ifdef	JUNK
#endif/*JUNK*/
	if (surf != NULL) {
		aqw->reset_ambulant_surface();
	}
	paint.flush();
	paint.end();
	if (m_trans_engine && surf) {
		AM_DBG logger::get_logger()->trace("qt_active_image_renderer.redraw: drawing to view");
		m_trans_engine->step(m_event_processor->get_timer()->elapsed());
		typedef lib::no_arg_callback<qt_active_image_renderer> transition_callback;
		lib::event *ev = new transition_callback(this, &qt_active_image_renderer::transition_step);
		m_event_processor->add_event(ev, m_trans_engine->next_step_delay());
	}
	m_lock.leave();

}

void
qt_active_image_renderer::transition_step()
{
  AM_DBG logger::get_logger()->trace("qt_active_image_renderer::transition_step(0x%x) m_dest=0x%x",(void*)this, (void*) m_dest);
	if (m_dest) m_dest->need_redraw();
}
