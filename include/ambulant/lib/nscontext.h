
/* 
 * @$Id$ 
 */
// A utility for managing the namespaces appearing in a document.

#ifndef AMBULANT_LIB_NSCONTEXT_H
#define AMBULANT_LIB_NSCONTEXT_H

#include "ambulant/lib/sax_types.h"

#include <map>

namespace ambulant {

namespace lib {

class nscontext {
  public:
	void set_prefix_mapping(const xml_string& prefix, const xml_string& uri);
	
	bool is_known_prefix(const xml_string& prefix) const;
	bool is_known_namespace(const xml_string& uri) const;
	 
	const char* get_namespace(const xml_string& prefix) const;
	const char* get_default_namespace() const;
	 
	const char* get_namespace_prefix(const xml_string& uri) const;
	 
	static const char* 
	get_namespace_prefix(const nscontext *p, const xml_string& uri);
	 
	const std::map<xml_string, xml_string>&
	get_pre2uri() const { return m_pre2uri;}
	
	const std::map<xml_string, xml_string>&
	get_uri2pre() const { return m_uri2pre;}
	
  private:
	std::map<xml_string, xml_string> m_pre2uri;
	std::map<xml_string, xml_string> m_uri2pre;  
};


inline void nscontext::set_prefix_mapping(const xml_string& prefix, const xml_string& uri) {
	m_pre2uri[prefix] = uri;
	m_uri2pre[uri] = prefix;
}

inline const char* nscontext::get_namespace(const xml_string& prefix) const {
	std::map<xml_string, xml_string>::const_iterator it = m_pre2uri.find(prefix);
	return (it == m_pre2uri.end())?0:(*it).second.c_str();
}

inline const char* nscontext::get_default_namespace() const {
	std::map<xml_string, xml_string>::const_iterator it = m_pre2uri.find(xml_string());
	return (it == m_pre2uri.end())?0:(*it).second.c_str();
}

inline bool nscontext::is_known_prefix(const xml_string& prefix) const {
	std::map<xml_string, xml_string>::const_iterator it = m_pre2uri.find(prefix);
	return it != m_uri2pre.end();
}

inline bool nscontext::is_known_namespace(const xml_string& uri) const {
	std::map<xml_string, xml_string>::const_iterator it = m_uri2pre.find(uri);
	return it != m_uri2pre.end();
}

inline const char* nscontext::get_namespace_prefix(const xml_string& uri) const {
	std::map<xml_string, xml_string>::const_iterator it = m_uri2pre.find(uri);
	return (it == m_uri2pre.end())?0:(*it).second.c_str();
}

// static
inline const char* 
nscontext::get_namespace_prefix(const nscontext *p, const xml_string& uri) {
	return p?p->get_namespace_prefix(uri):0;
}

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_NSCONTEXT_H
