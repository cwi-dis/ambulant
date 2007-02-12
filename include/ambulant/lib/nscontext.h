/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* 
 * @$Id$ 
 */
// A utility for managing the namespaces appearing in a document.

#ifndef AMBULANT_LIB_NSCONTEXT_H
#define AMBULANT_LIB_NSCONTEXT_H

#include "ambulant/config/config.h"

#include "ambulant/lib/sax_types.h"

#include <map>

namespace ambulant {

namespace lib {

/// XML namespace information.
/// This class holds information on XML namespaces.
class nscontext {
  public:
	/// Signals that short name prefix matches long name uri.
	void set_prefix_mapping(const xml_string& prefix, const xml_string& uri);
	
	/// Return true if short name prefix is known.
	bool is_known_prefix(const xml_string& prefix) const;
	
	// Return true if long name uri is known.
	bool is_known_namespace(const xml_string& uri) const;
	
	/// Return the uri for a given short name prefix (or NULL).
	const char* get_namespace(const xml_string& prefix) const;
	
	/// Return the uri for the default name space.
	const char* get_default_namespace() const;
	 
	/// Return the short name for a given uri.
	const char* get_namespace_prefix(const xml_string& uri) const;
	
	/// Convenience function version of get_namespace_prefix method.
	static const char* 
	get_namespace_prefix(const nscontext *p, const xml_string& uri);
	
	/// Return map of short names to long names.
	const std::map<xml_string, xml_string>&
	get_pre2uri() const { return m_pre2uri;}
	
	/// Return map of long names to short names.
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
