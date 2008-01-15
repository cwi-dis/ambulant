/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
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

#ifndef AMBULANT_LIB_NODE_DUMMY_H
#define AMBULANT_LIB_NODE_DUMMY_H

#include "ambulant/config/config.h"
#include "ambulant/lib/node.h"


namespace ambulant {

namespace lib {

/// Dummy implementation of the node_interface virtual class.
/// This implementation returns either an error or does nothing
/// for all operations, but it can be used as a boilerplate
/// for creating an interface to another DOM implementation.

class node_dummy : public node_interface {

  public:
  
	///////////////////////////////
	// tree iterators
	typedef tree_iterator<node_dummy> iterator;
	typedef const_tree_iterator<node_dummy> const_iterator;
	
	/// Destruct this node and its contents.
	/// If this node is part of a tree, detach it first
	/// and then delete the node and its contents.
	virtual ~node_dummy();

	/// Return first child of this node.
	const node_dummy *down() const;
	
	/// Return parent of this node.
	const node_dummy *up() const;
	
	/// Return next sibling of this node.
	const node_dummy *next() const;

	/// Return first child of this node.
	node_dummy *down();
	
	/// Return parent of this node.
	node_dummy *up();
	
	/// Return next sibling of this node.
	node_dummy *next();

	/// Set first child of this node.
	void down(node_dummy *n);

	/// Set first child of this node, after dynamic typecheck
	void down(node_interface *n);
	
	/// Set parent of this node.
	void up(node_dummy *n);

	/// Set parent of this node, after dynamic typecheck
	void up(node_interface *n);
	
	/// Set next sibling of this node.
	void next(node_dummy *n);
	
	/// Set next sibling of this node, after dynamic typecheck
	void next(node_interface *n);
	
	/// Returns the previous sibling node 
	/// or null when this is the first child.
	const node_dummy* previous() const;
	
	/// Returns the last child 
	/// or null when this has not any children.
	const node_dummy* get_last_child() const;
	
	/// Appends the children of this node (if any) to the provided list.
	void get_children(std::list<const node*>& l) const;

	///////////////////////////////
	// search operations 
	// this section should be extented to allow for XPath selectors

	/// Find a node given a path of the form tag/tag/tag.
	node_dummy* locate_node(const char *path);
	
	/// Find the first direct child with the given tag.
	node_dummy *get_first_child(const char *name);
	
	/// Find the first direct child with the given tag.
	const node_dummy *get_first_child(const char *name) const;
		
	/// Find the root of the tree to which this node belongs.
	node_dummy* get_root();
	
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
	node_dummy* append_child(node_dummy* child);

	/// Append a child node to this node, after dynamic typecheck
	node_interface* append_child(node_interface* child);

	/// Append a new child node with the given name to this node.
	node_dummy* append_child(const char *name);

	/// Detach this node and its subtree from its parent tree.
	node_dummy* detach();
	
	/// Create a deep copy of this node and its subtree.
	node_dummy* clone() const;
	
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
		
	/// Override prefix mapping for this node and descendents
	virtual void set_prefix_mapping(const std::string& prefix, const std::string& uri) = 0;

	/////////////////////
	// data queries

	/// Return the namespace part of the tag for this node.
	const xml_string& get_namespace() const;
	
	/// Return the local part of the tag for this node.
	const xml_string& get_local_name() const;
	
	/// Return namespace and local part of the tag for this node.
	const q_name_pair& get_qname() const;
	
	/// Return the unique numeric ID for this node.
	int get_numid() const;
	
	/// Return the data for this node.
	const xml_string& get_data() const;
	
	/// Return true if this is a pure data node (i.e. no tag/attrs)
	virtual bool is_data_node() const = 0;

	/// Return the trimmed data for this node.
	xml_string get_trimmed_data() const;
	
	/// Return the value for the given attribute.
	const char *get_attribute(const char *name) const;
	
	/// Return the value for the given attribute.
	const char *get_attribute(const std::string& name) const;
	
	/// Remove the first occurrence of the given attribute.
	virtual void del_attribute(const char *name) = 0;
	
	/// Return the value for the given attribute, interpreted as a URL.
	/// Relative URLs are resolved against the document base URL, if possible.
	net::url get_url(const char *attrname) const;
	
	/// Return the number of nodes of the xml (sub-)tree starting at this node.
	unsigned int size() const;
	
	/// Returns a "friendly" path desription of this node.
	std::string get_path_display_desc() const;
	
	/// Return a friendly string describing this node.
	/// The string will be of a form similar to \<tag id="...">
	std::string get_sig() const;
	
	/////////////////////
	// string repr
	
	/// Return the
	xml_string xmlrepr() const;
	
	/////////////////////
	// node context
	
	/// Return the node_context for this node.
	const node_context* get_context() const;
	
	/// Set the node_context for this node.
	void set_context(node_context *c);
	
	/// Return the next unique ID.
	static int get_node_counter();
};

} // namespace lib
} // namespace ambulant

#endif // AMBULANT_LIB_NODE_DUMMY_H
