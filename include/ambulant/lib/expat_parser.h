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

#ifndef AMBULANT_LIB_EXPAT_PARSER_H
#define AMBULANT_LIB_EXPAT_PARSER_H

#include "ambulant/config/config.h"

#include "ambulant/lib/sax_handler.h"
#include "ambulant/common/parser_factory.h"

#include "expat.h"

// To execute samples depending on libexpat.dll assert 
// that the dll is reachable at runtime 
// (e.g. its in the path).


namespace ambulant {

namespace lib {
	
class expat_factory : public common::parser_factory {
  public:

	expat_factory(
  		sax_content_handler* content_handler, 
  		sax_error_handler* error_handler) {};
	~expat_factory() {};
		
	lib::xml_parser* new_parser(
		sax_content_handler* content_handler, 
		sax_error_handler* error_handler);
};


///////////////////////////
// Adapter for expat parser


class expat_parser : public xml_parser {
  public:
	enum {NS_SEP = '|'};
	
	expat_parser(sax_content_handler *content_handler, sax_error_handler *error_handler);
	virtual ~expat_parser();

	// parses a chunk of data using expat parser
	// throws a sax_error if the error handler is null
	virtual bool parse(const char *buf, size_t len, bool final);
	virtual void set_content_handler(sax_content_handler *h) {
		m_content_handler = h;
	}
	virtual void set_error_handler(sax_error_handler *h) {
		m_error_handler = h;
	}	
	
  private:
	static void start_element(void *usrptr, const char *name, const char **attrs); 
	static void end_element(void *usrptr, const char *name);
	static void characters(void *usrptr, const char *buf, int len);
	static void start_prefix_mapping(void *usrptr, const char *prefix, const char *uri);
	static void end_prefix_mapping(void *usrptr, const char *prefix);	
	static void to_qattrs(const char **attrs, q_attributes_list& list); 
	static q_name_pair to_q_name_pair(const char *name);
	
	sax_content_handler *m_content_handler;
	XML_Parser m_expatParser;
	sax_error_handler *m_error_handler;
	bool m_parsing;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_EXPAT_PARSER_H
