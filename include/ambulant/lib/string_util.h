/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_STRING_UTIL_H
#define AMBULANT_LIB_STRING_UTIL_H

#include <string>
#include <vector>

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

	size_t i2 = s.length()-1;
	while(i2>=0 && isspace(s[i2])) i2--;
	if(i2==-1) return "";

	return std::string(s.c_str()+i1,s.c_str()+i2+1);
}

inline std::string trim(const char* psz) {
	std::string s(psz);
	return trim(s);
}

inline std::string xml_quote(const char *p) {
	std::string qs;
	if(p != NULL) {
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

///////////////////////////
// A generic string scanner/tokenizer
// May be used to tokenize URLs when we pass ":/?" as delimiters.

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
	scanner(const std::string& sa, const std::string& d)
	:	s(sa), i(0),end(sa.length()),tok(0), delims(d) {}
	
	char next() {
		tokval.clear();
		if(i == end) return 0;
		size_type ix = delims.find_first_of(s[i]);
		if(ix != std::string::npos) {
			tok = delims[ix];
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
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_STRING_UTIL_H
