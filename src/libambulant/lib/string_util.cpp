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

#include "ambulant/lib/string_util.h"

#include "ambulant/lib/logger.h"

using namespace ambulant;


//////////////////
// tokens_vector

lib::tokens_vector::tokens_vector(const char* entry, const char* delims) {
	std::string s = (!entry || !entry[0])?"":entry;
	typedef std::string::size_type size_type;
	size_type offset = 0;
	while(offset != std::string::npos) {
		size_type i = s.find_first_of(delims, offset);
		if(i != std::string::npos) {
			push_back(std::string(s.c_str() + offset, i-offset));
			offset = i+1;
		} else {
			push_back(std::string(s.c_str() + offset));
			offset = std::string::npos;
		}
	}	
}			

std::string lib::tokens_vector::join(size_type i, char sep) {
	std::string s;
	size_type n = size();
	if(i<n) s +=  (*this)[i++]; // this->at(i) seems missing from gcc 2.95
	for(;i<n;i++) {
		s += sep;
		s += (*this)[i];
	}
	return s;
}

// Splits the list, trims white space, skips any empty strings 
void lib::split_trim_list(const std::string& s, 
	std::list<std::string>& c, char ch) {
	typedef std::string::size_type size_type;
	size_type offset = 0;
	while(offset != std::string::npos) {
		size_type i = s.find_first_of(ch, offset);
		if(i != std::string::npos) {
			std::string entry = trim(std::string(s.c_str() + offset, i-offset));
			if(!entry.empty()) c.push_back(entry);
			offset = i+1;
		} else {
			std::string entry = trim(std::string(s.c_str() + offset));
			if(!entry.empty()) c.push_back(entry);
			offset = std::string::npos;
		}
	}	
}




