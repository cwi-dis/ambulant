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

#include "ambulant/config/config.h"

#include "ambulant/lib/sax_handler.h"
#include "ambulant/lib/expat_parser.h"
#include "ambulant/lib/node.h"
#include "ambulant/net/url.h"

#include <string>

namespace ambulant {

namespace lib {

class node_context;

class tree_builder : 
	public sax_content_handler, 
	public sax_error_handler {

  ///////////////
  public:
	tree_builder(node_context *context = 0);
	
	~tree_builder();

	// build tree functions
	bool build_tree_from_file(const char *filename);
	bool build_tree_from_str(const std::string& str);
	bool build_tree_from_str(const char *begin, const char *end);
	bool build_tree_from_url(const net::url& u);
	
	// check result of build tree functions
	bool was_well_formed() const {return m_well_formed;}
	
	// get a pointer to the root node
	// use detach() to become owner
	node* get_tree() { return m_root;}
	const node* get_tree() const { return m_root;}
	
	// call this function to get the tree and become owner
	node* detach();

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
	node_context *m_context;
};


} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_TREE_BUILDER_H

