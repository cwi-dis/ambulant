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

#include <objbase.h>
#include <ddrawex.h>
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
	m_text_size(0),
	m_text_font(NULL)
{
}

gui::dx::text_renderer::~text_renderer() {
	if(m_ddsurf) m_ddsurf->Release();
}

void
gui::dx::text_renderer::set_text_color(lib::color_t color) {
	m_text_color = color;
}

void
gui::dx::text_renderer::set_text_size(float size) {
	m_text_size = size;
}

void
gui::dx::text_renderer::set_text_font(const char *fontname) {
	m_text_font = fontname;
}

void gui::dx::text_renderer::open(net::datasource_factory *df) {
	char *data;
	size_t datalen;
	if (!net::read_data_from_url(m_url, df, &data, &datalen)) {
		// Error message has already been produced
		data = NULL;
		datalen = 0;
	}
	m_ddsurf = m_viewport->create_surface(m_size);
	if(!m_ddsurf) {
		if (data) free(data);
		return;
	}
	m_viewport->clear_surface(m_ddsurf, RGB(255,255,255));
	
	//////////////
	// Draw text 
	
	HDC hdc;
	HRESULT hr = m_ddsurf->GetDC(&hdc);
	if (FAILED(hr)) {
		win_report_error("DirectDrawSurface::GetDC()", hr);
		if (data) free(data);
		return;
	}

	// Set the passed <param> values in the device context
	SetBkMode(hdc, TRANSPARENT);
	COLORREF crTextColor = (m_text_color == CLR_INVALID)?::GetSysColor(COLOR_WINDOWTEXT):m_text_color;
	::SetTextColor(hdc, crTextColor);
	
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
	RECT dstRC = {0, 0, m_size.w, m_size.h};
	UINT uFormat = DT_CENTER | DT_WORDBREAK;
	if (data) {
		lib::textptr tp(data, datalen);
		int res = ::DrawText(hdc, tp, (int)tp.length(), &dstRC, uFormat); 
		if(res == 0)
			win_report_last_error("DrawText()");
		free(data);
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
}
 

