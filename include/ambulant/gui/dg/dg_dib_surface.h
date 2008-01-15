/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_DG_DIB_SURFACE
#define AMBULANT_GUI_DG_DIB_SURFACE

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/lib/colors.h"
#include "ambulant/gui/dg/dg_surface.h"
#include "ambulant/lib/basic_types.h"


namespace ambulant {

namespace gui {

namespace dg {

template<class ColorType>
struct dib_surface {
	HBITMAP m_hbmp;
	surface<ColorType> *m_surf;

	dib_surface(HBITMAP hbmp = 0, surface<ColorType> *surf = 0)
	:	m_hbmp(hbmp), m_surf(surf) {}

	~dib_surface() {
		if(m_surf) delete m_surf;
		if(m_hbmp) ::DeleteObject(m_hbmp);
	}
	surface<ColorType>* get_pixmap() { return m_surf;}
	HBITMAP get_handle() { return m_hbmp;}
	
	HBITMAP detach_handle() {HBITMAP hbmp = m_hbmp; m_hbmp = 0; return hbmp;}
	
	surface<ColorType>* detach_pixmap() {
		surface<ColorType> *surf = m_surf; m_surf = 0; return surf;}
};

typedef surface<lib::color_trible> surface_t;
typedef dib_surface<lib::color_trible> dib_surface_t; 

inline BITMAPINFO* get_bmp_info(int width, int height, size_t depth) {
	static BITMAPINFO bmi;
	BITMAPINFOHEADER& h = bmi.bmiHeader;
	memset(&h, 0, sizeof(BITMAPINFOHEADER));
    h.biSize = sizeof(BITMAPINFOHEADER);
    h.biWidth = long(width);
    h.biHeight = long(height);
    h.biPlanes = 1;
    h.biBitCount = WORD(depth);
    h.biCompression = BI_RGB;
    h.biSizeImage = 0;
    h.biXPelsPerMeter = 0;
    h.biYPelsPerMeter = 0;
    h.biClrUsed = 0;
    h.biClrImportant = 0;
	memset(&bmi.bmiColors[0], 0, sizeof(RGBQUAD));
	return &bmi;
}


} // namespace dg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DG_DIB_SURFACE
