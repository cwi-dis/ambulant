
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_GUI_JPG_DECODER_H
#define AMBULANT_GUI_JPG_DECODER_H

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include "ambulant/gui/dx/img_decoder.h"
#include "ambulant/lib/logger.h"

#ifndef JPEGLIB_H
// avoid macro redefinition warning
#undef HAVE_STDDEF_H
#undef HAVE_STDLIB_H
extern "C" {
#include "jpeglib.h"
}
#endif

#pragma comment (lib,"libjpeg.lib")

namespace ambulant {

namespace gui {

namespace dx {

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
    if((b1 == 0xFF) && (b2 == 0xD8))
		return true;
	return false;
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
	BITMAPINFO *pbmpi = get_bitmapinfo(width, height, ColorType::get_bits_size());
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
	
} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_JPG_DECODER_H
