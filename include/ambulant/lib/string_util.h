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

#include "ambulant/config/config.h"

#include <string>
#include <vector>
#include <ctype.h>

namespace ambulant {

namespace lib {

class tokens_vector : public std::vector<std::string> {
  public:
    tokens_vector(const char* entry, const char* delims);
	std::string join(size_type i, char sep);
};

const std::string space_chars = " \r\n\t\v";
const std::string dec_digits = "0123456789";

//const std::wstring wspace_chars = L" \r\n\t\v";
//const std::wstring wdec_digits = L"0123456789";


inline std::string trim(const std::string& s) {
	size_t i1 = s.find_first_not_of(space_chars);
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

template <class CharType>
class basic_scanner {
  public:
 	typedef CharType char_type;
 	typedef std::basic_string<char_type> string_type;
	typedef typename string_type::size_type size_type;
	enum {EOS = 0, NUMBER = 'd', NAME = 'n', SPACE = 'w'};
	
	// Creates a basic_scanner for the source string 's' and delimiters 'd'.
	basic_scanner(const string_type& s, const string_type& d)
	:	src(s), 
		delims(d), 
		end(s.length()),
		pos(0), tok(EOS) {}
	
	// Returns the next token or EOS if none is available.
	// The current position is at the start of the next token or at end.
	// The token returned is either a delimiter character
	// or the meta-character NAME, NUMBER or SPACE.
	// The NAME token represents a sequence of one or more name characters,
	// the NUMBER a sequence of one or more digits, and
	// the SPACE a sequence of one or more space chars.
	// A digit can not start a name otherwise digits are name characters.  
	char next() {
		tok = EOS;
		tokval = "";
		if(pos == end) return tok;
		size_type ix = delims.find_first_of(src[pos]);
		if(vpos(ix)) {
			tokval = tok = delims[ix];
			pos++;
		} else {
			if(isdigit(src[pos]))
				scan_set_as(dec_digits, NUMBER);
			else if(isspace(src[pos])) 
				scan_set_as(space_chars, SPACE);
			else {
				std::string exdelims = delims + space_chars;
				scan_not_in_set_as(exdelims.c_str(), NAME);
			}
		}
		toks += tok;
		vals.push_back(tokval);
		return tok;
	}
		
	// Returns true when there are more tokens
	bool has_more() const { return pos != end;}
	
	// Returns the current token
	char get_tok() const { return tok;}
	
	// Returns the current token value
	const string_type& get_tokval() const { return tokval;}
	
	// Returns the src string 
	const string_type& get_src() const { return src;}
	
	// Returns the tokens seen
	const string_type& get_tokens() const { return toks;}
	
	// Returns the token values seen.
	const std::vector<string_type>& get_values() const { return vals;}
		
	// Tokenizes the source string.
	void tokenize() {
		if(pos>0) reset();
		while(next() != EOS);
	}
	
	// Returns the i_th token value.
	string_type val_at(size_type i) const {
		return (i<vals.size())?vals[i]:"";
	}
	
	// Joins token values with indices in [b,e).
	string_type join(size_type b, size_type e) const {
		string_type s;
		for(size_type ix = b; ix<e && ix<vals.size();ix++)
			s += vals[ix];
		return s;
	}
	
	// Joins token values with indices >= b.
	string_type join(size_type b) const {
		return join(b, vals.size());
	}
	
  protected:
  
	// Scans chars in the set 's' as token 't'.
	void scan_set_as(const string_type& s, char t) {
		tok = t; 
		size_type ni = src.find_first_not_of(s, pos);
		if(vpos(ni)) {
			tokval = string_type(src.c_str() + pos, ni-pos);
			pos = ni;
		} else {
			tokval = string_type(src.c_str() + pos);
			pos = end;
		}
	}
	
	// Scans chars not in the set 's' as token 't'.
	void scan_not_in_set_as(const string_type& s, char t) {
		tok = t; 
		size_type ni = src.find_first_of(s, pos);
		if(vpos(ni)) {
			tokval = string_type(src.c_str() + pos, ni-pos);
			pos = ni;
		} else {
			tokval = string_type(src.c_str() + pos);
			pos = end;
		}
	}
	
	// Skips chars in set 's'.
	void skip_set(const string_type& s) {
		size_type ni = src.find_first_not_of(s, pos);
		if(vpos(ni)) pos = ni;
		else pos = end;
	}
	
	// Skips space chars.
	void skip_space() { skip_set(space_chars);}
	
	// Resets this scanner; erases its memory
	void reset() {
		pos = 0;
		tok = EOS;
		toks = "";
		vals.clear();
	}
	
	bool vpos(size_type ix)  const { 
		return ix != std::basic_string<char_type>::npos;}

  private:
	
	// The source of this scanner
	const string_type src;
	
	// The literals to recognize
	const string_type delims;
	
	// Source end position 
	const size_type end;
	
	// Current pos
	size_type pos;
	
	// Current token
	char tok;
	
	// Current token value
	string_type tokval;
	
	// The tokens seen
	string_type toks;
	
	// The tokens values seen
	std::vector<string_type> vals;
};

typedef basic_scanner<char> scanner;
typedef basic_scanner<wchar_t> wscanner;


} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_STRING_UTIL_H
