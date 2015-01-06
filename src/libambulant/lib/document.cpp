// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#include "ambulant/lib/document.h"
#include "ambulant/lib/tree_builder.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/filesys.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/lib/asb.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::document::document()
:	m_root(NULL),
	m_root_owned(false),
	m_state(NULL)
{
}

lib::document::~document() {
	if (m_root_owned) delete m_root;
	// m_state is borrowed.
	m_node2xpaths.clear();
	m_xpath2callbacks.clear();
	m_avtcache.clear();
}

lib::node*
lib::document::get_root(bool detach) {
	if(!detach)
		return m_root;
	node* tmp = m_root;
	m_root = 0;
	return tmp;
}

const lib::node*
lib::document::get_root() const {
	return m_root;
}

//static
lib::document*
lib::document::create_from_file(common::factories* factory, const std::string& filename) {
	document *d = new document();
	tree_builder builder(factory->get_node_factory(), d);
	if(!builder.build_tree_from_file(filename.c_str())) {
		// build_tree_from_file has reported the error already
		// logger::get_logger()->error(gettext("%s: Not a valid XML document"), filename.c_str());
		delete d;
		return 0;
	}
	if (!builder.assert_root_tag("smil")) {
		logger::get_logger()->error(gettext("%s: Not a SMIL document"), filename.c_str());
		delete d;
		return NULL;
	}
	d->set_root(builder.detach());
	d->set_src_url(ambulant::net::url::from_filename(filename));

//	std::string base = filesys::get_base(filename, file_separator.c_str());
//	d->set_src_base(ambulant::net::url(base));

	return d;
}

//static
lib::document*
lib::document::create_from_url(common::factories* factory, const net::url& u) {
	document *d = new document();
	tree_builder builder(factory->get_node_factory(), d);
	char *data;
	size_t datasize;
	if (!net::read_data_from_url(u, factory->get_datasource_factory(), &data, &datasize)) {
		logger::get_logger()->error(gettext("%s: Not a valid XML document"), u.get_url().c_str());
		delete d;
		return NULL;
	}
	if(!builder.build_tree_from_str(data, data+datasize)) {
		// build_tree_from_url has already given the error message
		// logger::get_logger()->error(gettext("%s: Not a valid XML document"), u.get_url().c_str());
		delete d;
		return NULL;
	}
	if (data) free(data);
	if (!builder.assert_root_tag("smil")) {
		delete d;
		logger::get_logger()->error(gettext("%s: Not a SMIL document"), u.get_url().c_str());
		logger::get_logger()->trace(gettext("expected: `%s', got: `%s'."), "smil", builder.get_tree()->get_local_name().c_str());
		return NULL;
	}
	d->set_root(builder.detach());
	d->set_src_url(u);
	return d;
}

//static
lib::document*
lib::document::create_from_string(common::factories* factory, const std::string& smil_src, const std::string& src_id) {
	document *d = new document();
	tree_builder builder(factory->get_node_factory(), d, src_id.c_str());
	if(!builder.build_tree_from_str(smil_src)) {
		logger::get_logger()->error(gettext("%s: Not a valid XML document"), src_id.c_str());
		delete d;
		return NULL;
	}
	if (!builder.assert_root_tag("smil")) {
		logger::get_logger()->error(gettext("%s: Not a SMIL document"), src_id.c_str());
		delete d;
		return NULL;
	}
	d->set_root(builder.detach());
	return d;
}

void
lib::document::set_prefix_mapping(const std::string& prefix, const std::string& uri) {
	m_namespaces.set_prefix_mapping(prefix, uri);
}

const lib::xml_string&
lib::document::get_namespace_prefix(const xml_string& uri) const {
	return m_namespaces.get_namespace_prefix(uri);
}

bool
lib::document::is_supported_prefix(const xml_string& prefix) const {
	return m_namespaces.is_supported_prefix(prefix) || m_namespaces.is_supported_namespace(prefix);
}

bool
lib::document::is_supported_namespace(const xml_string& uri) const {
	return m_namespaces.is_supported_namespace(uri);
}

net::url
lib::document::resolve_url(const net::url& rurl) const {
	net::url loc(rurl);
	if (loc.is_absolute()) {
		AM_DBG lib::logger::get_logger()->debug("document::resolve_url(%s): absolute URL", repr(rurl).c_str());
		return rurl;
	}
	net::url rv(rurl.join_to_base(m_src_url));
	AM_DBG lib::logger::get_logger()->debug("document::resolve_url(%s): %s\n", repr(rurl).c_str(), repr(rv).c_str());
	return rv;
}

void lib::document::set_root(node* n) {
	if(m_root_owned && m_root != n) delete m_root;
	m_root_owned = true;
	m_root = n;
	build_id2node_map();
	read_custom_attributes();
}

void
lib::document::tree_changed() {
	set_root(m_root);
}

void lib::document::build_id2node_map() {
	m_id2node.clear();
	if(!m_root) return;
	lib::node::iterator it;
	lib::node::iterator end = m_root->end();
	for(it = m_root->begin(); it != end; it++) {
		bool start_element = (*it).first;
		const lib::node *n = (*it).second;
		if(start_element) {
			const char *pid = n->get_attribute("id");
			if(pid) {
				const node *o = get_node(pid);
				if(o) logger::get_logger()->trace("Duplicate id: %s", pid);
				else m_id2node[pid] = n;
			}
		}
	}
}

void lib::document::read_custom_attributes() {
	if(!m_root) return;
	m_custom_tests.clear();
	const lib::node* ca = locate_node("/smil/head/customAttributes");
	if(!ca) return;
	lib::node::const_iterator it;
	lib::node::const_iterator end = ca->end();
	for(it = ca->begin(); it != end; it++) {
		std::pair<bool, const lib::node*> pair = *it;
		bool start_element = pair.first;
		const lib::node *n = pair.second;
		const std::string& tag = n->get_local_name();
		if(tag != "customTest") continue;
		const char *p = n->get_attribute("id");
		if(start_element && p) {
			std::string id = to_c_lower(p);
			custom_test t;
			t.idd = id;
			p = n->get_attribute("defaultState");
			std::string s = p?p:"";
			t.state = (s == "true")?true:false;
			p = n->get_attribute("title");
			t.title = p?p:"";
			p = n->get_attribute("override");
			s = p?p:"";
			t.override = (s=="visible")?true:false;
			p = n->get_attribute("uid");
			t.uid = p?p:"";
			m_custom_tests[id] = t;
			AM_DBG logger::get_logger()->debug("Custom test: %s", ::repr(t).c_str());
		}
	}
}

const lib::xml_string&
lib::document::apply_avt(const node* n, const lib::xml_string& attrname, const lib::xml_string& attrvalue) const
{
	if (!m_state) return attrvalue;
	/* XXXJACK Check that applying AVT to attribute "name" is allowed */
	xml_string rv = "";
	xml_string rest = attrvalue;
	while (rest != "") {
		xml_string::size_type openpos = rest.find('{');
		xml_string::size_type closepos = rest.find('}');
		if (openpos == std::string::npos && closepos == std::string::npos) {
			rv += rest;
			rest = "";
			break;
		}
		if (openpos == std::string::npos || closepos == std::string::npos ||
				openpos > closepos) {
			lib::logger::get_logger()->trace("Unmatched {} in %s=\"%s\"", attrname.c_str(), attrvalue.c_str());
			return attrvalue;
		}
		rv += rest.substr(0, openpos);
		xml_string expr = rest.substr(openpos+1, closepos-openpos-1);
		std::string expr_value = m_state->string_expression(expr.c_str());
		const_cast<document *>(this)->_register_node_avt_dependence(n, expr);
		rv += expr_value;
		rest = rest.substr(closepos+1);
	}
	std::map<const node *, std::map<xml_string, xml_string> >::iterator cachepos = const_cast<document *>(this)->m_avtcache.find(n);
	if (cachepos == m_avtcache.end()) {
		std::map<xml_string, xml_string> emptymap;
		const_cast<document *>(this)->m_avtcache.insert(make_pair(n, emptymap));
		cachepos = const_cast<document *>(this)->m_avtcache.find(n);
	}
	cachepos->second[attrvalue] = rv;
	const xml_string& rvholder = cachepos->second[attrvalue];
	return rvholder;
}
void
lib::document::_register_node_avt_dependence(const node* n, const xml_string& expr)
{
	AM_DBG lib::logger::get_logger()->debug("_register_node_avt_dependence(%s, %s)", n->get_sig().c_str(), expr.c_str());
	// XXXJACK for now, assume only xpaths, not expressions
	typedef std::multimap<const lib::node*, xml_string >::iterator ITER;
	std::pair<ITER, ITER> bnds = m_node2xpaths.equal_range(n);
	ITER i;
	bool found = false;
	for(i=bnds.first; i != bnds.second; i++) {
		if (i->second == expr) {
			found = true;
			break;
		}
	}
	if (found) {
		AM_DBG lib::logger::get_logger()->debug("_register_node_avt_dependence(%s, %s): already there", n->get_sig().c_str(), expr.c_str());
	} else {
		AM_DBG lib::logger::get_logger()->debug("_register_node_avt_dependence(%s, %s): inserted", n->get_sig().c_str(), expr.c_str());
		m_node2xpaths.insert(make_pair(n, expr));
	}
}


void
lib::document::register_for_avt_changes(const node* n, avt_change_notification *handler)
{
	AM_DBG lib::logger::get_logger()->debug("register_for_avt_changes(%s)", n->get_sig().c_str());
	typedef std::multimap<const lib::node*, xml_string >::iterator ITER;
	std::pair<ITER, ITER> bnds = m_node2xpaths.equal_range(n);
	ITER i;
	for(i=bnds.first; i != bnds.second; i++) {
		xml_string& expr = i->second;
		std::pair<const avt_change_notification*, const lib::node*> value(handler, n);
		typedef std::multimap<const xml_string, std::pair<const avt_change_notification*, const lib::node*> >::iterator JTER;
		std::pair<JTER, JTER> bnds2 = m_xpath2callbacks.equal_range(expr);
		JTER j;
		bool anyfound = false;
		bool found = false;
		for(j=bnds2.first; j != bnds2.second; j++) {
			anyfound = true;
			if (j->second == value) {
				found = true;
				break;
			}
		}
		if (found) {
			AM_DBG lib::logger::get_logger()->debug("register_for_avt_changes(%s): already there for %s", n->get_sig().c_str(), expr.c_str());
		} else {
			AM_DBG lib::logger::get_logger()->debug("register_for_avt_changes(%s): inserted for %s", n->get_sig().c_str(), expr.c_str());
			m_xpath2callbacks.insert(make_pair(expr, value));
			if (!anyfound) {
				AM_DBG lib::logger::get_logger()->debug("register_for_avt_changes: request callback for %s", expr.c_str());
				assert(m_state);
				m_state->want_state_change(expr.c_str(), this);
			}
		}
	}
}

void
lib::document::on_state_change(const char *ref)
{
	AM_DBG lib::logger::get_logger()->debug("document::on_state_change(%s)", ref);
	xml_string key(ref);
	typedef std::multimap<const xml_string, std::pair<const avt_change_notification*, const lib::node*> >::iterator JTER;
	std::pair<JTER, JTER> bnds = m_xpath2callbacks.equal_range(key);
	JTER j;
	for(j=bnds.first; j != bnds.second; j++) {
		avt_change_notification *handler = const_cast<avt_change_notification *>(j->second.first);
		const lib::node *n = j->second.second;
		AM_DBG lib::logger::get_logger()->debug("document::on_state_change(%s): call handler for %s", ref, n->get_sig().c_str());
		m_avtcache[n].clear();
		handler->avt_value_changed_for(n);
	}
}
