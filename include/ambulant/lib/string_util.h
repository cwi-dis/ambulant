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

#ifndef AMBULANT_LIB_STRING_UTIL_H
#define AMBULANT_LIB_STRING_UTIL_H

#include <string>
#include <vector>
#include <ctype.h>

namespace ambulant {

namespace lib {

class tokens_vector : public std::vector<std::string> {
  public:
    tokens_vector(const char* entry, const char* delims) {
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
	std::string join(size_type i, char sep) {
		std::string s;
		size_type n = size();
		if(i<n) s += at(i++);
		for(;i<n;i++) {
			s += sep;
			s += at(i);
		}
		return s;
	}
};

inline std::string trim(const std::string& s) {
	size_t i1 = s.find_first_not_of(" \r\n\t\v");
	if(i1==std::string::npos) return "";

	// XXXX Need a signed type here...
	size_t i2 = s.length();
	while(i2>0 && isspace(s[i2-1])) i2--;
	if(i2==0) return "";

	return std::string(s.c_str()+i1,s.c_str()+i2);
}

inline std::string trim(const char* psz) {
	std::string s(psz);
	return trim(s);
}

inline std::string xml_quote(const char *p) {
	std::string qs;
	if(p) {
		while(*p) {
			if(*p=='<') qs += "&lt;";
			else if(*p=='>') qs += "&gt;";
			else if(*p=='&') qs += "&amp;";
			else qs += *p;
			p++;
		}
	}
	return qs;
}

inline std::string xml_quote(const std::string& s)
	{ return xml_quote(s.c_str());}

inline bool starts_with(const std::string& s, int offset, const char *p) {
	if(!p) return true;
	for(std::string::size_type i=offset;i<s.length() && *p;i++,p++) {
		if(*p != s[i]) break;
	}
	return *p == 0;
}

inline bool starts_with(const std::string& s, const char *p) {
	return starts_with(s, 0, p);
}

inline bool starts_with(const std::string& s, const std::string& b) {
	return starts_with(s, b.c_str());
}

inline bool ends_with(const std::string& s, const char *p) {
	if(!p) return true;
	const char *rend = p - 1;
	p += strlen(p) - 1;
	for(std::string::size_type i = s.length()-1;i>=0 && p != rend;i++,p++) {
		if(*p != s[i]) break;
	}
	return p == rend;
}
inline bool ends_with(const std::string& s, const std::string& e) {
	return ends_with(s, e.c_str());
}

///////////////////////////
// A generic string scanner/tokenizer
// May be used to tokenize URLs when we pass ":/?#" as delimiters.

class scanner {
  private:
	typedef std::string::size_type size_type;
	std::string s;
	size_type i, end;
	char tok;
	std::string tokval;
	std::string sig;
	std::string delims;
	
  public:
	scanner(const std::string& sa, const char *d)
	:	s(sa), i(0),end(sa.length()),tok(0), 
		delims(d?d:"") {}
		
	scanner(const std::string& sa, const std::string& d)
	:	s(sa), i(0),end(sa.length()),tok(0), 
		delims(d) {}
	
	char next() {
		tokval.clear();
		if(i == end) return 0;
		size_type ix = delims.find_first_of(s[i]);
		if(ix != std::string::npos) {
			tokval = tok = delims[ix];
			i++;
		} else {
			tok = 'n'; 
			scan_part_not_of(delims.c_str());
		}
		sig += tok;
		return tok;
	}
	
	void scan_part_not_of(const char *delim) {
		size_type ni = s.find_first_of(delim, i);
		if(ni != std::string::npos) {
			tokval = std::string(s.c_str() + i, ni-i);
			i = ni;
		} else {
			tokval = std::string(s.c_str() + i);
			i = end;
		}
	}
	bool has_tok() const { return i != end;}
	char get_tok() const { return tok;}
	const std::string& get_tokval() const { return tokval;}
	const std::string& get_sig() const { return sig;}
	const std::string& get_str() const { return s;}
};

class reg_scanner {
  public:
	typedef std::vector<std::string>::size_type size_type;
	std::string str;
	std::vector<std::string> vals;
	std::string toks;
	
	reg_scanner(const std::string& sa, const char *d) 
	:	str(sa) {
		scanner sc(sa, d);
		reg(sc);
	}
	
	reg_scanner(const std::string& sa, const std::string& d) 
	:	str(sa) {
		scanner sc(sa, d);
		reg(sc);
	}
	
	const std::vector<std::string>& get_vals() const {
		return vals;
	}
	
	const std::string& get_toks() const {
		return toks;
	}
	
	std::string val_at(size_type i) const {
		return (i<vals.size())?vals.at(i):"";
	}
	
	std::string join(size_type b, size_type e) const {
		std::string s;
		for(size_type i = b; i<e && i< vals.size();i++)
			s += vals.at(i);
		return s;
	}
	std::string join(size_type b) const {
		return join(b, vals.size());
	}
	
	const std::string& get_str() const { return str;}
	
  private:
	void reg(scanner& sc) {
		while(sc.next()) {
			vals.push_back(sc.get_tokval());
			toks += sc.get_tok();
		}
	}
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_STRING_UTIL_H
