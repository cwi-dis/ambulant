/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

/*
 * @$Id$
 */

#ifndef AMBULANT_GUI_BMP_DECODER_H
#define AMBULANT_GUI_BMP_DECODER_H

#include "ambulant/config/config.h"
#ifdef WITH_D2D
#error Including dx include file while building for Direct2D
#endif
#ifndef _WINDOWS_
#include <windows.h>
#endif

#include <cassert>

#include "ambulant/gui/dx/img_decoder.h"
#include "ambulant/lib/logger.h"

namespace ambulant {

namespace gui {

namespace dx {

template <class DataSource, class ColorType>
class bmp_decoder : public img_decoder<DataSource, ColorType> {
  public:
	bmp_decoder(DataSource *src, HDC hdc);
	virtual ~bmp_decoder();
	virtual bool can_decode();
	virtual dib_surface<ColorType>* decode();
  private:
	size_t get_pitch_from_bpp(size_t bpp, size_t width) { return (width*(bpp/8)+3) & ~3;}
	lib::logger *m_logger;
};

template <class DataSource, class ColorType>
inline bmp_decoder<DataSource, ColorType>::bmp_decoder(DataSource* src, HDC hdc)
:	img_decoder<DataSource, ColorType>(src, hdc),
	m_logger(lib::logger::get_logger()) {
}

template <class DataSource, class ColorType>
inline bmp_decoder<DataSource, ColorType>::~bmp_decoder() {
}

template <class DataSource, class ColorType>
inline bool bmp_decoder<DataSource, ColorType>::can_decode() {
	m_src->seekg(0);
	BITMAPFILEHEADER bfh;
	if(m_src->read((BYTE*)&bfh, sizeof(bfh)) != sizeof(bfh))
		return false;
	char* ptr = (char*)&bfh.bfType;
	if(*ptr!='B' || *++ptr != 'M')
		return false;
	return true;
}

template <class DataSource, class ColorType>
inline dib_surface<ColorType>*
bmp_decoder<DataSource, ColorType>::decode() {
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

	ColorType *pBits = NULL;
	BITMAPINFO *pbmpi = get_bmp_info(width, height, (int) ColorType::get_bits_size());
	HBITMAP hBmp = CreateDIBSection(NULL, pbmpi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
	if(hBmp==NULL || pBits==NULL) {
		m_logger->error("CreateDIBSection() failed");
		return NULL;
	}
	surface<ColorType> *psurf =
		new surface<ColorType>(width, height, ColorType::get_bits_size(), pBits);
	m_src->seekg(sizeof(bfh) + bmi.biSize);
	if(depth == 24 && ColorType::get_bits_size() == 24) {
		memcpy(psurf->get_buffer(), m_src->data(), bmi.biSizeImage);
	} else {
		return 0;
	}
	return new dib_surface<ColorType>(hBmp, psurf);
}
} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_BMP_DECODER_H
