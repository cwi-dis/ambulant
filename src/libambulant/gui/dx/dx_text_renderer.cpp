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

#include "ambulant/config/config.h"
#include <objbase.h>
#ifdef AMBULANT_DDRAW_EX
#include <ddrawex.h>
#else
#include <ddraw.h>
#endif
#include <windows.h>

#include "ambulant/gui/dx/dx_text_renderer.h"
#include "ambulant/gui/dx/dx_viewport.h"

#include "ambulant/lib/memfile.h"
#include "ambulant/lib/string_util.h"

#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_error.h"

#include "ambulant/lib/textptr.h"
#ifdef _UNICODE
#define STR_TO_TSTR(s) ambulant::lib::textptr(s).c_wstr()
#else
#define STR_TO_TSTR(s) (s)
#endif

using namespace ambulant;
using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_report_last_error;
using ambulant::lib::logger;

// XXX: We need to pass the color as read from the text param.

gui::dx::text_renderer::text_renderer(const net::url& u, const lib::size& bounds, viewport* v)
:	m_url(u),
	m_size(bounds),
	m_viewport(v),
	m_ddsurf(0),
	m_text_color(GetSysColor(COLOR_WINDOWTEXT)),
	m_text_bgcolor(GetSysColor(COLOR_WINDOW)),
	m_text_size(0),
	m_text_font(NULL),
	m_text_data(NULL),
	m_text_datalen(0)
{
}

gui::dx::text_renderer::~text_renderer() {
	if(m_ddsurf) m_ddsurf->Release();
}

void
gui::dx::text_renderer::set_text_data(const char* data, size_t datalen) {
	if (data != NULL && datalen > 0) {
		m_text_data = (char*) malloc(datalen+1);
		strncpy(m_text_data, data, datalen);
		m_text_datalen = datalen;
		m_text_data[m_text_datalen] = '\0';
	}
}

void
gui::dx::text_renderer::free_text_data() {
	if (m_text_data) {
		free((void*)m_text_data);
	    m_text_data = NULL;
	}
	m_text_datalen = 0;
}

void
gui::dx::text_renderer::set_text_color(lib::color_t color) {
	m_text_color = color;
}

void
gui::dx::text_renderer::set_text_bgcolor(lib::color_t color) {
	m_text_bgcolor = color;
}

void
gui::dx::text_renderer::set_text_size(float size) {
	m_text_size = size;
}

void
gui::dx::text_renderer::set_text_font(const char *fontname) {
	m_text_font = fontname;
}

void 
gui::dx::text_renderer::open(net::datasource_factory *df) {
	m_ddsurf = m_viewport->create_surface(m_size);
	if(!m_ddsurf) {
		free_text_data();
		return;
	}
	m_viewport->clear_surface(m_ddsurf, RGB(255,255,255));
	if (!net::read_data_from_url(m_url, df, &m_text_data, &m_text_datalen)) {
		// Error message has already been produced
		m_text_data = NULL;
		m_text_datalen = 0;
	}
	return;
}

SIZE
gui::dx::text_renderer::render(LONG x, LONG y, UINT uFormat) {
	SIZE rv;
	rv.cx = rv.cy = 0;
	if ( ! m_ddsurf) {
		m_ddsurf = m_viewport->create_surface(m_size);
		m_viewport->clear_surface(m_ddsurf, RGB(255,255,255));
	}
	if ( ! m_ddsurf) {
		free_text_data();
		return rv;
	}
	
	//////////////
	// Draw text 
	
	HDC hdc;
	HRESULT hr = m_ddsurf->GetDC(&hdc);
	if (FAILED(hr)) {
		win_report_error("DirectDrawSurface::GetDC()", hr);
		free_text_data();
		return rv;
	}

	// Set the passed <param> values in the device context
				// set color
//	SetBkMode(hdc, TRANSPARENT);
	COLORREF crTextColor = (m_text_color == CLR_INVALID)?::GetSysColor(COLOR_WINDOWTEXT):m_text_color;
	::SetTextColor(hdc, crTextColor);
	COLORREF crBkColor = (m_text_bgcolor == CLR_INVALID)?::GetSysColor(COLOR_WINDOW):m_text_bgcolor;
	::SetBkColor(hdc, crBkColor);
	
	DWORD family = FF_DONTCARE | DEFAULT_PITCH;
	const char *fontname = m_text_font;
	if (m_text_font) {
		if (strcmp(m_text_font, "serif") == 0) {
			family = FF_ROMAN | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(m_text_font, "sans-serif") == 0) {
			family = FF_SWISS | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(m_text_font, "monospace") == 0) {
			family = FF_DONTCARE | FIXED_PITCH;
			fontname = NULL;
		} else if (strcmp(m_text_font, "cursive") == 0) {
			family = FF_SCRIPT | VARIABLE_PITCH;
			fontname = NULL;
		} else if (strcmp(m_text_font, "fantasy") == 0) {
			family = FF_DECORATIVE | VARIABLE_PITCH;
			fontname = NULL;
		}
	}

	HFONT fontobj = ::CreateFont(
			-(int)m_text_size,	// height of font
			0,					// average character width
			0,					// angle of escapement
			0,					// base-line orientation angle
			0,					// font weight
			0,					// italic attribute option
			0,					// underline attribute option
			0,					// strikeout attribute option
			ANSI_CHARSET,		// character set identifier
			OUT_DEFAULT_PRECIS, // output precision
			CLIP_DEFAULT_PRECIS, // clipping precision
			DEFAULT_QUALITY,	// output quality
			family,				// pitch and family
			STR_TO_TSTR(fontname));			// typeface name
	::SelectObject(hdc, fontobj);
	RECT dstRC = {x, y, m_size.w, m_size.h};
	if (m_text_data) {
		lib::textptr tp(m_text_data, m_text_datalen);
		int res = ::GetTextExtentPoint32(hdc, tp, (int)tp.length(), &rv);
		if(res == 0)
			win_report_last_error("GetTextExtentPoint32()");
		if (x+rv.cx > m_size.w) { //should also work for DT_RTLREADING
			y += rv.cy;
			dstRC.left = 0;
			dstRC.top = y;
			rv.cx -= x;
		} else rv.cy = 0;
		res = ::DrawText(hdc, tp, (int)tp.length(), &dstRC, uFormat);
#ifdef JUNK
		UINT fMode = 0;
		if (uFormat|DT_LEFT) fMode |= TA_LEFT;
		if (uFormat|DT_RIGHT) fMode |= TA_RIGHT;
		if (uFormat|DT_CENTER) fMode |= TA_CENTER;
		if (uFormat|DT_RTLREADING) fMode |= TA_RTLREADING;
		::SetTextAlign(hdc, fMode);
		::TextOut(hdc, x, y, tp, (int)tp.length());
#endif/*JUNK*/
		if(res == 0)
			win_report_last_error("DrawText()");

		free_text_data();

	}
	m_ddsurf->ReleaseDC(hdc);
		
	//////////////
	// Text is always transparent; set the color
	
	DWORD ddTranspColor = m_viewport->convert(RGB(255,255,255));
	DWORD dwFlags = DDCKEY_SRCBLT;
	DDCOLORKEY ck;
	ck.dwColorSpaceLowValue = ddTranspColor;
	ck.dwColorSpaceHighValue = ddTranspColor;
	hr = m_ddsurf->SetColorKey(dwFlags, &ck);
	if (FAILED(hr)) {
		win_report_error("SetColorKey()", hr);
	}
	return rv;
}
 

