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

#ifndef AMBULANT_LIB_FILESYS_H
#define AMBULANT_LIB_FILESYS_H

#include "ambulant/config/config.h"

#include <string>
#include <list>

#include "ambulant/lib/asb.h"

namespace ambulant {

namespace lib {

namespace filesys {

// Notes:
// * Illegal path components are allowed.
// * The navigation operators ../ ./ are not removed
// * Paths starting with / are not interpretated as special
// * On windows we may have two separators '/' and '\'
//	 and therefore, we use sep as a string for simplicity.

const std::string url_path_sep("/");

//////////////////////////
// Export the following functions:

// -- getcwd()
// Returns the current working directory
// std::string lib::filesys::getcwd();

// -- resolve_path()
// Returns the absolute and normalized form of a path.
// Takes into account the current working directory
// Removes '.', '..' from the path name
// std::string lib::filesys::resolve_path(const char *p);

#ifdef WIN32
using ambulant::lib::win32::getcwd;
using ambulant::lib::win32::resolve_path;
#else
inline std::string getcwd() {return "";}
inline std::string resolve_path(const char *p) { return p;}
#endif

//////////////////////////
// The following are utility functions for
// path manipulations.

inline
std::string join(const std::string& str1, const std::string& str2,
	const char *ossep = 0) {
	char ch1 = *str1.rbegin();
	char ch2 = *str2.begin();
	std::string sep = (ossep && ossep[0])?ossep:url_path_sep;
	int nsep = 0;
	if(sep.find_first_of(ch1) != std::string::npos) nsep++;
	if(sep.find_first_of(ch2) != std::string::npos) nsep++;
	std::string str;
	if(nsep == 0)
		str = str1 + sep[0] + str2;
	else if(nsep == 1)
		str = str1 + str2;
	else if(nsep == 2)
		str = str1 + str2.substr(1);
	return str;
}

inline
std::string join(const char *psz1, const char *psz2, 
	const char *ossep = 0) {
	if(!psz1 && !psz2) return "";
	if(!psz2 || !psz2[0]) return psz1;
	if(!psz1 || !psz1[0]) return psz2;
	return join(std::string(psz1), std::string(psz2), ossep);
}

inline
std::string join(const std::string& str1, const char *psz2,
	const char *ossep = 0) {
	if(!psz2 || !psz2[0]) return str1;
	return join(str1, std::string(psz2), ossep);
}

inline
void split(const std::string& s, std::list<std::string>& c, 
	const char *ossep = 0) {
	std::string sep = (ossep && ossep[0])?ossep:url_path_sep;
	typedef std::string::size_type size_type;
	size_type offset = 0;
	while(offset != std::string::npos) {
		size_type i = s.find_first_of(sep.c_str(), offset);
		if(i != std::string::npos) {
			c.push_back(std::string(s.c_str() + offset, i-offset));
			offset = i+1;
		} else {
			c.push_back(std::string(s.c_str() + offset));
			offset = std::string::npos;
		}
	}	
}

inline
std::string join(const std::list<std::string>& c, 
	const char *ossep = 0) {
	std::string sep = (ossep && ossep[0])?ossep:url_path_sep;
	std::list<std::string>::const_iterator it = c.begin();
	if(it == c.end()) return std::string();
	std::string s(*it++);
	while(it != c.end()) {
		s += sep[0];
		s += (*it);
		it++;
	}
	return s;
}

inline 
std::string get_base(const std::string& s,
	const char *ossep = 0) {
	std::string sep = (ossep && ossep[0])?ossep:url_path_sep;
	typedef std::string::size_type size_type;
	size_type i = s.find_last_of(sep);
	if(i == std::string::npos)
		return std::string();
	return s.substr(0, i);
}

} // namespace filesys
 
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_FILESYS_H
