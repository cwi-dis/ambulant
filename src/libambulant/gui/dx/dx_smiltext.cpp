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

#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_smiltext.h"
#include "ambulant/gui/dx/dx_transition.h"
#include "ambulant/common/region_info.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/smil2/params.h"
#include "ambulant/smil2/test_attrs.h"
#include "ambulant/common/factory.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/colors.h"

/* windows includes begin*/
#include <ddraw.h>
#include <windows.h>
#include <tchar.h>
/* windows includes end*/

#ifdef _UNICODE
#define STR_TO_TSTR(s) ambulant::lib::textptr(s).c_wstr()
#else
#define STR_TO_TSTR(s) (s)
#endif

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_report_last_error;
using ambulant::lib::logger;

using namespace ambulant;
extern const char dx_smiltext_playable_tag[] = "smilText";
extern const char dx_smiltext_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirectX");
extern const char dx_smiltext_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererDirectXSmilText");
extern const char dx_smiltext_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererSmilText");

common::playable_factory *
gui::dx::create_dx_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectX"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectXSmilText"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererSmilText"), true);
	return new common::single_playable_factory<
		gui::dx::dx_smiltext_renderer,
		dx_smiltext_playable_tag,
		dx_smiltext_playable_renderer_uri,
		dx_smiltext_playable_renderer_uri2,
		dx_smiltext_playable_renderer_uri3 >(factory, mdp);
}

#pragma warning(push)
#pragma warning(disable:4355)
gui::dx::dx_smiltext_renderer::dx_smiltext_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories* factory,
	common::playable_factory_machdep *dxplayer)
:	dx_renderer_playable(context, cookie, node, evp, factory, dynamic_cast<dx_playables_context*>(dxplayer)),
	m_context(context),
	m_size(0,0),
	m_hdc(NULL),
	m_viewport(NULL),
	m_bgopacity(1.0),
	m_region_dds(NULL),
	m_df(factory->get_datasource_factory()),
	m_layout_engine(smil2::smiltext_layout_engine(node, evp, this, this, true))
{
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer(0x%x)", this);
}
#pragma warning(pop)
void
gui::dx::dx_smiltext_renderer::set_surface(common::surface *dest) {
	m_lock.enter();
	m_dest = dest;

	lib::rect rc = dest->get_rect();
	lib::size bounds(rc.width(), rc.height());
	m_size = bounds;
	dx_window *dxwindow = static_cast<dx_window*>(m_dest->get_gui_window());
	m_viewport = dxwindow->get_viewport();
	m_lock.leave();
}

gui::dx::dx_smiltext_renderer::~dx_smiltext_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~dx_smiltext_renderer(0x%x), m_region_dds=0x%x, m_hdc=0x%x", this, m_region_dds, m_hdc);
	m_lock.enter();

	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;

	if (m_region_dds) {
		if (m_hdc) {
			HRESULT hr = m_region_dds->ReleaseDC(m_hdc);
			if (hr != DD_OK)
			if (m_region_dds->ReleaseDC(m_hdc) == 0)
				lib::logger::get_logger()->warn("~dx_smiltext_renderer(0x%x): ReleaseDC(m_hdc=0x%x) fails, code %d", this, m_hdc, hr);
			m_hdc = NULL;
		}
		m_viewport->release_surface(m_region_dds);
		m_region_dds = NULL;
	}
	m_lock.leave();
}

void
gui::dx::dx_smiltext_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::start(0x%x)", this);

	m_lock.enter();
	m_layout_engine.start(t);
	m_layout_engine.set_dest_rect(m_dest->get_rect());
	renderer_playable::start(t);
	m_lock.leave();
}

bool
gui::dx::dx_smiltext_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::stop(0x%x)", this);
	m_lock.enter();
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	m_dxplayer->stopped(this);
	m_lock.leave();
	return true;
}

void
gui::dx::dx_smiltext_renderer::marker_seen(const char *name)
{
	m_lock.enter();
	m_context->marker_seen(m_cookie, name);
	m_lock.leave();
}

void
gui::dx::dx_smiltext_renderer::smiltext_changed() {
	assert(m_hdc == NULL);
	m_hdc = CreateCompatibleDC(m_hdc);
	if (m_hdc == NULL) {
		win_report_error("dx_smiltext_changed()::CreateCompatibleDC()", GetLastError());
		return;
	}
	m_layout_engine.smiltext_changed();
	if ( ! DeleteDC(m_hdc)) {
		lib::logger::get_logger()->warn("gui::dx::dx_smiltext_renderer::redraw(): DeleteDC failed");
	}
	m_hdc = NULL;
	m_dest->need_redraw();
}

smil2::smiltext_metrics
gui::dx::dx_smiltext_renderer::get_smiltext_metrics(const smil2::smiltext_run& run) {
	unsigned int ascent = 0, descent = 0, height = 0, width = 0, line_spacing = 0, word_spacing = 0;
	SIZE SZ;
	HGDIOBJ old_obj = NULL;
	HFONT font = NULL;
	if (run.m_data.length() != 0) {

		old_obj = _dx_smiltext_set_font (run, m_hdc, &font);

		TEXTMETRIC tm;
		BOOL res = ::GetTextMetrics(m_hdc, &tm);
		if (res == 0)
			win_report_last_error("GetTextMetric()");
		else {
			ascent	= tm.tmAscent;
			descent = tm.tmDescent;
			height	= tm.tmHeight+tm.tmExternalLeading;
			line_spacing = height+tm.tmInternalLeading+tm.tmExternalLeading;

			lib::textptr tp(run.m_data.c_str());
			LPCTSTR ttp = tp;
			res = ::GetTextExtentPoint32(m_hdc, ttp, _tcslen(ttp), &SZ);
			if (res == 0)
				win_report_last_error("GetTextExtentPoint32()");
			else width = SZ.cx;
		}
	}
	if (font)
		::DeleteObject(font);
	if (old_obj)
		::SelectObject(m_hdc, old_obj);
	return smil2::smiltext_metrics(ascent, descent, height, width, line_spacing);
}

const lib::rect&
gui::dx::dx_smiltext_renderer::get_rect() {
	return m_dest->get_rect();
}
/*
	Operation: first, called from redraw(), _dx_smiltext_get_ddsurf() copies the
	background area in m_region_dds. Then redraw() calls m_layout_engine->redraw().
	From this, for each string (word) in a smiltext_run, if textBackgroundColor/
	mediaBackgroundOpacity is set, an aditional textbg_dds is created and filled
	with textBackgroundColor. Similarly, if textColor/mediaOpacity is set, an
	additional text_dds is created. Text is drawn in both text_dds/textbg dds.
	Then first textbg_dds is blended with m_region_dds, next textbg_dds is blended
	with m_region_dds, in both cases only using the text[background]Color with
	the desired opacity for the blend, keeping the original pixels in m_region_dds
	when not blending them.
	Finally, after all smiltext_run items are processed in this way, redraw() blits
	m_region_dds to the viewport area (screen).
*/
void
gui::dx::dx_smiltext_renderer::render_smiltext(const smil2::smiltext_run& run, const lib::rect& r) {
	if (run.m_command != smil2::stc_data)
		return;
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_render(): command=%d data=%s color=0x%x",run.m_command,run.m_data.c_str()==NULL?"(null)":run.m_data.c_str(),run.m_color);

	double alpha_media = 1.0, alpha_media_bg = 1.0, alpha_chroma = 1.0;
	lib::color_t chroma_low = lib::color_t(0x000000), chroma_high = lib::color_t(0xFFFFFF);
	const common::region_info *ri = m_dest->get_info();
	if (ri) {
		alpha_media = ri->get_mediaopacity();
		alpha_media_bg = ri->get_mediabgopacity();
		m_bgopacity = ri->get_bgopacity();
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			lib::compute_chroma_range(chromakey, chromakeytolerance, &chroma_low, &chroma_high);
		}
	}
	IDirectDrawSurface* text_dds = NULL;
	IDirectDrawSurface* textbg_dds = NULL;
	HRESULT hr = S_OK;
	HDC hdc = m_hdc;
	HDC textbg_hdc = NULL;
	HBITMAP hbm = NULL;
	HGDIOBJ old_bm = NULL;
	// White is always transparent; set the color key
	DWORD ddTranspColorWhite = m_viewport->convert(RGB(255,255,255));
	DWORD ddTranspColorBg = ddTranspColorWhite;
	DWORD dwFlags = DDCKEY_SRCBLT;
	DDCOLORKEY ck;
	ck.dwColorSpaceLowValue = ddTranspColorWhite;
	ck.dwColorSpaceHighValue = ddTranspColorWhite;
	bool blending = false;
	lib::rect rr(r);
	if ( ! (alpha_media == 1.0 && alpha_media_bg == 1.0 && alpha_chroma == 1.0) ) {
		// prepare for blending
		if ( ! run.m_bg_transparent) {
			textbg_dds = m_viewport->create_surface();
			assert(textbg_dds);
			m_viewport->clear_surface(textbg_dds, run.m_bg_color, 1.0);
			if (FAILED(hr = textbg_dds->GetDC(&textbg_hdc))
				|| FAILED(hr = textbg_dds->SetColorKey(dwFlags, &ck))) {
				win_report_last_error("dx_smiltext_render: textbg GetDC()/SetColorKey()");
			}
		}
		text_dds = m_viewport->create_surface();
		assert(text_dds);
		hr = text_dds->GetDC(&hdc);
		if (SUCCEEDED(hr)) {
			hbm = CreateCompatibleBitmap(hdc, rr.width(), rr.height());
		} else {
			win_report_last_error("dx_smiltext_render: GetDC()");
			hdc = m_hdc;
		}
		if ( hbm && SUCCEEDED(hr)) {
			old_bm = ::SelectObject(hdc,  (HGDIOBJ) hbm);
			blending = true;
		} else {
			win_report_last_error("dx_smiltext_render: CreateCompatibleBitmap()");
			hdc = m_hdc;
		}

		if ( ! blending)
			alpha_media = alpha_chroma = 1.0;
	}
	// set the foreground color
	COLORREF old_color = dxparams::I()->invalid_color();
	COLORREF old_textbg_color = dxparams::I()->invalid_color();
	lib::color_t fg_color_t = (run.m_color == dxparams::I()->invalid_color())?
						::GetSysColor(COLOR_WINDOWTEXT) : run.m_color;
	// Transparency needs a bit of work, because of support for non-alpha
	// bitmaps (where we use a special color).
	if (run.m_transparent)
		fg_color_t = dxparams::I()->transparent_color();
	else if (fg_color_t == dxparams::I()->transparent_color())
		fg_color_t = dxparams::I()->transparent_replacement_color();
	COLORREF crTextColor = fg_color_t;

	// set the background color
	COLORREF old_bgcolor = dxparams::I()->invalid_color();
	COLORREF old_textbg_bgcolor = dxparams::I()->invalid_color();
	lib::color_t bg_color_t =
		(run.m_bg_color == dxparams::I()->invalid_color()) ?
			::GetSysColor(COLOR_WINDOW) :
				(run.m_bg_color == dxparams::I()->transparent_color())?
					dxparams::I()->transparent_replacement_color() :
					run.m_bg_color;
	if (run.m_bg_transparent)
		bg_color_t = dxparams::I()->transparent_color();
	else if (fg_color_t == dxparams::I()->transparent_color())
		fg_color_t = dxparams::I()->transparent_replacement_color();
	COLORREF crBkColor = bg_color_t;

	if (ri && ri->is_chromakey_specified()) {
		if (lib::color_t_in_range (fg_color_t, chroma_low, chroma_high))
			alpha_media = alpha_chroma;
		if (lib::color_t_in_range (bg_color_t, chroma_low, chroma_high))
			alpha_media_bg = alpha_chroma;
	}
	if (blending) {
		// on text surface, draw text in required color
		// and background in transparent color
		old_color = ::SetTextColor(hdc, crTextColor);
		if (old_color != dxparams::I()->invalid_color())
			old_bgcolor = ::SetBkColor(hdc, dxparams::I()->transparent_color());
		// on background surface, draw text in transparent color
		// and background in required color
		if (old_bgcolor != dxparams::I()->invalid_color())
			old_textbg_color = ::SetTextColor(textbg_hdc, dxparams::I()->transparent_color());
		if (old_textbg_color != dxparams::I()->invalid_color())
			old_textbg_bgcolor = ::SetBkColor(textbg_hdc, crBkColor);
	} else {
		old_color = ::SetTextColor(hdc, crTextColor);
		if (old_color != dxparams::I()->invalid_color())
		old_bgcolor = ::SetBkColor(hdc, crBkColor);
	}
	if (old_color == dxparams::I()->invalid_color())
		win_report_last_error("::SetTextColor");
	else if (old_bgcolor == dxparams::I()->invalid_color())
		win_report_last_error("::SetBkColor");
	else if (blending) {
		if (old_textbg_color == dxparams::I()->invalid_color())
			win_report_last_error("::SetTextColor(background)");
		else if (old_textbg_bgcolor == dxparams::I()->invalid_color())
			win_report_last_error("::SetBkColor(background)");
	}
	// set the font
	HFONT font = NULL;
	HGDIOBJ old_font = _dx_smiltext_set_font(run, hdc, &font);
	HGDIOBJ old_textbg_font = NULL;
	if (textbg_hdc)
		old_textbg_font = ::SelectObject(textbg_hdc, font);

	// draw the text
	const char* text = run.m_data.c_str();
	lib::textptr tp(text);
	RECT dstRC;
	dstRC.left	 = rr.left();
	dstRC.top	 = rr.top();
	dstRC.right	 = rr.right();
	dstRC.bottom = rr.bottom();
	UINT uFormat = DT_NOPREFIX | DT_LEFT;
	LPCTSTR ttp = tp;
	int ttplen = _tcslen(ttp);
	hr = ::DrawText(hdc, ttp, ttplen, &dstRC, uFormat);
	if (SUCCEEDED(hr) && textbg_hdc)
		hr = ::DrawText(textbg_hdc, ttp, ttplen, &dstRC, uFormat);
	if(FAILED(hr))
		win_report_last_error("DrawText()");
	::SelectObject(hdc, old_font);
	if (font && ! ::DeleteObject(font))
		win_report_error("DeleteObject(font)", DDERR_GENERIC);


	if (text_dds)
		hr = text_dds->SetColorKey(dwFlags, &ck);
	if (SUCCEEDED(hr))
		hr = m_region_dds->SetColorKey(dwFlags, &ck);
	if (FAILED(hr)) {
		win_report_error("SetColorKey()", hr);
	}
	if (blending) {
		hr = m_region_dds->ReleaseDC(m_hdc);
		hr = text_dds->ReleaseDC(hdc);
		if (textbg_hdc)
			hr = textbg_dds->ReleaseDC(textbg_hdc);
		if (SUCCEEDED(hr)) {
			if ( ! run.m_bg_transparent) {
				m_viewport->blend_surface(
					m_region_dds,
					rr,
					textbg_dds,
					rr,
					/*use color key from source*/true,
					alpha_media_bg,
					/*don't touch out of range pixels*/0,
					bg_color_t,
					bg_color_t);
			}
			m_viewport->blend_surface(
				m_region_dds,
				rr,
				text_dds,
				rr,
				/*use color key from source*/true,
				alpha_media,
				/*do't touch out of range pixels*/0,
				fg_color_t,
				fg_color_t);
			hr = m_region_dds->GetDC(&m_hdc);
			if (SUCCEEDED(hr))
				hr = text_dds->GetDC(&hdc);
		}
		if (FAILED(hr)) {
			win_report_error("dx_smiltext_render: ReleaseDC()/GetDC()", hr);
		}
	}
	// reset the text color
	if( old_color != dxparams::I()->invalid_color()) {
		::SetTextColor(hdc, old_color);
	}
	// reset the background color
	if( old_bgcolor != dxparams::I()->invalid_color()) {
		::SetBkColor(hdc, old_bgcolor);
	}
	if (text_dds) {
		hr = text_dds->ReleaseDC(hdc);
		if (FAILED(hr))
			win_report_last_error("dx_smiltext_render: ReleaseDC(hdc)");
	}
	if (old_bm)
		::DeleteObject(hbm);
	if (text_dds) {
		m_viewport->release_surface(text_dds);
	}
	if (textbg_dds) {
		m_viewport->release_surface(textbg_dds);
	}
}
void
gui::dx::dx_smiltext_renderer::smiltext_stopped() {
	m_context->stopped(m_cookie);
}

HGDIOBJ
gui::dx::dx_smiltext_renderer::_dx_smiltext_set_font(const smil2::smiltext_run run, HDC hdc, HFONT* hfontp) {
	DWORD family = FF_DONTCARE | DEFAULT_PITCH;
	const char *fontname = run.m_font_families[0].c_str();
	int adjust_height = 0;
	if (hfontp == NULL)
		return NULL;
	if (fontname) {
		if (strcmp(fontname, "serif") == 0) {
			family = FF_ROMAN | VARIABLE_PITCH;
			fontname = NULL;
			switch (run.m_font_size) {
			/* KB Some font/size combinations look ugly without slight adjustment */
			case 12:
			case 16:
				adjust_height = -1;
				break;
			default:
				break;
			}
		} else if (strcmp(fontname, "sansSerif") == 0) {
			family = FF_SWISS | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(fontname, "monospace") == 0) {
			family = FF_DONTCARE | FIXED_PITCH;
			fontname = NULL;
		} else if (strcmp(fontname, "cursive") == 0) {
			family = FF_SCRIPT | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(fontname, "fantasy") == 0) {
			family = FF_DECORATIVE | VARIABLE_PITCH;
			fontname = NULL;
		}
	}
	int weight;
	switch(run.m_font_weight) {
		default:
		case smil2::stw_normal:
			weight = FW_NORMAL;
			break;
		case smil2::stw_bold:
			weight = FW_BOLD;
			break;
	}
	DWORD italic;
	switch(run.m_font_style) {
		default:
		case smil2::sts_normal:
			italic = false;
			break;
		case smil2::sts_italic:
		case smil2::sts_oblique:
		case smil2::sts_reverse_oblique:
		// no (reverse) oblique fonts available in Windows GDI.
		// For an implementation, see:
		// http://www.codeproject.com/useritems/oblique_txt.asp
			italic = true;
			break;
	}
	*hfontp = ::CreateFont(
			-(int)run.m_font_size-adjust_height,	// height of font
			0,					// average character width
			0,					// angle of escapement
			0,					// base-line orientation angle
			weight,				// font weight
			italic,				// italic attribute option
			0,					// underline attribute option
			0,					// strikeout attribute option
			DEFAULT_CHARSET,		// character set identifier
			OUT_DEFAULT_PRECIS, // output precision
			CLIP_DEFAULT_PRECIS, // clipping precision
			DEFAULT_QUALITY,	// output quality
			family,				// pitch and family
			STR_TO_TSTR(fontname)); // typeface name
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_run_set_attr(0x%x): m_data=%s font=0x%x, m_font_size=%d,weight=0x%x,italic=%d,family=0x%x,run.m_font_families[...]=%s",this,run.m_data.c_str(),*hfontp,run.m_font_size,weight,italic,family,fontname);
	return ::SelectObject(hdc, *hfontp);
}

bool
gui::dx::dx_smiltext_renderer::user_event(const lib::point& pt, int what) {
	if (!user_event_sensitive(pt)) return false;
	m_lock.enter();
	if(what == common::user_event_click) {
		m_context->clicked(m_cookie);
	}
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	m_lock.leave();
	return true;
}

void
gui::dx::dx_smiltext_renderer::redraw(const lib::rect& dirty, common::gui_window *window) {
	m_lock.enter();
	// Get the top-level surface
	_dx_smiltext_get_ddsurf(window);
	if ( ! (m_viewport && m_region_dds)) {
		m_lock.leave(); // give up
		return;
	}
	dx_window *dxwindow = static_cast<dx_window*>(window);
	if (m_hdc == NULL) {
		HRESULT hr = m_region_dds->GetDC(&m_hdc);
		if (FAILED(hr)) {
			win_report_error("DirectDrawSurface::GetDC()", hr);
			return;
		}
	}

	m_layout_engine.redraw(dirty);
	if (FAILED(m_region_dds->ReleaseDC(m_hdc))) {
		lib::logger::get_logger()->warn("gui::dx::dx_smiltext_renderer::redraw(): ReelaseDC failed");
	}
	m_hdc = NULL;

	lib::rect smiltext_rc = dirty;
	lib::rect reg_rc = dirty;
	// Translate smiltext region dirty rect. to viewport coordinates
	lib::point pt = m_dest->get_global_topleft();
	reg_rc.translate(pt);

	dx_transition *tr = get_transition();
	if (tr && tr->is_fullscreen()) {
		m_viewport->set_fullscreen_transition(tr);
		tr = NULL;
	}

	// Finally blit/blend m_ddsurf to viewport
	m_viewport->draw(m_region_dds, smiltext_rc, reg_rc, true, tr);

	if (m_erase_never) m_dest->keep_as_background();
	bool finished = m_layout_engine.is_finished();
	m_lock.leave();
	if (finished)
		m_context->stopped(m_cookie);
}

void
gui::dx::dx_smiltext_renderer::_dx_smiltext_get_ddsurf(common::gui_window *window) {
	assert(m_dest && m_size != lib::size(0,0));
	lib::rect src_rc(lib::point(0,0), m_size);
	lib::rect dst_rc(src_rc);
	lib::point pt = m_dest->get_global_topleft();
	src_rc.translate(pt);
	if(!m_region_dds) {
//OLD	dx_window *srcwin = static_cast<dx_window*>(m_dest->get_gui_window());
//		dx_window *srcwin = static_cast<dx_window*>(window);
//		viewport *srcvp = srcwin->get_viewport();
		viewport *srcvp = m_viewport;
		m_region_dds = srcvp->create_surface();
		AM_DBG lib::logger::get_logger()->debug("dx_smiltext_get_ddsurf(0x%x) m_size=%d,%d m_region_dds=0x%x", this, m_size.w,m_size.h,m_region_dds);
		if ( ! m_region_dds) {
			lib::logger::get_logger()->fatal("DirectDrawSurface::create_surface failed()");
			return;
		}
		DWORD ddTranspColorLow = m_viewport->convert(RGB(255,255,255));
		DWORD ddTranspColorHigh = m_viewport->convert(RGB(255,255,255));
		DWORD dwFlags = DDCKEY_SRCBLT;
		DDCOLORKEY ck;
		ck.dwColorSpaceLowValue = ddTranspColorLow;
		ck.dwColorSpaceHighValue = ddTranspColorHigh;
		HRESULT hr = m_region_dds->SetColorKey(dwFlags, &ck);
		if (FAILED(hr)) {
			win_report_error("SetColorKey()", hr);
		}
		// get (cumulative) background
		srcvp->copy_surface(m_region_dds, dst_rc, srcvp->get_surface(), src_rc);
	} else {
		m_viewport->copy_surface(m_region_dds, dst_rc, m_viewport->get_surface(), src_rc);
	}
}

