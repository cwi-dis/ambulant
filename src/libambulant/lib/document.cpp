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

lib::document::document(node *root) 
:	m_root(root) {
	build_id2node_map();
	read_custom_attributes();
}

lib::document::document(node *root, const std::string& src_url) 
:	m_root(root),
	m_src_url(src_url) {
	build_id2node_map();
	read_custom_attributes();
}


lib::document::~document() {
	delete m_root;
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
lib::document::create_from_file(const std::string& filename) {
	document *d = new document();
	tree_builder builder(d);
	if(!builder.build_tree_from_file(filename.c_str())) {
		logger::get_logger()->error(
			"Could not build tree for file: %s", filename.c_str());
		return 0;
	}
	d->set_root(builder.detach());
	d->set_src_url(ambulant::net::url(filename));
	
//	std::string base = filesys::get_base(filename, file_separator.c_str());
//	d->set_src_base(ambulant::net::url(base));
	
	return d;
}

//static 
lib::document* 
lib::document::create_from_string(const std::string& smil_src) {
	document *d = new document();
	tree_builder builder(d);
	if(!builder.build_tree_from_str(smil_src)) {
		logger::get_logger()->error(
			"Could not build tree for the provided string");
		return 0;
	}
	d->set_root(builder.detach());
	return d;
}

void 
lib::document::set_prefix_mapping(const std::string& prefix, const std::string& uri) {
	m_namespaces.set_prefix_mapping(prefix, uri);
}

const char* 
lib::document::get_namespace_prefix(const xml_string& uri) const {
	return m_namespaces.get_namespace_prefix(uri);
}

net::url 
lib::document::resolve_url(const node *n, const net::url& rurl) const {
	// XXX This code is incomplete. It currently handles only full absolute
	// urls (with scheme and all) and relative urls if the document is
	// a local file. 
	net::url loc(rurl);
	if (loc.is_absolute()) {
		AM_DBG lib::logger::get_logger()->trace("document::resolve_url(%s): absolute URL", repr(rurl).c_str());
		return rurl;
	}
	net::url rv(rurl.join_to_base(m_src_url));
	AM_DBG lib::logger::get_logger()->trace("document::resolve_url(%s): %s\n", repr(rurl).c_str(), repr(rv).c_str());
	return rv;
}

void lib::document::set_root(node* n) {
	if(m_root != n) delete m_root;
	m_root = n;
	build_id2node_map();
	read_custom_attributes();
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
				if(o) logger::get_logger()->warn("Duplicate id: %s", pid);
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
			AM_DBG logger::get_logger()->trace("Custom test: %s", ::repr(t).c_str());
		}
	}
}


