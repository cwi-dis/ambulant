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
	m_epoch(0),
	m_size(0,0),
	m_x(0),
	m_y(0),
	m_max_ascent(0),
	m_max_descent(0),
	m_hdc(NULL),
	m_viewport(NULL),
	m_ddsurf(NULL),
	m_df(factory->get_datasource_factory()),
	m_engine(smil2::smiltext_engine(node, evp, this, true)),
	m_params(m_engine.get_params())
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
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_engine.start(t);
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

void
gui::dx::dx_smiltext_renderer::_dx_smiltext_changed() {
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::_dx_smiltext_changed(0x%x)", this);
	HRESULT hr;
	int nbr = 0; // number of breaks (newlines) before current line

	if (m_hdc == NULL) {
		hr = m_ddsurf->GetDC(&m_hdc);
		if (FAILED(hr)) {
			win_report_error("DirectDrawSurface::GetDC()", hr);
			return;
		}
		AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::_dx_smiltext_changed(0x%x): m_hdc=0x%x", this, m_hdc);
	}
	// Compute the shifted position of what we want to draw w.r.t. the visible origin
	lib::point logical_origin(0, 0);
	if (m_params.m_mode == smil2::stm_crawl) {
		long int elapsed = m_event_processor->get_timer()->elapsed();
		double now = elapsed - m_epoch;
		logical_origin.x += (int) now * m_params.m_rate / 1000;
		if (logical_origin.x < 0)
			AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::_dx_smiltext_changed(0x%x): strange: logical_x=%d, m_epoch=%ld, elpased=%ld !", this, logical_origin.x, m_epoch, elapsed);
	}
	if (m_params.m_mode == smil2::stm_scroll) {
		long int elapsed = m_event_processor->get_timer()->elapsed();
		double now = elapsed - m_epoch;
		logical_origin.y += (int) now * m_params.m_rate / 1000;
	}
	AM_DBG logger::get_logger()->debug("_dx_smiltext_changed: logical_origin(%d,%d)", logical_origin.x, logical_origin.y);

	if (logical_origin.x || logical_origin.y)
		_dx_smiltext_shift( m_dest->get_rect(), logical_origin);

	// Always re-compute and re-render everything when new text is added.
	// Therefore, m_engine.newbegin() is NOT used
	lib::xml_string data;
	smil2::smiltext_runs::const_iterator cur = m_engine.begin();

	m_y = m_dest->get_rect().y;
	m_max_ascent = m_max_descent = 0;
	// count number of initial breaks before first line
	while (cur->m_command == smil2::stc_break) {
		nbr++;
		cur++;
	}
	while (cur != m_engine.end()) {
		// compute layout of next line
		smil2::smiltext_runs::const_iterator bol = cur; // begin of line pointer
		bool fits = false;
		m_x = m_dest->get_rect().x;
		if (nbr > 0 && (m_max_ascent != 0 || m_max_descent != 0)) {
			// correct m_y for proper size of previous line
			m_y += (m_max_ascent + m_max_descent);
			nbr--;
		}
		m_max_ascent = m_max_descent = 0;
		AM_DBG lib::logger::get_logger()->debug("_dx_smiltext_changed(): command=%d data=%s",cur->m_command,cur->m_data.c_str()==NULL?"(null)":cur->m_data.c_str());
		while (cur != m_engine.end()) {
			fits = _dx_smiltext_fits(*cur, m_dest->get_rect());
			if ( ! fits || cur->m_command == smil2::stc_break)
				break;
			cur++;
		}
		m_x = m_dest->get_rect().x; // was used by dx_smiltext_fits()
		// move down number of breaks times height of current line
		m_y += (m_max_ascent + m_max_descent) * nbr;
		// count number of breaks for next line
		nbr = 0;
		while (cur->m_command == smil2::stc_break) {
			nbr++;
			cur++;
		}
		if ( ! fits && nbr == 0)
			nbr = 1;
		while (bol != cur) {
			lib::rect r = _dx_smiltext_compute(*bol, m_dest->get_rect());
			_dx_smiltext_render(*bol, r, logical_origin);
			bol++;
		}
	}
	m_engine.done();
	
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_changed(0x%x): ReleaseDC(m_hdc=0x%x)", this, m_hdc);
	if (m_hdc && FAILED(hr = m_ddsurf->ReleaseDC(m_hdc)))
		lib::logger::get_logger()->warn("%s failed.", "_dx_smiltext_changed(): ReleaseDC()");
	m_hdc = NULL;
}

bool
gui::dx::dx_smiltext_renderer::_dx_smiltext_fits(const smil2::smiltext_run strun, const lib::rect r) {
	bool rv = true;
	a_extent ax = _dx_smiltext_get_a_extent (strun, m_hdc);

	if (ax.get_ascent() > m_max_ascent)
		m_max_ascent = ax.get_ascent();
	if (ax.get_descent() > m_max_descent)
		m_max_descent = ax.get_descent();
	if (m_x + (int) ax.get_width() > r.right())
		rv = false;
	m_x += ax.get_width();
	if (m_font && ! ::DeleteObject(m_font))
		win_report_error("DeleteObject(m_font)", DDERR_GENERIC);
	m_font = NULL;
	return rv;
}

lib::rect
gui::dx::dx_smiltext_renderer::_dx_smiltext_compute(const smil2::smiltext_run strun, const lib::rect r) {
	lib::rect rv = lib::rect(lib::point(0,0),lib::size(100,100));
	a_extent ax = _dx_smiltext_get_a_extent (strun, m_hdc);

	rv.x = m_x;
	m_x += ax.get_width();
	rv.y = m_y + m_max_ascent - ax.get_ascent();
	rv.w = ax.get_width();
	rv.h = ax.get_ascent() + ax.get_descent();
	return rv;
}

gui::dx::a_extent
gui::dx::dx_smiltext_renderer::_dx_smiltext_get_a_extent(const smil2::smiltext_run strun, HDC hdc) {
	unsigned int ascent, descent, width;

	if (strun.m_command != smil2::stc_data || hdc == NULL 
		|| strun.m_data.length() == 0)
		return a_extent(0,0,0);

	_dx_smiltext_set_font (strun, hdc);
	TEXTMETRIC tm;
	BOOL res = ::GetTextMetrics(hdc, &tm);
	if (res == 0)
		win_report_last_error("GetTextMetric()");
	ascent  = tm.tmAscent;
	descent = tm.tmDescent;

	lib::textptr tp(strun.m_data.c_str(), strun.m_data.length());
	SIZE SZ;
	res = ::GetTextExtentPoint32(hdc, tp, (int)tp.length(), &SZ);
	if (res == 0)
		win_report_last_error("GetTextExtentPoint32()");
	width  = SZ.cx;

	return a_extent(ascent, descent, width);
}

void
gui::dx::dx_smiltext_renderer::_dx_smiltext_render(const smil2::smiltext_run strun, const lib::rect r, lib::point p) {
	if (strun.m_command != smil2::stc_data)
		return;
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_render(): command=%d data=%s color=0x%x",strun.m_command,strun.m_data.c_str()==NULL?"(null)":strun.m_data.c_str(),strun.m_color);
	// set the foreground color
	if( ! strun.m_transparent) {
		COLORREF crTextColor = (strun.m_color == CLR_INVALID)?::GetSysColor(COLOR_WINDOWTEXT):strun.m_color;;
		::SetTextColor(m_hdc, crTextColor);
	}
	// set the background color
	if( ! strun.m_bg_transparent) {
		COLORREF crBkColor = (strun.m_bg_color == CLR_INVALID)?::GetSysColor(COLOR_WINDOW):strun.m_bg_color;
		::SetBkColor(m_hdc, crBkColor);
	} // else SetBkMode(m_hdc, TRANSPARENT);

	// set the font
	_dx_smiltext_set_font(strun, m_hdc);

	// draw the text
	const char* text = strun.m_data.c_str();
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
	if( ! strun.m_bg_transparent) {
		//XX KB I don't know whether it's a good idea to re-set bgcolor here
		COLORREF crBkColor = ::GetSysColor(COLOR_WINDOW);
		::SetBkColor(m_hdc, crBkColor);
	} 
}

void
gui::dx::dx_smiltext_renderer::_dx_smiltext_shift(const lib::rect r, const lib::point p) {
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_shift(): r=(%d,%d,%d,%d) p=%d,%d,",r.left(),r.top(),r.right(),r.bottom(),p.x,p.y);
	// ref: http://msdn2.microsoft.com/en-us/library/ms532661.aspx
	// shift the logical window w.r.t to the final viewport
	BOOL res = ::SetWindowOrgEx(m_hdc, p.x, p.y, NULL);
	if( ! res)
		win_report_last_error("SetWindowOrgEx()");
}

void
gui::dx::dx_smiltext_renderer::_dx_smiltext_set_font(const smil2::smiltext_run strun, HDC hdc) {
	DWORD family = FF_DONTCARE | DEFAULT_PITCH;
	const char *fontname = strun.m_font_family;
	if (strun.m_font_family) {
		if (strcmp(strun.m_font_family, "serif") == 0) {
			family = FF_ROMAN | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(strun.m_font_family, "sans-serif") == 0) {
			family = FF_SWISS | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(strun.m_font_family, "monospace") == 0) {
			family = FF_DONTCARE | FIXED_PITCH;
			fontname = NULL;
		} else if (strcmp(strun.m_font_family, "cursive") == 0) {
			family = FF_SCRIPT | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(strun.m_font_family, "fantasy") == 0) {
			family = FF_DECORATIVE | VARIABLE_PITCH;
			fontname = NULL;
		}
	}
	int weight;
	switch(strun.m_font_weight) {
		default:
		case smil2::stw_normal:
			weight = FW_NORMAL;
			break;
		case smil2::stw_bold:
			weight = FW_BOLD;
			break;
	}
	DWORD italic;
	switch(strun.m_font_style) {
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
			-(int)strun.m_font_size,	// height of font
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
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_run_set_attr(0x%x): m_data=%s font=0x%x, m_font_size=%d,weight=0x%x,italic=%d,family=0x%x,strun.m_font_family=%s",this,strun.m_data.c_str(),m_font,strun.m_font_size,weight,italic,family,strun.m_font_family);
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
	_dx_smiltext_changed();

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
		srcvp->clear_surface(m_ddsurf, RGB(255,255,255));
	}		
}
#endif //WITH_SMIL30

