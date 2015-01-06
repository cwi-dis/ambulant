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

#include "ambulant/lib/tree_builder.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/logger.h"
#include "ambulant/net/url.h"
#include "ambulant/common/preferences.h"

#include <fstream>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::tree_builder::tree_builder(node_factory *nf, node_context *context, const char *id)
:	m_xmlparser(0),
	m_root(0),
	m_current(0),
	m_well_formed(false),
	m_node_factory(nf),
	m_context(context),
	m_bufsize(1024),
	m_filename(id)
{
	assert(m_node_factory);
#if 0
    // XXXJACK Unsure why this assert would be needed, disabling it for now (untested)
	assert(m_node_factory == get_builtin_node_factory());
#endif
	m_buf = (char*) malloc(m_bufsize);
	assert(m_buf);
	reset();
}

lib::tree_builder::~tree_builder()
{
	if (m_buf != NULL)
		free(m_buf);
	if(m_xmlparser != 0)
		delete m_xmlparser;
	if(m_root != 0)
		delete m_root;
	// m_node_factory is a borrowed reference
}

lib::node*
lib::tree_builder::detach() {
	node* temp = m_root;
	m_root = m_current = 0;
	return temp;
}

bool
lib::tree_builder::build_tree_from_file(const char *filename) {
	if(!m_xmlparser) return false;
	if(!filename || !*filename) return false;
	m_filename = filename;

	std::ifstream ifs(filename);
	if(!ifs) return false;
	const size_t buf_size = 1024;
	char *buf = new char[buf_size];
	m_well_formed = true;
	while(!ifs.eof() && ifs.good()) {
		ifs.read(buf, buf_size);
		if(!m_xmlparser->parse(buf, (size_t)ifs.gcount(), ifs.eof())){
			m_well_formed = false;
			break;
		}
	}
	delete[] buf;
	ifs.close();
	return m_well_formed;
}

bool
lib::tree_builder::build_tree_from_str(const std::string& str) {
	if ( ! m_xmlparser) return false;
	m_well_formed = m_xmlparser->parse(str.data(), int(str.length()), true);
	return m_well_formed;
}

bool
lib::tree_builder::build_tree_from_str(const char *begin, const char *end) {
	if ( ! m_xmlparser) return false;
	m_well_formed = m_xmlparser->parse(begin, int(end-begin), true);
	return m_well_formed;
}

void
lib::tree_builder::reset() {
	m_xml_space_stack.clear();
	global_parser_factory* pf;
	pf = lib::global_parser_factory::get_parser_factory();
	if(m_xmlparser != 0) {
		delete m_xmlparser;
		m_xmlparser = 0;
	}
	if(m_root != 0) {
		delete m_root;
		m_root = 0;
		m_current = 0;
	}
	m_well_formed = false;
	std::string& parser_id = common::preferences::get_preferences()->m_parser_id;

	lib::logger::get_logger()->trace("Using parser %s", parser_id.c_str());
	AM_DBG lib::logger::get_logger()->debug("tree_builder::reset():	 pf = 0x%x, this = 0x%x", (void*) pf, (void*) this);

	if (m_xmlparser == NULL) {
		m_xmlparser = pf->new_parser(this, this);
	}
	if (m_xmlparser == NULL) {
		lib::logger::get_logger()->fatal(gettext("Could not create any XML parser (configuration error?)"));
	}
	if (m_filename == "")
		m_filename = "<no filename>";
}

void
lib::tree_builder::start_document() {
}

void
lib::tree_builder::end_document() {
}

void
lib::tree_builder::start_element(const q_name_pair& qn, const q_attributes_list& qattrs) {
	if(m_root == 0) {
		m_root = m_current = m_node_factory->new_node(qn, qattrs, m_context);
	} else if(m_current != 0) {
		node *p;
		p = m_node_factory->new_node(qn, qattrs, m_context);
		m_current->append_child(p);
		m_current = p;
	} else {
		m_well_formed = false;
    }
	while (m_pending_namespaces.size()) {
		std::pair<std::string, std::string>& item = m_pending_namespaces.back();
		m_current->set_prefix_mapping(item.first, item.second);
		m_pending_namespaces.pop_back();
	}
	q_attributes_list::const_iterator it;
	for(it = qattrs.begin(); it != qattrs.end(); it++) {
		if((*it).first.second == "space") {
			std::pair<std::string,node*> xml_space_value((*it).second, m_current);
			m_xml_space_stack.push_back(xml_space_value);
			break;
		}
	}
}

void
lib::tree_builder::end_element(const q_name_pair& qn) {
	if (m_xml_space_stack.size() > 0 &&	 m_xml_space_stack.back().second == m_current)
		m_xml_space_stack.pop_back();
	if(m_current != 0)
		m_current = m_current->up();
	else
		m_well_formed = false;
}

void
lib::tree_builder::characters(const char *buf, size_t len) {
	if(m_current != 0) {
		// The <smiltext> tag has embedded data and tags
		lib::node* n = NULL;
		if (m_xml_space_stack.size() > 0
			&& m_xml_space_stack.back().first == "preserve")
			n = m_node_factory->new_data_node(buf, len, m_context);
		else { // collapse whitespace
			const char* s = buf;
			char* d;
			if (m_bufsize < (len+1)) {
				// ensure m_buf is big enough
				m_bufsize = len+1;
				m_buf = (char*) realloc (m_buf, m_bufsize);
			}
			d = m_buf;
			assert(m_buf);
			bool skipspace = false;
			while (s < buf+len) {
				if (isascii(*s) && isspace(*s)) {
					if (!skipspace) *d++ = ' ';
					skipspace = true;
				} else {
					*d++ = *s;
					skipspace = false;
				}
				s++;
			}
			// Final tweak: if there's only whitespace between two tags we remove it
			if (d == m_buf+1 && *m_buf == ' ')
				d = m_buf;
			if (d > m_buf)
				n = m_node_factory->new_data_node(m_buf, d-m_buf, m_context);
		}
		if (n) m_current->append_child(n);
	} else
		m_well_formed = false;
}

void
lib::tree_builder::start_prefix_mapping(const std::string& prefix, const std::string& uri) {
	AM_DBG lib::logger::get_logger()->debug("xmlns:%s=\"%s\"", prefix.c_str(), uri.c_str());
	if(m_context)
		m_context->set_prefix_mapping(prefix, uri);
	std::pair<std::string,std::string> item(prefix, uri);
	m_pending_namespaces.push_back(item);
}

void
lib::tree_builder::end_prefix_mapping(const std::string& prefix) {
	(void)prefix; // UNREFERENCED_PARAMETER(prefix)
}

void
lib::tree_builder::error(const sax_error& err) {
	m_well_formed = false;
	lib::logger::get_logger()->trace("%s, line %d, column %d: Parse error: %s", m_filename.c_str(), err.get_line(), err.get_column(), err.what());
	lib::logger::get_logger()->error(gettext("%s: Error parsing document"), m_filename.c_str());
}
