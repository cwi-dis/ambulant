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

#ifndef AMBULANT_LIB_NODE_H
#define AMBULANT_LIB_NODE_H

#include "ambulant/config/config.h"
#include "ambulant/net/url.h"

// q_name_pair and qattr
#include "ambulant/lib/sax_types.h"

// tree iterators
#include "ambulant/lib/node_iterator.h" 

// attribute pair
#include <utility>

// q_attributes_list
#include <vector>

// return list of nodes
#include <list>

// return map of id -> nodes
#include <map>

#ifndef AMBULANT_NO_IOSTREAMS
// operator<<
#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM*/
#include <ostream.h>
#endif/*AMBULANT_NO_OSTREAM*/
#endif

namespace ambulant {

namespace lib {

class node_context;

/// Simple tree node with tag, data and attributes.
/// The node trees are not fully DOM compliant, but should
/// be compatible with a bit of glue code.
/// The parent of each node is also its owner and container.

class node {

  public:
  
	///////////////////////////////
	// tree iterators
	typedef tree_iterator<node> iterator;
	typedef const_tree_iterator<node> const_iterator;
	
	///////////////////////////////
	// Constructors
	
	/// Construct a new, unconnected, node.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	node(const char *local_name, const char **attrs = 0, const node_context *ctx = 0);

	/// Construct a new, unconnected, node.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	node(const xml_string& local_name, const char **attrs = 0, const node_context *ctx = 0);

	/// Construct a new, unconnected, node.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	node(const q_name_pair& qn, const q_attributes_list& qattrs, const node_context *ctx = 0);
	
	// shallow copy from other.
	node(const node* other);
	
	/// Destruct this node and its contents.
	/// If this node is part of a tree, detach it first
	/// and then delete the node and its contents.
	virtual ~node();


	/// Return first child of this node.
	const node *down() const { return m_child;}
	
	/// Return parent of this node.
	const node *up() const { return m_parent;}
	
	/// Return next sibling of this node.
	const node *next() const { return m_next;}

	/// Return first child of this node.
	node *down()  { return m_child;}
	
	/// Return parent of this node.
	node *up()  { return m_parent;}
	
	/// Return next sibling of this node.
	node *next()  { return m_next;}


	/// Set first child of this node.
	void down(node *n)  { m_child = n;}
	
	/// Set parent of this node.
	void up(node *n)  { m_parent = n;}
	
	/// Set next sibling of this node.
	void next(node *n)  { m_next = n;}
	
	/// Returns the previous sibling node 
	/// or null when this is the first child.
	const node* previous() const;
	
	/// Returns the last child 
	/// or null when this has not any children.
	const node* get_last_child() const;
	
	/// Appends the children of this node (if any) to the provided list.
	void get_children(std::list<const node*>& l) const;

	///////////////////////////////
	// search operations 
	// this section should be extented to allow for XPath selectors

	/// Find a node given a path of the form tag/tag/tag.
	node* locate_node(const char *path);
	
	/// Find the first direct child with the given tag.
	node *get_first_child(const char *name);
	
	/// Find the first direct child with the given tag.
	const node *get_first_child(const char *name) const;
	
	/// Find all descendants with the given tag.
	void find_nodes_with_name(const xml_string& name, std::list<node*>& list);
	
	/// Find the root of the tree to which this node belongs.
	node* get_root();
	
	/// Get an attribute from this node or its nearest ancestor that has the attribute.
	const char *get_container_attribute(const char *name) const;
	///////////////////////////////
	// iterators

	/// Return iterator for this node and its subtree.
    iterator begin() { return iterator(this);}

	/// Return iterator for this node and its subtree.
    const_iterator begin() const { return const_iterator(this);}

    iterator end() { return iterator(0);}
    const_iterator end() const { return const_iterator(0);}

	///////////////////////
	// build tree functions
	
	/// Append a child node to this node.
	node* append_child(node* child);
		
	
	/// Append a new child node with the given name to this node.
	node* append_child(const char *name);

	/// Detach this node and its subtree from its parent tree.
	node* detach();
	
	/// Create a deep copy of this node and its subtree.
	node* clone() const;
	
	/// Append data to the data of this node.
	void append_data(const char *data, size_t len);
	
	/// Append c_str to the data of this node.
	void append_data(const char *c_str);
	
	/// Append str to the data of this node.
	void append_data(const xml_string& str);

	/// Add an attribute/value pair.
	void set_attribute(const char *name, const char *value);

	/// Add an attribute/value pair.
	void set_attribute(const char *name, const xml_string& value);

	/// Set a number of attribute/value pairs.
	/// Note: attrs are as per expat parser
	/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	void set_attributes(const char **attrs);
	
	/// Set the namespace for this node.
	void set_namespace(const xml_string& ns);
	
	/////////////////////
	// data queries

	/// Return the namespace part of the tag for this node.
	const xml_string& get_namespace() const { return m_qname.first;}
	
	/// Return the local part of the tag for this node.
	const xml_string& get_local_name() const { return m_qname.second;}
	
	/// Return namespace and local part of the tag for this node.
	const q_name_pair& get_qname() const { return m_qname;}
	
	/// Return the unique numeric ID for this node.
	int get_numid() const {return m_numid;}
	
	/// Return the data for this node.
	const xml_string& get_data() const { return m_data;}
	
	/// Return the trimmed data for this node.
	xml_string get_trimmed_data() const;

	bool has_graph_data() const;
	
	/// Return the value for the given attribute.
	const char *get_attribute(const char *name) const;
	
	/// Return the value for the given attribute.
	const char *get_attribute(const std::string& name) const;
	
	/// Return the value for the given attribute, interpreted as a URL.
	/// Relative URLs are resolved against the document base URL, if possible.
	net::url get_url(const char *attrname) const;
	
	/// Return a reference to all attributes.
	const q_attributes_list& get_attrs() const { return m_qattrs;}
	

	/// Return the number of nodes of the xml (sub-)tree starting at this node.
	unsigned int size() const;

	/// Fills in a map with node ids.
	/// the map may be used for retreiving nodes from their id.
	void create_idmap(std::map<std::string, node*>& m) const; 
	
	/// Returns a "friendly" path desription of this node.
	std::string get_path_display_desc() const;
	
	/// Return a friendly string describing this node.
	/// The string will be of a form similar to <tag id="...">
	std::string get_sig() const;
	
	/////////////////////
	// string repr
	
	/// Return the
	xml_string xmlrepr() const;
	xml_string to_string() const;
	xml_string to_trimmed_string() const;
	
#ifndef AMBULANT_NO_IOSTREAMS
	void dump(std::ostream& os) const;
#endif

	/////////////////////
	// node context
	
	/// Return the node_context for this node.
	const node_context* get_context() const { return m_context;}
	
	/// Set the node_context for this node.
	void set_context(node_context *c) { m_context = c;}
	
	/// Return the next unique ID.
	static int get_node_counter() {return node_counter;}
	
  /////////////
  protected:
	// node data 
	// sufficient for a generic xml element
	
	// the qualified name of this element as std::pair
	q_name_pair m_qname;
	
	// the qualified name of this element as std::pair
	q_attributes_list m_qattrs;
	
	// the text data of this node
	xml_string m_data;
	
	// the context of this node
	const node_context *m_context;
	
	// a magic id
	int m_numid;
	
	const node& operator =(const node& o);
	
  private:
	// tree bonds
	node *m_parent;
	node *m_next;
	node *m_child;
	
	// verifier
	static int node_counter;
};


} // namespace lib
 
} // namespace ambulant

#ifndef AMBULANT_NO_IOSTREAMS
// global operator<< for node objects
std::ostream& operator<<(std::ostream& os, const ambulant::lib::node& n);
#endif

#endif // AMBULANT_LIB_TREE_NODE_H
