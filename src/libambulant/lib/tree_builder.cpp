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

#include "ambulant/lib/tree_builder.h"
#include "ambulant/lib/document.h"
#include "ambulant/common/region_node.h"

#include "ambulant/lib/logger.h"

#ifndef AMBULANT_NO_IOSTREAMS
#include <fstream>
#endif

using namespace ambulant;

lib::tree_builder::tree_builder(node_context *context)
:	m_xmlparser(0),
	m_root(0),
	m_current(0),
	m_well_formed(false),
	m_context(context) {
	m_xmlparser = new expat_parser(this, this);
}

lib::tree_builder::~tree_builder()
	{
	if(m_xmlparser != 0)
		delete m_xmlparser;
	if(m_root != 0)
		delete m_root;
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

#ifndef AMBULANT_NO_IOSTREAMS
	std::ifstream ifs(filename);
	if(!ifs) return false;

	const size_t buf_size = 1024;
	char *buf = new char[buf_size];
	m_well_formed = true;
	while(!ifs.eof() && ifs.good())
		{
		ifs.read(buf, buf_size);
		if(!m_xmlparser->parse(buf, ifs.gcount(), ifs.eof()))
			{
			m_well_formed = false;
			break;
			}
		}
	delete[] buf;
	ifs.close();
#endif

	return m_well_formed;
}

bool 
lib::tree_builder::build_tree_from_str(const std::string& str) {
	m_well_formed = m_xmlparser->parse(str.data(), int(str.length()), true);
	return m_well_formed;
}

void 
lib::tree_builder::reset() {
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
	m_xmlparser = new expat_parser(this, this);
}

void 
lib::tree_builder::start_document() {
	m_parsing_layout = false;
}

void 
lib::tree_builder::end_document() {
}

void 
lib::tree_builder::start_element(const q_name_pair& qn, const q_attributes_list& qattrs) {
	if(m_root == 0) {
		m_root = m_current = new node(qn, qattrs, m_context);
	} else if(m_current != 0) {
		node *p;
		if (m_parsing_layout)
			p = new region_node(qn, qattrs, m_context);
		else
			p = new node(qn, qattrs, m_context);
		// XXXX This should use schema:
		if (qn.second == "layout")
			m_parsing_layout = true;
		m_current->append_child(p);
		m_current = p;
	} else
		m_well_formed = false;
}

void 
lib::tree_builder::end_element(const q_name_pair& qn) {
	if (qn.second == "layout")
		m_parsing_layout = false;
	if(m_current != 0)
		m_current = m_current->up();
	else
		m_well_formed = false;
}

void 
lib::tree_builder::characters(const char *buf, size_t len) {
	if(m_current != 0)
		m_current->append_data(buf, len);
	else
		m_well_formed = false;
}

void 
lib::tree_builder::start_prefix_mapping(const std::string& prefix, const std::string& uri) {
	lib::logger::get_logger()->debug("xmlns:%s=\"%s\"", prefix.c_str(), uri.c_str());
	if(m_context)
		m_context->set_prefix_mapping(prefix, uri);
}

void 
lib::tree_builder::end_prefix_mapping(const std::string& prefix) {
	(prefix); // UNREFERENCED_PARAMETER(prefix)
}

void 
lib::tree_builder::error(const sax_error& error) {
	m_well_formed = false;
	lib::logger::get_logger()->error("%s at line %d column %d", error.what(), error.get_line(), error.get_column());
}
