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

#ifndef AMBULANT_GUI_JPG_DECODER_H
#define AMBULANT_GUI_JPG_DECODER_H

#ifndef _WINDOWS_
#include <windows.h>
#endif
#include "ambulant/config/config.h"

#include "ambulant/gui/dg/img_decoder.h"
#include "ambulant/lib/logger.h"

#ifndef JPEGLIB_H
// avoid macro redefinition warning
#undef HAVE_STDDEF_H
#undef HAVE_STDLIB_H
extern "C" {
#include "jpeglib.h"
}
#endif

namespace ambulant {

namespace gui {

namespace dg {

template <class DataSource, class ColorType>
class jpg_decoder : public img_decoder<DataSource, ColorType> {
  public:
	typedef unsigned int JDIMENSION;
	typedef unsigned char JSAMPLE;
	typedef JSAMPLE* JSAMPROW;	
	typedef JSAMPROW* JSAMPARRAY;

	jpg_decoder(DataSource *src, HDC hdc);
	virtual ~jpg_decoder();

	virtual bool can_decode();
	virtual dib_surface<ColorType>* decode();

	private:
	void write_pixel_rows(j_decompress_ptr cinfo, surface<ColorType> *psurf);
	
	void create_buffer(int row_width);
	void free_buffer();

	JSAMPARRAY m_dbuffer;
	JDIMENSION m_dbuffer_height;
	JDIMENSION m_cur_output_row;
	lib::logger *m_logger;
};

template <class DataSource, class ColorType>
jpg_decoder<DataSource, ColorType>::jpg_decoder(DataSource* src, HDC hdc)
:	img_decoder<DataSource, ColorType>(src, hdc), 
	m_dbuffer(0), m_dbuffer_height(1),
	m_cur_output_row(0),
	m_logger(lib::logger::get_logger()) {
}

template <class DataSource, class ColorType>
jpg_decoder<DataSource, ColorType>::~jpg_decoder() {
	if(m_dbuffer != 0) free_buffer();
}

template <class DataSource, class ColorType>
void jpg_decoder<DataSource, ColorType>::create_buffer(int row_width) {
	if(m_dbuffer != 0) free_buffer();
	m_dbuffer = new JSAMPROW[m_dbuffer_height];
	for(JDIMENSION i=0;i<m_dbuffer_height;i++)
		m_dbuffer[i] = new JSAMPLE[row_width];
}

template <class DataSource, class ColorType>
void jpg_decoder<DataSource, ColorType>::free_buffer() {
	if(m_dbuffer != 0) {
		for(JDIMENSION i=0;i<m_dbuffer_height;i++)
			delete[] m_dbuffer[i];
		delete[] m_dbuffer;
		m_dbuffer = 0;
	}
}

template <class DataSource, class ColorType>
bool jpg_decoder<DataSource, ColorType>::can_decode() {
	m_src->seekg(0);
    uchar_t b1 = m_src->get();
    uchar_t b2 = m_src->get();
    return ((b1 == 0xFF) && (b2 == 0xD8))?true:false;
}

template <class DataSource, class ColorType>
dib_surface<ColorType>* 
jpg_decoder<DataSource, ColorType>::decode() {
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	
	// Initialize the JPEG decompression object with default error handling.
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// Specify data source for decompression
	file_reader<DataSource> fr(m_src);
	jpeg_stdio_src(&cinfo, &fr);

	// Read file header, set default decompression parameters
	int res = jpeg_read_header(&cinfo, TRUE);

	// Calculate output image dimensions so we can allocate space
	jpeg_calc_output_dimensions(&cinfo);
	
	// Start decompressor
	jpeg_start_decompress(&cinfo);
		
	JDIMENSION row_width = cinfo.output_width * cinfo.output_components;
	
	// release/create buffer
	if(m_dbuffer != 0) free_buffer();
	create_buffer(row_width);

	int width = cinfo.output_width;
	int height = cinfo.output_height;

	// create a bmp surface
	ColorType *pBits = NULL;
	BITMAPINFO *pbmpi = get_bmp_info(width, height, ColorType::get_bits_size());
	HBITMAP bmp = CreateDIBSection(m_hdc, pbmpi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
	if(bmp==NULL || pBits==NULL) {
		m_logger->error("CreateDIBSection() failed");
		jpeg_destroy_decompress(&cinfo);
		return NULL;
	}
	
	surface<ColorType> *psurf = 
		new surface<ColorType>(width, height, ColorType::get_bits_size(), pBits);

	// Process data
	m_cur_output_row = 0;
	while(cinfo.output_scanline < cinfo.output_height)  {
		int num_scanlines = jpeg_read_scanlines(&cinfo, m_dbuffer, m_dbuffer_height);
		if(cinfo.out_color_space == JCS_RGB && cinfo.quantize_colors != TRUE)
			write_pixel_rows(&cinfo, psurf);
	}

	// jpeg cleanup
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return new dib_surface<ColorType>(bmp, psurf);
}

template <class DataSource, class ColorType>
void jpg_decoder<DataSource, ColorType>::write_pixel_rows(j_decompress_ptr cinfo, 
	surface<ColorType> *psurf) {
	JSAMPROW inptr = m_dbuffer[0];
	ColorType* outptr = psurf->get_row(m_cur_output_row);
	for(JDIMENSION col = 0; col < cinfo->output_width; col++)  {
		BYTE r = *inptr++;
		BYTE g = *inptr++;
		BYTE b = *inptr++;
		*outptr++ = ColorType(r, g, b);
	}
	m_cur_output_row++;
}
	
} // namespace dg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_JPG_DECODER_H
