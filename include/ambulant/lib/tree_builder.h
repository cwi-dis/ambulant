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

// A utility for building a dom tree
// from a file or a string.
// Uses expat parser as the xml parser
// and nodes from node.h

#ifndef AMBULANT_LIB_TREE_BUILDER_H
#define AMBULANT_LIB_TREE_BUILDER_H

//#define WITH_XERCES 
#ifdef	WITH_XERCES
#else /*WITH_XERCES*/
#endif/*WITH_XERCES*/
#include "ambulant/config/config.h"




#include "ambulant/lib/sax_handler.h"
#ifdef WITH_EXPAT
#include "ambulant/lib/expat_parser.h"
#endif
//#ifdef WITH_XERCES_BUILTIN
//#include "ambulant/lib/xerces_parser.h"
//#endif/*WITH_XERCES_BUILTIN*/
#include "ambulant/lib/node.h"
#include "ambulant/net/url.h"
#include "ambulant/common/factory.h"

#include <string>

namespace ambulant {

namespace lib {

class node_context;

/// Build a DOM tree from a document.
class tree_builder : 
	public sax_content_handler, 
	public sax_error_handler {

  ///////////////
  public:
	tree_builder(node_context *context = 0, const char *filename = "");
	//tree_builder() {};
	~tree_builder();

	/// build DOM tree from a local file.
	bool build_tree_from_file(const char *filename);

	/// build DOM tree from std::string data.
	bool build_tree_from_str(const std::string& str);

	/// build DOM tree from a memory buffer.
	bool build_tree_from_str(const char *begin, const char *end);

	/// build DOM tree from a file anywhere on the net.
	bool build_tree_from_url(const net::url& u);
	
	/// Return true if the document was parsed correctly.
	bool was_well_formed() const {return m_well_formed;}
	
	/// Get a pointer to the root node.
	/// Use detach() to become owner.
	node* get_tree() { return m_root;}

	/// Get a pointer to the root node.
	const node* get_tree() const { return m_root;}
	
	/// Get a pointer to the root node and become the owner of it.
	node* detach();

	/// Set ready to build next xml tree.
	void reset();
	
	///////////////
	/// sax_content_handler interface method.
	virtual void start_document();
	/// sax_content_handler interface method.
	virtual void end_document();
	/// sax_content_handler interface method.
	virtual void start_element(const q_name_pair& qn, const q_attributes_list& qattrs);
	/// sax_content_handler interface method.
	virtual void end_element(const q_name_pair& qn);
	/// sax_content_handler interface method.
	virtual void start_prefix_mapping(const xml_string& prefix, const xml_string& uri);
	/// sax_content_handler interface method.
	virtual void end_prefix_mapping(const xml_string& prefix);
	/// sax_content_handler interface method.
	virtual void characters(const char *buf, size_t len);
	
	///////////////
	/// sax_error_handler interface method.
	virtual void error(const sax_error& error);
	
  ///////////////
  private:
	xml_parser *m_xmlparser;
	node *m_root;
	node *m_current;
	bool m_well_formed;
	node_context *m_context;
	std::string m_filename;		// For error messages only!
};


} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_TREE_BUILDER_H
