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
#include "ambulant/gui/qt/qt_video_renderer.h"
#include "ambulant/common/region_info.h"


//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::qt;

void 
qt_active_video_renderer::show_frame(char* frame, int size)
{
	m_lock.enter();
	memcpy(m_data, frame, size);
	m_data_size = size;
	m_dest->need_redraw();
	m_lock.leave();
}



void
qt_active_video_renderer::redraw(const lib::screen_rect<int> &dirty,
				 common::abstract_window* w) {
	m_lock.enter();
	const lib::point p = m_dest->get_global_topleft();
	const lib::screen_rect<int> &r = m_dest->get_rect();
	AM_DBG lib::logger::get_logger()->trace
		("qt_active_video_renderer.redraw(0x%x):"
		" ltrb=(%d,%d,%d,%d), p=(%d,%d)",
		(void *)this,
		r.left(), r.top(), r.right(), r.bottom(),
		p.x,p.y);
	if (m_data && !m_image_loaded) {
		m_image_loaded = m_image.loadFromData(
			(const uchar*)m_data, m_data_size);
	}
	// XXXX WRONG! This is the info for the region, not for the node!
	const common::region_info *info = m_dest->get_info();
	AM_DBG lib::logger::get_logger()->trace(
		"qt_active_video_renderer.redraw: info=0x%x", info);
	ambulant_qt_window* aqw = (ambulant_qt_window*) w;
	QPainter paint;
	paint.begin(aqw->ambulant_widget());
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
			"qt_active_video_renderer.redraw:"
			" clearing to 0x%x", (long)bgcolor);
		QColor* bgc = new QColor(lib::redc(bgcolor),
					 lib::greenc(bgcolor),
					 lib::bluec(bgcolor));
		paint.setBrush(*bgc);
		paint.drawRect(L,T,W,H);
	}
	if (m_image_loaded) {
		QSize qsize = aqw->ambulant_widget()->frameSize();
		lib::size srcsize = lib::size(qsize.width(), qsize.height());
		lib::rect srcrect = lib::rect(lib::size(0,0));
		lib::screen_rect<int> dstrect = m_dest->get_fit_rect(
			srcsize, &srcrect);
		dstrect.translate(m_dest->get_global_topleft());
		int L = dstrect.left(), 
		    T = dstrect.top(),
		    W = dstrect.width(),
		    H = dstrect.height();
		AM_DBG lib::logger::get_logger()->trace(
			" qt_active_video_renderer.redraw(0x%x):"
			" drawImage at (L=%d,T=%d,W=%d,H=%d)",
			(void *)this,L,T,W,H);
		paint.drawImage(L,T,m_image,0,0,W,H);
	}
	else {
		AM_DBG lib::logger::get_logger()->error(
			"qt_active_video_renderer.redraw(0x%x):"
			" no m_image",
			(void *)this
		);
	}
	paint.flush();
	paint.end();
	m_lock.leave();
}
