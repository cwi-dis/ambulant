// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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
#include "ambulant/gui/gtk/gtk_smiltext.h"
#include "ambulant/gui/gtk/gtk_util.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef WITH_SMIL30

namespace ambulant {

using namespace lib;

namespace gui {

namespace gtk {

gtk_smiltext_renderer::gtk_smiltext_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp)
:	gtk_renderer<renderer_playable>(context, cookie, node, evp),
	m_engine(smil2::smiltext_engine(node, evp, this, false)),
	m_params(m_engine.get_params()),
	m_pango_attr_list(pango_attr_list_new()),
	m_bg_layout(NULL),
	m_bg_pango_attr_list(NULL),
	m_pango_layout(NULL),
	m_pango_context(NULL),
	m_transparent(GTK_TRANSPARENT_COLOR),
	m_alternative(GTK_ALTERNATIVE_COLOR),
	m_alpha_media(1.0),
	m_alpha_media_bg(1.0),
	m_alpha_chroma(1.0),
	m_chroma_low(0x000000), //black
	m_chroma_high(0xFFFFFF) //white
{
#ifdef	TBD
	m_render_offscreen = (m_params.m_mode != smil2::stm_replace && m_params.m_mode != smil2::stm_append) || !m_params.m_wrap;
#endif//TBD
}

gtk_smiltext_renderer::~gtk_smiltext_renderer()
{
	m_lock.enter();
	if ( m_pango_attr_list != NULL) {
		pango_attr_list_unref( m_pango_attr_list);
		m_pango_attr_list = NULL;
	}
	g_object_unref (G_OBJECT (m_pango_context));
	g_object_unref(m_pango_layout);
	if ( m_bg_pango_attr_list != NULL) {
		pango_attr_list_unref(m_bg_pango_attr_list);
		m_bg_pango_attr_list = NULL;
	}
	if (m_bg_layout)
	  g_object_unref(m_bg_layout);
	m_lock.leave();
}

void
gtk_smiltext_renderer::start(double t)
{
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_engine.start(t);
	renderer_playable::start(t);
}

void
gtk_smiltext_renderer::seek(double t)
{
	m_engine.seek(t);
	//renderer_playable::seek(t);
}

void
gtk_smiltext_renderer::stop()
{
	m_engine.stop();
	renderer_playable::stop();
}

void
gtk_smiltext_renderer::marker_seen(const char *name)
{
	m_lock.enter();
	m_context->marker_seen(m_cookie, name);
	m_lock.leave();
}

void
gtk_smiltext_renderer::smiltext_changed()
{
	AM_DBG lib::logger::get_logger()->debug("gtk_smiltext_changed(0x%x)",this);
	m_lock.enter();
	if ( ! m_pango_context) {
		// initialize the pango context, layout...
		m_pango_context = gdk_pango_context_get();
		PangoLanguage* language = gtk_get_default_language();
		pango_context_set_language (m_pango_context, language);
		pango_context_set_base_dir (m_pango_context, PANGO_DIRECTION_LTR);
		const common::region_info *ri = m_dest->get_info();
		if (ri) {
			m_alpha_media = ri->get_mediaopacity();
			m_alpha_media_bg = ri->get_mediabgopacity();
			m_alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			compute_chroma_range(chromakey, chromakeytolerance,
					     &m_chroma_low, &m_chroma_high);
		}
#ifndef	WITH_GTK_ANTI_ALIASING
		if (m_alpha_media != 1.0 || m_alpha_media_bg != 1.0
		    || m_alpha_chroma != 1.0) {
			cairo_font_options_t* 
			  cairo_font_options = cairo_font_options_create();
			/* anti-aliasing by pango/cairo is disabled
			 * when blending is necessary, because this
			 * sometimes results in ugly glyphs.
			 */
			cairo_font_options_set_antialias (cairo_font_options,
							  CAIRO_ANTIALIAS_NONE);
			pango_cairo_context_set_font_options (m_pango_context,
							      cairo_font_options);
			cairo_font_options_destroy (cairo_font_options);
		}
#endif//WITH_GTK_ANTI_ALIASING
	}
	if ( ! m_pango_layout) {
		m_pango_layout = pango_layout_new (m_pango_context);
		pango_layout_set_alignment (m_pango_layout, PANGO_ALIGN_LEFT);
	}
	if ( ! m_bg_layout 
	     && (m_alpha_media != 1.0 
		 || m_alpha_media_bg != 1.0 
		 || m_alpha_chroma != 1.0)) {
		// prepare for blending: layout is setup twice:
		// m_bg_layout has textColor as m_transparent
		// m_pango_layout has textBackGroundColor as m_transparent
		// when blending, pixels in m_transparent are ignored
       		m_bg_layout = pango_layout_new(m_pango_context);
		pango_layout_set_alignment (m_bg_layout, PANGO_ALIGN_LEFT);
		m_bg_pango_attr_list = pango_attr_list_new();
	}
	if (m_engine.is_changed()) {
		lib::xml_string data;
		smil2::smiltext_runs::const_iterator i;
		if (m_engine.is_cleared()) {
			// Completely new text. Clear our copy and render everything.
	        	m_text_storage = "";
			i = m_engine.begin();
		} else {
			// Only additions. Don't clear and only copy the new stuff.
			i = m_engine.newbegin();
		}
		while (i != m_engine.end()) {
			// Add the new characters
			gint start_index = m_text_storage.size();
			if (i->m_command == smil2::stc_break)
				m_text_storage += "\n";
			else if (i->m_command == smil2::stc_data)
				m_text_storage +=  i->m_data;
			// Set font attributes
			_gtk_set_font_attr(m_pango_attr_list, 
					   i->m_font_family,
					   i->m_font_style, 
					   i->m_font_weight,
					   i->m_font_size,
					   start_index,
					   m_text_storage.size());
			if (m_bg_pango_attr_list) 
				_gtk_set_font_attr(m_bg_pango_attr_list, 
						   i->m_font_family,
						   i->m_font_style, 
						   i->m_font_weight,
						   i->m_font_size,
						   start_index,
						   m_text_storage.size());
			// text foreground/background color settings.
			if ( ! i->m_transparent) {
				// Set foreground color attribute
				color_t fg_color = i->m_color == m_transparent ?
					m_alternative : i->m_color;
				_gtk_set_color_attr(m_pango_attr_list,
						    fg_color,
						    pango_attr_foreground_new,
						    start_index, m_text_storage.size());
				if (m_bg_layout) {
					_gtk_set_color_attr(m_bg_pango_attr_list,
							    m_transparent,
							    pango_attr_foreground_new,
							    start_index,
							    m_text_storage.size());
				}
			}
			if ( ! i->m_bg_transparent) {
				// Set background color attribute
				// Select altenative color for m_transparent
				color_t bg_color = i->m_bg_color == m_transparent ?
					m_alternative : i->m_bg_color;
				_gtk_set_color_attr(m_pango_attr_list, 
						    m_bg_layout ?
						    	m_transparent : bg_color,
						    pango_attr_background_new,
						    start_index, m_text_storage.size());
				if (m_bg_layout) {
					_gtk_set_color_attr(m_bg_pango_attr_list, 
							    bg_color,
							    pango_attr_background_new,
							    start_index,
							    m_text_storage.size());
				}
			}
			// Set the attributes and text
			pango_layout_set_attributes(m_pango_layout, m_pango_attr_list);
			pango_layout_set_text(m_pango_layout, m_text_storage.c_str(), -1);
			pango_layout_context_changed(m_pango_layout);
			if (m_bg_layout) {
				pango_layout_set_attributes(m_bg_layout, m_bg_pango_attr_list);
				pango_layout_set_text(m_bg_layout, m_text_storage.c_str(), -1);
				pango_layout_context_changed(m_bg_layout);
			}
			i++;
		}
		m_engine.done();
	}
	bool finished = m_engine.is_finished();
	m_lock.leave();
	AM_DBG  lib::logger::get_logger()->debug("gtk_smiltext_changed(0x%x), m_text_storage=%s",this,m_text_storage.c_str());
	m_dest->need_redraw();
	if (finished)
		m_context->stopped(m_cookie);
}

void
gtk_smiltext_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("gtk_smiltext_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	// Compute the shifted position of what we want to draw w.r.t. the visible origin
	lib::point logical_origin(0, 0);
	if (m_params.m_mode == smil2::stm_crawl) {
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		logical_origin.x += (int) now * m_params.m_rate / 1000;
	}
	if (m_params.m_mode == smil2::stm_scroll) {
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		logical_origin.y += (int) now * m_params.m_rate / 1000;
	}
	AM_DBG logger::get_logger()->debug("gtk_smiltext_renderer.redraw: logical_origin(%d,%d)", logical_origin.x, logical_origin.y);

	_gtk_smiltext_render(r, logical_origin,(ambulant_gtk_window*)window);
	m_lock.leave();
}

// private methods
void
gtk_smiltext_renderer::_gtk_set_font_attr (PangoAttrList* pal,
		const char* smiltext_font_family,
		smil2::smiltext_font_style smiltext_font_style,
		smil2::smiltext_font_weight smiltext_font_weight,
		int smiltext_font_size,
		unsigned int start_index, unsigned int end_index)

{
	PangoFontDescription* pango_font_description = pango_font_description_new();

	// smiltext font style to pango font style conversion
	PangoStyle pango_font_style;
	switch (smiltext_font_style) {
	default:
	case smil2::sts_normal:
	case smil2::sts_reverse_oblique:	// Not supported by pango
		pango_font_style = PANGO_STYLE_NORMAL;
		break;
	case smil2::sts_italic:
		pango_font_style = PANGO_STYLE_ITALIC;
		break;
	case smil2::sts_oblique:
		pango_font_style = PANGO_STYLE_OBLIQUE;
		break;
	}
	pango_font_description_set_style(pango_font_description, pango_font_style);
	// smiltext font weight to pango font weight conversion
	PangoWeight pango_font_weight;
	switch (smiltext_font_weight) {
	default:
	case smil2::stw_normal:
		pango_font_weight = PANGO_WEIGHT_NORMAL;
		break;
	case smil2::stw_bold:
		pango_font_weight = PANGO_WEIGHT_BOLD;
		break;
	}
	pango_font_description_set_weight(pango_font_description, pango_font_weight);
	// smiltext font size to pango font size conversion
	double pango_font_size = smiltext_font_size*PANGO_SCALE;
	pango_font_description_set_absolute_size(pango_font_description, pango_font_size);
	// smiltext font family to pango font family conversion
	const char* pango_font_family = smiltext_font_family;
	pango_font_description_set_family(pango_font_description, pango_font_family);
	PangoAttribute* pango_attribute = pango_attr_font_desc_new(pango_font_description);
	// add the new PangoAttribute to the PangoAttrList for the range given
	pango_attribute->start_index = start_index;
	pango_attribute->end_index   = end_index;
	pango_attr_list_insert(pal, pango_attribute);
}

void
gtk_smiltext_renderer::_gtk_set_color_attr(PangoAttrList* pal,
		lib::color_t smiltext_color,
		PangoAttribute* (*pango_attr_color_new)(guint16 r, guint16 g, guint16 b),
		unsigned int start_index, unsigned int end_index)
{
	// smiltext color to pango color conversion
	guint16 pango_red   = redc(smiltext_color)*0x101;
	guint16 pango_green = greenc(smiltext_color)*0x101;
	guint16 pango_blue  = bluec(smiltext_color)*0x101;
	// Create new PangoAttribute containing the specified color
	PangoAttribute* pango_attribute = pango_attr_color_new(pango_red,
							       pango_green,
							       pango_blue);
	// add the new PangoAttribute to the PangoAttrList for the range given
	pango_attribute->start_index = start_index;
	pango_attribute->end_index   = end_index;
	pango_attr_list_insert(pal, pango_attribute);
}
		   
void
gtk_smiltext_renderer::_gtk_smiltext_render(const lib::rect r, const lib::point offset, ambulant_gtk_window* window )
{
	// Determine current position and size.
	const lib::point p = m_dest->get_global_topleft();
        const char* data = m_text_storage.c_str();

	AM_DBG lib::logger::get_logger()->debug(
		"gtk_smiltext_render(0x%x):"
		"ltrb=(%d,%d,%d,%d)\nm_text_storage = %s, p=(%d,%d):",
		(void *)this, r.left(), r.top(), r.width(), r.height(),
		data == NULL ? "(null)": data, p.x, p.y);
	if ( ! (m_pango_layout && window))
	  return; // nothing to do


	int L = r.left()+p.x, 
	    T = r.top()+p.y,
	    W = r.width(),
	    H = r.height();
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (window->get_ambulant_pixmap()));
	GdkRectangle gdk_rectangle;
	gdk_rectangle.x = L;
	gdk_rectangle.y = T;
	gdk_rectangle.width = W;
	gdk_rectangle.height = H;

	if (offset.x || offset.y) {
		gdk_gc_set_clip_rectangle(gc, &gdk_rectangle);
	}
	// include the text
	pango_layout_set_width(m_pango_layout, W*1000);
	if (m_bg_layout) {
		// blending
	  	pango_layout_set_width(m_bg_layout, W*1000);

		GdkPixmap* text_pixmap
		  = gdk_pixmap_new( (window->get_ambulant_pixmap()),
				    W, H, -1);
		GdkPixmap* bg_pixmap
		  = gdk_pixmap_new((window->get_ambulant_pixmap()),
				   W, H, -1);
		GdkGC* text_gc = gdk_gc_new (GDK_DRAWABLE (text_pixmap));
		GdkGC* bg_gc =  gdk_gc_new (GDK_DRAWABLE (bg_pixmap));
		if (offset.x || offset.y) {
			gdk_gc_set_clip_rectangle(text_gc, &gdk_rectangle);
			gdk_gc_set_clip_rectangle(bg_gc, &gdk_rectangle);
		}

		GdkPixbuf* screen_pixbuf = gdk_pixbuf_get_from_drawable
				(NULL, window->get_ambulant_pixmap(), NULL,
				 L, T, 0, 0, W, H);
		GdkColor gdk_transparent;
		gdk_transparent.red = redc(m_transparent)*0x101;
		gdk_transparent.blue = bluec(m_transparent)*0x101;
		gdk_transparent.green = greenc(m_transparent)*0x101;

		gdk_gc_set_rgb_bg_color (bg_gc, &gdk_transparent);
		gdk_gc_set_rgb_fg_color (bg_gc, &gdk_transparent);
		gdk_gc_set_rgb_bg_color (text_gc, &gdk_transparent);
		gdk_gc_set_rgb_fg_color (text_gc, &gdk_transparent);

		// clear text_pixmap and bg_pixmap to transparent color
		gdk_draw_rectangle (bg_pixmap, bg_gc, TRUE, 0, 0, W, H);
		gdk_draw_rectangle (text_pixmap, text_gc, TRUE, 0, 0, W, H);

		// draw m_bg_layout containing smilText runs with text in
		// m_transparent color and background in required color
		gdk_draw_layout(GDK_DRAWABLE (bg_pixmap),
				bg_gc , 0-offset.x, 0-offset.y, m_bg_layout);
		g_object_unref (G_OBJECT (bg_gc));
//		gdk_pixmap_dump(bg_pixmap, "bg");
		GdkPixbuf* bg_pixbuf = gdk_pixbuf_get_from_drawable
		  			       	(NULL, bg_pixmap, NULL,
						 0,0,0,0,W,H);
		lib::rect rc(lib::point(L,T),lib::size(W,H));
		// blend the screen pixbuf with th background pixbuf
		gdk_pixbuf_blend (screen_pixbuf, rc, bg_pixbuf, rc, 
				  m_alpha_chroma, m_alpha_media_bg,
				  m_chroma_low, m_chroma_high, m_transparent);

		// draw m_pango_layout containing smilText runs with text in
		// required colors and background in m_transparant color
		gdk_draw_layout(GDK_DRAWABLE (text_pixmap),
				text_gc , 0-offset.x, 0-offset.y, m_pango_layout);
		g_object_unref (G_OBJECT (text_gc));
//		gdk_pixmap_dump(text_pixmap, "text");
		GdkPixbuf* text_pixbuf = gdk_pixbuf_get_from_drawable
		  			       	(NULL, text_pixmap, NULL,
						0,0,0,0,W,H);
		gdk_pixbuf_blend (screen_pixbuf, rc, text_pixbuf, rc, 
				  m_alpha_chroma, m_alpha_media,
				  m_chroma_low, m_chroma_high, m_transparent);
//		gdk_pixmap_dump( window->get_ambulant_pixmap(), "screen0");
		// draw the blended pixbuf on the screen
		gdk_draw_pixbuf(window->get_ambulant_pixmap(),
				gc, screen_pixbuf, 0, 0, L, T, W, H, GDK_RGB_DITHER_NONE,0,0);
//		gdk_pixmap_dump( window->get_ambulant_pixmap(), "screen1");
		g_object_unref (G_OBJECT (text_pixbuf));
		g_object_unref (G_OBJECT (bg_pixbuf));
		g_object_unref (G_OBJECT (screen_pixbuf));
		g_object_unref (G_OBJECT (bg_pixmap));
		g_object_unref (G_OBJECT (text_pixmap));
	} else {
		gdk_draw_layout(GDK_DRAWABLE (window->get_ambulant_pixmap()),
				gc , L-offset.x, T-offset.y, m_pango_layout);
	}
	g_object_unref (G_OBJECT (gc));
}

} // namespace gtk

} // namespace gui

} //namespace ambulant

#endif // WITH_SMIL30
