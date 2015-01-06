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

#include "ambulant/gui/gtk/gtk_factory.h"
#include "ambulant/gui/gtk/gtk_renderer.h"
#include "ambulant/gui/gtk/gtk_text_renderer.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define FONT "Times 6"

using namespace ambulant;
using namespace gui::gtk;

extern const char gtk_text_playable_tag[] = "text";
extern const char gtk_text_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererGtk");
extern const char gtk_text_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererText");

common::playable_factory *
gui::gtk::create_gtk_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererGtk"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererText"), true);
	return new common::single_playable_factory<
		gtk_text_renderer,
		gtk_text_playable_tag,
		gtk_text_playable_renderer_uri,
		gtk_text_playable_renderer_uri2,
		gtk_text_playable_renderer_uri2>(factory, mdp);
}

gtk_text_renderer::gtk_text_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	gtk_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory, mdp),
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
gtk_text_renderer::redraw_body(const lib::rect &dirty, common::gui_window* w) {
// No m_lock needed, protected by base class
	PangoContext *context;
	PangoLanguage *language;
	PangoFontDescription *font_desc;
	PangoLayout *layout;

	const lib::point p = m_dest->get_global_topleft();
	//XXX need to be fixed in renderer_playable_dsl
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
		(void *)this, dirty.left(), dirty.top(), dirty.width(), dirty.height(),
		m_text_storage == NULL ? "(null)": (const char*) m_text_storage,
		p.x, p.y, m_text_font == NULL ? "(null)": (const char*) m_text_font);
	if (m_text_storage) {
	  	const lib::rect &r = m_dest->get_rect();
		lib::rect dstrect_whole = r;
		dstrect_whole.translate(m_dest->get_global_topleft());

		int	L = dstrect_whole.left(),
			T = dstrect_whole.top(),
			W = dstrect_whole.width(),
			H = dstrect_whole.height();
		ambulant_gtk_window* agtkw = (ambulant_gtk_window*) w;

		// initialize the pango context, layout...
		context = gdk_pango_context_get();
		language = gtk_get_default_language();
		pango_context_set_language (context, language);
		pango_context_set_base_dir (context, PANGO_DIRECTION_LTR);
		// We initialize the font as Sans 10
		font_desc = pango_font_description_from_string ("sans 10");
		// in case we have some specific font style and type
		if (m_text_font) {
			// TBD: smil font name/style to pango font name/style conversion
			pango_font_description_set_family(font_desc, m_text_font);
		}

		// in case we have some point size (taken from gtk_smiltext.cpp)
		if (m_text_size) {
			// smil font size to pango font size conversion
			double pango_font_size = m_text_size*PANGO_SCALE;
			pango_font_description_set_absolute_size(font_desc, pango_font_size);
		}

		pango_context_set_font_description (context, font_desc);
		layout = pango_layout_new(context);
		pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);

		// include the text
		pango_layout_set_text (layout, m_text_storage, -1);
		// according to the documentation, Pango sets the width in thousandths of a device unit (why? I don't know)
		pango_layout_set_width(layout, W*1000);
		// Foreground Color of the text
#if GTK_MAJOR_VERSION >= 3
		cairo_t* cr = cairo_create(agtkw->get_target_surface());
		cairo_set_source_rgb (cr, redf(m_text_color),greenf(m_text_color),bluef(m_text_color));
		cairo_move_to (cr, L, T);
		pango_cairo_show_layout (cr, layout);
		cairo_destroy(cr);
#else // GTK_MAJOR_VERSION < 3
		GdkColor gtk_color;
		gtk_color.red = redc(m_text_color)*0x101;
		gtk_color.blue = bluec(m_text_color)*0x101;
		gtk_color.green = greenc(m_text_color)*0x101;
		GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
		gdk_gc_set_rgb_fg_color (gc, &gtk_color);
		gdk_draw_layout(GDK_DRAWABLE (agtkw->get_ambulant_pixmap()),gc , L, T, layout);
		g_object_unref (G_OBJECT (gc));
#endif // GTK_MAJOR_VERSION < 3
		pango_font_description_free(font_desc);
		g_object_unref (G_OBJECT (context));
		g_object_unref(layout);
	}
}
