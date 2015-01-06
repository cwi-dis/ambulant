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

#ifndef AMBULANT_LIB_EXPAT_PARSER_H
#define AMBULANT_LIB_EXPAT_PARSER_H

#include "ambulant/config/config.h"

#include <string.h>
#include "ambulant/lib/sax_handler.h"
#include "ambulant/lib/parser_factory.h"

#include "expat.h"

namespace ambulant {

namespace lib {

class expat_factory : public lib::parser_factory {
  public:

	expat_factory() {};
	~expat_factory() {};

	lib::xml_parser* new_parser(
		sax_content_handler* content_handler,
		sax_error_handler* error_handler);

	std::string get_parser_name();
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
