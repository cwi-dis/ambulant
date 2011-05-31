// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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

#include "ambulant/gui/d2/d2_smiltext.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace d2 {

inline D2D1_RECT_F d2_rectf(lib::rect r) {
	return D2D1::RectF((float) r.left(), (float) r.top(), (float) r.right(), (float) r.bottom());
}

extern const char d2_smiltext_playable_tag[] = "smilText";
extern const char d2_smiltext_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirect2D");
extern const char d2_smiltext_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererSmilText");

IDWriteFactory *ambulant::gui::d2::d2_smiltext_renderer::s_write_factory = NULL;

common::playable_factory *
create_d2_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirect2D"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSmilText"), true);
	return new common::single_playable_factory<
		d2_smiltext_renderer,
		d2_smiltext_playable_tag,
		d2_smiltext_playable_renderer_uri,
		d2_smiltext_playable_renderer_uri2,
		d2_smiltext_playable_renderer_uri2>(factory, mdp);
}


d2_smiltext_renderer::d2_smiltext_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	d2_renderer<renderer_playable>(context, cookie, node, evp, factory, mdp),
	m_text_format(NULL),
	m_text_layout(NULL),
	m_brush(NULL),
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
	if (s_write_factory == NULL) {
		HRESULT hr;
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(s_write_factory),
			reinterpret_cast<IUnknown**>(&s_write_factory));
		if (!SUCCEEDED(hr)) {
			lib::logger::get_logger()->error("Cannot create DirectWrite factory: error 0x%x", hr);
		}
	}
}

void
d2_smiltext_renderer::init_with_node(const lib::node *node)
{
	lib::textptr font_name("Helvetica");
	float font_size = 14.0;
	m_text_color = 0;

	smil2::params *params = smil2::params::for_node(node);
	if (params) {
		font_name = params->get_str("font-family");
		font_size = params->get_float("font-size", 14.0);
		m_text_color = params->get_color("color", 0);
		delete params;
	}
	if (m_text_format) {
		m_text_format->Release();
		m_text_format = NULL;
	}
	if (m_brush) {
		m_brush->Release();
		m_brush = NULL;
	}
	if (!s_write_factory) return;
	HRESULT hr;
	hr = s_write_factory->CreateTextFormat(
		font_name.wstr(),
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		font_size,
		L"",
		&m_text_format);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->error("Cannot create DirectWrite TextFormat: error 0x%x", hr);
	}
}

d2_smiltext_renderer::~d2_smiltext_renderer()
{
	m_lock.enter();
	m_lock.leave();
}

void
d2_smiltext_renderer::start(double t)
{
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_context->started(m_cookie);
	m_engine.start(t);
	renderer_playable::start(t);
}

void
d2_smiltext_renderer::seek(double t)
{
	assert( t >= 0);
	m_engine.seek(t);
	//renderer_playable::seek(t);
}

bool
d2_smiltext_renderer::stop()
{
	m_engine.stop();
	renderer_playable::stop();
	m_context->stopped(m_cookie);
	return true; // Don't re-use this renderer
}
void
d2_smiltext_renderer::pause(pause_display d)
{
}

void
d2_smiltext_renderer::resume()
{
}

void
d2_smiltext_renderer::marker_seen(const char *name)
{
	m_lock.enter();
	m_context->marker_seen(m_cookie, name);
	m_lock.leave();
}

void
d2_smiltext_renderer::smiltext_changed()
{
	m_lock.enter();
	m_engine.lock();
	if (_collect_text())
		_recreate_layout();
	bool finished = m_engine.is_finished();
	m_engine.unlock();
	m_lock.leave();

	m_dest->need_redraw();
	if (finished)
		m_context->stopped(m_cookie);
}

bool
d2_smiltext_renderer::_collect_text()
{
	if (!m_engine.is_changed()) return false;
	m_data = L"";
	lib::xml_string newdata;
	m_run_begins.clear();
	m_run_begins.push_back(0);
	smil2::smiltext_runs::const_iterator i;
	
	// For DirectWrite we always re-format the complete text
	m_needs_conditional_space = false;
	m_needs_conditional_newline = false;
	for( i=m_engine.begin(); i!=m_engine.end(); i++) {
		AM_DBG lib::logger::get_logger()->debug("cocoa_smiltext: another run");
		switch((*i).m_command) {
		case smil2::stc_break:
			newdata = "\n\n";
			m_needs_conditional_space = false;
			m_needs_conditional_newline = false;
			break;
		case smil2::stc_condbreak:
			if (m_needs_conditional_newline) {
				newdata = "\n";
				m_needs_conditional_space = false;
				m_needs_conditional_newline = false;
			}
			break;
		case smil2::stc_condspace:
			if (m_needs_conditional_space) {
				newdata = " ";
				m_needs_conditional_newline = true;
				m_needs_conditional_space = false;
			}
			break;
		case smil2::stc_data:
			if ( (*i).m_data == "") {
				// I think we leave the conditionals alone, for an empty block.
			} else {
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
				newdata = (*i).m_data.c_str();
			}
			break;
		default:
			assert(0);
		}
		// Handle override textDirection here, by inserting the magic unicode
		// commands
		if ((*i).m_direction == smil2::stw_ltro) {
			lib::logger::get_logger()->debug("cocoa_smiltext: should do ltro text");
			newdata = "\u202d" + newdata + "\u202c";
		} else if ((*i).m_direction == smil2::stw_rtlo) {
			lib::logger::get_logger()->debug("cocoa_smiltext: should do rtlo text");
			newdata = "\u202e" + newdata + "\u202c";
		}
		lib::textptr convert(newdata.c_str());
		const wchar_t *wnewdata = convert.c_wstr();
		m_data = m_data + wnewdata;
		m_run_begins.push_back(m_data.length());
	}

	m_engine.done();
	return true;
}


void
d2_smiltext_renderer::_recreate_layout()
{
	// First we convert the wide string into a dw object
	if (m_text_layout) m_text_layout->Release();
	if (m_dest == NULL) return;
	rect destrect = m_dest->get_rect();
	FLOAT w = (float) destrect.width();
	FLOAT h = (float) destrect.height();
	HRESULT hr;
	if (s_write_factory == NULL) return;
	hr = s_write_factory->CreateTextLayout(m_data.c_str(), m_data.length(), m_text_format, w, h, &m_text_layout);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("d2_smiltext: Cannot CreateTextLayout: error 0x%x", hr);
		return;
	}
#ifdef JNK
	//JNK	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	lib::xml_string data;
	smil2::smiltext_runs::const_iterator i;
//JNK		[m_text_storage beginEditing];
	if (m_engine.is_cleared()) {
		// Completely new text. Clear our copy and render everything.
		m_needs_conditional_space = false;
		m_needs_conditional_newline = false;
//JNK			NSRange all;
//JNK			all.location = 0;
//JNK			all.length = [m_text_storage length];
//JNK			if (all.length);
//JNK				[m_text_storage deleteCharactersInRange:all];
		i = m_engine.begin();
	} else {
		// Only additions. Don't clear and only render the new stuff.
		i = m_engine.newbegin();
	}
	while (i != m_engine.end()) {
		AM_DBG lib::logger::get_logger()->debug("cocoa_smiltext: another run");
//JNK			NSRange newrange;
//JNK			// Add the new characters
//JNK			newrange.location = [m_text_storage length];
//JNK			newrange.length = 0;
//JNK			NSMutableString *newdata = [NSMutableString stringWithUTF8String:""];
		switch((*i).m_command) {
		case smil2::stc_break:
//JNK				newdata = [NSMutableString stringWithUTF8String:"\n\n"];
			m_needs_conditional_space = false;
			m_needs_conditional_newline = false;
			break;
		case smil2::stc_condbreak:
			if (m_needs_conditional_newline) {
//JNK					newdata = [NSMutableString stringWithUTF8String:"\n"];
				m_needs_conditional_space = false;
				m_needs_conditional_newline = false;
			}
			break;
		case smil2::stc_condspace:
			if (m_needs_conditional_space) {
//JNK					newdata = [NSMutableString stringWithUTF8String:" "];
				m_needs_conditional_newline = true;
				m_needs_conditional_space = false;
			}
			break;
		case smil2::stc_data:
			if ( (*i).m_data == "") {
				// I think we leave the conditionals alone, for an empty block.
			} else {
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
//JNK					newdata = [NSMutableString stringWithUTF8String:(*i).m_data.c_str()];
			}
			break;
		default:
			assert(0);
		}
		// Handle override textDirection here, by inserting the magic unicode
		// commands
		if ((*i).m_direction == smil2::stw_ltro) {
			lib::logger::get_logger()->debug("cocoa_smiltext: should do ltro text");
//JNK				[newdata insertString: @"\u202d" atIndex: 0]; // LEFT-TO-RIGHT OVERRIDE
//JNK				[newdata appendString: @"\u202c"]; // POP DIRECTIONAL FORMATTING
		} else if ((*i).m_direction == smil2::stw_rtlo) {
			lib::logger::get_logger()->debug("cocoa_smiltext: should do rtlo text");
//JNK				[newdata insertString: @"\u202e" atIndex: 0]; // RIGHT-TO-LEFT OVERRIDE
//JNK				[newdata appendString: @"\u202c"]; // POP DIRECTIONAL FORMATTING
		}
//JNK			[m_text_storage replaceCharactersInRange:newrange withString:newdata];

		// Prepare for setting the attribute info
//JNK			NSMutableDictionary *attrs = [[NSMutableDictionary alloc] init];
//JNK			newrange.length = [newdata length];
		// Find font info
//JNK			NSFont *text_font = NULL;
		std::vector<std::string>::const_iterator fi;
		for (fi=(*i).m_font_families.begin(); fi != (*i).m_font_families.end(); fi++) {
			AM_DBG lib::logger::get_logger()->debug("cocoa_smiltext: look for font '%s'", (*fi).c_str());
//JNK				text_font = _select_font((*fi).c_str(), (*i).m_font_style, (*i).m_font_weight, (*i).m_font_size);
//JNK				if (text_font) break;
			AM_DBG lib::logger::get_logger()->debug("cocoa_smiltext: not found, try next");
		}
//JNK			if (!text_font) {
//JNK				text_font = [NSFont userFontOfSize: (*i).m_font_size];
//JNK			}

//JNK			[attrs setValue:text_font forKey:NSFontAttributeName];

#ifdef JNK
		if (!(*i).m_transparent) {
			// Find color info
			double alfa = 1.0;
			const common::region_info *ri = m_dest->get_info();
			if (ri) alfa = ri->get_mediaopacity();
			NSColor *color = [NSColor colorWithCalibratedRed:redf((*i).m_color)
					green:greenf((*i).m_color)
					blue:bluef((*i).m_color)
					alpha:(float)alfa];
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
					alpha:(float)alfa];
			[attrs setValue:color forKey:NSBackgroundColorAttributeName];
		}
#endif // JNK
		// Finally do paragraph settings (which are cached)
		if (m_cur_paragraph_style == NULL ||
				m_cur_para_align != (*i).m_align ||
				m_cur_para_writing_mode != (*i).m_writing_mode ||
				m_cur_para_wrap != (*i).m_wrap) {
			// Delete the old one, if needed
//JNK				if (m_cur_paragraph_style)
//JNK					[m_cur_paragraph_style release];
			// Remember the values
			m_cur_para_align = (*i).m_align;
			m_cur_para_writing_mode = (*i).m_writing_mode;
			m_cur_para_wrap = (*i).m_wrap;
			// Allocate the new one
//JNK				NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
//JNK				m_cur_paragraph_style = [ps retain];
			// Set the paragraph writing direction
			if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
//JNK					[ps setBaseWritingDirection: NSWritingDirectionRightToLeft];
			} else {
				// All other directions are treated as left-to-right
//JNK					[ps setBaseWritingDirection: NSWritingDirectionLeftToRight];
			}
			if (m_params.m_mode != smil2::stm_crawl) {
				// Set the paragraph text alignment, unless we have moving text
				switch (m_cur_para_align) {
				case smil2::sta_start:
					if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
//JNK							[ps setAlignment: NSRightTextAlignment];
					} else {
						// All other directions are treated as left-to-right
//JNK							[ps setAlignment: NSLeftTextAlignment];
					}
					break;
				case smil2::sta_end:
					if (m_cur_para_writing_mode == smil2::stw_rl_tb) {
//JNK							[ps setAlignment: NSLeftTextAlignment];
					} else {
						// All other directions are treated as left-to-right
//JNK							[ps setAlignment: NSRightTextAlignment];
					}
					break;
				case smil2::sta_left:
//JNK						[ps setAlignment: NSLeftTextAlignment];
					break;
				case smil2::sta_right:
//JNK						[ps setAlignment: NSRightTextAlignment];
					break;
				case smil2::sta_center:
//JNK						[ps setAlignment: NSCenterTextAlignment];
					break;
				}
			}
			// Set the paragraph wrap option
//JNK				if (m_cur_para_wrap)
//JNK					[ps setLineBreakMode: NSLineBreakByWordWrapping];
//JNK				else
//JNK					[ps setLineBreakMode: NSLineBreakByClipping];
		}
//JNK			[attrs setValue:m_cur_paragraph_style forKey:NSParagraphStyleAttributeName];

		// Set the attributes
//JNK			[m_text_storage setAttributes:attrs range:newrange];

		i++;
	}
//JNK		[m_text_storage endEditing];
	m_engine.done();
#endif JNK
}

void
d2_smiltext_renderer::redraw_body(const rect &dirty, gui_window *window, ID2D1RenderTarget* rt)
{
	recreate_d2d();

	m_lock.enter();
	// XXXJACK: need to re-create if xywh has changed
	if (!m_text_layout || !m_text_format || !m_brush) {
		m_lock.leave();
		return;
	}
	assert(rt);
	if (rt == NULL)
		return;
	rect destrect = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("d2_smiltext_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, destrect.left(), destrect.top(), destrect.right(), destrect.bottom());

	destrect.translate(m_dest->get_global_topleft());

	D2D1_POINT_2F origin = { (float) destrect.left(), (float) destrect.top() };
	rt->DrawTextLayout(origin, m_text_layout, m_brush);

#ifdef JNK
	NSRect cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];
	NSPoint visible_origin = NSMakePoint(NSMinX(cocoa_dstrect), NSMinY(cocoa_dstrect));
	NSSize visible_size = NSMakeSize(NSWidth(cocoa_dstrect), NSHeight(cocoa_dstrect));

// Determine text container layout size. This depends on the type of container.
#define INFINITE_WIDTH 1000000
#define INFINITE_HEIGHT 1000000
	NSSize layout_size = visible_size;
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
		[m_text_container release]; // The layoutManager will retain the textContainer
		[m_text_storage addLayoutManager:m_layout_manager];
		[m_layout_manager release]; // The textStorage will retain the layoutManager
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
	// If we do auto-scrolling we should now layout the text, so we can determine the rate
	if (m_engine.is_auto_rate()) {
		(void)[m_layout_manager glyphRangeForTextContainer: m_text_container];
		NSSize nsfull_size = [m_layout_manager usedRectForTextContainer: m_text_container].size;
		lib::size full_size((int)nsfull_size.width, (int)nsfull_size.height);
		unsigned int dur = 11; // XXXX
		smil2::smiltext_align align = m_cur_para_align;
		unsigned int rate = _compute_rate(align, full_size, r, dur);
		m_engine.set_rate(rate);
		m_params = m_engine.get_params();
	}
	// Next compute the layout position of what we want to draw at visible_origin
	NSPoint logical_origin = NSMakePoint(0, 0);
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
	AM_DBG logger::get_logger()->debug("d2_smiltext_renderer.redraw at Cocoa-point (%f, %f) logical (%f, %f)", visible_origin.x, visible_origin.y, logical_origin.x, logical_origin.y);
	if (m_render_offscreen) {
	}
	// Now we need to determine which glyphs to draw. Unfortunately glyphRangeForBoundingRect gives us
	// full lines (which is apparently more efficient, google for details) which is not good enough
	// for ticker tape, so we adjust.
	NSRect logical_rect = NSMakeRect(logical_origin.x, logical_origin.y, visible_size.width, visible_size.height);
	NSRange glyph_range = [m_layout_manager glyphRangeForBoundingRect: logical_rect inTextContainer: m_text_container];
	AM_DBG NSLog(@"Glyph range was %d, %d, origin-x %f", glyph_range.location, glyph_range.length, logical_origin.x);
	if (glyph_range.location >= 0 && glyph_range.length > 0) {
		if (m_any_semiopaque_bg) {
			// Background opacity 1.0 is implemented correctly in NSLayoutManager, but intermediate
			// values are a bit funny: they still override the underlying image (i.e. they don't
			// use NSCompositeSourceOver or something similar. Therefore, if this is the case we
			// draw the background color to a separate buffer and bitblit this onto the existing
			// bits.
			NSImage *tmpsrc = [view getTransitionTmpSurface];
			[tmpsrc lockFocus];
			[[NSColor colorWithDeviceWhite: 1.0f alpha: 0.0f] set];
			NSRectFill(cocoa_dstrect);
			[m_layout_manager drawBackgroundForGlyphRange: glyph_range atPoint: visible_origin];
			[tmpsrc unlockFocus];
			[tmpsrc drawInRect: cocoa_dstrect fromRect: cocoa_dstrect operation: NSCompositeSourceOver fraction: 1.0f];
		} else {
			// Otherwise we simply let NSLayoutManager do the work
			[m_layout_manager drawBackgroundForGlyphRange: glyph_range atPoint: visible_origin];
		}
		[m_layout_manager drawGlyphsForGlyphRange: glyph_range atPoint: visible_origin];
	}
	layout_size = [m_text_container containerSize];
	if (m_render_offscreen) {
	}
	[view unlockFocus];

#endif // JNK
	m_lock.leave();
}

void
d2_smiltext_renderer::recreate_d2d()
{
	if (m_brush) return;
	m_lock.enter();
	HRESULT hr = S_OK;
	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	assert(rt);

	double alfa = 1.0;
	const common::region_info *ri = m_dest->get_info();
	if (ri) alfa = ri->get_mediaopacity();
	hr = rt->CreateSolidColorBrush(D2D1::ColorF(redf(m_text_color), greenf(m_text_color), bluef(m_text_color), alfa), &m_brush);
	if (!SUCCEEDED(hr)) lib::logger::get_logger()->trace("CreateSolidColorBrush: error 0x%x", hr);
	m_lock.leave();
}


void
d2_smiltext_renderer::discard_d2d()
{
#ifdef JNK
	if (m_d2bitmap) {
//		m_d2bitmap->Release();
		m_d2bitmap = NULL;
	}
#endif // JNK
}


unsigned int
d2_smiltext_renderer::_compute_rate(smil2::smiltext_align align, lib::size size, lib::rect r,  unsigned int dur) {
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

} // namespace d2

} // namespace gui

} //namespace ambulant
