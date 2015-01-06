/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

// A utility for building a dom tree
// from a file or a string.
// Uses expat parser as the xml parser
// and nodes from node.h

#ifndef AMBULANT_LIB_TREE_BUILDER_H
#define AMBULANT_LIB_TREE_BUILDER_H

#include "ambulant/config/config.h"
#include "ambulant/lib/sax_handler.h"
#include "ambulant/lib/node.h"
#include "ambulant/net/url.h"
#include "ambulant/common/factory.h"

#include <string>

namespace ambulant {

namespace lib {

class node_context;

/// Build a DOM tree from a document.
class tree_builder :
	public sax_content_handler,
	public sax_error_handler {

  ///////////////
  public:
	/// Constructor, only initializes various internal variables.
	tree_builder(node_factory *nf, node_context *context = 0, const char *filename = "");
	//tree_builder() {};
	~tree_builder();

	/// build DOM tree from a local file.
	bool build_tree_from_file(const char *filename);

	/// build DOM tree from std::string data.
	bool build_tree_from_str(const std::string& str);

	/// build DOM tree from a memory buffer.
	bool build_tree_from_str(const char *begin, const char *end);

//	/// build DOM tree from a file anywhere on the net.
//	bool build_tree_from_url(const net::url& u);

	/// Return true if the document was parsed correctly.
	bool was_well_formed() const {return m_well_formed;}

	/// Get a pointer to the root node.
	/// Use detach() to become owner.
	node* get_tree() { return m_root;}

	/// Get a pointer to the root node.
	const node* get_tree() const { return m_root;}

	/// Get a pointer to the root node and become the owner of it.
	node* detach();

	/// Set ready to build next xml tree.
	void reset();

	/// Check that the root node is of a specific type
	bool assert_root_tag(const xml_string& tag) { return m_root && m_root->get_local_name() == tag; }

	/// Check that the root node is of a specific type
	bool assert_root_tag(const q_name_pair& tag) { return m_root && m_root->get_qname() == tag; }

	///////////////
	/// sax_content_handler interface method.
	virtual void start_document();
	/// sax_content_handler interface method.
	virtual void end_document();
	/// sax_content_handler interface method.
	virtual void start_element(const q_name_pair& qn, const q_attributes_list& qattrs);
	/// sax_content_handler interface method.
	virtual void end_element(const q_name_pair& qn);
	/// sax_content_handler interface method.
	virtual void start_prefix_mapping(const xml_string& prefix, const xml_string& uri);
	/// sax_content_handler interface method.
	virtual void end_prefix_mapping(const xml_string& prefix);
	/// sax_content_handler interface method.
	virtual void characters(const char *buf, size_t len);

	///////////////
	/// sax_error_handler interface method.
	virtual void error(const sax_error& error);

  ///////////////
  private:
	xml_parser *m_xmlparser;
	node *m_root;
	node *m_current;
	bool m_well_formed;
	node_factory *m_node_factory;
	node_context *m_context;
	std::string m_filename;		// For error messages only!
	std::vector<std::pair<std::string,std::string> > m_pending_namespaces;
	std::vector<std::pair<std::string,node*> > m_xml_space_stack;
	char* m_buf;
	size_t m_bufsize;
};


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_TREE_BUILDER_H
