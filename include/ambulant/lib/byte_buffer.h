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
