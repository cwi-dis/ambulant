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

#ifndef AMBULANT_GUI_GIF_DECODER_H
#define AMBULANT_GUI_GIF_DECODER_H

#include "ambulant/config/config.h"

#ifndef _WINDOWS_
#include <windows.h>
#endif

#include <cassert>


#include "ambulant/gui/dx/img_decoder.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/basic_types.h"

using ambulant::lib::uchar;

namespace ambulant {

namespace gui {

namespace dx {

template <class DataSource, class ColorType>
class gif_decoder : public img_decoder<DataSource, ColorType> {
  public:
	typedef unsigned short uint16_t;

	gif_decoder(DataSource* src, HDC hdc);

	virtual ~gif_decoder();

	virtual bool can_decode();
	virtual dib_surface<ColorType>* decode();
	virtual bool is_transparent()
		{ return m_transparent>=0;}
	virtual void get_transparent_color(BYTE *rgb) {
		if(m_transparent>=0) {
			color_quad& t = m_palette[m_transparent];
			rgb[0] = t.r; rgb[1] = t.g; rgb[2] = t.b;
			AM_DBG m_logger->debug("Gif transparent color (%d, %d, %d)", int(t.r), int(t.g), int(t.b));
		} else {
			rgb[0] = 0; rgb[1] = 0; rgb[2] = 0;
		}
	}

  private:
	dib_surface<ColorType>* parse_metadata();
	dib_surface<ColorType>* parse_image();
	bool parse_image_pixels(int width, int height,
		int interlace, surface<ColorType> *psurf);

	int next_block(uchar *m_buf);
	int next_code(int code_size, bool flag);
	int next_lzwbyte(bool flag, int input_code_size);
	void skip_block();
	int get_data_block(uchar *buf);

	UINT to_uint(uchar a, uchar b) { return (b<<8) | a;}

	color_quad *m_palette;
	int m_scr_width;
	int m_scr_height;
	int m_scr_colors;

	int m_transparent;
	int m_delayTime;
	int m_inputFlag;
	int m_disposal;

	int m_curbit, m_lastbit, m_lastbyte;
	bool m_done;
	int m_last_block_size;
	uchar m_buf[280];

	enum { MAX_LZW_BITS = 12};
	lib::logger *m_logger;
};

template <class DataSource, class ColorType>
gif_decoder<DataSource, ColorType>::gif_decoder(DataSource* src, HDC hdc)
:	img_decoder<DataSource, ColorType>(src, hdc),
	m_palette(NULL),
	m_curbit(0), m_lastbit(0), m_lastbyte(0),
	m_last_block_size(-1), m_done(false),
	m_transparent(-1),
	m_logger(lib::logger::get_logger()) {
}

template <class DataSource, class ColorType>
inline gif_decoder<DataSource, ColorType>::~gif_decoder() {
	if(m_palette) delete[] m_palette;
}

template <class DataSource, class ColorType>
inline bool gif_decoder<DataSource, ColorType>::can_decode() {
	m_src->seekg(0);
	uchar_t b[16];
	if(m_src->read(b, 6) != 6) return false;
	return b[0] == 'G' && b[1] == 'I' && b[2] == 'F' && b[3] == '8' &&
		(b[4] == '7' || b[4] == '9') && b[5] == 'a';
}

template <class DataSource, class ColorType>
inline dib_surface<ColorType>*
gif_decoder<DataSource, ColorType>::decode() {
	if(!can_decode()) return NULL;

	m_scr_width = m_src->get_be_ushort();
	m_scr_height = m_src->get_be_ushort();

	uchar_t uch = m_src->get();
	m_scr_colors = 2 << (uch & 0x07);
	int screen_color_res = ((uch & 0x70) >> 3) + 1;

	int screen_bg = m_src->get();
	int screen_aspect_ration = m_src->get();

	if(m_palette != NULL) {
		delete[] m_palette;
		m_palette = NULL;
	}
	if((uch & 0x80) == 0x80) {
		AM_DBG m_logger->debug("Gif palette entries = %d", int(m_scr_colors));
		m_palette = new color_quad[m_scr_colors];
		memset(m_palette, 0, m_scr_colors*sizeof(color_quad));
		for(int i=0; i<m_scr_colors;i++) {
			uchar_t rgb[3];
			if(m_src->read(rgb, 3) != 3) break;
			m_palette[i].a = 0;
			m_palette[i].r = rgb[0];
			m_palette[i].g = rgb[1];
			m_palette[i].b = rgb[2];
		}
	}
	return parse_metadata();
}

template <class DataSource, class ColorType>
inline dib_surface<ColorType>*
gif_decoder<DataSource, ColorType>::parse_metadata() {
	uchar ext_buf[256];
	while(true) {
		uchar_t blockType = m_src->get();
		if(blockType == 0x2c) {
			AM_DBG m_logger->debug("Image Descriptor");
			return parse_image();
		} else if (blockType == 0x21) {
			AM_DBG m_logger->debug("Extension block");
			uchar_t label = m_src->get();
			if(label == 0xf9) {
				AM_DBG m_logger->debug("Graphics Control Extension");
				if(get_data_block(ext_buf)>0) {
					m_disposal= (ext_buf[0]>>2)	& 0x7;
					m_inputFlag	= (ext_buf[0]>>1) & 0x1;
					m_delayTime	= to_uint(ext_buf[1], ext_buf[2]);
					if((ext_buf[0] & 0x1) != 0) {
						m_transparent = ext_buf[3];
						AM_DBG m_logger->debug("Gif transparent index = %d", int(m_transparent));
					}
				}
				skip_block();
			} else if (label == 0x1) {
				AM_DBG m_logger->debug("Plain text extension");
				skip_block();
			} else if (label == 0xfe) {
				AM_DBG m_logger->debug("Comment extension");
				skip_block();
			} else if (label == 0xff) {
				AM_DBG m_logger->debug("Application extension");
				skip_block();
			} else {
				AM_DBG m_logger->debug("Unknown extension");
				skip_block();
			}
		}
	}
	return 0;
}

template <class DataSource, class ColorType>
inline void gif_decoder<DataSource, ColorType>::skip_block() {
	int length = 0;
	do	{
		length = m_src->get();
		m_src->skip(length);
	} while (length > 0);
}

template <class DataSource, class ColorType>
inline int gif_decoder<DataSource, ColorType>::get_data_block(uchar *buf) {
	int length = m_src->get();
	if(m_src->read(buf, length) != length)
		return -1;
	return length;
}

// 9 bytes header + color_map + data
template <class DataSource, class ColorType>
inline dib_surface<ColorType>*
gif_decoder<DataSource, ColorType>::parse_image() {
	uint16_t imageLeftPosition = m_src->get_be_ushort();
	uint16_t imageTopPosition = m_src->get_be_ushort();
	uint16_t imageWidth = m_src->get_be_ushort();
	uint16_t imageHeight = m_src->get_be_ushort();

	uchar_t packedFields = m_src->get();
	bool localColorTableFlag = (packedFields & 0x80) != 0;
	bool interlaceFlag = (packedFields & 0x40) != 0;
	bool sortFlag = (packedFields & 0x20) != 0;
	int numLCTEntries = 1 << ((packedFields & 0x7) + 1);
	if(localColorTableFlag) {
		// read color table
		// numLCTEntries colors
		//cout << "has local color table" << endl;
		m_src->skip(numLCTEntries*3);
	}
	//else cout << "uses global color table" << endl;
	AM_DBG  m_logger->debug("ImageDescription: %d x %d",  int(imageWidth), int(imageHeight));

	/////////////
	// create a bmp surface
	ColorType *pBits = NULL;
	BITMAPINFO *pbmpi = get_bmp_info(imageWidth, imageHeight, ColorType::get_bits_size());
	HBITMAP bmp = CreateDIBSection(NULL, pbmpi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
	if(bmp==NULL || pBits==NULL) {
		m_logger->error("CreateDIBSection() failed");
		return NULL;
	}

	surface<ColorType> *psurf =
		new surface<ColorType>(imageWidth, imageHeight, ColorType::get_bits_size(), pBits);

	parse_image_pixels(imageWidth, imageHeight, interlaceFlag, psurf);
	//uchar_t img_terminator = m_src->get(); // read ';'
	return new dib_surface<ColorType>(bmp, psurf);
}

template <class DataSource, class ColorType>
inline int gif_decoder<DataSource, ColorType>::next_block(uchar *buf) {
	uchar count = m_src->get();
	m_last_block_size = count;
	m_src->read(buf, count);
	return count;
}

template <class DataSource, class ColorType>
inline int gif_decoder<DataSource, ColorType>::next_code(int code_size, bool flag) {
	int i,j;
	uchar count;
	if(flag)  {
		m_curbit=0;
		m_lastbit=0;
		m_done = false;
		return 0;
	}

	if((m_curbit+code_size) >= m_lastbit) {
		if(m_done) {
			if(m_curbit >= m_lastbit)  {
				// "Ran off the end of bits";
				return 0;
			}
			return -1;
		}
		m_buf[0] = m_buf[m_lastbyte-2];
		m_buf[1] = m_buf[m_lastbyte-1];

		count = next_block(&m_buf[2]);
		if(count == 0) m_done = true;

		m_lastbyte = 2 + count;

		m_curbit = (m_curbit - m_lastbit) + 16;

		m_lastbit = (2 + count) * 8;
	}

	int ret=0;
	for (i=m_curbit,j=0; j<code_size;++i,++j)
		ret |= ((m_buf[i/8]&(1<<(i% 8)))!=0)<<j;
	m_curbit += code_size;
	return ret;
}

template <class DataSource, class ColorType>
inline int gif_decoder<DataSource, ColorType>::next_lzwbyte(bool flag, int input_code_size) {
	static bool fresh = false;
	int code, incode;
	static int code_size, set_code_size;
	static int max_code, max_code_size;
	static int firstcode, oldcode;
	static int clear_code, end_code;

	static unsigned short  next[1 << MAX_LZW_BITS];
	static uchar  vals[1 << MAX_LZW_BITS];
	static uchar  stack[1 << (MAX_LZW_BITS+1)];
	static uchar  *sp;

	int i;

	if(flag) {
		set_code_size = input_code_size;
		code_size = set_code_size + 1;
		clear_code = 1 << set_code_size;
		end_code = clear_code + 1;
		max_code = clear_code+2;
		max_code_size = 2*clear_code;

		next_code(0, true);

		fresh = TRUE;

		for(i=0;i<clear_code;++i) {
			next[i]=0;
			vals[i]=i;
		}

		for (;i<(1<<MAX_LZW_BITS);++i)
			next[i]=vals[0]=0;

		sp=stack;

		return 0;

	} else if (fresh) {
		fresh = false;
		do	{
			firstcode = oldcode = next_code(code_size, false);
		}	while (firstcode == clear_code);
		return firstcode;
	}

	if (sp > stack)
		return *--sp;

	while((code = next_code(code_size, false)) >=0) {
		if (code == clear_code) {
			for (i=0;i<clear_code;++i) {
				next[i]=0;
				vals[i]=i;
			}
			for (;i<(1<<MAX_LZW_BITS);++i)
				next[i] = vals[i] = 0;
			code_size = set_code_size + 1;
			max_code_size = 2*clear_code;
			max_code = clear_code+2;
			sp = stack;
			firstcode = oldcode = next_code(code_size, false);
			return firstcode;
		} else if (code == end_code) {
			int count;
			uchar m_buf[260];

			if (m_last_block_size == 0)
				return -2;

			while ((count = next_block(m_buf)) >0);

			if (count != 0)
				return -2;
		}

		incode = code;

		if (code >= max_code) {
			*sp++=firstcode;
			code=oldcode;
		}

		while (code >=clear_code) {
			*sp++=vals[code];
			if (code==(int)next[code]) {
				// "Circular table entry";
				return -1;
			}
			code=next[code];
		}

		*sp++ = firstcode=vals[code];

		if((code=max_code) <(1<<MAX_LZW_BITS)) {
			next[code]=oldcode;
			vals[code]=firstcode;
			++max_code;
			if ((max_code >=max_code_size) &&
				(max_code_size < (1<<MAX_LZW_BITS)))
			{
				max_code_size*=2;
				++code_size;
			}
		}

		oldcode = incode;

		if (sp > stack)
			return *--sp;
	}
	return code;
}


template <class DataSource, class ColorType>
inline bool gif_decoder<DataSource, ColorType>::parse_image_pixels(
	int width, int height, int interlace, surface<ColorType> *psurf) {
	assert(psurf);
	uchar c = m_src->get();
	int color;
	int xpos=0, ypos=0, pass=0;

	if (next_lzwbyte(true, c) < 0)
		return false;

	while((color = next_lzwbyte(false, c)) >= 0) {
		ColorType rgb(m_palette[color].r, m_palette[color].g, m_palette[color].b);
		psurf->set_pixel(xpos, ypos, rgb);
		++xpos;
		if(xpos==width) {
			xpos=0;
			if (interlace) {
				switch(pass) {
					case 0:
					case 1: ypos+=8; break;
					case 2: ypos+=4; break;
					case 3: ypos+=2; break;
				}

				if (ypos>=height) {
					++pass;
					switch (pass) {
						case 1: ypos=4;break;
						case 2: ypos=2;break;
						case 3: ypos=1;break;
						default : goto fini;
					}
				}
			} else {
				++ypos;
			}
		}
		if (ypos >= height)
			break;
	}

	fini:
	if(next_lzwbyte(false, c)>=0) {}
	return true;
}


} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_GIF_DECODER_H
