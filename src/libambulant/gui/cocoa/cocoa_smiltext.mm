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

#include "ambulant/gui/cocoa/cocoa_smiltext.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"

#include <Cocoa/Cocoa.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef WITH_SMIL30

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

static NSFont *
_select_font(const char *family, smil2::smiltext_font_style style, smil2::smiltext_font_weight weight, int size)
{
	NSFont *font = NULL;
	NSFontTraitMask mask = 0;
	NSFontManager *fm = [NSFontManager sharedFontManager];
	NSString *ffname = NULL;
	
	if (strcmp(family, "serif") == 0) {
		ffname = [NSString stringWithCString: "Times"];
	} else if (strcmp(family, "monospace") == 0) {
		ffname = [NSString stringWithCString: "Monaco"];
	} else if (strcmp(family, "sansSerif") == 0) {
		ffname = [NSString stringWithCString: "Helvetica Neue"];
	} else {
		ffname = [NSString stringWithCString:family];
	}
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
	font = [fm fontWithFamily: ffname traits: mask weight: 5 size: size];
//	[ffname release];
	if (font) return font;
	lib::logger::get_logger()->trace("smilText: Could not find exact font for family %s, trying without", family);
	font = [fm fontWithFamily: NULL traits: mask weight: 5 size: size];
	if (font) return font;
	lib::logger::get_logger()->trace("smilText: Could not find font without family either, converting userFont");
	font = [NSFont userFontOfSize: (float)size];		
	font = [fm convertFont: font toHaveTrait: mask];
	return font;
}

cocoa_smiltext_renderer::cocoa_smiltext_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp)
:	cocoa_renderer<renderer_playable>(context, cookie, node, evp),
	m_text_storage(NULL),
	m_layout_manager(NULL),
	m_text_container(NULL),
	m_engine(smil2::smiltext_engine(node, evp, this, false)),
	m_needs_conditional_newline(false),
	m_needs_conditional_space(false),
	m_params(m_engine.get_params()),
	m_cur_paragraph_style(NULL),
	m_cur_para_align(smil2::sta_start),
	m_cur_para_writing_mode(smil2::stw_lr_tb),
	m_cur_para_wrap(true),
	m_any_semiopaque_bg(false)
{
	m_text_storage = [[NSTextStorage alloc] initWithString:@""];
	m_render_offscreen = (m_params.m_mode != smil2::stm_replace && m_params.m_mode != smil2::stm_append);
}

cocoa_smiltext_renderer::~cocoa_smiltext_renderer()
{
	m_lock.enter();
	[m_text_storage release];
	m_text_storage = NULL;
	m_lock.leave();
}

void
cocoa_smiltext_renderer::start(double t)
{
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_engine.start(t);
	renderer_playable::start(t);
	m_context->started(m_cookie);
}

void
cocoa_smiltext_renderer::seek(double t)
{
	m_engine.seek(t);
	//renderer_playable::seek(t);
}

void
cocoa_smiltext_renderer::stop()
{
	m_engine.stop();
	renderer_playable::stop();
	m_context->stopped(m_cookie);
}

void
cocoa_smiltext_renderer::marker_seen(const char *name)
{
	m_lock.enter();
	m_context->marker_seen(m_cookie, name);
	m_lock.leave();
}

void
cocoa_smiltext_renderer::smiltext_changed()
{
	m_lock.enter();
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	assert(m_text_storage);
	if (m_engine.is_changed()) {
		lib::xml_string data;
		smil2::smiltext_runs::const_iterator i;
		[m_text_storage beginEditing];
		if (m_engine.is_cleared()) {
			// Completely new text. Clear our copy and render everything.
			m_needs_conditional_space = false;
			m_needs_conditional_newline = false;
			NSRange all;
			all.location = 0;
			all.length = [m_text_storage length];
			if (all.length);
				[m_text_storage deleteCharactersInRange:all];
			i = m_engine.begin();
		} else {
			// Only additions. Don't clear and only render the new stuff.
			i = m_engine.newbegin();
		}
		while (i != m_engine.end()) {
			AM_DBG lib::logger::get_logger()->debug("cocoa_smiltext: another run");
			NSRange newrange;
			// Add the new characters
			newrange.location = [m_text_storage length];
			newrange.length = 0;
			NSString *newdata = @"";
			switch((*i).m_command) {
			case smil2::stc_break:
				newdata = @"\n\n";
				m_needs_conditional_space = false;
				m_needs_conditional_newline = false;
				break;
			case smil2::stc_condbreak:
				if (m_needs_conditional_newline) {
					newdata = @"\n";
					m_needs_conditional_space = false;
					m_needs_conditional_newline = false;
				}
				break;
			case smil2::stc_condspace:
				if (m_needs_conditional_space) {
					newdata = @" ";
					m_needs_conditional_newline = true;
					m_needs_conditional_space = false;
				}
				break;
			case smil2::stc_data:
				char lastch = *((*i).m_data.rbegin());
				if (lastch == '\r' || lastch == '\n' || lastch == '\f' || lastch == '\v') {
					m_needs_conditional_newline = false;
					m_needs_conditional_space = false;
				} else
				if (lastch == ' ' || lastch == '\t') {
					m_needs_conditional_newline = true;
					m_needs_conditional_space = false;
				} else {
					m_needs_conditional_newline = true;
					m_needs_conditional_space = true;
				}
				newdata = [[NSString alloc] initWithCString:(*i).m_data.c_str()];
				break;
			default:
				assert(0);
			}
			[m_text_storage replaceCharactersInRange:newrange withString:newdata];
			
			// Prepare for setting the attribute info
			NSMutableDictionary *attrs = [[NSMutableDictionary alloc] init];
			newrange.length = (*i).m_data.length();
			// Find font info
			NSFont *text_font = _select_font((*i).m_font_family, (*i).m_font_style, (*i).m_font_weight, (*i).m_font_size);
			if (text_font)
				[attrs setValue:text_font forKey:NSFontAttributeName];
				
			if (!(*i).m_transparent) {
				// Find color info
				double alfa = 1.0;
				const common::region_info *ri = m_dest->get_info();
				if (ri) alfa = ri->get_mediaopacity();
				NSColor *color = [NSColor colorWithCalibratedRed:redf((*i).m_color)
						green:greenf((*i).m_color)
						blue:bluef((*i).m_color)
						alpha:alfa];
				[attrs setValue:color forKey:NSForegroundColorAttributeName];
			}
			if (!(*i).m_bg_transparent) {
				// Find background color info
				double alfa = 1.0;
				const common::region_info *ri = m_dest->get_info();
				if (ri) alfa = ri->get_mediabgopacity();
				if (alfa != 1.0)
					m_any_semiopaque_bg = true;
				NSColor *color = [NSColor colorWithCalibratedRed:redf((*i).m_bg_color)
						green:greenf((*i).m_bg_color)
						blue:bluef((*i).m_bg_color)
						alpha:alfa];
				[attrs setValue:color forKey:NSBackgroundColorAttributeName];
			}
			// Finally do paragraph settings (which are cached)
			if (m_cur_paragraph_style == NULL ||
					m_cur_para_align != (*i).m_align ||
					m_cur_para_writing_mode != (*i).m_writing_mode ||
					m_cur_para_wrap != (*i).m_wrap) {
				// Delete the old one, if needed
				if (m_cur_paragraph_style)
					[m_cur_paragraph_style release];
				// Remember the values
				m_cur_para_align = (*i).m_align;
				m_cur_para_writing_mode = (*i).m_writing_mode;
				m_cur_para_wrap = (*i).m_wrap;
				// Allocate the new one
				NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
				m_cur_paragraph_style = [ps retain];
				// Set the paragraph writing direction
				if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
					[ps setBaseWritingDirection: NSWritingDirectionRightToLeft];
				} else {
					// All other directions are treated as left-to-right
					[ps setBaseWritingDirection: NSWritingDirectionLeftToRight];
				}
				// Set the paragraph text alignment
				switch (m_cur_para_align) {
				case smil2::sta_start:
					if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
						[ps setAlignment: NSRightTextAlignment];
					} else {
						// All other directions are treated as left-to-right
						[ps setAlignment: NSLeftTextAlignment];
					}
					break;
				case smil2::sta_end:
					if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
						[ps setAlignment: NSLeftTextAlignment];
					} else {
						// All other directions are treated as left-to-right
						[ps setAlignment: NSRightTextAlignment];
					}
					break;
				case smil2::sta_left:
					[ps setAlignment: NSLeftTextAlignment];
					break;
				case smil2::sta_right:
					[ps setAlignment: NSRightTextAlignment];
					break;
				case smil2::sta_center:
					[ps setAlignment: NSCenterTextAlignment];
					break;
				}
				// Set the paragraph wrap option
				if (m_cur_para_wrap)
					[ps setLineBreakMode: NSLineBreakByWordWrapping];
				else
					[ps setLineBreakMode: NSLineBreakByClipping];
			}
			[attrs setValue:m_cur_paragraph_style forKey:NSParagraphStyleAttributeName];

			// Set the attributes
			[m_text_storage setAttributes:attrs range:newrange];
			
			i++;
		}
		[m_text_storage endEditing];
		m_engine.done();
	}
	bool finished = m_engine.is_finished();
	[pool release];
	m_lock.leave();
	m_dest->need_redraw();
	if (finished)
		m_context->stopped(m_cookie);
}

void
cocoa_smiltext_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_smiltext_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	// Determine current position and size.
	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	rect dstrect = r;
	dstrect.translate(m_dest->get_global_topleft());
	NSRect cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];
	NSPoint visible_origin = NSMakePoint(NSMinX(cocoa_dstrect), NSMinY(cocoa_dstrect));
	NSSize visible_size = NSMakeSize(NSWidth(cocoa_dstrect), NSHeight(cocoa_dstrect));

// Determine text container layout size. This depends on the type of container.
#define INFINITE_WIDTH 1000000
#define INFINITE_HEIGHT 1000000
	NSSize layout_size = visible_size;
	switch(m_params.m_mode) {
#if 0
	// Handled through paragraph wrapping now
	case smil2::stm_replace:
	case smil2::stm_append:
		if (!m_params.m_wrap)
			layout_size.width = INFINITE_WIDTH;
		break;
#endif
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
	AM_DBG logger::get_logger()->debug("cocoa_smiltext_renderer.redraw at Cocoa-point (%f, %f)", visible_origin.x, visible_origin.y);
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
		if (m_any_semiopaque_bg) {
			// Background opacity 1.0 is implemented correctly in NSLayoutManager, but intermediate
			// values are a bit funny: they still override the underlying image (i.e. they don't
			// use NSCompositeSourceOver or something similar. Therefore, if this is the case we
			// draw the background color to a separate buffer and bitblit this onto the existing
			// bits.
			NSImage *tmpsrc = [view getTransitionTmpSurface];
			[tmpsrc lockFocus];
			[[NSColor colorWithDeviceWhite: 1.0 alpha: 0.0] set];
			NSRectFill(cocoa_dstrect);
			[m_layout_manager drawBackgroundForGlyphRange: glyph_range atPoint: visible_origin];
			[tmpsrc unlockFocus];
			[tmpsrc drawInRect: cocoa_dstrect fromRect: cocoa_dstrect operation: NSCompositeSourceOver fraction: 1.0];
		} else {
			// Otherwise we simply let NSLayoutManager do the work
			[m_layout_manager drawBackgroundForGlyphRange: glyph_range atPoint: visible_origin];
		}
		[m_layout_manager drawGlyphsForGlyphRange: glyph_range atPoint: visible_origin];
	}
	layout_size = [m_text_container containerSize];
	if (m_render_offscreen) {
	}
	m_lock.leave();
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant

#endif // WITH_SMIL30
