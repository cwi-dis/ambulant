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

// "infinite" width or height for text layout
#define INFINITE_SIZE 1000000

namespace ambulant {

using namespace lib;

namespace gui {

namespace d2 {

IDWriteFontCollection *s_font_collection = NULL;

// Per-range parameter storage helper class.
class d2_range_params
{
public:
	d2_range_params(int begin_, int end_, const smil2::smiltext_run& run);
	~d2_range_params();
	void apply(IDWriteTextLayout *engine);
	void apply_colors(IDWriteTextLayout *engine, ID2D1RenderTarget* rt, D2D1_POINT_2F origin);
	void discard_d2d();
	int begin;
	int end;
	DWRITE_FONT_WEIGHT weight;
	DWRITE_FONT_STYLE style;
	float fontsize;
	std::wstring fontfamily;
	D2D1::ColorF fgcolor;
	ID2D1SolidColorBrush *fgbrush;
	D2D1::ColorF bgcolor;
	bool bgtransparent;
	ID2D1SolidColorBrush *bgbrush;
};

d2_range_params::d2_range_params(int begin_, int end_, const smil2::smiltext_run& run)
:	begin(begin_),
	end(end_),
	weight(DWRITE_FONT_WEIGHT_NORMAL),
	style(DWRITE_FONT_STYLE_NORMAL),
	fontsize((float)run.m_font_size),
	fontfamily(L""),
	fgcolor(D2D1::ColorF(redf(run.m_color), greenf(run.m_color), bluef(run.m_color))),
	fgbrush(NULL),
	bgcolor(D2D1::ColorF(redf(run.m_bg_color), greenf(run.m_bg_color), bluef(run.m_bg_color))),
	bgtransparent(run.m_bg_transparent),
	bgbrush(NULL)
{
	if (run.m_font_weight == smil2::stw_bold)
		weight = DWRITE_FONT_WEIGHT_BOLD;

	if (run.m_font_style == smil2::sts_italic)
		style = DWRITE_FONT_STYLE_ITALIC;
	else if (run.m_font_style == smil2::sts_oblique)
		style = DWRITE_FONT_STYLE_OBLIQUE;

	std::vector<std::string>::const_iterator i;
	for (i=run.m_font_families.begin(); i!=run.m_font_families.end(); i++) {
		lib::textptr familyname((*i).c_str());
		// Cater for generic type names
		if (*i == "serif")
			familyname = "Times New Roman";
		else if (*i == "monospace")
			familyname = "Courier New";
		else if (*i == "sansSerif")
			familyname = "Arial";
		UINT32 dummy;
		BOOL exists = false;
		(void)s_font_collection->FindFamilyName(familyname.c_wstr(), &dummy, &exists);
		if (exists) {
			fontfamily = familyname.c_wstr();
			break;
		}
	}
}

d2_range_params::~d2_range_params()
{
	if (fgbrush)
		fgbrush->Release();
	if (bgbrush)
		bgbrush->Release();
}

void
d2_range_params::apply(IDWriteTextLayout *engine)
{
	DWRITE_TEXT_RANGE range = {begin, end};

	HRESULT hr = engine->SetFontWeight(weight, range);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("SMILText: SetFontWeight: error 0x%x", hr);
	}

	hr = engine->SetFontStyle(style, range);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("SMILText: SetFontStyle: error 0x%x", hr);
	}

	hr = engine->SetFontSize(fontsize, range);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("SMILText: SetFontSize: error 0x%x", hr);
	}

	if (fontfamily != L"") {
		hr = engine->SetFontFamilyName(fontfamily.c_str(), range);
		if (!SUCCEEDED(hr)) {
			lib::logger::get_logger()->trace("SMILText: SetFontFamilyName: error 0x%x", hr);
		}
	}
}

	
void
d2_range_params::apply_colors(IDWriteTextLayout *engine, ID2D1RenderTarget* rt, D2D1_POINT_2F origin)
{
	DWRITE_TEXT_RANGE range = {begin, end};

	HRESULT hr;
	// Apparently, a SetDrawingEffect will not only affect the given range, but also
	// any characters after it (despite what the MS documentation states). For this
	// reason we create a brush for every single range.
	if (/*fgcolor &&*/ fgbrush == NULL) {
		hr = rt->CreateSolidColorBrush(fgcolor, &fgbrush);
		if (!SUCCEEDED(hr)) lib::logger::get_logger()->trace("CreateSolidColorBrush: error 0x%x", hr);

	}
	if (fgbrush) {
		hr = engine->SetDrawingEffect(fgbrush, range);
		if (!SUCCEEDED(hr)) {
			lib::logger::get_logger()->trace("SMILText: SetDrawingEffect: error 0x%x", hr);
		}
	}
	
	// Should we draw background boxes?
	if (bgtransparent) return;
	if (/*fgcolor &&*/ bgbrush == NULL) {
		hr = rt->CreateSolidColorBrush(bgcolor, &bgbrush);
		if (!SUCCEEDED(hr)) lib::logger::get_logger()->trace("CreateSolidColorBrush: error 0x%x", hr);

	}
	if (bgbrush == NULL) return;
	DWRITE_HIT_TEST_METRICS boxes[10];
	UINT32 nboxes;
	hr = engine->HitTestTextRange(begin, end-begin, origin.x, origin.y, boxes, 10, &nboxes);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("SMILText: HitTestTextRange: error 0x%x", hr);
		return;
	}
	UINT32 i;
	for(i=0; i<nboxes; i++) {
		D2D1_RECT_F rect = {
			boxes[i].left, 
			boxes[i].top, 
			boxes[i].left+boxes[i].width, 
			boxes[i].top+boxes[i].height};
		rt->FillRectangle(rect, bgbrush);
	}
}

void
d2_range_params::discard_d2d()
{
	if (fgbrush) fgbrush->Release();
	fgbrush = NULL;
	if (bgbrush) bgbrush->Release();
	bgbrush = NULL;
}

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
	HRESULT hr;
	if (s_write_factory == NULL) {
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(s_write_factory),
			reinterpret_cast<IUnknown**>(&s_write_factory));
		if (!SUCCEEDED(hr)) {
			lib::logger::get_logger()->error("Cannot create DirectWrite factory: error 0x%x", hr);
		}
	}
	if (s_font_collection == NULL && s_write_factory) {
		hr = s_write_factory->GetSystemFontCollection(&s_font_collection, false);
		if (!SUCCEEDED(hr)) {
			lib::logger::get_logger()->error("Cannot GetSystemFontCollection: error 0x%x", hr);
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
		lib::textptr fn = params->get_str("font-family");
		if (fn != (lib::textptr::const_char_ptr) NULL) {
			font_name = fn;
		}
		if (fn != (lib::textptr::const_char_ptr) NULL) {
			font_name = fn;
		}
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
	_discard_range_params();
	m_lock.leave();
}

void
d2_smiltext_renderer::_discard_range_params()
{
	std::vector<d2_range_params*>::iterator i;
	for (i=m_range_params.begin(); i != m_range_params.end(); i++) {
		// Clear out any objects
		delete *i;
	}
	m_range_params.clear();
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
	AM_DBG lib::logger::get_logger()->debug("d2_smiltext_renderer::smiltext_changed");
	m_engine.lock();
	if (_collect_text())
		_recreate_layout();
	bool finished = m_engine.is_finished();
	m_engine.unlock();
	if (finished)
		m_context->stopped(m_cookie);
	if (m_dest != NULL) {
		m_dest->need_redraw();
	}
	m_lock.leave();
}

bool
d2_smiltext_renderer::_collect_text()
{
	// XXXJACK: should also recreate if we cleared out our parameters
	if (!m_engine.is_changed()) return false;
	m_data = L"";
	lib::xml_string newdata;
	m_run_begins.clear();
	_discard_range_params();
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
				newdata = "";
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
#pragma setlocale("C")
		if ((*i).m_direction == smil2::stw_ltro) {
			lib::logger::get_logger()->debug("cocoa_smiltext: should do ltro text");
			newdata = "\u202d" + newdata + "\u202c";
		} else if ((*i).m_direction == smil2::stw_rtlo) {
			lib::logger::get_logger()->debug("cocoa_smiltext: should do rtlo text");
			newdata = "\u202e" + newdata + "\u202c";
		}
		lib::textptr convert(newdata.c_str());
		const wchar_t *wnewdata = convert.c_wstr();
		int oldpos = m_data.length();
		m_data = m_data + wnewdata;
		int newpos = m_data.length();
		m_run_begins.push_back(m_data.length());
		if (oldpos != newpos) {
			d2_range_params *p = new d2_range_params(oldpos, newpos, *i);
			m_range_params.push_back(p);
		}
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

	// Set width (or height) to pretty much infinite for crawl or scroll
	if (m_params.m_mode == smil2::stm_crawl) {
		w = INFINITE_SIZE;
	}

	if (m_params.m_mode == smil2::stm_scroll) {
		h = INFINITE_SIZE;
	}

	HRESULT hr;
	if (s_write_factory == NULL) return;
	hr = s_write_factory->CreateTextLayout(m_data.c_str(), m_data.length(), m_text_format, w, h, &m_text_layout);
	if (!SUCCEEDED(hr)) {
		lib::logger::get_logger()->trace("d2_smiltext: Cannot CreateTextLayout: error 0x%x", hr);
		return;
	}
	// Set the parameters
	std::vector<d2_range_params *>::iterator i;
	for (i=m_range_params.begin(); i != m_range_params.end(); i++) {
		(*i)->apply(m_text_layout);
	}

	if (m_engine.is_auto_rate()) {
		DWRITE_TEXT_METRICS textMetrics;
		m_text_layout->GetMetrics(&textMetrics);
		unsigned int dur = m_engine.get_dur();
		smil2::smiltext_align align = m_cur_para_align;
		lib::size full_size((int)textMetrics.width, (int)textMetrics.height);
		unsigned int rate = _compute_rate(align, full_size, m_dest->get_rect(), dur);
		m_engine.set_rate(rate);
	}
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

	// Find where we should draw things, taking crawl/scroll into account
	D2D1_POINT_2F visible_origin = { (float) destrect.left(), (float) destrect.top() };
	D2D1_POINT_2F logical_origin = visible_origin;
	if (m_params.m_mode == smil2::stm_crawl) {
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		logical_origin.x -= float(now * m_params.m_rate / 1000);
	}
	if (m_params.m_mode == smil2::stm_scroll) {
		double now = m_event_processor->get_timer()->elapsed() - m_epoch;
		logical_origin.y -= float(now * m_params.m_rate / 1000);
	}

	AM_DBG lib::logger::get_logger()->debug("d2_smiltext_renderer::redraw_body: visible (%f, %f) logical (%f, %f)", visible_origin.x, visible_origin.y, logical_origin.x, logical_origin.y);
	// Set the color parameters
	std::vector<d2_range_params *>::iterator i;
	for (i=m_range_params.begin(); i != m_range_params.end(); i++) {
		(*i)->apply_colors(m_text_layout, rt, logical_origin);
	}
	// Draw the whole thing, clipped w.r.t. the destination rectangle
	D2D1_RECT_F cliprect =  D2D1::RectF((float) destrect.left(),(float) destrect.top(),(float)  destrect.right(),(float) destrect.bottom());
	rt->PushAxisAlignedClip(cliprect, D2D1_ANTIALIAS_MODE_ALIASED);
	rt->DrawTextLayout(logical_origin, m_text_layout, m_brush);
	rt->PopAxisAlignedClip();

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
	hr = rt->CreateSolidColorBrush(D2D1::ColorF(redf(m_text_color), greenf(m_text_color), bluef(m_text_color), (float)alfa), &m_brush);
	if (!SUCCEEDED(hr)) lib::logger::get_logger()->trace("CreateSolidColorBrush: error 0x%x", hr);
	rt->Release();
	m_lock.leave();
}

void
d2_smiltext_renderer::discard_d2d()
{
	if (m_brush) m_brush->Release();
	m_brush = NULL;
	std::vector<d2_range_params*>::iterator i;
	for (i=m_range_params.begin(); i != m_range_params.end(); i++) {
		// Clear out any objects
		(*i)->discard_d2d();
	}

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
