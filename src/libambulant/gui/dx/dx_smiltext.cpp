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
#ifdef  WITH_SMIL30
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_smiltext.h"
#include "ambulant/gui/dx/dx_transition.h"

#include "ambulant/common/region_info.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/memfile.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/smil2/params.h"
#include "ambulant/common/factory.h"

#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/textptr.h"

/* windows includes begin*/
#ifdef AMBULANT_DDRAW_EX
#include <ddrawex.h>
#else
#include <ddraw.h>
#endif
#include <windows.h>
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

gui::dx::dx_smiltext_renderer::dx_smiltext_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories* factory,
	dx_playables_context *dxplayer)
:   dx_renderer_playable(context, cookie, node, evp, dxplayer),
	m_size(0,0),
	m_hdc(NULL),
	m_viewport(NULL),
	m_ddsurf(NULL),
	m_df(factory->get_datasource_factory()),
	m_layout_engine(smil2::smiltext_layout_engine(node, evp, this, this))
{
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer(0x%x)", this);
}

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
	AM_DBG lib::logger::get_logger()->debug("~dx_smiltext_renderer(0x%x), m_ddsurf=0x%x, m_hdc=0x%x", this, m_ddsurf, m_hdc);
	m_lock.enter();

	if (m_font && ! ::DeleteObject(m_font))
		win_report_error("DeleteObject(m_font)", DDERR_GENERIC);
	m_font = NULL;

	if (m_ddsurf) {
		if (m_hdc) {
		 	if (m_ddsurf->ReleaseDC(m_hdc) == 0)
				lib::logger::get_logger()->warn("~dx_smiltext_renderer(0x%x): ReleaseDC(m_hdc=0x%x) fails", this, m_hdc);		;
			m_hdc = NULL;
		}
		m_ddsurf->Release();
		m_ddsurf = NULL;
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

void 
gui::dx::dx_smiltext_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::stop(0x%x)", this);
	m_lock.enter();
	m_dest->renderer_done(this);
	m_activated = false;
	m_dxplayer->stopped(this);
	m_lock.leave();
}

void
gui::dx::dx_smiltext_renderer::smiltext_changed() {
	m_dest->need_redraw();
}

smil2::smiltext_metrics 
gui::dx::dx_smiltext_renderer::get_smiltext_metrics(const smil2::smiltext_run& run) {
	unsigned int ascent = 0, descent = 0, height = 0, width = 0, line_spacing = 0, word_spacing = 0;

	if (run.m_command == smil2::stc_data && run.m_data.length() != 0) {

		_dx_smiltext_set_font (run, m_hdc);

		TEXTMETRIC tm;
		BOOL res = ::GetTextMetrics(m_hdc, &tm);
		if (res == 0)
			win_report_last_error("GetTextMetric()");
		ascent  = tm.tmAscent;
		descent = tm.tmDescent;	
		height	= tm.tmHeight;
		line_spacing = height+tm.tmInternalLeading+tm.tmExternalLeading;

		lib::textptr tp(run.m_data.c_str(), run.m_data.length());
		SIZE SZ;
		res = ::GetTextExtentPoint32(m_hdc, tp, (int)tp.length(), &SZ);
		if (res == 0)
			win_report_last_error("GetTextExtentPoint32()");
		width = SZ.cx;
		char blank[2];
		blank[0] = ' ';
		blank[1] = '\0';
		lib::textptr tbp(blank, 1);
		res = ::GetTextExtentPoint32(m_hdc, tbp, (int)tbp.length(), &SZ);
		if (res == 0)
			win_report_last_error("GetTextExtentPoint32()");
		word_spacing = SZ.cx;
	}
	if (m_font) {
		::DeleteObject(m_font);
		m_font = NULL;
	}
	return smil2::smiltext_metrics(ascent, descent, height, width, line_spacing, word_spacing);
}

void
gui::dx::dx_smiltext_renderer::render_smiltext(const smil2::smiltext_run& run, const lib::rect& r) {
	if (run.m_command != smil2::stc_data)
		return;
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_render(): command=%d data=%s color=0x%x",run.m_command,run.m_data.c_str()==NULL?"(null)":run.m_data.c_str(),run.m_color);
	// set the foreground color
	if( ! run.m_transparent) {
		COLORREF crTextColor = (run.m_color == CLR_INVALID)?::GetSysColor(COLOR_WINDOWTEXT):run.m_color;;
		::SetTextColor(m_hdc, crTextColor);
	}
	// set the background color
	if( ! run.m_bg_transparent) {
		COLORREF crBkColor = (run.m_bg_color == CLR_INVALID)?::GetSysColor(COLOR_WINDOW):run.m_bg_color;
		::SetBkColor(m_hdc, crBkColor);
	} // else SetBkMode(m_hdc, TRANSPARENT);

	// set the font
	_dx_smiltext_set_font(run, m_hdc);

	// draw the text
	const char* text = run.m_data.c_str();
	lib::textptr tp(text, strlen(text));
	RECT dstRC;
	dstRC.left   = r.left();
	dstRC.top    = r.top();
	dstRC.right  = r.right();
	dstRC.bottom = r.bottom();
	UINT uFormat = DT_NOPREFIX | DT_WORDBREAK;
	HRESULT res = ::DrawText(m_hdc, tp, (int)tp.length(), &dstRC, uFormat);
	if(res == 0)
		win_report_last_error("DrawText()");

	// Text is always transparent; set the color
	DWORD ddTranspColor = m_viewport->convert(RGB(255,255,255));
	DWORD dwFlags = DDCKEY_SRCBLT;
	DDCOLORKEY ck;
	ck.dwColorSpaceLowValue = ddTranspColor;
	ck.dwColorSpaceHighValue = ddTranspColor;
	HRESULT hr = m_ddsurf->SetColorKey(dwFlags, &ck);
	if (FAILED(hr)) {
		win_report_error("SetColorKey()", hr);
	}
	if (m_font && ! ::DeleteObject(m_font))
		win_report_error("DeleteObject(m_font)", DDERR_GENERIC);
	m_font = NULL;
	// reset the background color
	if( ! run.m_bg_transparent) {
		//XX KB I don't know whether it's a good idea to re-set bgcolor here
		COLORREF crBkColor = ::GetSysColor(COLOR_WINDOW);
		::SetBkColor(m_hdc, crBkColor);
	} 
}

void
gui::dx::dx_smiltext_renderer::_dx_smiltext_set_font(const smil2::smiltext_run run, HDC hdc) {
	DWORD family = FF_DONTCARE | DEFAULT_PITCH;
	const char *fontname = run.m_font_family;
	if (run.m_font_family) {
		if (strcmp(run.m_font_family, "serif") == 0) {
			family = FF_ROMAN | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(run.m_font_family, "sans-serif") == 0) {
			family = FF_SWISS | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(run.m_font_family, "monospace") == 0) {
			family = FF_DONTCARE | FIXED_PITCH;
			fontname = NULL;
		} else if (strcmp(run.m_font_family, "cursive") == 0) {
			family = FF_SCRIPT | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(run.m_font_family, "fantasy") == 0) {
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
	m_font = ::CreateFont(
			-(int)run.m_font_size,	// height of font
			0,					// average character width
			0,					// angle of escapement
			0,					// base-line orientation angle
			weight,				// font weight
			italic,				// italic attribute option
			0,					// underline attribute option
			0,					// strikeout attribute option
			ANSI_CHARSET,		// character set identifier
			OUT_DEFAULT_PRECIS, // output precision
			CLIP_DEFAULT_PRECIS, // clipping precision
			DEFAULT_QUALITY,	// output quality
			family,				// pitch and family
			STR_TO_TSTR(fontname));	// typeface name
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_run_set_attr(0x%x): m_data=%s font=0x%x, m_font_size=%d,weight=0x%x,italic=%d,family=0x%x,run.m_font_family=%s",this,run.m_data.c_str(),m_font,run.m_font_size,weight,italic,family,run.m_font_family);
	::SelectObject(hdc, m_font);
}

void
gui::dx::dx_smiltext_renderer::user_event(const lib::point& pt, int what) {
	m_lock.enter();
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	m_lock.leave();
}

void
gui::dx::dx_smiltext_renderer::redraw(const lib::rect& dirty, common::gui_window *window) {
	m_lock.enter();
	// Get the top-level surface
	_dx_smiltext_get_ddsurf(window);
	if ( ! (m_viewport && m_ddsurf)) {
		m_lock.leave(); // give up
		return;
	}
	dx_window *dxwindow = static_cast<dx_window*>(window);
	if (m_hdc == NULL) {
		HRESULT hr = m_ddsurf->GetDC(&m_hdc);
		if (FAILED(hr)) {
			win_report_error("DirectDrawSurface::GetDC()", hr);
			return;
		}
	}
	
	m_layout_engine.redraw(dirty);
	if (FAILED(m_ddsurf->ReleaseDC(m_hdc))) {
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
		
	// Finally blit m_ddsurf to viewport
	m_viewport->draw(m_ddsurf, smiltext_rc, reg_rc, true, tr);

	if (m_erase_never) m_dest->keep_as_background();
	m_ddsurf->Release();
	m_ddsurf = NULL;
	m_lock.leave();
}

void
gui::dx::dx_smiltext_renderer::_dx_smiltext_get_ddsurf(common::gui_window *window) {
	if(!m_ddsurf) {
//		dx_window *srcwin = static_cast<dx_window*>(m_dest->get_gui_window());
		dx_window *srcwin = static_cast<dx_window*>(window);
		viewport *srcvp = srcwin->get_viewport();
		m_ddsurf = srcvp->create_surface(m_size);
		AM_DBG lib::logger::get_logger()->debug("dx_smiltext_get_ddsurf(0x%x) m_size=%d,%d m_ddsurf=0x%x", this, m_size.w,m_size.h,m_ddsurf);	
		if ( ! m_ddsurf) {
			lib::logger::get_logger()->fatal("DirectDrawSurface::create_surface failed()");
			return;
		}
		srcvp->clear_surface(m_ddsurf, RGB(255,255,255), 1.0);
	}		
}
#endif //WITH_SMIL30

