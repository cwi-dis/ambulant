/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#include <windows.h>

#include "ambulant/gui/dg/dg_viewport.h"

#include "ambulant/lib/logger.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/win32/win32_error.h"

#include <cassert>

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_report_last_error;
const lib::color_t CLR_DEFAULT = RGB(255, 255, 255);

gui::dg::viewport::viewport(int width, int height, HWND hwnd) 
:	m_width(width), 
	m_height(height),
	m_hwnd(hwnd?hwnd:GetDesktopWindow()),
	m_surf(0),
	m_memdc(0),
	m_hbmp(0),
	m_hold(0),
	m_bgd(CLR_DEFAULT), 
	m_bits_size(24),
	m_logger(lib::logger::get_logger()) {
	HDC hdc = GetDC(NULL);
	if(!hdc) {
		win_report_last_error("GetDC");
		return;
	}
	lib::color_trible *bits = 0;
	BITMAPINFO *pbmpi = get_bmp_info(width, height, m_bits_size);
	m_hbmp = CreateDIBSection(hdc, pbmpi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);
	if(!m_hbmp || !bits) {
		win_report_last_error("CreateDIBSection");
		ReleaseDC(hwnd, hdc);
		return;
	}
	m_surf = new surface_t(width, height, m_bits_size, bits);
	m_surf->fill(lib::color_trible(m_bgd));
	m_memdc = CreateCompatibleDC(hdc);
	m_hold = (HBITMAP) SelectObject(m_memdc, m_hbmp);
	DeleteDC(hdc);
	AM_DBG m_logger->trace("dg::viewport() succeeded");
}

gui::dg::viewport::~viewport() {
	if(m_surf) delete m_surf;
	if(m_hold) SelectObject(m_memdc, (HGDIOBJ) m_hold);
	if(m_memdc) DeleteDC(m_memdc);
	if(m_hbmp) DeleteObject((HGDIOBJ) m_hbmp);
#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
	RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
#endif
}

// Sets the background color of this viewport
void gui::dg::viewport::set_background(lib::color_t color) {
	m_bgd = (color == CLR_INVALID)?CLR_DEFAULT:color;
}

// Blt back buffer to primary surface
void gui::dg::viewport::redraw() {
	if(!m_surf || !m_memdc)
		return;
	HDC hdc = GetDC(m_hwnd);
	if(!hdc) {
		win_report_last_error("GetDC");
		return;
	}
	int x_dst = 0, y_dst = 0;
	int w_dst = m_width, h_dst = m_height;
	int x_src = 0, y_src = 0;
	BOOL res = BitBlt(hdc, x_dst, y_dst, w_dst, h_dst, m_memdc, x_src, y_src, SRCCOPY);
	if(!res) win_report_last_error("BitBlt");
	ReleaseDC(m_hwnd, hdc);
}

void gui::dg::viewport::redraw(HDC hdc) {
	if(!hdc) {
		redraw();
		return;
	}
	int x_dst = 0, y_dst = 0;
	int w_dst = m_width, h_dst = m_height;
	int x_src = 0, y_src = 0;
	BOOL res = BitBlt(hdc, x_dst, y_dst, w_dst, h_dst, m_memdc, x_src, y_src, SRCCOPY);
	if(!res) win_report_last_error("BitBlt");
}

void gui::dg::viewport::redraw(const lib::screen_rect<int>& rc) {
	if(!m_surf || !m_memdc)
		return;
	
	HDC hdc = GetDC(m_hwnd);
	if(!hdc) {
		win_report_last_error("GetDC");
		return;
	}
	int x_dst = rc.left(), y_dst = rc.top();
	int w_dst = rc.width(), h_dst = rc.height();
	int x_src = rc.left(), y_src = rc.top();
	BOOL res = BitBlt(hdc, x_dst, y_dst, w_dst, h_dst, m_memdc, x_src, y_src, SRCCOPY);
	if(!res) win_report_last_error("BitBlt");
	ReleaseDC(m_hwnd, hdc);
}

// Clears the back buffer using this viewport bgd color
void gui::dg::viewport::clear() {
	if(m_surf) m_surf->fill(lib::color_trible(m_bgd));
}

// Clears the specified back buffer rectangle using the provided color 
void gui::dg::viewport::clear(const lib::screen_rect<int>& rc, lib::color_t clr) {
	if(m_surf) m_surf->fill(rc, lib::color_trible(clr));
	/*
	if(!m_memdc) return;
	HBRUSH brush = CreateSolidBrush(clr);
	HPEN pen = CreatePen(PS_NULL, 1, clr);
	HGDIOBJ oldbrush = SelectObject(m_memdc, (HGDIOBJ) brush);
	HGDIOBJ oldpen = SelectObject(m_memdc, (HGDIOBJ) pen);
	if(Rectangle(m_memdc, rc.left(), rc.top(), rc.right(), rc.bottom()) == 0)
		win_report_last_error("Rectangle()");
	DeleteObject(SelectObject(m_memdc, oldpen));
	DeleteObject(SelectObject(m_memdc, oldbrush));
	*/
}


// Draw the src_rc of the DD surface to the back buffer and destination rectangle
void gui::dg::viewport::draw(dib_surface_t* src, const lib::screen_rect<int>& src_rc,
	const lib::screen_rect<int>& dst_rc, bool keysrc, lib::color_t transp) {
	if(!src || !m_surf || !m_memdc) return;
	
	if(src_rc != dst_rc) {
		HDC bmpdc = CreateCompatibleDC(m_memdc);
		HGDIOBJ old = SelectObject(bmpdc, (HGDIOBJ) src->get_handle());
		int x_dst = dst_rc.left(), y_dst = dst_rc.top();
		int w_dst = dst_rc.width(), h_dst = dst_rc.height();
		int x_src = src_rc.left(), y_src = src_rc.top();
		int w_src = src_rc.width(), h_src = src_rc.height();
		BOOL res = StretchBlt(m_memdc, x_dst, y_dst, w_dst, h_dst, bmpdc, x_src, y_src, w_src, h_src, SRCCOPY);
		if(!res) win_report_last_error("BitBlt");
		SelectObject(bmpdc, old);
		DeleteDC(bmpdc);
		return;
	}
	
	if(keysrc && transp != CLR_INVALID) {
		m_surf->blit(src->get_pixmap(), dst_rc, src_rc.left(), src_rc.top(), lib::color_trible(transp));
	} else {
		HDC bmpdc = CreateCompatibleDC(m_memdc);
		HGDIOBJ old = SelectObject(bmpdc, (HGDIOBJ) src->get_handle());
		int x_dst = dst_rc.left(), y_dst = dst_rc.top();
		int w_dst = dst_rc.width(), h_dst = dst_rc.height();
		int x_src = src_rc.left(), y_src = src_rc.top();
		BOOL res = BitBlt(m_memdc, x_dst, y_dst, w_dst, h_dst, bmpdc, x_src, y_src, SRCCOPY);
		if(!res) win_report_last_error("BitBlt");
		SelectObject(bmpdc, old);
		DeleteDC(bmpdc);
	}
}

// Draw the whole dib surface to the back buffer and destination rectangle
void gui::dg::viewport::draw(dib_surface_t* src, const lib::screen_rect<int>& dst_rc, 
	bool keysrc, lib::color_t transp) {
	surface_t *p = src->get_pixmap();
	lib::screen_rect<int> src_rc(lib::point(0,0), lib::point(p->get_width(),p->get_height()));
	draw(src, src_rc, dst_rc, keysrc, transp);
}

// Paints the provided string
void gui::dg::viewport::draw(const std::basic_string<text_char>& text, 
	const lib::screen_rect<int>& dst_rc, lib::color_t clr) {
	if(!m_memdc) return;
	SetBkMode(m_memdc, TRANSPARENT);
	COLORREF crTextColor = (clr == CLR_INVALID)?::GetSysColor(COLOR_WINDOWTEXT):clr;
	::SetTextColor(m_memdc, crTextColor);	
	RECT dstRC = {dst_rc.left(), dst_rc.top(), dst_rc.right(), dst_rc.bottom()};
	UINT uFormat = DT_CENTER | DT_WORDBREAK;
	int res = ::DrawText(m_memdc, text.c_str(), int(text.length()), &dstRC, uFormat); 
	if(res == 0)
		win_report_last_error("DrawText()");
}

#ifdef _WIN32_WCE
static int wce_FrameRect(HDC hdc, const RECT* lprc, HBRUSH hbr)
{
  // Fill a "line-size" rectangle for each edge of the frame, using hbr
  // Note that right/bottom borders not painted by FillRect (or FrameRect)
  RECT rectEdge;	
  if (SetRect(&rectEdge, lprc->left, lprc->top, lprc->right, 
    lprc->top + 1) == FALSE) // Top edge of frame
    return FALSE;
  if (FillRect(hdc, &rectEdge, hbr) == FALSE)
    return FALSE;
  
  if (SetRect(&rectEdge, lprc->right - 1, lprc->top, lprc->right, 
    lprc->bottom) == FALSE) // Right edge of frame
    return FALSE;
  if (FillRect(hdc, &rectEdge, hbr) == FALSE)
    return FALSE;
  if (SetRect(&rectEdge, lprc->left, lprc->bottom - 1, lprc->right, 
    lprc->bottom) == FALSE) // Bottom edge of frame
    return FALSE;
  if (FillRect(hdc, &rectEdge, hbr) == FALSE)
    return FALSE;
  if (SetRect(&rectEdge, lprc->left, lprc->top, lprc->left + 1, 
    lprc->bottom) == FALSE) // Left edge of frame
    return FALSE;
  return FillRect(hdc, &rectEdge, hbr);
}
#endif

// Frames the provided rect
void gui::dg::viewport::frame_rect(const lib::screen_rect<int>& rc, lib::color_t clr) {
	if(!m_memdc) return;
	RECT RC = {rc.left(), rc.top(), rc.right(), rc.bottom()};
	HBRUSH hbr = CreateSolidBrush(clr);
#ifdef _WIN32_WCE
	if(wce_FrameRect(m_memdc, &RC, hbr) == 0)
		win_report_last_error("FrameRect()");
#else
	if(FrameRect(m_memdc, &RC, hbr) == 0)
		win_report_last_error("FrameRect()");
#endif
	DeleteObject((HGDIOBJ) hbr);
}




