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

#include <objbase.h>
#include <ddrawex.h>
#include <windows.h>

#include "ambulant/gui/dx/dx_text_renderer.h"
#include "ambulant/gui/dx/dx_viewport.h"

#include "ambulant/lib/colors.h"
#include "ambulant/lib/memfile.h"

#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_error.h"

using namespace ambulant;
using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_report_last_error;
using ambulant::lib::logger;

// XXX: We need to pass the color as read from the text param.

gui::dx::text_renderer::text_renderer(const std::string& url, const lib::size& bounds, viewport* v)
:	m_size(bounds),
	m_ddsurf(0){
	open(url, v);
}

gui::dx::text_renderer::~text_renderer() {
	if(m_ddsurf) m_ddsurf->Release();
}

void gui::dx::text_renderer::open(const std::string& url, viewport* v) {
	if(!lib::memfile::exists(url)) {
		lib::logger::get_logger()->warn("Failed to locate text file %s.", url.c_str());
		return;
	}
	lib::memfile mf(url);
	mf.read();
	lib::databuffer& db = mf.get_databuffer();
	std::basic_string<text_char> text;
	text.assign(db.begin(), db.end());
		
	m_ddsurf = v->create_surface(m_size);
	if(!m_ddsurf) {
		return;
	}
	v->clear_surface(m_ddsurf, RGB(255,255,255));
	
	//////////////
	// Draw text 
	
	HDC hdc;
	HRESULT hr = m_ddsurf->GetDC(&hdc);
	if (FAILED(hr)) {
		win_report_error("DirectDrawSurface::GetDC()", hr);
		return;
	}
	lib::color_t clr = GetSysColor(COLOR_WINDOWTEXT); // should be passed in from text param
	SetBkMode(hdc, TRANSPARENT);
	COLORREF crTextColor = (clr == CLR_INVALID)?::GetSysColor(COLOR_WINDOWTEXT):clr;
	::SetTextColor(hdc, crTextColor);	
	RECT dstRC = {0, 0, m_size.w, m_size.h};
	UINT uFormat = DT_CENTER | DT_WORDBREAK;
	int res = ::DrawText(hdc, text.c_str(), int(text.length()), &dstRC, uFormat); 
	if(res == 0)
		win_report_last_error("DrawText()");
	m_ddsurf->ReleaseDC(hdc);
		
	//////////////
	// Text is always transparent; set the color
	
	DWORD ddTranspColor = v->convert(RGB(255,255,255));
	DWORD dwFlags = DDCKEY_SRCBLT;
	DDCOLORKEY ck;
	ck.dwColorSpaceLowValue = ddTranspColor;
	ck.dwColorSpaceHighValue = ddTranspColor;
	hr = m_ddsurf->SetColorKey(dwFlags, &ck);
	if (FAILED(hr)) {
		win_report_error("SetColorKey()", hr);
	}
}
 

