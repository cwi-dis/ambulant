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

#ifndef AMBULANT_GUI_DX_SURFACE_H
#define AMBULANT_GUI_DX_SURFACE_H

#include "ambulant/config/config.h"

namespace ambulant {

namespace gui {

namespace dx {

typedef unsigned char uchar_t;
typedef unsigned char* uchar_ptr;

// type T properties
// operator=(const& T)
// operator=(const& rgbquadT)
// T(int r, int g, int b) family
// operator==(const trible &lhs, const trible &rhs)

template<class T>
class surface {
	public:
	typedef T value_type;

	typedef T* pointer;
	typedef const T* const_pointer;

	typedef T& reference;
	typedef const T& const_reference;

	surface(size_t width, size_t height, size_t depth, pointer data)
	:	m_width(width), 
		m_height(height), 
		m_depth(depth), 
		m_pitch((width*sizeof(T)+3) & ~3), 
		m_data(data) {
	}
	
	~surface() {}

	void fill(value_type bkcolor) {
		uchar_ptr pb = uchar_ptr(m_data);
		for (int y=m_height-1;y>=0;y--) {
			pointer ptr = pointer(pb);
			for(size_t x=0;x<m_width;x++)
				*ptr++ = bkcolor;
			pb += m_pitch;
		}
	}

	reference pixel(size_t x, size_t y) { 
		if(x>=m_width || y>=m_height) throw_range_error();
		uchar_ptr pb = uchar_ptr(m_data) + (m_height - 1 - y)*m_pitch;
		pointer ptr = pointer(pb);
		return  ptr[x];
	}

	const_reference pixel(size_t x, size_t y) const { 
		return pixel(x,y);
	}

	value_type set_pixel(size_t x, size_t y, T v) {
		reference r = pixel(x, y); 
		value_type t = r; r = v; 
		return t;
	}

	value_type get_pixel(size_t x, size_t y) const { return pixel(x, y);}

	pointer get_row(size_t y) {
		if(y>=m_height) throw_range_error();
		uchar_ptr pb = uchar_ptr(m_data) + (m_height - 1 - y)*m_pitch;
		return pointer(pb);
	}

	size_t get_width() const {return m_width;}
	size_t get_height() const {return m_height;}
	size_t get_depth() const {return m_depth;}
	uchar_ptr get_buffer() {return uchar_ptr(m_data);}
	
	// argument is a surface with palette
	template <class rgbquadT>
	void fill(surface<uchar_t>& surf, rgbquadT *pquad, int n)  {
		for (int y=m_height-1;y>=0;y--) {
			pointer ptr = get_row(y);
			surface<uchar_t>::const_pointer pc = surf.get_row(y);
			for(size_t x=0;x<m_width;x++) {
				int cix = pc[x];
				if(cix < n) ptr[x] = pquad[cix];
				else ptr[x] = 0;
			}
		}
	}

	void blend(surface<T>& from, surface<T>& to, double prop) {
		prop = prop<0.0?0.0:(prop>1.0?1.0:prop);
		int weight = int(prop*256.0+0.5);
		for (int y=m_height-1;y>=0;y--) {
			pointer ptr = get_row(y);
			surface<T>::pointer pfrom = from.get_row(y);
			surface<T>::pointer pto = to.get_row(y);
			for(size_t x=0;x<m_width;x++) {
				value_type& tfrom = pfrom[x];
				value_type& tto = pto[x];
				ptr[x] = value_type(
					blend(weight, tfrom.red(), tto.red()),
					blend(weight, tfrom.green(), tto.green()),
					blend(weight, tfrom.blue(), tto.blue()));
			}
		}
	}

	void copy_transparent(surface<T> *from, uchar_ptr rgb, int from_dx = 0, int from_dy = 0) {
		value_type transp(rgb[0], rgb[1], rgb[2]);
		for (int y=m_height-1;y>=0;y--) {
			pointer ptr = get_row(y);
			surface<T>::pointer pfrom = from->get_row(from_dy + y);
			if(pfrom == 0) break;
			for(size_t x=0;x<m_width;x++) {
				value_type& tfrom = pfrom[from_dx + x];
				if(tfrom != transp)
					ptr[x] = tfrom;
			}
		}
	}

  private:
	uchar_t blend(int w, uchar_t c1, uchar_t c2) {
		return (uchar_t)(c1==c2)?c1:(c1 + w*(c2-c1)/256);
	}

	void throw_range_error() {
		throw std::range_error("index out of range");
	}

	size_t m_width;
	size_t m_height;
	size_t m_depth;
	size_t m_pitch;
	pointer m_data;
};

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_SURFACE_H
