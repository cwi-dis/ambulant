/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/tree_builder.h"

#include "ambulant/lib/logger.h"

#include <fstream>

using namespace ambulant;

lib::tree_builder::tree_builder()
:	m_xmlparser(0),
	m_root(0),
	m_current(0),
	m_well_formed(false) {
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
}

void 
lib::tree_builder::end_document() {
}

void 
lib::tree_builder::start_element(const q_name_pair& qn, const q_attributes_list& qattrs) {
	if(m_root == 0) {
		m_root = m_current = new node(qn, qattrs);
	} else if(m_current != 0) {
		node *p = new node(qn, qattrs);
		m_current->append_child(p);
		m_current = p;
	} else
		m_well_formed = false;
}

void 
lib::tree_builder::end_element(const q_name_pair& qn) {
	(qn); // UNREFERENCED_PARAMETER(qn);
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
	m_nscontext.set_prefix_mapping(prefix, uri);
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
