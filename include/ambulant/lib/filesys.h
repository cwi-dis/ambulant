/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_FILESYS_H
#define AMBULANT_LIB_FILESYS_H

#include <string>
#include <list>

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

std::string join(const char *psz1, const char *psz2, 
	const char *ossep = 0) {
	if(!psz1 && !psz2) return "";
	if(!psz2 || !psz2[0]) return psz1;
	if(!psz1 || !psz1[0]) return psz2;
	return join(std::string(psz1), std::string(psz2), ossep);
}

std::string join(const std::string& str1, const char *psz2,
	const char *ossep = 0) {
	if(!psz2 || !psz2[0]) return str1;
	return join(str1, std::string(psz2), ossep);
}

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

} // namespace filesys
 
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_FILESYS_H
