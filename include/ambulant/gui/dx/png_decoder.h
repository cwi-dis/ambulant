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

#ifndef AMBULANT_GUI_PNG_DECODER_H
#define AMBULANT_GUI_PNG_DECODER_H

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include "png.h"

#include <cassert>

#include "ambulant/config/config.h"
#include "ambulant/gui/dx/img_decoder.h"
#include "ambulant/lib/logger.h"

namespace ambulant {

namespace gui {

namespace dx {

template <class DataSource, class ColorType>
class png_decoder : public img_decoder<DataSource, ColorType> {
  public:
	png_decoder(DataSource *src, HDC hdc);
	virtual ~png_decoder();
	virtual bool can_decode();
	virtual dib_surface<ColorType>* decode();
	static void png_read_mem_data(png_structp png_ptr, png_bytep data, png_uint_32 length);
  private:
	png_color m_bgrclr;
	bool m_has_bgr;

	lib::logger *m_logger;
};

template <class DataSource, class ColorType>
png_decoder<DataSource, ColorType>::png_decoder(DataSource* src, HDC hdc)
:	img_decoder<DataSource, ColorType>(src, hdc),
	m_has_bgr(false),
	m_logger(lib::logger::get_logger()) {
}

template <class DataSource, class ColorType>
png_decoder<DataSource, ColorType>::~png_decoder() {
}

template <class DataSource, class ColorType>
void png_decoder<DataSource, ColorType>::png_read_mem_data(png_structp png_ptr,
	png_bytep data, png_uint_32 length) {
	DataSource *src = (DataSource*)png_ptr->io_ptr;
	DataSource::size_type n = src->read(data, length);
	if(n != DataSource::size_type(length)) {
		png_error(png_ptr, "Read Error");
	}
}

template <class DataSource, class ColorType>
inline bool png_decoder<DataSource, ColorType>::can_decode() {
	m_src->seekg(0);
	uchar_t sig[8];
	if(m_src->read(sig, 8) != 8) return false;
	return (sig[0] == (uchar_t)137 &&
		sig[1] == (uchar_t)80 &&
		sig[2] == (uchar_t)78 &&
		sig[3] == (uchar_t)71 &&
		sig[4] == (uchar_t)13 &&
		sig[5] == (uchar_t)10 &&
		sig[6] == (uchar_t)26 &&
		sig[7] == (uchar_t)10);
}

template <class DataSource, class ColorType>
inline dib_surface<ColorType>*
png_decoder<DataSource, ColorType>::decode() {
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);
	if(!png_ptr) {
		m_logger->error("png_create_read_struct() failed");
		return 0;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		m_logger->error("png_create_info_struct() failed");
		return 0;
	}

	m_src->seekg(8);
	png_set_read_fn(png_ptr, (png_voidp)m_src, (png_rw_ptr)png_read_mem_data);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	// get width, height, bit-depth and color-type
	png_uint_32 width, height;
	int depth, clrtype;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &depth, &clrtype, NULL, NULL, NULL);
	AM_DBG m_logger->debug("PNG: %dx%d [depth:%d clrtype:%d]", width, height, depth, clrtype);

	// expand images of all color-type and bit-depth to 3x8 bit RGB images
	// let the library process things like alpha, transparency, background
	if(depth == 16) png_set_strip_16(png_ptr);
	if(clrtype == PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr);
	if(depth < 8) png_set_expand(png_ptr);
	if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_expand(png_ptr);
	if(clrtype == PNG_COLOR_TYPE_GRAY || clrtype == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	// set the background color to draw transparent and alpha images over.
	png_color_16 *pbgr;
	if(png_get_bKGD(png_ptr, info_ptr, &pbgr)) {
		png_set_background(png_ptr, pbgr, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
		m_bgrclr.red   = (byte) pbgr->red;
		m_bgrclr.green = (byte) pbgr->green;
		m_bgrclr.blue  = (byte) pbgr->blue;
		m_has_bgr = true;
	}

	// if required set gamma conversion
	double gamma;
	if(png_get_gAMA(png_ptr, info_ptr, &gamma))
		png_set_gamma(png_ptr, (double) 2.2, gamma);

	// after the transformations have been registered update info_ptr data
	png_read_update_info(png_ptr, info_ptr);

	// get again width, height and the new bit-depth and color-type
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &depth, &clrtype, NULL, NULL, NULL);
	AM_DBG m_logger->debug("PNG: %dx%d [depth:%d clrtype:%d]", width, height, depth, clrtype);

	// row_bytes = width x channels
	png_uint_32 row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	png_uint_32 channels = png_get_channels(png_ptr, info_ptr);
	assert(row_bytes == width * channels);
	AM_DBG m_logger->debug("PNG: row_bytes = %d, channels=%d", row_bytes, channels);
	if(channels	!= 3 && channels != 4) {
		m_logger->warn("PNG: Seen: %d channels. Supported: 3/4 channels", channels);
		png_destroy_info_struct(png_ptr, &info_ptr);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 0; // failed
	} else if (channels == 4)
		// ignore alpha channel
		png_set_strip_alpha (png_ptr);

	// create a bmp surface
	ColorType *pBits = NULL;
	BITMAPINFO *pbmpi = get_bmp_info(width, height, ColorType::get_bits_size());
	HBITMAP bmp = CreateDIBSection(NULL, pbmpi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
	if(bmp==NULL || pBits==NULL) {
		m_logger->error("CreateDIBSection() failed");
		png_destroy_info_struct(png_ptr, &info_ptr);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 0; // failed
	}
	surface<ColorType> *psurf =
		new surface<ColorType>(width, height, ColorType::get_bits_size(), pBits);

	png_bytep *row_ptrs = new png_bytep[height];
	for(png_uint_32 i=0;i<height;i++)
		row_ptrs[i] = (png_bytep) psurf->get_row(i);
	png_read_image(png_ptr, row_ptrs);
	png_read_end(png_ptr, NULL);
	delete[] row_ptrs;
	png_destroy_info_struct(png_ptr, &info_ptr);
	png_destroy_read_struct(&png_ptr, NULL, NULL);

	// reverse channels inline
	psurf->rev_rgb_channels();

	return new dib_surface<ColorType>(bmp, psurf);
}

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_PNG_DECODER_H
