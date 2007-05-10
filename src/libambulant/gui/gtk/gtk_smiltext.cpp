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

#define AM_DBG if(1)
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

#ifdef	JUNK
static NSFont *
_select_font(const char *family, smil2::smiltext_font_style style, smil2::smiltext_font_weight weight, int size)
{
	NSFont *font = [NSFont userFontOfSize: (float)size];
	NSFontTraitMask mask;
	NSFontManager *fm = [NSFontManager sharedFontManager];
	
	if (strcmp(family, "serif") == 0) 
		font = [fm convertFont: font toFamily: @"Times"];
	if (strcmp(family, "monospace") == 0) mask |= NSFixedPitchFontMask;
	switch(style) {
	case smil2::sts_normal:
	case smil2::sts_reverse_oblique: // Not supported
		mask |= NSUnitalicFontMask;
		break;
	case smil2::sts_italic:
	case smil2::sts_oblique:
		mask |= NSItalicFontMask;
		break;
	}
	switch(weight) {
	case smil2::stw_normal:
		mask |= NSUnboldFontMask;
		break;
	case smil2::stw_bold:
		mask |= NSBoldFontMask;
		break;
	}
	
	font = [fm convertFont: font toHaveTrait: mask];
	return font;
}
#endif//JUNK

gtk_smiltext_renderer::gtk_smiltext_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp)
:	gtk_renderer<renderer_playable>(context, cookie, node, evp),
//JUNK	m_gtk_window(NULL),
	m_engine(smil2::smiltext_engine(node, evp, this, false)),
	m_params(m_engine.get_params()),
	m_attr(pango_attr_list_new())
{
#ifdef	JUNK
	m_render_offscreen = (m_params.m_mode != smil2::stm_replace && m_params.m_mode != smil2::stm_append) || !m_params.m_wrap;
#endif//JUNK
}

gtk_smiltext_renderer::~gtk_smiltext_renderer()
{
	m_lock.enter();
	if (m_attr != NULL) {
		pango_attr_list_unref(m_attr);
		m_attr = NULL;
	}
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
			// Prepare for setting the attribute info
			// Find font info
#ifdef	JUNK
//JUNK			NSFont *text_font = _select_font((*i).m_font_family, (*i).m_font_style, (*i).m_font_weight, (*i).m_font_size);
			if (text_font)
				[attrs setValue:text_font forKey:NSFontAttributeName];
#endif//JUNK
			if ( ! i->m_transparent) {
				// Find color info
				guint16 red   = redc(i->m_color)*0x101;
				guint16 green = greenc(i->m_color)*0x101;
				guint16 blue  = bluec(i->m_color)*0x101;
				PangoAttribute* pa = pango_attr_foreground_new(red, green, blue);
				pa->start_index = start_index;
				pa->end_index   = m_text_storage.size();
				pango_attr_list_insert(m_attr, pa);
//TBD				pango_attribute_destroy(pa);
			}
			if ( ! i->m_bg_transparent) {
				// Find background color info
				guint16 red   = redc(i->m_bg_color)*0x101;
				guint16 green = greenc(i->m_bg_color)*0x101;
				guint16 blue  = bluec(i->m_bg_color)*0x101;
				PangoAttribute* pa = pango_attr_background_new(red, green, blue);
				pa->start_index = start_index;
				pa->end_index   = m_text_storage.size();
				pango_attr_list_insert(m_attr, pa);
//TBD				pango_attribute_destroy(pa);
#ifdef	JUNK
				NSColor *color = [NSColor colorWithCalibratedRed:redf((*i).m_bg_color)
						green:greenf((*i).m_bg_color)
						blue:bluef((*i).m_bg_color)
						alpha:1.0];
				[attrs setValue:color forKey:NSBackgroundColorAttributeName];
#endif//JUNK
			}
			// Set the attributes
			i++;
		}
		m_engine.done();
	}

	if (m_engine.is_changed()) {
		// Always re-compute and re-render everything when new text is added.
		// Thus, m_engine.newbegin() is NOT used
		smil2::smiltext_runs::const_iterator cur = m_engine.begin();
		int m_x = m_dest->get_rect().x; //KB
		int m_y = m_dest->get_rect().y; //KB
		while (cur != m_engine.end()) {
			smil2::smiltext_runs::const_iterator bol = cur;
			AM_DBG lib::logger::get_logger()->debug("gtk_smiltext_changed(): command=%d data=%s",cur->m_command,cur->m_data.c_str()==NULL?"(null)":cur->m_data.c_str());
			while (cur != m_engine.end()) {
				// layout line-by-line
				if (cur->m_command == smil2::stc_break 
				    ) {
//KB					|| ! gtk_smiltext_fits(*cur, m_dest->get_rect())) {
					if (cur->m_command == smil2::stc_break)
						cur++;
					break;
				}
				cur++;
			}
			m_x = m_dest->get_rect().x; // was used by gtk_smiltext_fits()
			while (bol != cur) {
//KB				lib::rect r = gtk_smiltext_compute(*bol, m_dest->get_rect());
//KB				gtk_smiltext_render(*bol, m_dest->get_rect());
				bol++;
			}
//KB			m_y = m_y + m_max_ascent + m_max_descent;
			m_x = m_dest->get_rect().x;
//KB			m_max_ascent = m_max_descent = 0;
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
	gtk_smiltext_render(r, (ambulant_gtk_window*)window);
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
gtk_smiltext_renderer::gtk_smiltext_render(const lib::rect r, ambulant_gtk_window* window )
{
	PangoContext *context;
  	PangoLanguage *language;
  	PangoFontDescription *font_desc;
  	PangoLayout *layout;

	const lib::point p = m_dest->get_global_topleft();
	//XXX need to be fixed in renderer_playable_dsl
#ifdef	JUNK
	if (m_data && !m_text_storage) {
		m_text_storage = (char*) malloc(m_data_size+1);
		strncpy(m_text_storage,
			(const char*) m_data,
			m_data_size);
		m_text_storage[m_data_size] = '\0';
	}
#endif//JUNK
        const char* data = m_text_storage.c_str();	//KB
	const char* font = NULL;		//KB
	AM_DBG lib::logger::get_logger()->debug(
		"gtk_smiltext_render(0x%x):"
		"ltrb=(%d,%d,%d,%d)\nm_text_storage = %s, p=(%d,%d):"
		"font-family=(%s)",
		(void *)this, r.left(), r.top(), r.width(), r.height(),
		data == NULL ? "(null)": data,
		p.x, p.y, font == NULL ? "(null)": font);
	if (data && window) {
		int L = r.left()+p.x, 
		    T = r.top()+p.y,
		    W = r.width(),
		    H = r.height();
		
		// initialize the pango context, layout...
		context = gdk_pango_context_get();
 	 	language = gtk_get_default_language();
  		pango_context_set_language (context, language);
  		pango_context_set_base_dir (context, PANGO_DIRECTION_LTR);
		// We initialize the font as Sans 12
		font_desc = pango_font_description_from_string ("sans 10");
  		pango_context_set_font_description (context, font_desc);
		layout = pango_layout_new(context);
  		pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);

		// include the text
  		pango_layout_set_text (layout, data, -1);
		// according to the documentation, Pango sets the width in thousandths of a device unit (why? I don't know)
		pango_layout_set_width(layout, W*1000);

		// in case we have some specific font style and type
		if (font){
			printf("We are entering to some bad place\n");
			PangoFontDescription* pfd = pango_font_description_new();
			pango_font_description_set_family(pfd, font);
			pango_layout_set_font_description(layout, pfd);
			pango_font_description_free(pfd);
		}

		// in case we have some point size (it is not done yet for Gtk)
/*		if (m_text_size)
			gtk_font.setPointSizeFloat(m_text_size);
*/
#ifdef	JUNK
		// Foreground Color of the text
		GdkColor gtk_color;
		unsigned int m_text_color=0xFF0000; //KB
		gtk_color.red = redc(m_text_color)*0x101;
		gtk_color.blue = bluec(m_text_color)*0x101;
		gtk_color.green = greenc(m_text_color)*0x101;
		GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (window->get_ambulant_pixmap()));
		gdk_gc_set_rgb_fg_color (gc, &gtk_color);
#else //JUNK
		GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (window->get_ambulant_pixmap()));
#endif//JUNK
		pango_layout_set_attributes(layout, m_attr);
  		gdk_draw_layout(GDK_DRAWABLE (window->get_ambulant_pixmap()),gc , L, T, layout);
		pango_font_description_free(font_desc);
		g_object_unref (G_OBJECT (context));
		g_object_unref(layout);
		g_object_unref (G_OBJECT (gc));
	}
}

} // namespace gtk

} // namespace gui

} //namespace ambulant

#endif // WITH_SMIL30
