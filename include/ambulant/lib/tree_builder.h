
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

// A utility for building a dom tree
// from a file or a string.
// Uses expat parser as the xml parser
// and nodes from node.h

#ifndef AMBULANT_LIB_TREE_BUILDER_H
#define AMBULANT_LIB_TREE_BUILDER_H

#include <string>

#ifndef AMBULANT_LIB_SAX_HANDLER_H
#include "sax_handler.h"
#endif

#ifndef AMBULANT_LIB_EXPAT_PARSER_H
#include "expat_parser.h"
#endif

#ifndef AMBULANT_LIB_NODE_H
#include "node.h"
#endif

#ifndef AMBULANT_LIB_LOGGER_H
#include "logger.h"
#endif

#ifndef AMBULANT_LIB_NSCONTEXT_H
#include "nscontext.h"
#endif

namespace ambulant {

namespace lib {


class tree_builder : 
	public sax_content_handler, 
	public sax_error_handler {

  ///////////////
  public:

	tree_builder();
	
	~tree_builder();

	// build tree functions
	bool build_tree_from_file(const char *filename);
	bool build_tree_from_str(const std::string& str);

	// check result of build tree functions
	bool was_well_formed() const {return m_well_formed;}
	
	// get a pointer to the root node
	// use detach() to become owner
	node* get_tree() { return m_root;}
	const node* get_tree() const { return m_root;}
	
	// call this function to get the tree and become owner
	node* detach() {
		node* temp = m_root;
		m_root = m_current = 0;
		return temp;
	}

	const nscontext& get_namespace_context() const {
		return m_nscontext;
	}
	
	// set ready to build next xml tree
	void reset();
	
	///////////////
	// sax_content_handler interface	
	virtual void start_document();
	virtual void end_document();
	virtual void start_element(const q_name_pair& qn, const q_attributes_list& qattrs);
	virtual void end_element(const q_name_pair& qn);
	virtual void start_prefix_mapping(const xml_string& prefix, const xml_string& uri);
	virtual void end_prefix_mapping(const xml_string& prefix);
	virtual void characters(const char *buf, size_t len);
	
	///////////////
	// sax_error_handler interface	
	virtual void error(const sax_error& error);
	
  ///////////////
  private:
	expat_parser *m_xmlparser;
	node *m_root;
	node *m_current;
	bool m_well_formed;
	nscontext m_nscontext;
};


/////////////////////////////////////
// implementation

inline tree_builder::tree_builder()
:	m_xmlparser(0),
	m_root(0),
	m_current(0),
	m_well_formed(false) {
	m_xmlparser = new expat_parser(this, this);
}

inline tree_builder::~tree_builder()
	{
	if(m_xmlparser != 0)
		delete m_xmlparser;
	if(m_root != 0)
		delete m_root;
	}

inline bool tree_builder::build_tree_from_file(const char *filename) {
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

inline bool tree_builder::build_tree_from_str(const std::string& str) {
	m_well_formed = m_xmlparser->parse(str.data(), int(str.length()), true);
	return m_well_formed;
}

inline void tree_builder::reset() {
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

inline  void tree_builder::start_document() {
}

inline  void tree_builder::end_document() {
}

inline void tree_builder::start_element(const q_name_pair& qn, const q_attributes_list& qattrs) {
	if(m_root == 0) {
		m_root = m_current = new node(qn, qattrs);
	} else if(m_current != 0) {
		node *p = new node(qn, qattrs);
		m_current->append_child(p);
		m_current = p;
	} else
		m_well_formed = false;
}

inline void tree_builder::end_element(const q_name_pair& qn) {
	if(m_current != 0)
		m_current = m_current->up();
	else
		m_well_formed = false;
}

inline void tree_builder::characters(const char *buf, size_t len) {
	if(m_current != 0)
		m_current->append_data(buf, len);
	else
		m_well_formed = false;
}

inline void tree_builder::start_prefix_mapping(const std::string& prefix, const std::string& uri) {
	m_nscontext.set_prefix_mapping(prefix, uri);
}

inline void tree_builder::end_prefix_mapping(const std::string& prefix) {
}

inline void tree_builder::error(const sax_error& error) {
	m_well_formed = false;
	log_error_event("%s at line %d column %d", error.what(), error.get_line(), error.get_column());
}


} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_TREE_BUILDER_H

