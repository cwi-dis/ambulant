
/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_IMG_DECODER_H
#define AMBULANT_GUI_IMG_DECODER_H

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include "ambulant/gui/dx/dx_surface.h"

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
	virtual bool is_transparent() { return false; }
	virtual void get_transparent_color(uchar_t *rgb) {}

  protected:
	DataSource* m_src;
	HDC m_hdc;
};

inline BITMAPINFO* get_bitmapinfo(size_t width, size_t height, size_t depth) {
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

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_IMG_DECODER_H
