/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
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

#ifndef AMBULANT_GUI_BMP_DECODER_H
#define AMBULANT_GUI_BMP_DECODER_H

#include "ambulant/config/config.h"

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include <cassert>

#include "ambulant/gui/dg/img_decoder.h"
#include "ambulant/lib/logger.h"

namespace ambulant {

namespace gui {

namespace dg {

template <class DataSource>
class bmp_decoder : public img_decoder<DataSource> {
  public:
	bmp_decoder(DataSource *src, HDC hdc);
	virtual ~bmp_decoder();
	virtual bool can_decode();
	virtual dib_surface<surf_color_t>* decode();
  private:
	size_t get_pitch_from_bpp(size_t bpp, size_t width) { return (width*(bpp/8)+3) & ~3;}
	lib::logger *m_logger;
};

template <class DataSource>
inline bmp_decoder<DataSource>::bmp_decoder(DataSource* src, HDC hdc)
:	img_decoder<DataSource>(src, hdc), 
	m_logger(lib::logger::get_logger()) {
}

template <class DataSource>
inline bmp_decoder<DataSource>::~bmp_decoder() {
}

template <class DataSource>
inline bool bmp_decoder<DataSource>::can_decode() {
	m_src->seekg(0);
	BITMAPFILEHEADER bfh;
	if(m_src->read((BYTE*)&bfh, sizeof(bfh)) != sizeof(bfh))
		return false;
	char* ptr = (char*)&bfh.bfType;
	if(*ptr!='B' || *++ptr != 'M')
		return false;
	return true;
}

template <class DataSource>
inline dib_surface<surf_color_t>* 
bmp_decoder<DataSource>::decode() {
	m_src->seekg(0);
	BITMAPFILEHEADER bfh;
	if(m_src->read((BYTE*)&bfh, sizeof(bfh)) != sizeof(bfh)) {
		m_logger->error("decode() failed. Not a valid BMP");
		return NULL;
	}

	char* ptr = (char*)&bfh.bfType;
	if (*ptr!='B' || *++ptr!='M') {
		m_logger->error("decode() failed. Not a valid BMP");
		return NULL;
	}
	
	BITMAPINFOHEADER bmi;
	if(m_src->read((BYTE*)&bmi, sizeof(bmi)) != sizeof(bmi)) {
		m_logger->error("decode() failed. Not a valid BMP");
		return NULL;
	}
	if(bmi.biCompression != BI_RGB) {
		m_logger->error("decode() failed. unsupported compressed BMP format");
		return NULL;
	}
	
	if(bmi.biBitCount != 24 && bmi.biBitCount != 16 && bmi.biBitCount != 8) {
		m_logger->error("decode() failed. Unsupported bits per pixel BMP");
		return NULL;
	}

	int width = bmi.biWidth;
	int height = bmi.biHeight;
	int depth = bmi.biBitCount;
	if (bmi.biSizeImage == 0) 
		bmi.biSizeImage = (DWORD)get_pitch_from_bpp(depth, width)*height;

	surf_color_t *pBits = NULL;
	BITMAPINFO *pbmpi = get_bmp_info(width, height, surf_color_t::get_bits_size());
	HBITMAP hBmp = CreateDIBSection(NULL, pbmpi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
	if(hBmp==NULL || pBits==NULL) {
		m_logger->error("CreateDIBSection() failed");
		return NULL;
	}
	surface<surf_color_t> *psurf = 
		new surface<surf_color_t>(width, height, surf_color_t::get_bits_size(), pBits);
	m_src->seekg(sizeof(bfh) + bmi.biSize);
	if(depth == 24 && surf_color_t::get_bits_size() == 24) {
		memcpy(psurf->get_buffer(), m_src->data(), bmi.biSizeImage);
	} else {
		return 0;
	}
	return new dib_surface<surf_color_t>(hBmp, psurf);
}
} // namespace dg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_BMP_DECODER_H
