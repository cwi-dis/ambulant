/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef AMBULANT_COMMON_SMIL_HANDLER_H
#define AMBULANT_COMMON_SMIL_HANDLER_H

#include "ambulant/config/config.h"

#include <string>

// attribute pair
#include <utility>

// map
#include <utility>

#include "ambulant/lib/sax_handler.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/nscontext.h"
#include "ambulant/lib/logger.h"

////////////////
// Define local preprocessor symbols and macros
// The symbols and the macros will be undefined below

#define DEF_ELEMENT_HANDLER(name)\
void start_##name(const q_name_pair& qn, const q_attributes_list& qattrs);\
void end_##name(const q_name_pair& qn)

#define REG_ELEMENT_HANDLER(name)\
register_handler(q_name_pair(smil_xmlns, #name), &smil_handler::start_##name, &smil_handler::end_##name)

#define REG_ELEMENT_HANDLER2(ename, hname)\
register_handler(q_name_pair(smil_xmlns, #ename), &smil_handler::start_##hname, &smil_handler::end_##hname)

////////////////

namespace ambulant {

namespace lib {

const std::string smil_xmlns =
	"http://www.w3.org/TR/REC-smil/2000/SMIL20/Language";

class smil_handler :
	public sax_content_handler,
	public sax_error_handler {

  ///////////////
  public:
	smil_handler(node_factory *nf);
	~smil_handler();

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
  ///////////////

	///////////////
	// element handlers helpers
	typedef void (smil_handler::*START_HANDLER)(const q_name_pair& qn, const q_attributes_list& qattrs);
	typedef void (smil_handler::*END_HANDLER)(const q_name_pair& qn);
	struct handler_pair {
		handler_pair(START_HANDLER se = 0, END_HANDLER ee = 0)
		:	first(se), second(ee) {}
		START_HANDLER first;
		END_HANDLER second;
	};
	std::map<q_name_pair, handler_pair> m_handlers;
	void register_handler(const q_name_pair& qn, START_HANDLER se, END_HANDLER ee){
		m_handlers[qn] = handler_pair(se, ee);
	}

	///////////////
	// element handlers
	DEF_ELEMENT_HANDLER(smil);
	DEF_ELEMENT_HANDLER(head);
	DEF_ELEMENT_HANDLER(body);
	DEF_ELEMENT_HANDLER(layout);
	DEF_ELEMENT_HANDLER(root_layout);
	DEF_ELEMENT_HANDLER(top_layout);
	DEF_ELEMENT_HANDLER(region);
	DEF_ELEMENT_HANDLER(transition);
	DEF_ELEMENT_HANDLER(par);
	DEF_ELEMENT_HANDLER(seq);
	DEF_ELEMENT_HANDLER(switch);
	DEF_ELEMENT_HANDLER(excl);
	DEF_ELEMENT_HANDLER(priority_class);
	DEF_ELEMENT_HANDLER(ref);
	DEF_ELEMENT_HANDLER(reg_point);
	DEF_ELEMENT_HANDLER(prefetch);
	DEF_ELEMENT_HANDLER(a);
	DEF_ELEMENT_HANDLER(area);

	void default_start_element(const q_name_pair& qn, const q_attributes_list& qattrs);
	void default_end_element(const q_name_pair& qn);

	void unknown_start_element(const q_name_pair& qn, const q_attributes_list& qattrs);
	void unknown_end_element(const q_name_pair& qn);

	///////////////
	// data members
	node *m_root;
	node *m_current;
	nscontext m_nscontext;
	node_factory *m_node_factory;
};

/////////////////////////////////////
// implementation

inline smil_handler::smil_handler(node_factory *nf)
:	m_root(0),
	m_current(0),
	m_node_factory(nf) {

	REG_ELEMENT_HANDLER(smil);
	REG_ELEMENT_HANDLER(head);
	REG_ELEMENT_HANDLER(body);
	REG_ELEMENT_HANDLER(layout);
	REG_ELEMENT_HANDLER2(root-layout, root_layout);
	REG_ELEMENT_HANDLER2(topLayout, top_layout);
	REG_ELEMENT_HANDLER(region);
	REG_ELEMENT_HANDLER(transition);
	REG_ELEMENT_HANDLER(par);
	REG_ELEMENT_HANDLER(seq);
	REG_ELEMENT_HANDLER(switch);
	REG_ELEMENT_HANDLER(excl);
	REG_ELEMENT_HANDLER2(priorityClass, priority_class);
	REG_ELEMENT_HANDLER(ref);
	REG_ELEMENT_HANDLER2(regPoint, reg_point);
	REG_ELEMENT_HANDLER(prefetch);
	REG_ELEMENT_HANDLER(a);
	REG_ELEMENT_HANDLER(area);
}

inline smil_handler::~smil_handler() {
	if(m_root != 0)
		delete m_root;
	// m_node_factory is a borrowed reference
}

inline  void smil_handler::start_document() {
}

inline  void smil_handler::end_document() {
}

inline void smil_handler::start_element(const q_name_pair& qn, const q_attributes_list& qattrs) {
	std::map<q_name_pair, handler_pair>::iterator it = m_handlers.find(qn);
	if(it != m_handlers.end()) {
		START_HANDLER se = (*it).second.first;
		(this->*se)(qn, qattrs);
	} else
		unknown_start_element(qn, qattrs);
}

inline void smil_handler::end_element(const q_name_pair& qn) {
	// XXX: create a new
	std::map<q_name_pair, handler_pair>::iterator it = m_handlers.find(qn);
	if(it != m_handlers.end()) {
		END_HANDLER ee = (*it).second.second;
		(this->*ee)(qn);
	} else
		unknown_end_element(qn);
}

inline void smil_handler::characters(const char *buf, size_t len) {
	if(m_current != 0)
		m_current->append_data(buf, len);
}

inline void smil_handler::start_prefix_mapping(const std::string& prefix, const std::string& uri) {
	m_nscontext.set_prefix_mapping(prefix, uri);
}

inline void smil_handler::end_prefix_mapping(const std::string& prefix) {
}

inline void smil_handler::error(const sax_error& err) {
	lib::logger::get_logger()->error("%s at line %d column %d", err.what(), err.get_line(), err.get_column());
}

} // namespace lib

} // namespace ambulant

// Undefine preprocessor symbols and macros
#undef DEF_ELEMENT_HANDLER
#undef REG_ELEMENT_HANDLER
#undef REG_ELEMENT_HANDLER2
#undef XMLNS

#endif // AMBULANT_COMMON_SMIL_HANDLER_H

