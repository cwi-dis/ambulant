
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
#include <fstream>

#include "sax_handler.h"
#include "expat_parser.h"
#include "node.h"
#include "nscontext.h"


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
	node* detach();

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


} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_TREE_BUILDER_H

