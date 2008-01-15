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
#include "ambulant/lib/node_dummy.h"
#include "ambulant/lib/logger.h"

// trim strings
#include "ambulant/lib/string_util.h"

// tree helper iterators and visitors
#include "ambulant/lib/node_navigator.h" 


using namespace ambulant;

// Two static helper routines, to give error messages
static void
readonly()
{
	lib::logger::get_logger()->fatal("programmer error: attempt to write to readonly DOM node");
}

static void
unimplemented()
{
	lib::logger::get_logger()->fatal("programmer error: attempt to use unimplemented feature of DOM node");
}

///////////////////////////////////////////////
// lib::node_dummy implementation

//////////////////////
// Node destructor

lib::node_dummy::~node_dummy() {
	// Can probably remain empty
}

///////////////////////////////
// basic navigation

const lib::node_dummy *
lib::node_dummy::down() const
{
	unimplemented();
	return NULL;
}

const lib::node_dummy *
lib::node_dummy::up() const
{
	unimplemented();
	return NULL;
}

const lib::node_dummy *
lib::node_dummy::next() const
{
	unimplemented();
	return NULL;
}

lib::node_dummy *
lib::node_dummy::down()
{
	unimplemented();
	return NULL;
}

lib::node_dummy *
lib::node_dummy::up()
{
	unimplemented();
	return NULL;
}

lib::node_dummy *
lib::node_dummy::next()
{
	unimplemented();
	return NULL;
}

//////////////////////
// set down/up/next

void 
lib::node_dummy::down(node_dummy *n)
{
	readonly();
}

void 
lib::node_dummy::up(node_dummy *n)
{
	readonly();
}

void 
lib::node_dummy::next(node_dummy *n)
{
	readonly();
}

///////////////////////////////
// deduced navigation

const lib::node_dummy* 
lib::node_dummy::previous() const { 
	return node_navigator<const node_dummy>::previous(this); 
}

const lib::node_dummy* 
lib::node_dummy::get_last_child() const { 
	return node_navigator<const node_dummy>::last_child(this);
}

void lib::node_dummy::get_children(std::list<const lib::node*>& l) const {
	node_navigator<const lib::node>::get_children(this, l);
}

///////////////////////////////
// search operations 

lib::node_dummy* 
lib::node_dummy::get_first_child(const char *name) {
	node_dummy *e = down();
	if(!e) return 0;
	if(e->get_local_name() == name) return e;
	e = e->next();
	while(e != 0) {
		if(e->get_local_name() == name) 
			return e;
		e = e->next();
	}
	return 0;
}

const lib::node_dummy* 
lib::node_dummy::get_first_child(const char *name) const {
	const node_dummy *e = down();
	if(!e) return 0;
	if(e->get_local_name() == name) return e;
	e = e->next();
	while(e != 0) {
		if(e->get_local_name() == name) 
			return e;
		e = e->next();
	}
	return 0;
}

lib::node_dummy* 
lib::node_dummy::locate_node(const char *path) {
	if(!path || !path[0]) {
		return this;
	}
	tokens_vector v(path, "/");
	node_dummy *n = 0;
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

lib::node_dummy* 
lib::node_dummy::get_root() { 
	return node_navigator<node_dummy>::get_root(this); 
}

inline std::string get_path_desc_comp(const lib::node_dummy *n) {
	std::string sbuf;
	const char *pid = n->get_attribute("id");
	sbuf += n->get_local_name();
	if(pid) {sbuf += ":"; sbuf += pid;}
	return sbuf;
}

std::string lib::node_dummy::get_path_display_desc() const {
	std::string sbuf;
	std::list<const node_dummy*> path;
	node_navigator<const node_dummy>::get_path(this, path);
	int nc = 0;
	std::list<const node_dummy*>::reverse_iterator it = path.rbegin();
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
	
lib::node_dummy* 
lib::node_dummy::append_child(node_dummy* child) { 
	return node_navigator<node_dummy>::append_child(this, child);
}
		
lib::node_dummy* 
lib::node_dummy::append_child(const char *name) { 
	// Create a new node with the given tag and append
	readonly();
	return NULL;
}

lib::node_dummy* 
lib::node_dummy::detach() {
	readonly();
	return node_navigator<node_dummy>::detach(this); 
}
	
void lib::node_dummy::append_data(const char *data, size_t len) {
	// Append to the data of this node
	readonly();
}

void lib::node_dummy::append_data(const char *c_str) { 
	append_data(c_str, strlen(c_str));
}

void lib::node_dummy::append_data(const xml_string& str) {
	// Append to the data of this node
	readonly();
}

void lib::node_dummy::set_attribute(const char *name, const char *value) { 
	// Add (name, value) to the set of attributes on this node
	readonly();
}

void lib::node_dummy::set_attribute(const char *name, const xml_string& value) {
	// Add (name, value) to the set of attributes on this node
	readonly();
}

// Note: attrs are as per expat parser
// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
void lib::node_dummy::set_attributes(const char **attrs) {
	if(attrs == 0) return;
	for(int i=0;attrs[i];i+=2)
		set_attribute(attrs[i], attrs[i+1]);
}
	
void lib::node_dummy::set_namespace(const xml_string& ns) {
	// Set the namespace part of the tag for this node
	readonly();
}

// create a deep copy of this
lib::node_dummy* 
lib::node_dummy::clone() const {
	unimplemented();
	return NULL;
}

////////////////////////
// data queries
/// Return the namespace part of the tag for this node.
const lib::xml_string&
lib::node_dummy::get_namespace() const
{
	unimplemented();
	return "";
}

/// Return the local part of the tag for this node.
const lib::xml_string&
lib::node_dummy::get_local_name() const
{
	unimplemented();
	return "";
}

/// Return namespace and local part of the tag for this node.
const lib::q_name_pair&
lib::node_dummy::get_qname() const
{
	unimplemented();
	return q_name_pair("","");
}

/// Return the unique numeric ID for this node.
int
lib::node_dummy::get_numid() const
{
	unimplemented();
	return 0;
}

/// Return the data for this node.
const lib::xml_string&
lib::node_dummy::get_data() const
{
	unimplemented();
	return "";
}

lib::xml_string 
lib::node_dummy::get_trimmed_data() const { 
	return trim(get_data());
}

const char *
lib::node_dummy::get_attribute(const char *name) const {
	// Return the value of the given attribute
	unimplemented();
	return 0;
}

const char *
lib::node_dummy::get_attribute(const std::string& name) const {
	return get_attribute(name.c_str());
}

// returns the resolved url of an attribute
net::url 
lib::node_dummy::get_url(const char *attrname) const {
	const char *rurl = get_attribute(attrname);
	if(!rurl) return net::url();
	net::url url(rurl);
	unimplemented(); // Should resolve the URL against the base URL for this document
	return url;
}

const char *
lib::node_dummy::get_container_attribute(const char *name) const {
	if(!name || !name[0]) return 0;
	const node_dummy *n = this;
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
lib::node_dummy::xmlrepr() const {
	// Return a representation of the node, without < and >
	unimplemented();
	return "";
}

std::string lib::node_dummy::get_sig() const {
	std::string s = "<";
	s += get_local_name();
	const char *pid = get_attribute("id");
	if (pid) {
		s += " id=\"";
		s += pid;
		s += "\"";
	}
	s += ">";
	return s;
}

unsigned int 
lib::node_dummy::size() const {
	const_iterator it;
	const_iterator e = end();
	unsigned int count = 0;
	for(it = begin(); it != e; it++)
		if((*it).first) count++;
	return count;
}

/////////////////////
// node context
// these must be implemented

/// Return the node_context for this node.
const lib::node_context*
lib::node_dummy::get_context() const
{
	unimplemented();
	return NULL;
}

/// Set the node_context for this node.
void lib::node_dummy::set_context(lib::node_context *c)
{
	unimplemented();
}

/////////////////////
// dynamic_cast helper methods
// only needed for read-write DOM access, otherwise not

void
lib::node_dummy::down(lib::node_interface *n)
{
	readonly();
	down(dynamic_cast<node_dummy*>(n));
}

void
lib::node_dummy::up(lib::node_interface *n)
{
	readonly();
	up(dynamic_cast<node_dummy*>(n));
}

void
lib::node_dummy::next(lib::node_interface *n)
{
	readonly();
	next(dynamic_cast<node_dummy*>(n));
}

lib::node_interface*
lib::node_dummy::append_child(lib::node_interface* child)
{
	readonly();
	return append_child(dynamic_cast<node_dummy*>(child));
}

/////////////////////////
// Factory functions
// you need these for read-write DOM access. Also, the glue
// to connect the rest of the code to these routines is missing

namespace ambulant {
namespace lib {

// Factory functions
lib::node_interface *
dummy_node_factory(const char *local_name, const char **attrs, const node_context *ctx)
{
	readonly();
	return new node_dummy();
}

/// Construct a new, unconnected, node.
/// Note: attrs are as per expat parser
/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
lib::node_interface *
dummy_node_factory(const xml_string& local_name, const char **attrs, const node_context *ctx)
{
	readonly();
	return new node_dummy();
}

/// Construct a new, unconnected, node.
/// Note: attrs are as per expat parser
/// e.g. const char* attrs[] = {"attr_name", "attr_value", ..., 0};
lib::node_interface *
dummy_node_factory(const q_name_pair& qn, const q_attributes_list& qattrs, const node_context *ctx)
{
	readonly();
	return new node_dummy();
}

// shallow copy from other.
lib::node_interface *
dummy_node_factory(const lib::node_interface* other)
{
	readonly();
	return new node_dummy();
}

} // namespace lib
} // namespace ambulant

