/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_NODE_H
#define AMBULANT_LIB_NODE_H

// tree iterators
#include "ambulant/lib/node_iterator.h" 

// q_name_pair and qattr
#include "ambulant/lib/sax_types.h"

// attribute pair
#include <utility>

// q_attributes_list
#include <vector>

// return list of nodes
#include <list>

// return map of id -> nodes
#include <map>

// operator<<
#include <ostream>

namespace ambulant {

namespace lib {

class node_context;

// Simple tree node with tag, data and attrs
// The parent of each node is also its owner and container

class node {

  public:
  
	///////////////////////////////
	// tree iterators
	typedef tree_iterator<node> iterator;
	typedef const_tree_iterator<node> const_iterator;
	
	///////////////////////////////
	// Constructors
	
	// Note: attrs are as per expat parser
	// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	node(const char *local_name, const char **attrs = 0, const node_context *ctx = 0);

	node(const xml_string& local_name, const char **attrs = 0, const node_context *ctx = 0);

	node(const q_name_pair& qn, const q_attributes_list& qattrs, const node_context *ctx = 0);
	
	// shallow copy from other
	node(const node* other);
	
	///////////////////////////////
	// Destruct this node and its contents
	// If this node is part of a tree, detach it first
	// and then delete the node and its contents
	
	virtual ~node();

	///////////////////////////////
	// basic navigation

	const node *down() const { return m_child;}
	const node *up() const { return m_parent;}
	const node *next() const { return m_next;}

	node *down()  { return m_child;}
	node *up()  { return m_parent;}
	node *next()  { return m_next;}

	//////////////////////
	// set down/up/next
	
	void down(node *n)  { m_child = n;}
	void up(node *n)  { m_parent = n;}
	void next(node *n)  { m_next = n;}
	
	///////////////////////////////
	// deduced navigation

	const node* previous() const;
	const node* get_last_child() const;
	void get_children(std::list<const node*>& l) const;

	///////////////////////////////
	// search operations 
	// this section should be extented to allow for XPath selectors

	node *get_first_child(const char *name);
	node* locate_node(const char *path);
	void find_nodes_with_name(const xml_string& name, std::list<node*>& list);
	node* get_root();
	
	///////////////////////////////
	// iterators

    iterator begin() { return iterator(this);}
    const_iterator begin() const { return const_iterator(this);}

    iterator end() { return iterator(0);}
    const_iterator end() const { return const_iterator(0);}

	///////////////////////
	// build tree functions
	
	node* append_child(node* child);
		
	node* append_child(const char *name);

	node* detach();
	
	node* clone() const;
	
	void append_data(const char *data, size_t len);

	void append_data(const char *c_str);

	void append_data(const xml_string& str);

	void set_attribute(const char *name, const char *value);

	void set_attribute(const char *name, const xml_string& value);

	// Note: attrs are as per expat parser
	// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	void set_attributes(const char **attrs);
	
	void set_namespace(const xml_string& ns);
	
	/////////////////////
	// data queries

	const xml_string& get_namespace() const { return m_qname.first;}
	const xml_string& get_local_name() const { return m_qname.second;}
	const q_name_pair& get_qname() const { return m_qname;}

	const xml_string& get_data() const { return m_data;}

	xml_string get_trimmed_data() const;

	bool has_graph_data() const;
	
	const char *get_attribute(const char *name) const;
	const char *get_attribute(const std::string& name) const;
	
	// returns the resolved url of an attribute
	std::string get_url(const char *attrname) const;
	
	const q_attributes_list& get_attrs() const { return m_qattrs;}
	

	// return the number of nodes of the xml (sub-)tree starting at this node
	unsigned int size() const;

	// fills in a map with node ids
	// the map may be used for retreiving nodes from their id
	void create_idmap(std::map<std::string, node*>& m) const; 
		
	/////////////////////
	// string repr
	
	xml_string xmlrepr() const;
	xml_string to_string() const;
	xml_string to_trimmed_string() const;
	
	void dump(std::ostream& os) const;

	/////////////////////
	// node context
	
	const node_context* get_context() const { return m_context;}
	void set_context(node_context *c) { m_context = c;}
	
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
	
	const node& operator =(const node& o);
	
  private:
	// tree bonds
	node *m_parent;
	node *m_next;
	node *m_child;
};


} // namespace lib
 
} // namespace ambulant


// global operator<< for node objects
std::ostream& operator<<(std::ostream& os, const ambulant::lib::node& n);

#endif // AMBULANT_LIB_TREE_NODE_H
