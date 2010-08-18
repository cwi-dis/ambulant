// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2010 Stichting CWI,
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/*
 * @$Id$
 */

#include "ambulant/gui/cg/cg_smiltext.h"
#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"
//XXXX #include <cg/cg.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef WITH_SMIL30

namespace ambulant {

using namespace lib;

namespace gui {

namespace cg {

static CTFontRef
_select_font(const char *family, smil2::smiltext_font_style style, smil2::smiltext_font_weight weight, int size)
{
	CTFontRef font = NULL;
	CTFontSymbolicTraits mask = 0;
//	NSFontManager *fm = [NSFontManager sharedFontManager];
	NSString *ffname = NULL;

	if (strcmp(family, "serif") == 0) {
		ffname = [NSString stringWithUTF8String: "Times"];
	} else if (strcmp(family, "monospace") == 0) {
		ffname = [NSString stringWithUTF8String: "Monaco"];
	} else if (strcmp(family, "sansSerif") == 0) {
		ffname = [NSString stringWithUTF8String: "Helvetica Neue"];
	} else {
		ffname = [NSString stringWithUTF8String:family];
	}
	if (strcmp(family, "monospace") == 0) mask |= kCTFontCondensedTrait;
	switch(style) {
	case smil2::sts_normal:
	case smil2::sts_reverse_oblique: // Not supported
		mask &= ~kCTFontItalicTrait;
		break;
	case smil2::sts_italic:
	case smil2::sts_oblique:
		mask |= kCTFontItalicTrait;
		break;
	}
	switch(weight) {
	case smil2::stw_normal:
		mask &= ~kCTFontBoldTrait;
		break;
	case smil2::stw_bold:
		mask |= kCTFontBoldTrait;
		break;
	}
//	font = [fm fontWithFamily: ffname traits: mask weight: 5 size: size];
	return font;
}

extern const char cg_smiltext_playable_tag[] = "smilText";
extern const char cg_smiltext_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("Renderercg");
extern const char cg_smiltext_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererSmilText");

common::playable_factory *
create_cg_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("Renderercg"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSmilText"), true);
	return new common::single_playable_factory<
		cg_smiltext_renderer,
		cg_smiltext_playable_tag,
		cg_smiltext_playable_renderer_uri,
		cg_smiltext_playable_renderer_uri2,
		cg_smiltext_playable_renderer_uri2>(factory, mdp);
}

cg_smiltext_renderer::cg_smiltext_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *fp,
	common::playable_factory_machdep *mdp)
:	cg_renderer<renderer_playable>(context, cookie, node, evp, fp, mdp),
	m_text_storage(NULL),
	m_frame(NULL),
	m_framesetter(NULL),
	m_path(NULL),
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
	m_text_storage = CFAttributedStringCreateMutable(NULL, 0);
	m_render_offscreen = (m_params.m_mode != smil2::stm_replace && m_params.m_mode != smil2::stm_append);
}

cg_smiltext_renderer::~cg_smiltext_renderer()
{
	m_lock.enter();
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	CFRelease(m_text_storage);
	m_text_storage = NULL;
	CFRelease(m_framesetter);
	m_framesetter = NULL;
	CFRelease(m_frame);
	m_frame = NULL;
	CFRelease(m_path);
	m_path = NULL;
	m_lock.leave();
}

void
cg_smiltext_renderer::start(double t)
{
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_context->started(m_cookie);
	m_engine.start(t);
	renderer_playable::start(t);
}

void
cg_smiltext_renderer::seek(double t)
{
	assert( t >= 0);
	m_engine.seek(t);
	//renderer_playable::seek(t);
}

bool
cg_smiltext_renderer::stop()
{
	m_engine.stop();
	renderer_playable::stop();
	m_context->stopped(m_cookie);
	return true; // Don't re-use this renderer
}
void
cg_smiltext_renderer::pause(pause_display d)
{
}

void
cg_smiltext_renderer::resume()
{
}

void
cg_smiltext_renderer::marker_seen(const char *name)
{
	m_lock.enter();
	m_context->marker_seen(m_cookie, name);
	m_lock.leave();
}

void
cg_smiltext_renderer::smiltext_changed()
{
	m_lock.enter();
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	assert(m_text_storage);
	m_engine.lock();
	if (m_engine.is_changed()) {
		lib::xml_string data;
		smil2::smiltext_runs::const_iterator i;
		CFAttributedStringBeginEditing(m_text_storage);
//JUNK	[m_text_storage beginEditing];
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
			AM_DBG lib::logger::get_logger()->debug("cg_smiltext: another run");
			CFRange newrange;
			// Add the new characters
			newrange.location = [m_text_storage length];
			newrange.length = 0;
			NSMutableString *newdata = [NSMutableString stringWithUTF8String:""];
			switch((*i).m_command) {
			case smil2::stc_break:
				newdata = [NSMutableString stringWithUTF8String:"\n\n"];
				m_needs_conditional_space = false;
				m_needs_conditional_newline = false;
				break;
			case smil2::stc_condbreak:
				if (m_needs_conditional_newline) {
					newdata = [NSMutableString stringWithUTF8String:"\n"];
					m_needs_conditional_space = false;
					m_needs_conditional_newline = false;
				}
				break;
			case smil2::stc_condspace:
				if (m_needs_conditional_space) {
					newdata = [NSMutableString stringWithUTF8String:" "];
					m_needs_conditional_newline = true;
					m_needs_conditional_space = false;
				}
				break;
			case smil2::stc_data:
				{
					// Need a block, because of the variable:
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
					newdata = [NSMutableString stringWithUTF8String:(*i).m_data.c_str()];
				}
				break;
			default:
				assert(0);
			}
			// Handle override textDirection here, by inserting the magic unicode
			// commands
			if ((*i).m_direction == smil2::stw_ltro) {
				lib::logger::get_logger()->debug("cg_smiltext: should do ltro text");
				[newdata insertString: @"\u202d" atIndex: 0]; // LEFT-TO-RIGHT OVERRIDE
				[newdata appendString: @"\u202c"]; // POP DIRECTIONAL FORMATTING
			} else if ((*i).m_direction == smil2::stw_rtlo) {
				lib::logger::get_logger()->debug("cg_smiltext: should do rtlo text");
				[newdata insertString: @"\u202e" atIndex: 0]; // RIGHT-TO-LEFT OVERRIDE
				[newdata appendString: @"\u202c"]; // POP DIRECTIONAL FORMATTING
			}
//JUNK		[m_text_storage replaceCharactersInRange:newrange withString:newdata];
			CFAttributedStringReplaceString (m_text_storage, newrange, (CFStringRef) newdata);
			// Prepare for setting the attribute info
			NSMutableDictionary *attrs = [[NSMutableDictionary alloc] init];
			newrange.length = [newdata length];
			// Find font info
			CTFontRef text_font = NULL;
			std::vector<std::string>::const_iterator fi;
			for (fi=(*i).m_font_families.begin(); fi != (*i).m_font_families.end(); fi++) {
				AM_DBG lib::logger::get_logger()->debug("cg_smiltext: look for font '%s'", (*fi).c_str());
				text_font = _select_font((*fi).c_str(), (*i).m_font_style, (*i).m_font_weight, (*i).m_font_size);
				if (text_font) break;
				AM_DBG lib::logger::get_logger()->debug("cg_smiltext: not found, try next");
			}
			if (!text_font) {
//TBD				text_font = [NSFont userFontOfSize: (*i).m_font_size];
			}

//TBD			[attrs setValue:text_font forKey:NSFontAttributeName];

			if (!(*i).m_transparent) {
				// Find color info
				double alfa = 1.0;
				const common::region_info *ri = m_dest->get_info();
				if (ri) alfa = ri->get_mediaopacity();
				UIColor *color = [UIColor colorWithRed:redf((*i).m_color)
						green:greenf((*i).m_color)
						blue:bluef((*i).m_color)
						alpha:(float)alfa];
//TBD				[attrs setValue:color forKey:NSForegroundColorAttributeName];
			}
			if (!(*i).m_bg_transparent) {
				// Find background color info
				double alfa = 1.0;
				const common::region_info *ri = m_dest->get_info();
				if (ri) alfa = ri->get_mediabgopacity();
				if (alfa != 1.0)
					m_any_semiopaque_bg = true;
				UIColor *color = [UIColor colorWithRed:redf((*i).m_bg_color)
						green:greenf((*i).m_bg_color)
						blue:bluef((*i).m_bg_color)
						alpha:(float)alfa];
//TBD				[attrs setValue:color forKey:NSBackgroundColorAttributeName];
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
//TBD first create settings
				CTParagraphStyleRef ps = CTParagraphStyleCreate((const CTParagraphStyleSetting*) NULL, (CFIndex) 0);
				m_cur_paragraph_style = ps;
				// Set the paragraph writing direction
				if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
//TBD					[ps setBaseWritingDirection: NSWritingDirectionRightToLeft];
				} else {
					// All other directions are treated as left-to-right
//TBD					[ps setBaseWritingDirection: NSWritingDirectionLeftToRight];
				}
				if (m_params.m_mode != smil2::stm_crawl) {
					// Set the paragraph text alignment, unless we have moving text
					switch (m_cur_para_align) {
					case smil2::sta_start:
						if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
//TBD							[ps setAlignment: NSRightTextAlignment];
						} else {
							// All other directions are treated as left-to-right
//TBD							[ps setAlignment: NSLeftTextAlignment];
						}
						break;
					case smil2::sta_end:
						if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
//TBD							[ps setAlignment: NSLeftTextAlignment];
						} else {
							// All other directions are treated as left-to-right
//TBD							[ps setAlignment: NSRightTextAlignment];
						}
						break;
					case smil2::sta_left:
//TBD						[ps setAlignment: NSLeftTextAlignment];
						break;
					case smil2::sta_right:
//TBD						[ps setAlignment: NSRightTextAlignment];
						break;
					case smil2::sta_center:
//TBD						[ps setAlignment: NSCenterTextAlignment];
						break;
					}
				}
				// Set the paragraph wrap option
//TBD				if (m_cur_para_wrap)
//TBD					[ps setLineBreakMode: NSLineBreakByWordWrapping];
//TBD				else
//TBD					[ps setLineBreakMode: NSLineBreakByClipping];
			}
//TBD			[attrs setValue:m_cur_paragraph_style forKey:NSParagraphStyleAttributeName];

			// Set the attributes
//TBD		[m_text_storage setAttributes:attrs range:newrange];

			i++;
		}
		CFAttributedStringEndEditing(m_text_storage);
		m_engine.done();
	} else {
		AM_DBG lib::logger::get_logger()->debug("cg_smiltext::smiltext_changed: nothing changed");
	}
	bool finished = m_engine.is_finished();
	m_engine.unlock();
	[pool release];
	m_lock.leave();
	m_dest->need_redraw();
	if (finished)
		m_context->stopped(m_cookie);
}

void
cg_smiltext_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cg_smiltext_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	// Determine current position and size.
	cg_window *cwindow = (cg_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
//JNK	[view lockFocus]; // XXXJACK Attempt to workaround for problem in WebKitPlugin
	rect dstrect = r;
	dstrect.translate(m_dest->get_global_topleft());
	CGRect cg_dstrect = [view  CGRectForAmbulantRect: &dstrect];
	CGPoint visible_origin = CGPointMake(CGRectGetMinX (cg_dstrect), CGRectGetMinY(cg_dstrect));
	CGSize visible_size = CGSizeMake(cg_dstrect.size.width, cg_dstrect.size.height);

// Determine text container layout size. This depends on the type of container.
#define INFINITE_WIDTH 1000000
#define INFINITE_HEIGHT 1000000
	CGSize layout_size = visible_size;
	bool has_hmovement = false;
	bool has_vmovement = false;
	switch(m_params.m_mode) {
	case smil2::stm_scroll:
	case smil2::stm_jump:
		layout_size.height = INFINITE_HEIGHT;
		has_vmovement = true;
		break;
	case smil2::stm_crawl:
		layout_size.width = INFINITE_WIDTH;
		has_hmovement = true;
		break;
	case smil2::stm_replace:
	case smil2::stm_append:
		// Normal cases
		break;
	}

	CGSize old_layout_size;
	// Initialize the text engine if we have not already done so.
#ifdef JUNK
	if (! m_layout_manager) {
		// Initialize the text engine
		m_layout_manager = [[NSLayoutManager alloc] init];
		m_text_container = [[NSTextContainer alloc] initWithContainerSize: layout_size];
		old_layout_size = layout_size;	// Force resize
		[m_text_container setHeightTracksTextView: false];
		[m_text_container setWidthTracksTextView: false];
		[m_layout_manager addTextContainer:m_text_container];
		[m_text_container release]; // The layoutManager will retain the textContainer
		[m_text_storage addLayoutManager:m_layout_manager];
		[m_layout_manager release]; // The textStorage will retain the layoutManager
	} else {
		old_layout_size = [m_text_container containerSize];
	}
	assert(m_layout_manager);
	assert(m_text_container);
	assert(m_text_storage);
#endif//JUNK
	if (! m_framesetter) {
		m_path = CGPathCreateMutable();
		CGPathAddRect(m_path, NULL, cg_dstrect);
		m_framesetter = CTFramesetterCreateWithAttributedString(m_text_storage);
		m_frame = CTFramesetterCreateFrame (m_framesetter, CFRangeMake(0, 0), m_path, NULL);
		old_layout_size = layout_size;	// Force resize
	} else {
//TBD		old_layout_size = [m_text_container containerSize];
	}
	
	// If the layout size has changed (due to smil animation or so) change it
	if ( ! CGSizeEqualToSize(old_layout_size, layout_size)) {
		[m_text_container setContainerSize: layout_size];
	}
	// Now determine text placement (visible_origin)
	int firstlineheight = 14; // XXXJACK: should compute this...
	if (has_hmovement) {
		// For crawl, textConceal and textPlace determine horizontal position
		if (m_params.m_text_conceal == smil2::stc_initial || m_params.m_text_conceal == smil2::stc_both) {
			visible_origin.x += visible_size.width;
		} else if (m_cur_para_align == smil2::sta_right || m_cur_para_align == smil2::sta_end) {
			// XXX Incorrect: should look at writing direction...
			visible_origin.x += visible_size.width;
		} else if (m_cur_para_align == smil2::sta_center) {
			visible_origin.x += visible_size.width / 2;
		}
	} else if (has_vmovement) {
		// For scroll and jump, textConceal and textAlign determine vertical position
		if (m_params.m_text_conceal == smil2::stc_initial || m_params.m_text_conceal == smil2::stc_both) {
			visible_origin.y += visible_size.height;
		} else if (m_params.m_text_place == smil2::stp_from_end) {
			visible_origin.y += (visible_size.height - firstlineheight);
		} else if (m_params.m_text_place == smil2::stp_from_center) {
			visible_origin.y += (visible_size.height - firstlineheight) / 2;
		}
	} else {
		// For stationary text, textPlace determines vertical position
		if (m_params.m_text_place == smil2::stp_from_end) {
			visible_origin.y += (visible_size.height - firstlineheight);
		} else if (m_params.m_text_place == smil2::stp_from_center) {
			visible_origin.y += (visible_size.height - firstlineheight) / 2;
		}

	}
#ifdef TBD
	// If we do auto-scrolling we should now layout the text, so we can determine the rate
	if (m_engine.is_auto_rate()) {
		(void)[m_layout_manager glyphRangeForTextContainer: m_text_container];
		CGSize nsfull_size = [m_layout_manager usedRectForTextContainer: m_text_container].size;
		lib::size full_size((int)nsfull_size.width, (int)nsfull_size.height);
		unsigned int dur = 11; // XXXX
		smil2::smiltext_align align = m_cur_para_align;
		unsigned int rate = _compute_rate(align, full_size, r, dur);
		m_engine.set_rate(rate);
		m_params = m_engine.get_params();
	}
#endif//TBD
	// Next compute the layout position of what we want to draw at visible_origin
	CGPoint logical_origin = CGPointMake(0, 0);
	if (m_params.m_mode == smil2::stm_crawl) {
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		logical_origin.x += float(now * m_params.m_rate / 1000);
		visible_origin.x -= float(now * m_params.m_rate / 1000);
		// XXX see below
	}
	if (m_params.m_mode == smil2::stm_scroll) {
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		visible_origin.y -= float(now * m_params.m_rate / 1000);
		if (visible_origin.y < 0) {
			logical_origin.y -= visible_origin.y;
			// visible_origin.y = 0;
		}
	}
	AM_DBG logger::get_logger()->debug("cg_smiltext_renderer.redraw at cg-point (%f, %f) logical (%f, %f)", visible_origin.x, visible_origin.y, logical_origin.x, logical_origin.y);
	if (m_render_offscreen) {
	}
	// Now we need to determine which glyphs to draw. Unfortunately glyphRangeForBoundingRect gives us
	// full lines (which is apparently more efficient, google for details) which is not good enough
	// for ticker tape, so we adjust.
	CGRect logical_rect = CGRectMake(logical_origin.x, logical_origin.y, visible_size.width, visible_size.height);
//X	NSRange glyph_range = [m_layout_manager glyphRangeForBoundingRect: logical_rect inTextContainer: m_text_container];
	CFRange glyph_range = CTFrameGetVisibleStringRange(m_frame);
//X	[m_frame glyphRangeForBoundingRect: logical_rect inTextContainer: m_text_container];
	AM_DBG NSLog(@"Glyph range was %d, %d, origin-x %f", glyph_range.location, glyph_range.length, logical_origin.x);
	if (glyph_range.location >= 0 && glyph_range.length > 0) {
#ifdef JUNK
		if (m_any_semiopaque_bg) {
			// Background opacity 1.0 is implemented correctly in NSLayoutManager, but intermediate
			// values are a bit funny: they still override the underlying image (i.e. they don't
			// use NSCompositeSourceOver or something similar. Therefore, if this is the case we
			// draw the background color to a separate buffer and bitblit this onto the existing
			// bits.
			NSImage *tmpsrc = [view getTransitionTmpSurface];
			[tmpsrc lockFocus];
			[[NSColor colorWithDeviceWhite: 1.0f alpha: 0.0f] set];
			NSRectFill(cg_dstrect);
			[m_layout_manager drawBackgroundForGlyphRange: glyph_range atPoint: visible_origin];
			[tmpsrc unlockFocus];
			[tmpsrc drawInRect: cg_dstrect fromRect: cg_dstrect operation: NSCompositeSourceOver fraction: 1.0f];
		} else {
			// Otherwise we simply let NSLayoutManager do the work
			[m_layout_manager drawBackgroundForGlyphRange: glyph_range atPoint: visible_origin];
		}
		[m_layout_manager drawGlyphsForGlyphRange: glyph_range atPoint: visible_origin];
#else//JUNK
		CTFrameDraw (m_frame, [view getCGContext]);
#endif//JUNK
	}
//TBD	layout_size = [m_text_container containerSize];
	if (m_render_offscreen) {
	}
//X	[view unlockFocus];
	m_lock.leave();
}

unsigned int
cg_smiltext_renderer::_compute_rate(smil2::smiltext_align align, lib::size size, lib::rect r,  unsigned int dur) {
	// First find the distance to travel during scroll for various values
	// for textConceal and textPlace (w=window height, t=text height)
	// 
	// + ---------------------------------------------------+
	// | textConceal |	none	| initial |	 final	|  both |
	// |-------------|			|		  |			|		|
	// | textPlace	 |			|		  |			|		|
	// |----------------------------------------------------|
	// |   start	 | t>w?t-w:0|	 t	  |	   t	|  w+t	|
	// | ---------------------------------------------------|
	// |   center	|t>w?t-w/2:w/2|	 t	  | w/2+t	|  w+t	|
	// | ---------------------------------------------------|
	// |   end		 |	  t		|	 t	  |	 w+t	|  w+t	|
	// + ---------------------------------------------------+
	
	unsigned int dst = 0, win = 0, txt = 0;
	switch (m_params.m_mode) {
	case smil2::stm_crawl:
		win = r.w;
		txt = size.w;
		// Convert any sta_left/sta_right values into sta_start/sta_end
		// as defined by textWritingMode
		switch (align) {
		default:
			break;
		case smil2::sta_left:
			//TBD adapt for textWritingMode
			align = smil2::sta_start;
			break;
		case smil2::sta_right:
			//TBD adapt for textWritingMode
			align = smil2::sta_end;
			break;
		}
		switch (m_params.m_text_conceal) {
		default:
		case smil2::stc_none:
			switch (align) {
			default:
			case smil2::sta_start:
				dst = txt > win ? txt - win : 0;
				break;
			case smil2::sta_end:
				dst = txt;
				break;
			case smil2::sta_center:
				dst = txt > win/2 ? txt - win/2 : 0;
				break;
			}
			break;
		case smil2::stc_initial: // ignore textAlign
			dst = txt;
			break;
		case smil2::stc_final:
			switch (align) {
			default:
			case smil2::sta_start:
				dst = txt;
				break;
			case smil2::sta_end:
				dst = win+txt;
				break;
			case smil2::sta_center:
				dst = win/2+txt;
				break;
			}
			break;
		case smil2::stc_both: // ignore textAlign
			dst = win+txt;
			break;
		}
		break;
	case smil2::stm_scroll:
		win = r.h;
		txt = size.h;
		switch (m_params.m_text_conceal) {
		default:
		case smil2::stc_none:
			switch (m_params.m_text_place) {
			default:
			case smil2::stp_from_start:
				dst = txt > win ? txt - win : 0;
				break;
			case smil2::stp_from_end:
				dst = txt;
				break;
			case smil2::stp_from_center:
				dst = txt > win/2 ? txt - win/2 : 0;
				break;
			}
			break;
		case smil2::stc_initial: // ignore textPlace
			dst = txt;
			break;
		case smil2::stc_final:
			switch (m_params.m_text_place) {
			default:
			case smil2::stp_from_start:
				dst = txt;
				break;
			case smil2::stp_from_end:
				dst = win+txt;
				break;
			case smil2::stp_from_center:
				dst = win/2+txt;
				break;
			}
			break;
		case smil2::stc_both: // ignore textPlace
			dst = win+txt;
			break;
		}
		break;
	default:
		break;
	}
	return (dst+dur-1)/dur;
}

} // namespace cg

} // namespace gui

} //namespace ambulant

#endif // WITH_SMIL30
