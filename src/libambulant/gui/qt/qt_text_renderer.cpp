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
#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/gui/qt/qt_text_renderer.h"
#include "ambulant/smil2/params.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::qt;

qt_active_text_renderer::qt_active_text_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
    	lib::event_processor *const evp,
    	common::factories *factory)
:	qt_renderer(context, cookie, node, evp, factory),
 	m_text_storage(NULL),
 	m_text_color(0),
 	m_text_font(NULL),
 	m_text_size(0)
{
	smil2::params *params = smil2::params::for_node(node);
	if (params) {
		m_text_font = params->get_str("font-family");
//		const char *fontstyle = params->get_str("font-style");
		m_text_color = params->get_color("color", 0);
		m_text_size = params->get_float("font-size", 0.0);
		delete params;
	}
}

qt_active_text_renderer::~qt_active_text_renderer() {
	m_lock.enter();
	if (m_text_storage != NULL) {
		free(m_text_storage);
		m_text_storage =  NULL;
	}
	m_lock.leave();
}

void
qt_active_text_renderer::redraw_body(const lib::screen_rect<int> &r,
				     common::gui_window* w) {
// No m_lock needed, protected by base class
	const lib::point p = m_dest->get_global_topleft();
	AM_DBG lib::logger::get_logger()->debug(
		"qt_active_text_renderer.redraw(0x%x):"
		"ltrb=(%d,%d,%d,%d)\nm_data = %s, p=(%d,%d)",
		(void *)this, r.left(), r.top(), r.right(), r.bottom(),
		m_data == NULL ? "(null)": (const char*) m_data,
		p.x, p.y);
	if (m_data && !m_text_storage) {
		m_text_storage = (char*) malloc(m_data_size+1);
		strncpy(m_text_storage,
			(const char*) m_data,
			m_data_size);
		m_text_storage[m_data_size] = '\0';
	}
	if (m_text_storage) {
		int L = r.left()+p.x, 
		    T = r.top()+p.y,
		    W = r.width(),
		    H = r.height();
		ambulant_qt_window* aqw = (ambulant_qt_window*) w;
		QPainter paint;
		paint.begin(aqw->get_ambulant_pixmap());

		// Set <param> parameters
		QColor qt_color(redc(m_text_color), greenc(m_text_color), bluec(m_text_color));
		paint.setPen(qt_color);
		
		QFont qt_font(QApplication::font());
		if (m_text_font)
			qt_font.setFamily(m_text_font);
		if (m_text_size)
			qt_font.setPointSizeFloat(m_text_size);
		paint.setFont(qt_font);
		
		paint.drawText(L,T,W,H,
			Qt::AlignLeft|Qt::AlignTop|Qt::WordBreak,
			m_text_storage);
		paint.flush();
		paint.end();
	}
}
