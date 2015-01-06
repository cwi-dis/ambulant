// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/lib/string_util.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;


//////////////////
// tokens_vector

lib::tokens_vector::tokens_vector(const char* entry, const char* delims) {
	std::string s = (!entry || !entry[0])?"":entry;
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




