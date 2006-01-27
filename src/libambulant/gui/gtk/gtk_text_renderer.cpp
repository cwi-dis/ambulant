// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

#include "ambulant/gui/gtk/gtk_includes.h"
#include "ambulant/gui/gtk/gtk_factory.h"
#include "ambulant/gui/gtk/gtk_renderer.h"
#include "ambulant/gui/gtk/gtk_text_renderer.h"
#include "ambulant/smil2/params.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::gtk;

gtk_text_renderer::gtk_text_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
    	lib::event_processor *const evp,
    	common::factories *factory)
:	gtk_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory),
 	m_text_storage(NULL),
 	m_text_color(0),
 	m_text_font(NULL),
 	m_text_size(0)
{
/*
	smil2::params *params = smil2::params::for_node(node);
	AM_DBG lib::logger::get_logger()->debug("gtk_text_renderer(0x%x) params=0x%x",this,params);
	if (params) {
		m_text_font = params->get_str("font-family");
//		const char *fontstyle = params->get_str("font-style");
		m_text_color = params->get_color("color", 0);
		m_text_size = params->get_float("font-size", 0.0);
		delete params;
	}
*/
}

gtk_text_renderer::~gtk_text_renderer() {
/*	AM_DBG lib::logger::get_logger()->debug("~gtk_text_renderer(0x%x)", this);
	m_lock.enter();
	if (m_text_storage != NULL) {
		free(m_text_storage);
		m_text_storage =  NULL;
	}
	m_lock.leave();
*/
}

void
gtk_text_renderer::redraw_body(const lib::rect &r,
				     common::gui_window* w) {
// No m_lock needed, protected by base class
/*	const lib::point p = m_dest->get_global_topleft();
	if (m_data && !m_text_storage) {
		m_text_storage = (char*) malloc(m_data_size+1);
		strncpy(m_text_storage,
			(const char*) m_data,
			m_data_size);
		m_text_storage[m_data_size] = '\0';
	}
	AM_DBG lib::logger::get_logger()->debug(
		"gtk_text_renderer.redraw(0x%x):"
		"ltrb=(%d,%d,%d,%d)\nm_text_storage = %s, p=(%d,%d)",
		(void *)this, r.left(), r.top(), r.right(), r.bottom(),
		m_text_storage == NULL ? "(null)": (const char*) m_text_storage,
		p.x, p.y);
	if (m_text_storage) {
		int L = r.left()+p.x, 
		    T = r.top()+p.y,
		    W = r.width(),
		    H = r.height();
		ambulant_gtk_window* aqw = (ambulant_gtk_window*) w;
		QPainter paint;
		paint.begin(aqw->get_ambulant_pixmap());

		// Set <param> parameters
		QColor gtk_color(redc(m_text_color), greenc(m_text_color), bluec(m_text_color));
		paint.setPen(gtk_color);
		
		QFont gtk_font(QApplication::font());
		if (m_text_font)
			gtk_font.setFamily(m_text_font);
		if (m_text_size)
			gtk_font.setPointSizeFloat(m_text_size);
		paint.setFont(gtk_font);
		
		paint.drawText(L,T,W,H,
			Qt::AlignLeft|Qt::AlignTop|Qt::WordBreak,
			m_text_storage);
		paint.flush();
		paint.end();
	}
*/
}
