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
 
/* 
 * A byte buffer similar to java.nio package.
 */
 
#ifndef AMBULANT_LIB_BYTE_BUFFER_H
#define AMBULANT_LIB_BYTE_BUFFER_H

#include "ambulant/config/config.h"

// assert
#include <cassert>

namespace ambulant {

namespace lib {


class byte_buffer {	
  public:
	typedef unsigned char value_t;
	
	byte_buffer(int cap) 
	:	position(0),
		limit(cap),
		capacity(cap), 
		buf(new value_t[cap]) {
		assert(buf != 0);
	}
	
	~byte_buffer() {
		if(buf) delete[] buf;
	}
	
	int get_capacity() const { return capacity;}
	
	int get_limit() const { return limit;}
	
	int get_position() const { return position;}
	
	int remaining() const { return limit-position;}
	
	void set_position(int pos) {
		assert(pos>=0 && pos<=limit);
		position = pos;
	}
	
	void set_limit(int lim) {
		assert(lim>=0 && lim<=capacity);
		limit = lim;
	}
	
    void clear() {
		position = 0;
		limit = capacity;
    }
    
	void flip() {
		limit = position;
		position = 0;
	}
	
	void compact() {
		memmove(buf, buf+position, remaining());
		position = remaining();
		limit = capacity;
	}
	
	value_t* data() { return buf;}
	
  private:
	int position;
	int limit;
	const int capacity;
	value_t *buf;
};


} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_BYTE_BUFFER_H
