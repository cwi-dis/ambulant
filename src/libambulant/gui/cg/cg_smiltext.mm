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

#include "ambulant/gui/cg/cg_smiltext.h"
#include "ambulant/gui/cg/cg_gui.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {
	
using namespace lib;
	
namespace gui {
		
namespace cg {	
			
extern const char cg_smiltext_playable_tag[] = "smilText";
extern const char cg_smiltext_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCoreGraphics");
extern const char cg_smiltext_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererSmilText");
			
typedef struct _CTFont_info { CTFontRef font; CTFontDescriptorRef font_descr; } CTFont_info; //aux.

static CTFont_info
_select_font(const char *family, smil2::smiltext_font_style style, smil2::smiltext_font_weight weight, int size)
{
	CTFont_info rv = { NULL, NULL}; // return value
	CTFontRef font = NULL;
	CTFontSymbolicTraits mask = 0;
	CTFontSymbolicTraits value = 0;
	NSString *ffname = NULL;
	if (strcmp(family, "serif") == 0) {
		ffname = [NSString stringWithUTF8String: "Times New Roman"];
	} else if (strcmp(family, "monospace") == 0) {
		ffname = [NSString stringWithUTF8String: "Courier New"];
	} else if (strcmp(family, "sansSerif") == 0) {
		ffname = [NSString stringWithUTF8String: "Helvetica Neue"];
	} else {
		ffname = [NSString stringWithUTF8String:family];
	}
	CTFontDescriptorRef font_descr = CTFontDescriptorCreateWithNameAndSize((CFStringRef)ffname, size);
	if (font_descr == NULL) {
		return rv;
	}
	if (strcmp(family, "monospace") == 0) {
		mask |= kCTFontCondensedTrait;
		value |= kCTFontCondensedTrait;
	}
	switch(style) {
		case smil2::sts_normal:
		case smil2::sts_reverse_oblique: // Not supported
			mask |= kCTFontItalicTrait;
			value &= ~kCTFontItalicTrait;
			break;
		case smil2::sts_italic:
		case smil2::sts_oblique:
			mask |= kCTFontItalicTrait;
			value |= kCTFontItalicTrait;
			break;
	}
	switch(weight) {
		case smil2::stw_normal:
			mask |= kCTFontBoldTrait;
			value &= ~kCTFontBoldTrait;
			break;
		case smil2::stw_bold:
			mask |= kCTFontBoldTrait;
			value |= kCTFontBoldTrait;
			break;
	}
	font = CTFontCreateWithFontDescriptor(font_descr, size, NULL);
	AM_DBG NSLog(@"cg_smiltext_renderer::_select_font(%s) font=0x%@ font_descr=0x%@#%ld", family, font, font_descr, font_descr != NULL ? CFGetRetainCount(font_descr):0);
	if (font != NULL) {
		CTFontRef desired_font = CTFontCreateCopyWithSymbolicTraits(font, 0.0, NULL, value, mask);
#if 0
		// releasing `font_descr` here, where we don't use anymore, will cause a crash; thus, it is returned 
		// and to be CFRelease'd after the 'kCTFontAttributeName' value is stored in a 'CFMutableDictionary' or when
		// it is no longer used by the caller.
		CFRelease(font_descr);
#endif	
		if (desired_font != NULL) {
			CFRelease(font);
			font = desired_font;
		}
	}
	AM_DBG lib::logger::get_logger()->debug("cg_smiltext_renderer::_select_font(%s) returns {font=0x%x#%ld font_descr=0x%x#%ld", family, font, font != NULL ? CFGetRetainCount(font):0, font_descr, font_descr != NULL ? CFGetRetainCount(font_descr):0);
	rv.font = font;
	rv.font_descr = font_descr;
	return rv;
}

common::playable_factory *
create_cg_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCG"), true);
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
	m_rgb_colorspace(NULL),
	m_engine(smil2::smiltext_engine(node, evp, this, false)),
	m_needs_conditional_newline(false),
	m_needs_conditional_space(false),
	m_params(m_engine.get_params()),
	m_cur_paragraph_style(NULL),
	m_cur_para_align(smil2::sta_start),
	m_cur_para_writing_mode(smil2::stw_lr_tb),
	m_cur_para_wrap(true),
	m_any_semiopaque_bg(false),
	m_current_bgcolor(0),
	m_top_left()
{
	m_text_storage = CFAttributedStringCreateMutable(NULL, 0);
	m_rgb_colorspace = CGColorSpaceCreateDeviceRGB();
	m_render_offscreen = (m_params.m_mode != smil2::stm_replace && m_params.m_mode != smil2::stm_append);
}

cg_smiltext_renderer::~cg_smiltext_renderer()
{
	m_lock.enter();
	if (m_dest != NULL) {
		m_dest->renderer_done(this);
		m_dest = NULL;
	}
	if (m_text_storage != NULL) {
		CFRelease(m_text_storage);
		m_text_storage = NULL;
	}
	if (m_rgb_colorspace != NULL) {
		CGColorSpaceRelease (m_rgb_colorspace);
		m_rgb_colorspace = NULL;
	}
	if (m_frame != NULL) {
		CFRelease(m_frame);
		m_frame = NULL;
	}
	if (m_cur_paragraph_style != NULL) {
		CFRelease(m_cur_paragraph_style);
		m_cur_paragraph_style = NULL;
	}
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
	if (t >= 0 ) {
        m_engine.seek(t);
	}
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
	if (m_dest == NULL) {
		m_lock.leave();
		return;
	}
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	assert(m_text_storage);
	m_engine.lock();
	if (m_engine.is_changed()) {
		lib::xml_string data;
		smil2::smiltext_runs::const_iterator i;
		CFAttributedStringBeginEditing(m_text_storage);

		if (m_engine.is_cleared()) {
			// Completely new text. Clear our copy and render everything.
			m_needs_conditional_space = false;
			m_needs_conditional_newline = false;
			NSRange all;
			all.location = 0;
			all.length = [(NSMutableAttributedString*) m_text_storage length];
			if (all.length)
				[(NSMutableAttributedString*) m_text_storage deleteCharactersInRange:all];
			i = m_engine.begin();
		} else {
			// Only additions. Don't clear and only render the new stuff.
			i = m_engine.newbegin();
		}
		while (i != m_engine.end()) { // loop over smiltext_runs
			smil2::smiltext_run run = *i;
			AM_DBG lib::logger::get_logger()->debug("cg_smiltext_changed: another run");
			CFRange newrange;

			// Add the new characters
			newrange.location = [(NSMutableAttributedString*) m_text_storage length]; 
			newrange.length = 0;

			NSMutableString *newdata = [NSMutableString stringWithUTF8String:""];
			switch(run.m_command) {
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
					char lastch = *(run.m_data.rbegin());
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
			if (run.m_direction == smil2::stw_ltro) {
				lib::logger::get_logger()->debug("cg_smiltext: should do ltro text");
				[newdata insertString: @"\u202d" atIndex: 0]; // LEFT-TO-RIGHT OVERRIDE
				[newdata appendString: @"\u202c"]; // POP DIRECTIONAL FORMATTING
			} else if ((*i).m_direction == smil2::stw_rtlo) {
				lib::logger::get_logger()->debug("cg_smiltext: should do rtlo text");
				[newdata insertString: @"\u202e" atIndex: 0]; // RIGHT-TO-LEFT OVERRIDE
				[newdata appendString: @"\u202c"]; // POP DIRECTIONAL FORMATTING
			}
			CFAttributedStringReplaceString (m_text_storage, newrange, (CFStringRef) newdata);

			newrange.length = [newdata length];

			// optimization: skip layout if nothing really has changed
			if (attributes_are_changed(m_dest) || newrange.length > 0) {
				// recompute all attributes and store them together with the text in m_text_storage
				if (m_frame != NULL) {
					CFRelease(m_frame);
					m_frame = NULL; // trigger complete redraw
				}				
				// Prepare for setting the attribute info
				CFMutableDictionaryRef attrs =
				CFDictionaryCreateMutable(kCFAllocatorDefault,
					0/*inifite*/,
					&kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
				// Find font info
				CTFont_info text_font = {NULL, NULL};
				std::vector<std::string>::const_iterator fi;

				for (fi=run.m_font_families.begin(); fi != run.m_font_families.end(); fi++) {
					AM_DBG lib::logger::get_logger()->debug("cg_smiltext_changed: look for font '%s'", (*fi).c_str());
					text_font = _select_font((*fi).c_str(), run.m_font_style, run.m_font_weight, run.m_font_size);
					if (text_font.font) break;
					AM_DBG lib::logger::get_logger()->debug("cg_smiltext_changed: not found, try next");
					if (text_font.font_descr != NULL) {
						CFRelease(text_font.font_descr);
						text_font.font_descr = NULL;
					}
				}
				if (text_font.font != NULL) {
					CFDictionaryAddValue (attrs, kCTFontAttributeName, text_font.font);
					CFRelease(text_font.font);
				}
				if (text_font.font_descr != NULL) {
					CFRelease(text_font.font_descr);
				}
				if ( ! run.m_transparent) {
					// Find color info
					CGFloat alfa = 1.0;
					const common::region_info *ri = m_dest->get_info();
					if (ri) alfa = ri->get_mediaopacity();
					CGFloat fg_components[] = { redf(run.m_color),
												greenf(run.m_color),
												bluef(run.m_color),
												alfa};
					CGColorRef run_fg_color = CGColorCreate(m_rgb_colorspace, fg_components);
					CFDictionaryAddValue (attrs, kCTForegroundColorAttributeName, run_fg_color);
					CGColorRelease(run_fg_color);
				}
#ifdef kCTBackgroundColorAttributeName /* no text background in iOS4.0 */
				if ( ! run.m_bg_transparent) {
					// Find background color info
					CGFloat alfa = 1.0;
					const common::region_info *ri = m_dest->get_info();
					if (ri) alfa = ri->get_mediabgopacity();
					if (alfa != 1.0)
						m_any_semiopaque_bg = true;
					CGFloat bg_components[] = {	redf(run.m_bg_color),
												greenf(run.m_bg_color),
												bluef(run.m_bg_color),
												alfa};
					CGColorRef run_bg_color = CGColorCreate(m_rgb_colorspace, bg_components);
					CFDictionaryAddValue (attrs, kCTBackgroundColorAttributeName, run_bg_color);
					CGColorRelease(run_bg_color);
				}
#endif // kCTBackgroundColorAttributeName
			
				// Finally do paragraph settings (which are cached)
				if (m_cur_paragraph_style == NULL ||
						m_cur_para_align != run.m_align ||
						m_cur_para_writing_mode != run.m_writing_mode ||
						m_cur_para_wrap != run.m_wrap) {
					// Delete the old one, if needed
					if (m_cur_paragraph_style != NULL) {
						CFRelease(m_cur_paragraph_style);
					}
					// Remember the values
					m_cur_para_align = run.m_align;
					m_cur_para_writing_mode = run.m_writing_mode;
					m_cur_para_wrap = run.m_wrap;

					// Allocate the new one
					// see: http://lists.apple.com/archives/mac-opengl/2008/Aug/msg00104.html
					CTWritingDirection writing_direction = kCTWritingDirectionNatural;
					CTLineBreakMode line_break_mode = kCTLineBreakByWordWrapping; 
					CTTextAlignment text_alignment = kCTNaturalTextAlignment;
								
					// Set the paragraph writing direction
					if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
						writing_direction = kCTWritingDirectionRightToLeft;
					} else {
						// All other directions are treated as left-to-right
						writing_direction = kCTWritingDirectionLeftToRight;
					}
					if (m_params.m_mode != smil2::stm_crawl) {
						// Set the paragraph text alignment, unless we have moving text
						switch (m_cur_para_align) {
							case smil2::sta_start:
								if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
									text_alignment = kCTRightTextAlignment;
								} else {
									// All other directions are treated as left-to-right
									text_alignment = kCTLeftTextAlignment;
								}
								break;
							case smil2::sta_end:
								if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
									text_alignment = kCTLeftTextAlignment;
								} else {
									// All other directions are treated as left-to-right
									text_alignment = kCTRightTextAlignment;
								}
								break;
							case smil2::sta_left:
								text_alignment = kCTLeftTextAlignment;
								break;
							case smil2::sta_right:
								text_alignment = kCTRightTextAlignment;
								break;
							case smil2::sta_center:
								text_alignment = kCTCenterTextAlignment;
								break;
						}
					}
					// Set the paragraph wrap option
					if (m_cur_para_wrap) {
						line_break_mode = kCTLineBreakByWordWrapping;
					} else {
						line_break_mode = kCTLineBreakByClipping;					
					}
					CTParagraphStyleSetting settings[] = {
						{kCTParagraphStyleSpecifierAlignment, sizeof(text_alignment), &text_alignment },
						{kCTParagraphStyleSpecifierLineBreakMode, sizeof(line_break_mode), &line_break_mode },
						{kCTParagraphStyleSpecifierBaseWritingDirection, sizeof(writing_direction), &writing_direction }
					};
					m_cur_paragraph_style = CTParagraphStyleCreate(settings, sizeof(settings)/sizeof(settings[0]));
				}
				CFDictionaryAddValue (attrs, kCTParagraphStyleAttributeName, m_cur_paragraph_style);
			
				// Set the attributes
				CFAttributedStringSetAttributes (m_text_storage, newrange, (CFDictionaryRef) attrs, true);
				CFDictionaryRemoveAllValues(attrs);
				CFRelease(attrs);
			} // attributes_changed || newrange > 0
			i++;
		} // loop over smiltext_runs
		CFAttributedStringEndEditing(m_text_storage);

		m_engine.done();
		AM_DBG {
			CFStringRef cfstr = CFAttributedStringGetString(m_text_storage);
			const char* str = [(NSString*) cfstr UTF8String];
			lib::logger::get_logger()->debug("cg_smiltext_changed: m_text_storage=%s", str);
			//NSLog(@"cg_smiltext_changed: m_text_storage=%@", (NSString*) m_text_storage);
		}
	} else {
		AM_DBG lib::logger::get_logger()->debug("cg_smiltext_changed: nothing changed");
	} // m_engine.is_changed()
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
	
	// Core Graphics (cg_...) uses Carthesian X/Y coordinate system, ambulant (am_...) uses Top-Bottom-Width-Height
	rect am_final_dst_rect = r; // where the text ultimately will be shown
	lib::point am_top_left = m_dest->get_global_topleft();
	am_final_dst_rect.translate(am_top_left);
	AM_DBG logger::get_logger()->debug("cg_smiltext_renderer.redraw(0x%x, am_final_dst_rect=(%d,%d,%d,%d)", (void *)this, am_final_dst_rect.left(), am_final_dst_rect.top(), am_final_dst_rect.right(), am_final_dst_rect.bottom());
	CGRect cg_final_dst_rect = CGRectFromAmbulantRect(am_final_dst_rect);

	// Save the graphics state to be restored on return
	CGContext* context = [view getCGContext];	
	CGContextSaveGState(context);
	// Initialize Text Matrix (otherwise giant characters may sometimes appear) and
	// transformation matrix.
	CGAffineTransform matrix = [view transformForRect: &cg_final_dst_rect flipped: YES translated: NO];
	CGContextConcatCTM(context, matrix);
	CGContextSetTextMatrix(context, CGAffineTransformIdentity);
	
#define INFINITE_WIDTH 1000000
#define INFINITE_HEIGHT 1000000
	CGRect cg_frame_rect = cg_final_dst_rect;
	if (m_frame == NULL || am_top_left.x != m_top_left.x || am_top_left.y != m_top_left.y) {
		m_top_left.x = am_top_left.x;
		m_top_left.y = am_top_left.y;
		// Format text go get line height and width information, will be drawn later
		m_cg_origin = CGPointMake(0,0);//(cg_final_dst_rect.origin.x, cg_final_dst_rect.origin.y);

		if (m_params.m_mode == smil2::stm_crawl) {
			cg_frame_rect.size.width = INFINITE_WIDTH;
		} else if (m_params.m_mode == smil2::stm_jump || m_params.m_mode == smil2::stm_scroll) {
			cg_frame_rect.size.height = INFINITE_HEIGHT;
		}
	
		AM_DBG logger::get_logger()->debug("cg_frame_rect=(%d,%d, %d,%d)", (int) cg_frame_rect.origin.x, (int) cg_frame_rect.origin.y, (int) cg_frame_rect.size.width, (int) cg_frame_rect.size.height);
		m_frame = create_frame(m_text_storage, cg_frame_rect);

		int cg_firstlineheight;
		CGSize cg_layout_size = measure_frame (m_frame, context, &cg_firstlineheight);
		// Determine text container layout size. This depends on the type of container.
		bool has_hmovement = false;
		bool has_vmovement = false;
		switch(m_params.m_mode) {
        case smil2::stm_scroll:
        case smil2::stm_jump:
            has_vmovement = true;
            break;
        case smil2::stm_crawl:
            cg_layout_size.height = cg_firstlineheight;
            has_hmovement = true;
            break;
        case smil2::stm_replace:
        case smil2::stm_append:
            // Normal cases
            break;
		}
		CGSize old_layout_size;
		// If the layout size has changed (due to smil animation or so) change it
		if ( ! CGSizeEqualToSize(old_layout_size, cg_layout_size)) {
			// ??
		}
		// Now determine text placement (cg_origin)
		if (has_hmovement) {
			// For crawl, textConceal and textAlign determine horizontal position
			if (m_params.m_text_conceal == smil2::stc_initial || m_params.m_text_conceal == smil2::stc_both) {
				m_cg_origin.x += cg_final_dst_rect.size.width;
			} else if (m_cur_para_align == smil2::sta_right || m_cur_para_align == smil2::sta_end) {
				// XXX Incorrect: should look at writing direction...
				m_cg_origin.x += cg_final_dst_rect.size.width;
			} else if (m_cur_para_align == smil2::sta_center) {
				m_cg_origin.x += cg_final_dst_rect.size.width / 2;
			}
		} else if (has_vmovement) {
			// For scroll and jump, textConceal and textPlace determine vertical position
			if (m_params.m_text_conceal == smil2::stc_none || m_params.m_text_conceal == smil2::stc_final) {
				switch (m_params.m_text_place) {
                case smil2::stp_from_start:
                    m_cg_origin.y -= cg_frame_rect.size.height - cg_final_dst_rect.size.height;
                    break;
                case smil2::stp_from_center:
                    m_cg_origin.y -= cg_frame_rect.size.height - cg_final_dst_rect.size.height/2;
                    break;
                case smil2::stp_from_end:
                    m_cg_origin.y -= cg_frame_rect.size.height - cg_firstlineheight;
                    break;
                default:
                    break;
				}
			} else if (m_params.m_text_conceal == smil2::stc_initial || m_params.m_text_conceal == smil2::stc_both) {
				m_cg_origin.y -= cg_frame_rect.size.height; // ignore textPlace, SMIL 3.0 par.8.5.1
			} 
		} else {
			// For stationary text, textPlace determines vertical position
			if (m_params.m_text_place == smil2::stp_from_end) {
				m_cg_origin.y -= (cg_frame_rect.size.height - cg_firstlineheight);
			} else if (m_params.m_text_place == smil2::stp_from_center) {
				m_cg_origin.y -= cg_frame_rect.size.height  - cg_final_dst_rect.size.height / 2;
			} else {
				m_cg_origin.y -= cg_frame_rect.size.height - cg_final_dst_rect.size.height;
			}
		}
		// For auto-scrolling we did already layout the text, now determine the rate
		if (m_engine.is_auto_rate()) {
			unsigned int dur = m_engine.get_dur();
			if (dur > 0) {
				AM_DBG logger::get_logger()->debug("cg_smiltext_renderer.redraw: dur=%d, cg_final_dst_rect=(%f,%f),(%f,%f)", dur, cg_final_dst_rect.origin.x, cg_final_dst_rect.origin.y, cg_final_dst_rect.size.width, cg_final_dst_rect.size.height);
				smil2::smiltext_align align = m_cur_para_align;
				lib::size am_full_size((int) cg_layout_size.width, (int) cg_layout_size.height);
				unsigned int rate = _compute_rate(align, am_full_size, am_final_dst_rect, dur);
				m_engine.set_rate(rate);
				m_params = m_engine.get_params();
			}
		}
	}
	CGPoint cg_origin = m_cg_origin;
	// Next compute the layout position of what we want to draw at cg_frame_origin
	if (m_params.m_mode == smil2::stm_crawl) {
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		cg_origin.x -= float(now * m_params.m_rate / 1000);
	}
	if (m_params.m_mode == smil2::stm_scroll) {
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		cg_origin.y += float(now * m_params.m_rate / 1000);
        AM_DBG logger::get_logger()->debug("cg_smiltext_renderer.redraw: now=%lf, cg_origin.y=%f", now, cg_origin.y);
	}
	CGContextClipToRect(context, cg_final_dst_rect);
	CGContextTranslateCTM(context, cg_origin.x, cg_origin.y);
	CTFrameDraw (m_frame, context);
	CGContextRestoreGState(context);
	if (m_render_offscreen) {
		// ?
	}
	m_lock.leave();
}

CGSize
cg_smiltext_renderer::measure_frame(CTFrameRef frame, CGContext* cgContext, int* first_line_height) {
	// adapted from: http://lists.apple.com/archives/quartz-dev/2008/Mar/msg00079.html
	CGPathRef framePath = CTFrameGetPath(frame);
	CGRect frameRect = CGPathGetBoundingBox(framePath);
	
	CFArrayRef lines = CTFrameGetLines(frame);
	CFIndex numLines = CFArrayGetCount(lines);
	
	CGFloat maxWidth = 0;
	CGFloat textHeight = 0;
	
	// Now run through each line determining the maximum width of all the lines.
	// We special case the last line of text. While we've got it's descent handy,
	// we'll use it to calculate the typographic height of the text as well.
	CFIndex lastLineIndex = numLines - 1;
	for (CFIndex index = 0; index < numLines; index++) {
		CGFloat ascent, descent, leading, width;
		CTLineRef line = (CTLineRef) CFArrayGetValueAtIndex(lines, index);
		width = CTLineGetTypographicBounds(line, &ascent,  &descent, &leading);
		
		if (width > maxWidth) {
			maxWidth = width;
		}
		if (index == 0 & first_line_height != NULL) {
			*first_line_height = (int) ceil (ascent+descent+leading);
		}
		if(index == lastLineIndex) {
			// Get the origin of the last line. We add the descent to this
			// (below) to get the bottom edge of the last line of text.
			CGPoint lastLineOrigin;
			CTFrameGetLineOrigins(frame, CFRangeMake(lastLineIndex, 1), &lastLineOrigin);
			
			// The height needed to draw the text is from the bottom of the last line
			// to the top of the frame.
			textHeight =  CGRectGetMaxY(frameRect) - lastLineOrigin.y + descent;
		}
	}
		
	// For some text the exact typographic bounds is a fraction of a point too
	// small to fit the text when it is put into a context. We go ahead and round
	// the returned drawing area up to the nearest point.  This takes care of the
	// discrepencies.
	return CGSizeMake(ceil(maxWidth), ceil(textHeight));
}

CTFrameRef
cg_smiltext_renderer::create_frame (CFAttributedStringRef cf_astr, CGRect rect) {
	CTFrameRef frame = NULL;
	CTFramesetterRef framesetter = CTFramesetterCreateWithAttributedString(cf_astr);
	if (framesetter == NULL) {
		return NULL;
	}
	CGMutablePathRef path = CGPathCreateMutable();
	if (path != NULL) {
		AM_DBG logger::get_logger()->debug("cg_smiltext_renderer::create_frame: rect=(%d,%d, %d,%d)", (int) rect.origin.x, (int) rect.origin.y, (int) rect.size.width, (int) rect.size.height);
		CGPathAddRect(path, NULL, rect);
		frame = CTFramesetterCreateFrame (framesetter, CFRangeMake(0, 0), path, NULL);
		CFRelease(path);
	}
	CFRelease(framesetter);
	
	return frame;
}

bool
cg_smiltext_renderer::attributes_are_changed(common::surface* surf) {
	// check ALL animatable attributes for changes, and remember ALL current attributes as member variables.
	// return true if ANY attribute value was changed.
	bool rv = false;

	if (surf == NULL) {
		assert(surf == NULL);
		return rv;
	}
	// top, left
	lib::point top_left = surf->get_global_topleft();
	if (m_top_left.x != top_left.x || m_top_left.y != top_left.y) {
		m_top_left = top_left;
		rv |= true;
	}
	const common::region_info* ri = surf->get_info();
	if (ri == NULL) {
		return rv;
	}
	// background color
	lib::color_t current_bgcolor = ri->get_bgcolor();
	if (current_bgcolor != m_current_bgcolor) {
		m_current_bgcolor = current_bgcolor;
		rv |= true;
	}
	return rv;
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
