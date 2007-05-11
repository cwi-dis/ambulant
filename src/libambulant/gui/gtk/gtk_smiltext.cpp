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
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef	JUNK
#endif//JUNK

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
//JUNK	m_gtk_window(NULL),
	m_engine(smil2::smiltext_engine(node, evp, this, false)),
	m_params(m_engine.get_params()),
	 m_pango_attr_list(pango_attr_list_new())
{
#ifdef	TBD
	m_render_offscreen = (m_params.m_mode != smil2::stm_replace && m_params.m_mode != smil2::stm_append) || !m_params.m_wrap;
#endif//TBD
	// initialize the pango context, layout...
	m_context = gdk_pango_context_get();
  	PangoLanguage* language = gtk_get_default_language();
	pango_context_set_language (m_context, language);
	pango_context_set_base_dir (m_context, PANGO_DIRECTION_LTR);
	m_layout = pango_layout_new(m_context);
  	pango_layout_set_alignment (m_layout, PANGO_ALIGN_LEFT);
}

gtk_smiltext_renderer::~gtk_smiltext_renderer()
{
	m_lock.enter();
	if ( m_pango_attr_list != NULL) {
		pango_attr_list_unref( m_pango_attr_list);
		 m_pango_attr_list = NULL;
	}
	g_object_unref (G_OBJECT (m_context));
	g_object_unref(m_layout);
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
gtk_smiltext_renderer::smiltext_changed()
{
	AM_DBG lib::logger::get_logger()->debug("gtk_smiltext_changed(0x%x)",this);
	m_lock.enter();
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
			_gtk_set_font_attr(i->m_font_family,
					   i->m_font_style, 
					   i->m_font_weight,
					   i->m_font_size,
					   start_index, m_text_storage.size());
			if ( ! i->m_transparent) {
				// Set foreground color attribute
				 _gtk_set_color_attr(i->m_color,
						     pango_attr_foreground_new,
						     start_index, m_text_storage.size());
			}
			if ( ! i->m_bg_transparent) {
				// Set background color attribute
				_gtk_set_color_attr(i->m_bg_color,
						    pango_attr_background_new,
						    start_index, m_text_storage.size());
			}
			// Set the attributes and text
			pango_layout_set_attributes(m_layout, m_pango_attr_list);
			pango_layout_set_text(m_layout, m_text_storage.c_str(), -1);
			pango_layout_context_changed(m_layout);
			i++;
		}
		m_engine.done();
	}
	m_lock.leave();
	AM_DBG  lib::logger::get_logger()->debug("gtk_smiltext_changed(0x%x), m_text_storage=%s",this,m_text_storage.c_str());
	m_dest->need_redraw();
}

void
gtk_smiltext_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("gtk_smiltext_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	_gtk_smiltext_render(r, (ambulant_gtk_window*)window);
#ifdef	JUNK
	// Determine current position and size.
	gtk_window *cwindow = (gtk_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	rect dstrect = r;
	dstrect.translate(m_dest->get_global_topleft());
	NSRect gtk_dstrect = [view NSRectForAmbulantRect: &dstrect];
	NSPoint visible_origin = NSMakePoint(NSMinX(gtk_dstrect), NSMinY(gtk_dstrect));
	NSSize visible_size = NSMakeSize(NSWidth(gtk_dstrect), NSHeight(gtk_dstrect));

// Determine text container layout size. This depends on the type of container.
#define INFINITE_WIDTH 1000000
#define INFINITE_HEIGHT 1000000
	NSSize layout_size = visible_size;
	switch(m_params.m_mode) {
	case smil2::stm_replace:
	case smil2::stm_append:
		if (!m_params.m_wrap)
			layout_size.width = INFINITE_WIDTH;
		break;
	case smil2::stm_scroll:
	case smil2::stm_jump:
		layout_size.height = INFINITE_HEIGHT;
		break;
	case smil2::stm_crawl:
		layout_size.width = INFINITE_WIDTH;
		break;
	}

	NSSize old_layout_size;
	// Initialize the text engine if we have not already done so.
	if (!m_layout_manager) {
		// Initialize the text engine
		m_layout_manager = [[NSLayoutManager alloc] init];
		m_text_container = [[NSTextContainer alloc] initWithContainerSize: layout_size];
		old_layout_size = layout_size;	// Force resize
		[m_text_container setHeightTracksTextView: false];
		[m_text_container setWidthTracksTextView: false];
		[m_layout_manager addTextContainer:m_text_container];
		[m_text_container release];	// The layoutManager will retain the textContainer
		[m_text_storage addLayoutManager:m_layout_manager];
		[m_layout_manager release];	// The textStorage will retain the layoutManager
	} else {
		old_layout_size = [m_text_container containerSize];
	}
	assert(m_layout_manager);
	assert(m_text_container);
	assert(m_text_storage);
	
	// If the layout size has changed (due to smil animation or so) change it
	if (!NSEqualSizes(old_layout_size, layout_size)) {
		[m_text_container setContainerSize: layout_size];
	}

	// Next compute the layout position of what we want to draw at visible_origin
	NSPoint logical_origin = NSMakePoint(0, 0);
	if (m_params.m_mode == smil2::stm_crawl) {
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		logical_origin.x += now * m_params.m_rate / 1000;
		visible_origin.x -= now * m_params.m_rate / 1000;
	}
	if (m_params.m_mode == smil2::stm_scroll) {
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		logical_origin.y += now * m_params.m_rate / 1000;
		visible_origin.y -= now * m_params.m_rate / 1000;
	}
	AM_DBG logger::get_logger()->debug("gtk_smiltext_renderer.redraw at Gtk-point (%f, %f)", visible_origin.x, visible_origin.y);
	if (m_render_offscreen) {
	}
#if 0
	NSRange glyph_range = [m_layout_manager glyphRangeForTextContainer: m_text_container];
#else
	// Now we need to determine which glyphs to draw. Unfortunately glyphRangeForBoundingRect gives us
	// full lines (which is apparently more efficient, google for details) which is not good enough
	// for ticker tape, so we adjust.
	NSRect logical_rect = NSMakeRect(logical_origin.x, logical_origin.y, visible_size.width, visible_size.height);
	NSRange glyph_range = [m_layout_manager glyphRangeForBoundingRect: logical_rect inTextContainer: m_text_container];
	AM_DBG NSLog(@"Glyph range was %d, %d, origin-x %f", glyph_range.location, glyph_range.length, logical_origin.x);
#if 0
	float fraction;
	unsigned leftpoint = [ m_layout_manager glyphIndexForPoint: logical_origin inTextContainer: m_text_container 
		fractionOfDistanceThroughGlyph: &fraction];
	if (fraction > 0.01) leftpoint++;
	if (leftpoint > glyph_range.location) {
		glyph_range.length += (glyph_range.location-leftpoint); // XXXX unsigned
		glyph_range.location = leftpoint;
	}
	if (glyph_range.location > 0 && glyph_range.length > 0) {
		NSRect used_rect = [m_layout_manager boundingRectForGlyphRange: glyph_range inTextContainer: m_text_container];
		visible_origin.x -= used_rect.origin.x;
	}
	AM_DBG NSLog(@"Glyph range is now %d, %d", glyph_range.location, glyph_range.length);
#endif
#endif
	if (glyph_range.location >= 0 && glyph_range.length > 0) {
		[m_layout_manager drawBackgroundForGlyphRange: glyph_range atPoint: visible_origin];
		[m_layout_manager drawGlyphsForGlyphRange: glyph_range atPoint: visible_origin];
	}
	layout_size = [m_text_container containerSize];
	if (m_render_offscreen) {
	}
#endif//JUNK
	m_lock.leave();
}

// private methods
void
gtk_smiltext_renderer::_gtk_set_font_attr(const char* smiltext_font_family,
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
	pango_attr_list_insert(m_pango_attr_list, pango_attribute);
}

void
gtk_smiltext_renderer::_gtk_set_color_attr(lib::color_t smiltext_color,
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
	pango_attr_list_insert(m_pango_attr_list, pango_attribute);
}
		   
void
gtk_smiltext_renderer::_gtk_smiltext_render(const lib::rect r, ambulant_gtk_window* window )
{
	const lib::point p = m_dest->get_global_topleft();
        const char* data = m_text_storage.c_str();

	AM_DBG lib::logger::get_logger()->debug(
		"gtk_smiltext_render(0x%x):"
		"ltrb=(%d,%d,%d,%d)\nm_text_storage = %s, p=(%d,%d):",
		(void *)this, r.left(), r.top(), r.width(), r.height(),
		data == NULL ? "(null)": data, p.x, p.y);
	if (data && window) {
		int L = r.left()+p.x, 
		    T = r.top()+p.y,
		    W = r.width(),
		    H = r.height();
		GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (window->get_ambulant_pixmap()));
		// include the text
		pango_layout_set_width(m_layout, W*1000);
  		gdk_draw_layout(GDK_DRAWABLE (window->get_ambulant_pixmap()),
				gc , L, T, m_layout);
		g_object_unref (G_OBJECT (gc));
	}
}

} // namespace gtk

} // namespace gui

} //namespace ambulant

#endif // WITH_SMIL30
