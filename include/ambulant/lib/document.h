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

#ifndef AMBULANT_LIB_DOCUMENT_H
#define AMBULANT_LIB_DOCUMENT_H

#include "ambulant/config/config.h"

#include <string>

#include "ambulant/lib/node.h"
#include "ambulant/lib/nscontext.h"

// XXX: temp, will go to document.cpp
#include "ambulant/lib/tree_builder.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/filesys.h"
#include "ambulant/lib/asb.h"
#include "ambulant/net/url.h"

#ifndef AMBULANT_NO_IOSTREAMS
#include <ostream>
#endif

// A class respresenting an XML document.
//
// This class is reachable from node objects.
// and provides context services to them.
//

namespace ambulant {

namespace lib {

// Interface accesible to nodes.
class node_context {
  public:
	
	virtual void 
	set_prefix_mapping(const std::string& prefix, const std::string& uri) = 0;
	
	virtual const char* 
	get_namespace_prefix(const xml_string& uri) const = 0;
	
	virtual std::string 
	resolve_url(const node *n, const std::string& rurl) const = 0;
};


class document : public node_context {

  public:
	// A document factory function.
	// Creates documents from local files.
	static document* create_from_file(const std::string& str);
	
	// This class maybe extented to more specific documents.
	// Therfore, use the virtual table to invoke the destructor.
	virtual ~document();
	
	// Returns the root node of this document
	// The document remains the owner of the root unless detach is true.
	node* get_root(bool detach = false);
	const node* get_root() const;
	
	// Locate a node with path
	node* locate_node(const char *path) {
		return m_root?m_root->locate_node(path):0;
	}
	
	// Returns the source url of this document
	const ambulant::net::url& get_src_url() const { return m_src_url;}
	
	// node_context interface
	void set_prefix_mapping(const std::string& prefix, const std::string& uri);
	const char* get_namespace_prefix(const xml_string& uri) const;
	std::string resolve_url(const node *n, const std::string& rurl) const;
	
  protected:
	document(node *root = 0);
	document(node *root, const std::string& src_url);
	
  private:
	// the root of this document
	node *m_root;
	
	// the external source url
	ambulant::net::url m_src_url;
	
	// this base url
	ambulant::net::url m_src_base;
	
	bool m_is_file;
	
	// document namespaces registry
	nscontext m_namespaces;
};


/////////////////////////////////
// Inline implementation

inline 
document::document(node *root) 
:	m_root(root) {
}

inline
document::document(node *root, const std::string& src_url) 
:	m_root(root), m_src_url(src_url) {
}

inline
document::~document() {
	delete m_root;
}

inline node* 
document::get_root(bool detach) {
	if(!detach)
		return m_root;
	node* tmp = m_root;
	m_root = 0;
	return tmp;
}

inline const node* 
document::get_root() const {
	return m_root;
}

//static 
inline document* 
document::create_from_file(const std::string& str) {
	document *d = new document();
	tree_builder builder(d);
	if(!builder.build_tree_from_file(str.c_str())) {
		logger::get_logger()->error(
			"Could not build tree for file: %s", str.c_str());
		return 0;
	}
	d->m_root = builder.detach();
	d->m_src_url = ambulant::net::url(str);
	
	std::string base = filesys::get_base(str, file_separator.c_str());
	d->m_src_base = ambulant::net::url(base);
	
	return d;
}

inline void 
document::set_prefix_mapping(const std::string& prefix, const std::string& uri) {
	m_namespaces.set_prefix_mapping(prefix, uri);
}

inline const char* 
document::get_namespace_prefix(const xml_string& uri) const {
	return m_namespaces.get_namespace_prefix(uri);
}

inline std::string 
document::resolve_url(const node *n, const std::string& rurl) const {
	// locate node context xml:base
	// const char *p = n->get_container_attribute("base");
	// ...
	// if none is found use source.
	
	if(m_src_base.get_protocol() == "file") {
		std::string base_path = m_src_base.get_path();
		return filesys::join(base_path, rurl, file_separator.c_str());
	}
		
	return filesys::join(m_src_base.get_path(), rurl); 
}


} // namespace lib
 
} // namespace ambulant

#ifndef AMBULANT_NO_IOSTREAMS
inline 
std::ostream& operator<<(std::ostream& os, const ambulant::lib::document& d) {
	return os << "document(" << (void *)&d << ", \"" << d.get_src_url() << "\")";
}
#endif

#endif // AMBULANT_LIB_DOCUMENT_H


