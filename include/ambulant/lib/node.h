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

// tree iterators and visitors
#ifndef AMBULANT_LIB_NODE_ITERATOR_H
#include "node_iterator.h" 
#endif

// tree helper iterators and visitors
#ifndef AMBULANT_LIB_NODE_NAVIGATOR_H
#include "node_navigator.h" 
#endif

// trim strings
#ifndef AMBULANT_LIB_STRING_UTIL_H
#include "string_util.h"
#endif

// q_name_pair and qattr
#include "sax_types.h"

// attribute pair
#include <utility>

// q_attributes_list
#include <vector>

// return list of nodes
#include <list>

// return map of id -> nodes
#include <map>

// find_if, etc
#include <algorithm>

// assert
#include <cassert>

// ostringstream
#include <sstream>

// operator<<
#include <ostream>

namespace ambulant {

namespace lib {


// Simple tree node with tag, data and attrs
// The parent of each node is also its owner and container

class node {

  public:
	// iterators
	typedef tree_iterator<node> iterator;
	typedef const_tree_iterator<node> const_iterator;
	
	///////////////////////////////
	// Constructors
	
	// Note: attrs are as per expat parser
	// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	node(const char *local_name, const char **attrs = 0)
	:	m_parent(0), m_next(0), m_child(0), m_qname("",(local_name?local_name:"error")) {
		set_attributes(attrs);
	}

	node(const xml_string& local_name, const char **attrs = 0)
	:	m_parent(0), m_next(0), m_child(0), m_qname("", local_name) {
		set_attributes(attrs);
	}

	// shallow copy from other
	node(const node* other)
	:	m_parent(0), m_next(0), m_child(0), 
		m_qname(other->get_qname()),
		m_data(other->get_data()),
		m_qattrs(other->get_attrs()) {}

	node(const q_name_pair& qn, const q_attributes_list& qattrs)
	:	m_parent(0), m_next(0), m_child(0), m_qname(qn), m_qattrs(qattrs) {
	}
	
	///////////////////////////////
	// Destruct this node and its contents
	// If this node is part of a tree, detach it first
	// and then delete the node and its contents
	
	virtual ~node() { node_navigator<node>::delete_tree(this); }

	///////////////////////////////
	// basic navigation

	const node *down() const { return m_child;}
	const node *up() const { return m_parent;}
	const node *next() const { return m_next;}

	node *down()  { return m_child;}
	node *up()  { return m_parent;}
	node *next()  { return m_next;}

	//////////////////////
	void down(node *n)  { m_child = n;}
	void up(node *n)  { m_parent = n;}
	void next(node *n)  { m_next = n;}
	
	///////////////////////////////
	// deduced navigation

	const node* previous() const { return node_navigator<node>::previous(this); }
	const node* get_last_child() const { return node_navigator<node>::last_child(this);}
	void get_children(std::list<const node*>& l) const {node_navigator<node>::get_children(this, l);}

	///////////////////////////////
	// search operations 
	// this section should be extented to allow for XPath selectors

	node *get_first_child(const char *name);
	node* locate_node(const char *path);
	void find_nodes_with_name(const xml_string& name, std::list<node*>& list);
	node* get_root() { return node_navigator<node>::get_root(this); }
	
	///////////////////////////////
	// iterators

    iterator begin() { return iterator(this);}
    const_iterator begin() const { return const_iterator(this);}

    iterator end() { return iterator(0);}
    const_iterator end() const { return const_iterator(0);}

	///////////////////////
	// build tree functions
	
	node* append_child(node* child) { return node_navigator<node>::append_child(this, child);}
		
	node* append_child(const char *name)
		{ return append_child(new node(name));}

	node* detach() { return node_navigator<node>::detach(this); }
	
	node* clone() const;
	
	void append_data(const char *data, size_t len)
		{ if(len>0) m_data.append(data, len);}

	void append_data(const char *c_str) { 
		if(c_str == 0 || c_str[0] == 0) return;
		m_data.append(c_str, strlen(c_str));
	}

	void append_data(const xml_string& str)
		{ m_data += str;}

	void set_attribute(const char *name, const char *value){ 
		if(name && name[0]) {
			q_name_pair qn("", name);
			q_attribute_pair qattr(qn, (value?value:""));
			m_qattrs.push_back(qattr);
		}
	}

	void set_attribute(const char *name, const xml_string& value) {
		if(name && name[0]) {
			q_name_pair qn("", name);
			q_attribute_pair qattr(qn, value);
			m_qattrs.push_back(qattr);
		}
	}

	// Note: attrs are as per expat parser
	// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
	void set_attributes(const char **attrs) {
		if(attrs == 0) return;
		for(int i=0;attrs[i];i+=2)
			set_attribute(attrs[i], attrs[i+1]);
	}
	
	void set_namespace(const xml_string& ns) {
		m_qname.first = ns;
	}
	
	/////////////////////
	// data queries

	const xml_string& get_namespace() const { return m_qname.first;}
	const xml_string& get_local_name() const { return m_qname.second;}
	const q_name_pair& get_qname() const { return m_qname;}

	const xml_string& get_data() const { return m_data;}

	xml_string get_trimmed_data() const { return trim(m_data);}

	bool has_graph_data() const { 
		if(m_data.empty()) return false;
		return std::find_if(m_data.begin(), m_data.end(), isgraph) != m_data.end();
	}

	const char *get_attribute(const char *name) const {
		if(!name || !name[0]) return 0;
		q_attributes_list::const_iterator it;
		for(it = m_qattrs.begin(); it != m_qattrs.end(); it++)
			if((*it).first.second == name) return (*it).second.c_str();
		return 0;
	}
	const char *get_attribute(const std::string& name) const {
		return get_attribute(name.c_str());
	}

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
	
  /////////////
  protected:
	// node data 
	// sufficient for a generic xml element
	q_name_pair m_qname;
	q_attributes_list m_qattrs;
	xml_string m_data;
	
  private:
	// tree bonds
	node *m_parent;
	node *m_next;
	node *m_child;
};

////////////////////////////////////////
// Implementation

inline node *node::get_first_child(const char *name) {
	node *e = down();
	if(!e) return 0;
	if(e->m_qname.second == name) return e;
	while((e=e->next())) if(e->m_qname.second == name) return e;
	return 0;
}

// create a deep copy of this
inline node* node::clone() const {
	node* c = new node(this);
	const node *e = down();
	if(e != 0) {
		c->append_child(e->clone());
		while((e = e->next())) c->append_child(e->clone());
	}
	return c;
}

inline xml_string node::xmlrepr() const {
	xml_string s(m_qname.second);
	q_attributes_list::const_iterator it = m_qattrs.begin();
	while(it!=m_qattrs.end()) {
		s += " ";
		s += (*it).first.second;
		s += "=\"";
		s += (*it).second;
		s += "\"";
		it++;
	}
	return s;
}

inline xml_string node::to_string() const {
	std::ostringstream os;
	output_visitor<node> visitor(os);
	std::for_each(begin(), end(), visitor);
	return os.str();
}
	
inline xml_string node::to_trimmed_string() const {
	std::ostringstream os;
	trimmed_output_visitor<node> visitor(os);
	std::for_each(begin(), end(), visitor);
	return os.str();
}

inline unsigned int node::size() const {
	unsigned int count = 0;
	count_visitor<node> visitor(count);
	std::for_each(begin(), end(), visitor);
	return count;
}

inline void node::find_nodes_with_name(const xml_string& name, std::list<node*>& lst) {
	iterator last = end(); // call once
	for(iterator it = begin(); it != last; it++)
		if((*it).first && (*it).second->get_local_name() == name) lst.push_back((*it).second);
}

inline 
void node::create_idmap(std::map<std::string, node*>& m) const {
	attr_collector<node> visitor(m);
	std::for_each(begin(), end(), visitor);
}

inline 
node* node::locate_node(const char *path) {
	string_record r(path, "/");
	node *n = this;
	for(string_record::iterator it = r.begin(); it != r.end() && n != 0;it++)
		n = n->get_first_child(*it);
	return n;
}

///////////////////////

} // namespace lib
 
} // namespace ambulant


///////////////////////
// global operator<< for node objects

inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::node& n) {
	ambulant::lib::output_visitor<ambulant::lib::node> visitor(os);
	std::for_each(n.begin(), n.end(), visitor);
	return os;
}
#endif // AMBULANT_LIB_TREE_NODE_H
