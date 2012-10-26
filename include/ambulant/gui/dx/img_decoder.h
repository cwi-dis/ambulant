/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#ifndef AMBULANT_GUI_IMG_DECODER_H
#define AMBULANT_GUI_IMG_DECODER_H

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/lib/colors.h"
#include "ambulant/gui/dx/dx_surface.h"
#include "ambulant/lib/basic_types.h"

using ambulant::lib::uchar;

namespace ambulant {

namespace gui {

namespace dx {

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

template<class DataSource, class ColorType>
class img_decoder {
  public:
	img_decoder(DataSource *src, HDC hdc)
	:	m_src(src), m_hdc(hdc) {}
	virtual ~img_decoder() {}
	virtual bool can_decode() = 0;
	virtual dib_surface<ColorType>* decode() = 0;
	virtual bool is_transparent() { return false;}
	virtual void get_transparent_color(BYTE *rgb) {
		rgb[0] = 0; rgb[1] = 0; rgb[2] = 0;
	}
	virtual lib::color_t get_transparent_color() {
		BYTE rgb[3];get_transparent_color(rgb);
		return lib::to_color(rgb[0], rgb[1], rgb[2]);
	}

  protected:
	DataSource* m_src;
	HDC m_hdc;
};

inline BITMAPINFO* get_bmp_info(size_t width, size_t height, size_t depth) {
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

template<class DataSource>
struct file_reader {
	size_t (*read_file)(void *p, void *buf, unsigned long sizeofbuf);

	static size_t read_file_impl(void *p, void *buf, unsigned long sizeofbuf) {
		return ((file_reader*)p)->m_src->read((uchar_t*)buf, sizeofbuf);
	}

	file_reader(DataSource *src) : m_src(src) {
		m_src->seekg(0);
		read_file = &file_reader::read_file_impl;
	}

	DataSource *m_src;
};

struct color_quad {
	uchar b;
	uchar g;
	uchar r;
	uchar a;
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_IMG_DECODER_H
