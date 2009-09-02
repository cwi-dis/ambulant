// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* 
 * @$Id$ 
 */

#include "ambulant/lib/node.h"
#include "ambulant/lib/node_impl.h"

// find_if, etc
#include <algorithm>
#include <typeinfo>

#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
// ostringstream
#include <sstream>
#endif

// assert
#include <cassert>

// trim strings
#include "ambulant/lib/string_util.h"

// tree helper iterators and visitors
#include "ambulant/lib/node_navigator.h" 

// node context
#include "ambulant/lib/document.h" 

#include "ambulant/lib/logger.h" 

#include <stdio.h> 

using namespace ambulant;


// This module starts with a set of private node visitors
// and continues with the implementation of lib::node.

// Node visitors are used by lib::node member functions

////////////////////////
// private output_visitor
// Writes a tree to an ostream.

#ifndef AMBULANT_NO_IOSTREAMS
template <class Node>
class output_visitor {
	std::ostream& os;
	std::basic_string<char> writesp, strincr;
	size_t ns;

  public:
	output_visitor(std::ostream& os_) 
	:	os(os_), strincr("  ") {ns = strincr.length();}
	void operator()(std::pair<bool, const Node*> x);

  private:
	void write_start_tag_no_children(const Node*& pe);
	void write_start_tag_with_children(const Node*& pe);
	void write_end_tag_with_children(const Node*& pe);
	const output_visitor& operator=(const output_visitor& o);
};

#endif

////////////////////////
// private trimmed_output_visitor
// Writes a tree to an ostream without white space.

#ifndef AMBULANT_NO_IOSTREAMS
template <class Node>
class trimmed_output_visitor {
	std::ostream& os;

  public:
	trimmed_output_visitor(std::ostream& os_) : os(os_) {}
	void operator()(std::pair<bool, const Node*> x);
	
  private:
	void write_start_tag_no_children(const Node*& pe);
	void write_start_tag_with_children(const Node*& pe);
	void write_end_tag_with_children(const Node*& pe);
	const trimmed_output_visitor& operator=(const trimmed_output_visitor& o);	
};
#endif


////////////////////////
// private attr_collector
// Scans the tree and creates a map attr to nodes.
// Intented for unique attrs like name and id

template <class Node>
class attr_collector {
  public:
	attr_collector(std::map<std::string, Node*>& m, const char *attr = "id") : 
		m_attr(attr), m_map(m) {}
		
	void operator()(std::pair<bool, const Node*> x) {
		if(x.first) {
			const char *value = x.second->get_attribute(m_attr);
			if(value != 0) 
				m_map[value] = const_cast<Node*>(x.second);
		}
	}
  private:
	std::string m_attr;
	std::map<std::string, Node*>& m_map;
	const attr_collector& operator=(const attr_collector& o);	
};

///////////////////////////////////////////////
/// lib::node implementation

// static 
int lib::node_impl::node_counter = 0;

//////////////////////
// Node constructors

// Note: attrs are as per expat parser
// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};

lib::node_impl::node_impl(const char *local_name, const char **attrs, const node_context *ctx)
:	m_qname("",(local_name?local_name:"error")),
	m_local_name(local_name?local_name:"error"),
	m_is_data_node(false),
	m_context(ctx),
	m_parent(0), m_next(0), m_child(0) {
	set_attributes(attrs);
	m_numid = ++node_counter;
}

lib::node_impl::node_impl(const xml_string& local_name, const char **attrs, const node_context *ctx)
:   m_qname("", local_name),
	m_local_name(local_name),
	m_is_data_node(false),
	m_context(ctx),
	m_parent(0), m_next(0), m_child(0) {
	set_attributes(attrs);
	m_numid = ++node_counter;
}

lib::node_impl::node_impl(const q_name_pair& qn, const q_attributes_list& qattrs, const node_context *ctx)
:	m_qname(qn),
	m_qattrs(qattrs),
	m_is_data_node(false),
	m_context(ctx),
	m_parent(0), m_next(0), m_child(0)
{
	if (m_context) {
		if(m_context->is_supported_prefix(m_qname.first))
			m_local_name = m_qname.second;
		else
		if (m_qname.first == "")
			m_local_name = m_qname.second;
		else
			m_local_name = m_qname.first + ":" + m_qname.second;
	} else
		m_local_name = m_qname.second;
	m_numid = ++node_counter;
}

// shallow copy from other
lib::node_impl::node_impl(const node_impl* other)
:   m_qname(other->get_qname()),
	m_local_name(other->get_local_name()),
	m_qattrs(other->get_attrs()),
	m_data(other->get_data()),
	m_is_data_node(false),
	m_context(other->get_context()),
	m_parent(0), m_next(0), m_child(0) {
	m_numid = ++node_counter;
}

// Data node
lib::node_impl::node_impl(const char *data, int size, const node_context *ctx)
:	m_local_name(""),
	m_data(lib::xml_string(data, size)),
	m_is_data_node(true),
	m_context(ctx),
	m_parent(0), m_next(0), m_child(0) {
	m_numid = ++node_counter;
}

//////////////////////
// Node destructor

lib::node_impl::~node_impl() { 
	node_counter--;
	node_navigator<node_impl>::delete_tree(this); 
}

///////////////////////////////
// basic navigation
// inline

//////////////////////
// set down/up/next
// inline

///////////////////////////////
// deduced navigation

const lib::node_impl* 
lib::node_impl::previous() const { 
	return node_navigator<const node_impl>::previous(this); 
}

const lib::node_impl* 
lib::node_impl::get_last_child() const { 
	return node_navigator<const node_impl>::last_child(this);
}

void lib::node_impl::get_children(std::list<const lib::node*>& l) const {
	node_navigator<const lib::node>::get_children(this, l);
}

///////////////////////////////
// search operations 

lib::node_impl* 
lib::node_impl::get_first_child(const char *name) {
	node_impl *e = down();
	if(!e) return 0;
	if(e->m_local_name == name) return e;
	e = e->next();
	while(e != 0) {
		if(e->m_local_name == name) 
			return e;
		e = e->next();
	}
	return 0;
}

const lib::node_impl* 
lib::node_impl::get_first_child(const char *name) const {
	const node_impl *e = down();
	if(!e) return 0;
	if(e->m_local_name == name) return e;
	e = e->next();
	while(e != 0) {
		if(e->m_local_name == name) 
			return e;
		e = e->next();
	}
	return 0;
}

lib::node_impl* 
lib::node_impl::locate_node(const char *path) {
	if(!path || !path[0]) {
		return this;
	}
	tokens_vector v(path, "/");
	node_impl *n = 0;
	tokens_vector::iterator it = v.begin();
	if(path[0] == '/') { // or v.at(0) is empty
		// absolute
		it++; // skip empty
		n = get_root();
		if(it == v.end())
			return n; 
		if(n->get_local_name() != (*it))
			return 0;
		it++; // skip root
	} else n = this;
	for(; it != v.end() && n != 0;it++) {
		n = n->get_first_child((*it).c_str());
	}
	return n;
}

lib::node_impl* 
lib::node_impl::get_root() { 
	return node_navigator<node_impl>::get_root(this); 
}

inline std::string get_path_desc_comp(const lib::node_impl *n) {
	std::string sbuf;
	const char *pid = n->get_attribute("id");
	sbuf += n->get_local_name();
	if(pid) {sbuf += ":"; sbuf += pid;}
	return sbuf;
}

std::string lib::node_impl::get_path_display_desc() const {
	std::string sbuf;
	std::list<const node_impl*> path;
	node_navigator<const node_impl>::get_path(this, path);
	int nc = 0;
	std::list<const node_impl*>::reverse_iterator it = path.rbegin();
	sbuf += get_path_desc_comp(this);it++;nc++;
	for(;it != path.rend() && nc<3;it++) {
		std::string ln = (*it)->get_local_name();
		if(ln != "priorityClass" && ln != "switch") {
			sbuf.insert(0, "/");
			sbuf.insert(0, get_path_desc_comp(*it));
			nc++;
		}
	}
	return sbuf;
}
///////////////////////////////
// iterators
// inline

///////////////////////
// build tree functions
	
lib::node_impl* 
lib::node_impl::append_child(node_impl* child) { 
	return node_navigator<node_impl>::append_child(this, child);
}
		
lib::node_impl* 
lib::node_impl::append_child(const char *name) { 
	return append_child(new node_impl(name));
}

lib::node_impl* 
lib::node_impl::detach() { 
	return node_navigator<node_impl>::detach(this); 
}
	
void lib::node_impl::append_data(const char *data, size_t len) { 
	if(len>0) m_data.append(data, len);
}

void lib::node_impl::append_data(const char *c_str) { 
	if(c_str == 0 || c_str[0] == 0) return;
	m_data.append(c_str, strlen(c_str));
}

void lib::node_impl::append_data(const xml_string& str)
	{ m_data += str;}

void lib::node_impl::set_attribute(const char *name, const char *value){ 
	if(name && name[0]) {
		q_name_pair qn("", name);
		q_attribute_pair qattr(qn, (value?value:""));
		m_qattrs.push_back(qattr);
	}
}

void lib::node_impl::set_attribute(const char *name, const xml_string& value) {
	if(name && name[0]) {
		q_name_pair qn("", name);
		q_attribute_pair qattr(qn, value);
		m_qattrs.push_back(qattr);
	}
}

// Note: attrs are as per expat parser
// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
void lib::node_impl::set_attributes(const char **attrs) {
	if(attrs == 0) return;
	for(int i=0;attrs[i];i+=2)
		set_attribute(attrs[i], attrs[i+1]);
}
	
// create a deep copy of this
lib::node_impl* 
lib::node_impl::clone() const {
	node_impl* c = new node_impl(this);
	const node_impl *e = down();
	if(e != 0) {
		c->append_child(e->clone());
		e = e->next();
		while(e) {
			c->append_child(e->clone());
			e = e->next();
		}
	}
	return c;
}

////////////////////////
// data queries
// some are inline

lib::xml_string 
lib::node_impl::get_trimmed_data() const { 
	return trim(m_data);
}

bool 
lib::node_impl::has_graph_data() const {
#ifdef AMBULANT_PLATFORM_WIN32_WCE
	return false;
#else
	if(m_data.empty()) return false;
	return std::find_if(m_data.begin(), m_data.end(), isgraph) != m_data.end();
#endif
}

const char *
lib::node_impl::get_attribute(const char *name) const {
	if(!name || !name[0]) return 0;
	q_attributes_list::const_iterator it;
	for(it = m_qattrs.begin(); it != m_qattrs.end(); it++)
		if((*it).first.second == name) {
#ifdef WITH_SMIL30
			if (m_context) {
				const xml_string ns = name;
				const xml_string& attrval = (*it).second;
				if (attrval.find('{') != std::string::npos) {
					const_cast<lib::node_impl*>(this)->m_avtcache[ns] = m_context->apply_avt(ns, attrval);
					return const_cast<lib::node_impl*>(this)->m_avtcache[ns].c_str();
				}
			}
#endif
			return (*it).second.c_str();
		}
	return 0;
}

const char *
lib::node_impl::get_attribute(const std::string& name) const {
	return get_attribute(name.c_str());
}

void
lib::node_impl::del_attribute(const char *name) {
	if(!name || !name[0]) return;
	q_attributes_list::const_iterator it;
	// Cannot use a for loop here because we may modify the list
	it = m_qattrs.begin();
	while (it != m_qattrs.end()) {
		if((*it).first.second == name) {
			m_qattrs.remove(*it);
			it = m_qattrs.begin();
		} else {
			it++;
		}
	}
	return;
}

// returns the resolved url of an attribute
net::url 
lib::node_impl::get_url(const char *attrname) const {
	const char *rurl = get_attribute(attrname);
	if(!rurl) return net::url();
	net::url url = net::url::from_url(rurl);
	return m_context ? m_context->resolve_url(url) : url;
}

const char *
lib::node_impl::get_container_attribute(const char *name) const {
	if(!name || !name[0]) return 0;
	const node_impl *n = this;
	const char *p = 0;
	while(n->up()) {
		n = n->up();
		p = n->get_attribute(name);
		if(p) break;
	}
	return p;

}

/////////////////////
// string repr

lib::xml_string 
lib::node_impl::xmlrepr() const {
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

std::string lib::node_impl::get_sig() const {
	std::string s = "<";
	s += m_local_name;
	const char *pid = get_attribute("id");
	if (pid) {
		s += " id=\"";
		s += pid;
		s += "\"";
	}
    const char *debug = get_attribute("_debug");
    if (debug) {
        s += " _debug=\"";
        s += debug;
        s += "\"";
    }
	s += ">";
	return s;
}

unsigned int 
lib::node_impl::size() const {
	const_iterator it;
	const_iterator e = end();
	unsigned int count = 0;
	for(it = begin(); it != e; it++)
		if((*it).first) count++;
	return count;
}

#ifndef AMBULANT_NO_IOSTREAMS
lib::xml_string 
lib::node_impl::to_string() const {
	std::ostringstream os;
	output_visitor<node_impl> visitor(os);
	const_iterator it;
	const_iterator e = end();
	for(it = begin(); it != e; it++) visitor(*it);
	return os.str();
}
#endif
	
#ifndef AMBULANT_NO_IOSTREAMS
lib::xml_string 
lib::node_impl::to_trimmed_string() const {
	std::ostringstream os;
	trimmed_output_visitor<node_impl> visitor(os);
	const_iterator it;
	const_iterator e = end();
	for(it = begin(); it != e; it++) visitor(*it);
	return os.str();
}
#endif


void lib::node_impl::create_idmap(std::map<std::string, node_impl*>& m) const {
	attr_collector<node_impl> visitor(m);
	const_iterator it;
	const_iterator e = end();
	for(it = begin(); it != e; it++) visitor(*it);
}

#ifndef AMBULANT_NO_IOSTREAMS
void lib::node_impl::dump(std::ostream& os) const {
	output_visitor<ambulant::lib::node_impl> visitor(os);
	const_iterator it;
	const_iterator e = end();
	for(it = begin(); it != e; it++) visitor(*it);
}
#endif

void
lib::node_impl::down(lib::node_interface *n)
{
#if WITH_EXTERNAL_DOM
	down(dynamic_cast<node_impl*>(n));
#else
	assert(0);
#endif
}

void
lib::node_impl::up(lib::node_interface *n)
{
#if WITH_EXTERNAL_DOM
	up(dynamic_cast<node_impl*>(n));
#else
	assert(0);
#endif
}

void
lib::node_impl::next(lib::node_interface *n)
{
#if WITH_EXTERNAL_DOM
	next(dynamic_cast<node_impl*>(n));
#else
	assert(0);
#endif
}

lib::node_interface*
lib::node_impl::append_child(lib::node_interface* child)
{
#if WITH_EXTERNAL_DOM
	return append_child(dynamic_cast<node_impl*>(child));
#else
	assert(0);
	return NULL;
#endif
}

#ifndef AMBULANT_NO_IOSTREAMS
std::ostream& operator<<(std::ostream& os, const ambulant::lib::node_impl& n) {
	os << "node(" << (void *)&n << ", \"" << n.get_qname() << "\"";
	std::string url = repr(n.get_url("src"));
	if (url != "")
		os << ", url=\"" << url << "\"";
	os << ")";
	return os;
}
#endif

//////////////////////////////////////////////
//////////////////////////////////////////////
// Visitors implementations

////////////////////////
// output_visitor

#ifndef AMBULANT_NO_IOSTREAMS
template<class Node>
void output_visitor<Node>::operator()(std::pair<bool, const Node*> x) {
	const Node*& pe = x.second;
	if(x.first) {
		// start tag
		if(pe->is_data_node())
			os << pe->get_data();
		else if(!pe->down()) 
			write_start_tag_no_children(pe);
		else 
			write_start_tag_with_children(pe);
		}
	else if(pe->down())
		write_end_tag_with_children(pe);
}

template<class Node>
void output_visitor<Node>::write_start_tag_no_children(const Node*& pe) {
	const std::string& data = pe->get_data();
	if(data.length()==0 || !pe->has_graph_data())
		os <<  writesp << "<" + pe->xmlrepr() + "/>" << std::endl;
	else {
		os <<  writesp << "<" + pe->xmlrepr() + ">";
		os << pe->get_data();
		os << "</" << pe->get_local_name() << ">" << std::endl;
	}
}

template<class Node>
void output_visitor<Node>::write_start_tag_with_children(const Node*& pe) {
	os <<  writesp << "<" + pe->xmlrepr() + ">";
	const std::string& data = pe->get_data();
	if(data.length()>0 && pe->has_graph_data())
		os << pe->get_data();
	os << std::endl;
	writesp += strincr;
}

template<class Node>
void output_visitor<Node>::write_end_tag_with_children(const Node*& pe) {
	writesp = writesp.substr(0,writesp.length()-ns);
	os << writesp << "</" + pe->get_local_name() << ">" << std::endl;
}
#endif // AMBULANT_NO_IOSTREAMS


////////////////////////
// trimmed_output_visitor

#ifndef AMBULANT_NO_IOSTREAMS
template <class Node>
void trimmed_output_visitor<Node>::operator()(std::pair<bool, const Node*> x) {
	const Node*& pe = x.second;
	if(x.first) {
		// start tag
		if(!pe->down()) 
			write_start_tag_no_children(pe);
		else 
			write_start_tag_with_children(pe);
	}
	else if(pe->down())
		write_end_tag_with_children(pe);
}

template <class Node>
void trimmed_output_visitor<Node>::write_start_tag_no_children(const Node*& pe) {
	std::string data = pe->get_trimmed_data();
	if(data.length()==0)
		os <<  "<" + pe->xmlrepr() + "/>";
	else {
		os <<  "<" << pe->xmlrepr() << ">";
		os << data;
		os << "</" << pe->get_local_name() << ">";
	}
}

template <class Node>
void trimmed_output_visitor<Node>::write_start_tag_with_children(const Node*& pe) {
	os <<  "<" + pe->xmlrepr() + ">";
	std::string data = pe->get_trimmed_data();
	if(data.length()>0)
		os << data;
}

template <class Node>
void trimmed_output_visitor<Node>::write_end_tag_with_children(const Node*& pe) {
	os << "</" + pe->get_local_name() << ">";
}
#endif // AMBULANT_NO_IOSTREAMS

class builtin_node_factory : public lib::node_factory {
  public:
	lib::node *new_node(const char *local_name, const char **attrs, const lib::node_context *ctx);
	lib::node *new_node(const lib::xml_string& local_name, const char **attrs = 0, const lib::node_context *ctx = 0);
	lib::node *new_node(const lib::q_name_pair& qn, const lib::q_attributes_list& qattrs, const lib::node_context *ctx = 0);
	lib::node *new_node(const lib::node* other);
	lib::node *new_data_node(const char *data, size_t size, const lib::node_context *ctx);
};

// If we are building a player with an (optional) external DOM implementation
// we need to define a couple more things:
// - factory functions (which are defined inline for non-external DOM builds)
// - a couple of methods that accept node_interface parameters and do
//   dynamic typechecks that the arguments are actually node_impl's.

// Factory functions
lib::node *
builtin_node_factory::new_node(const char *local_name, const char **attrs, const lib::node_context *ctx)
{
	return new lib::node_impl(local_name, attrs, ctx);
}

/// Construct a new, unconnected, node.
/// Note: attrs are as per expat parser
/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
lib::node *
builtin_node_factory::new_node(const lib::xml_string& local_name, const char **attrs, const lib::node_context *ctx)
{
	return new lib::node_impl(local_name, attrs, ctx);
}

/// Construct a new, unconnected, node.
/// Note: attrs are as per expat parser
/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
lib::node *
builtin_node_factory::new_node(const lib::q_name_pair& qn, const lib::q_attributes_list& qattrs, const lib::node_context *ctx)
{
	return new lib::node_impl(qn, qattrs, ctx);
}

// shallow copy from other.
lib::node *
builtin_node_factory::new_node(const lib::node* other)
{
#if WITH_EXTERNAL_DOM
	return new lib::node_impl(dynamic_cast<const lib::node_impl*>(other));
#else
	return new lib::node_impl(other);
#endif
}

// create data node
lib::node *
builtin_node_factory::new_data_node(const char *data, size_t size, const lib::node_context *ctx)
{
	return new lib::node_impl(data, size, ctx);
}

lib::node_factory *lib::get_builtin_node_factory()
{
	static builtin_node_factory nf;
	
	return &nf;
}

//#define WITH_PRINT_NODE
#ifdef	WITH_PRINT_NODE
#include <typeinfo>

extern "C" {
	void
	print_node(lib::node_interface* p)
	{
		const char* type = "", *required = "N8ambulant3lib9node_implE";
		try {
	  		type = typeid(*p).name();
		} catch (std::bad_typeid) { 
			std::cerr << "Unable to find typeid\n";
			return;
		}
		if (strcmp (type, required) == 0) {
  			lib::node_impl* node = dynamic_cast<lib::node_impl*>(p);
			if (node) {
				static lib::xml_string s = node->to_string();
				std::cout << s.c_str();
			}
		}
	}
} // extern "C"
#endif//WITH_PRINT_NODE
