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

#include <fstream>
#include <vector>
#include <map>

// Stream like output operators for std::strings
template<class T>
inline std::string& operator<<(std::string& s, const T& c) { std::ostringstream os; os << c; s += os.str();}

inline std::string& operator<<(std::string& s, const std::string& c) { s+=c; return s;}
inline std::string& operator<<(std::string& s, const char *c) { s+=c; return s;}
inline std::string& operator<<(std::string& s, int c) { char sz[32];sprintf(sz,"%d",c); s+=sz; return s;}
inline std::string& operator<<(std::string& s, double c) { char sz[32];sprintf(sz,"%f",c); s+=sz; return s;}

namespace ambulant {

namespace lib {

class string_record : public std::vector<char*> {
  public:
    string_record(const char* entry,const char* delims)
	:	m_buf(new char[strlen(entry)+1]) {
		strcpy(m_buf, entry);
		char *look = strtok(m_buf, delims);
		while(look) {
			push_back(look);
			look = strtok(NULL, delims);
			}
		}
		
    ~string_record() {delete[] m_buf;}

	void removews() {
		for(iterator i=begin();i!=end();i++)
			{
			char *p = (*i);
			while(*p && isspace(*p)) p++;
			*i = p;
			}
	}
	
  private:
	char* m_buf;
};

class property_record : public std::vector<char*> {
  public:
    property_record(const char* pString)
	:	m_apBuf(new char[strlen(pString)+1]) {
		char *pBuf = m_apBuf.get();
		strcpy(pBuf, pString);
		push_back(pBuf);
		char *p = strchr(pBuf,'=');
		if(p) {
			*p = '\0';p++;
			push_back(p);
		}
	}
  private:
	std::auto_ptr<char> m_apBuf;
};

class properties : public std::map<std::string, std::string> {
  public:
	bool read(const char *fn) {
		std::ifstream ifs(fn);
		if(ifs) {
			char buf[512];
			while(ifs.getline(buf, 512)) {
				property_record r(buf);
				if(r.size()==2)
					insert(std::pair<std::string,std::string>(r[0],r[1]));
			}
			ifs.close();
			return true;
		}
		return false;
	}
	
	bool haskey(const std::string& key) { return find(key)!=end();}
	
	std::string get(const std::string& key) const {
		const_iterator it = find(key);
		if(it==end()) return "";
		return (*it).second;
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

inline std::string xml_quote(const std::string& s){return xml_quote(s.c_str());}

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_STRING_UTIL_H
