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

#ifndef AMBULANT_GUI_DG_DIB_SURFACE
#define AMBULANT_GUI_DG_DIB_SURFACE

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/lib/colors.h"
#include "ambulant/gui/dg/dg_surface.h"
#include "ambulant/lib/basic_types.h"

using ambulant::lib::uchar;

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
