/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
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

#ifndef AMBULANT_LIB_DOCUMENT_H
#define AMBULANT_LIB_DOCUMENT_H

#include "ambulant/config/config.h"

#include <string>
#include <map>

#include "ambulant/common/factory.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/nscontext.h"
#include "ambulant/net/url.h"

namespace ambulant {

namespace lib {

/// Information on custom test used in the document.
class custom_test {
  public:
	std::string idd;
	std::string title;
	bool state;
	bool override;
	std::string uid;
};

/// A class respresenting an XML document.
///
/// This class is reachable from node objects.
/// and provides context services to them.
class document : public node_context {

  public:
	/// A document factory function.
	/// Creates documents from a url.
	static document* create_from_url(common::factories* factory, const net::url& u);
  
	/// A document factory function.
	/// Creates documents from local files.
	static document* create_from_file(common::factories* factory, const std::string& filename);
	
	/// A document factory function.
	/// Creates documents from source strings.
	/// The src_id argument is used in error messages only
	static document* create_from_string(common::factories* factory, const std::string& smil_src, const std::string& src_id);
	
#if 0
	/// A document factory function.
	/// Creates a document from a given DOM tree.
	/// The tree is not freed when the document is, and the caller is
	/// responsible for keeping it alive.
	static document* create_from_tree(common::factories* factory, lib::node *root, const net::url& u);
#endif

	/// This class may be extented to more specific documents.
	/// Therefore, use the virtual table to invoke the destructor.
	virtual ~document();
	
	/// Returns the root node of this document.
	/// The document remains the owner of the root unless detach is true.
	node* get_root(bool detach = false);
	const node* get_root() const;
	
	/// Signal to the document that the underlying tree has changed.
	void tree_changed();
	
	/// Locate a node with a given path.
	node* locate_node(const char *path) {
		return m_root?m_root->locate_node(path):0;
	}

	/// Locate a node with a given path.
	const node* locate_node(const char *path) const {
		return m_root?m_root->locate_node(path):0;
	}
		
	/// Returns the source url of this document.
	const ambulant::net::url& get_src_url() const { return m_src_url;}
	
	// node_context interface
	void set_prefix_mapping(const std::string& prefix, const std::string& uri);	
	const char* get_namespace_prefix(const xml_string& uri) const;
	net::url resolve_url(const net::url& rurl) const;
	// Returns the node with the provided id or null on none
	const node* get_node(const std::string& idd) const {
		if(idd.empty()) return 0;
		std::map<std::string, const node*>::const_iterator it
			= m_id2node.find(idd);
		return (it != m_id2node.end())?(*it).second:0;
	}
	const std::map<std::string, custom_test>* get_custom_tests() const
		{ return &m_custom_tests;}
	
	/// Set the source URL of the document.
	void set_src_url(ambulant::net::url u) { m_src_url = u;}
  protected:
	document();
//	document(node *root = 0, bool owned=true);
//	document(node *root, const net::url& src_url);
	
	void set_root(node* n);
//	void set_src_base(ambulant::net::url u) { m_src_base = u;}
	
  private:
	// builds id to node map
	void build_id2node_map();
	
	// reads document custom tests attributes
	void read_custom_attributes();
	
	// the root of this document
	node *m_root;
	
	// Whether m_root should be freed
	bool m_root_owned;
	
	// the external source url
	ambulant::net::url m_src_url;
	
	// this base url
//	ambulant::net::url m_src_base;
	
	bool m_is_file;
	
	// document namespaces registry
	nscontext m_namespaces;
	
	// document custom tests
	std::map<std::string, custom_test> m_custom_tests;
	
	// map of id to nodes
	std::map<std::string, const node*> m_id2node;

	
};

} // namespace lib
 
} // namespace ambulant

// XXX Not sure about next #if. The construct doesn't work for emVC3 and gcc2.95.
#if !defined(AMBULANT_PLATFORM_WIN32_WCE) && !defined(AMBULANT_NO_OSTREAM)
inline std::string repr(const ambulant::lib::custom_test& t) {
	std::string s;
	return s << '(' << t.idd << ", state:" << t.state << ", override:" << t.override << ", \'" << t.title << "\')";
}
inline std::string repr(const ambulant::lib::document& d) {
	std::string s;
	return s << "document(" << &d << ", " << d.get_src_url() << ")";
}

#else
inline std::string repr(const ambulant::lib::custom_test& t) {
	return t.idd;
}
inline std::string repr(const ambulant::lib::document& d) {
	return "document";
}
#endif


#endif // AMBULANT_LIB_DOCUMENT_H
