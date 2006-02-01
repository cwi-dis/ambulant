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

#define AM_DBG
//#ifndef AM_DBG
//#define AM_DBG if(0)
//#endif

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
	smil2::params *params = smil2::params::for_node(node);
	AM_DBG lib::logger::get_logger()->debug("gtk_text_renderer(0x%x) params=0x%x",this,params);
	if (params) {
		m_text_font = params->get_str("font-family");
//		const char *fontstyle = params->get_str("font-style");
		m_text_color = params->get_color("color", 0);
		m_text_size = params->get_float("font-size", 0.0);
		delete params;
	}
}

gtk_text_renderer::~gtk_text_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~gtk_text_renderer(0x%x)", this);
	m_lock.enter();
	if (m_text_storage != NULL) {
		free(m_text_storage);
		m_text_storage =  NULL;
	}
	m_lock.leave();
}

void
gtk_text_renderer::redraw_body(const lib::rect &r,
				     common::gui_window* w) {
// No m_lock needed, protected by base class
	const lib::point p = m_dest->get_global_topleft();
	if (m_data && !m_text_storage) {
		m_text_storage = (char*) malloc(m_data_size+1);
		strncpy(m_text_storage,
			(const char*) m_data,
			m_data_size);
		m_text_storage[m_data_size] = '\0';
	}
	AM_DBG lib::logger::get_logger()->debug(
		"gtk_text_renderer.redraw(0x%x):"
		"ltrb=(%d,%d,%d,%d)\nm_text_storage = %s, p=(%d,%d):"
		"font-family=(%s)",
		(void *)this, r.left(), r.top(), r.width(), r.height(),
		m_text_storage == NULL ? "(null)": (const char*) m_text_storage,
		p.x, p.y, m_text_font == NULL ? "(null)": (const char*) m_text_font);
	if (m_text_storage) {
		int L = r.left()+p.x, 
		    T = r.top()+p.y,
		    W = r.width(),
		    H = r.height();
		ambulant_gtk_window* agtkw = (ambulant_gtk_window*) w;

		PangoContext* pc = gtk_widget_get_pango_context(GTK_WIDGET (agtkw->get_ambulant_widget()->get_gtk_widget()));
		
		GdkColor gtk_color;
		gtk_color.red = redc(m_text_color)*0x101;
		gtk_color.blue = bluec(m_text_color)*0x101;
		gtk_color.green = greenc(m_text_color)*0x101;
		GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
		gdk_gc_set_rgb_fg_color (gc, &gtk_color);

		PangoLayout* pl = pango_layout_new(pc);
		pango_layout_set_text(pl, m_text_storage, -1);
//gtk_widget_create_pango_layout (GTK_WIDGET (agtkw->get_ambulant_widget()->get_gtk_widget()), m_text_storage);
		//pango_layout_set_width(pl, W);
		if (m_text_font){
//			font = gdk_font_load(m_text_font);
			PangoFontDescription* pfd = pango_font_description_new();
			pango_font_description_set_family(pfd, m_text_font);
			pango_layout_set_font_description(pl, pfd);
			pango_font_description_free(pfd);
		}
		pango_layout_set_alignment(pl, PANGO_ALIGN_LEFT);
		int width; int height;
		pango_layout_get_size(pl, &width, &height);
		printf("The sizes are %i %i\n", width, height);
//		pango_layout_set_wrap(pl, PANGO_WRAP_WORD_CHAR);
//		if (m_text_size)
//			gtk_font.setPointSizeFloat(m_text_size);
//			font = gtk_style_get_font(agtkw->get_ambulant_widget()->get_gtk_widget()->style);
  		gdk_draw_layout(GDK_DRAWABLE (agtkw->get_ambulant_pixmap()),gc, L, T, pl);
		g_object_unref (pl);
		g_object_unref (G_OBJECT (gc));
		//gdk_draw_string(GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), font, gc, L, T, m_text_storage);
//		paint.drawText(L,T,W,H,
//			Qt::AlignLeft|Qt::AlignTop|Qt::WordBreak,
//			m_text_storage);
	}
}
