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

#ifndef AMBULANT_LIB_XERCES_PARSER_H
#define AMBULANT_LIB_XERCES_PARSER_H

#include <string>

#include "ambulant/config/config.h"

#include <string.h>

#include "ambulant/common/preferences.h"

#include "ambulant/lib/parser_factory.h"
#include "ambulant/lib/sax_handler.h"
#include "ambulant/lib/parser_factory.h"

// temp for inline impl
#include "ambulant/lib/logger.h"

#ifdef	WITH_XERCES
// Assuming "xml-xerces/c/src" of the distribution
// is in the include path and bin directory in the lib path

#include "xercesc/parsers/SAXParser.hpp"
#include "xercesc/sax/HandlerBase.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/PlatformUtils.hpp"



namespace ambulant {

namespace lib {


class xerces_factory : public parser_factory {
  public:
	xerces_factory() {};
	xml_parser* new_parser(
		sax_content_handler* content_handler,
		sax_error_handler* error_handler);
	std::string get_parser_name();
};

///////////////////////////
// Adapter for xerces parser

using namespace xercesc;

class xerces_sax_parser : public HandlerBase, public xml_parser {
  public:
	enum {NS_SEP = ':'};

	xerces_sax_parser(sax_content_handler*,sax_error_handler*);
	virtual ~xerces_sax_parser();

	bool parse(const char *filename);

	bool parse(const char *buf, size_t len, bool final);

	void set_content_handler(sax_content_handler *h);

	void set_error_handler(sax_error_handler *h);

	void startElement(const XMLCh* const name, AttributeList& attrs);

	void endElement(const XMLCh* const name);

	void characters(const XMLCh* const chars, const XMLSize_t length);

	void ignorableWhitespace(const XMLCh* const chars, const unsigned int length) {}

	void resetDocument() {}

	void warning(const SAXParseException& exception);

	void error(const SAXParseException& exception);

	void fatalError(const SAXParseException& exception);

	// -------------------------------------------------------------
	//  Handlers for the SAX EntityResolver interface
	// -------------------------------------------------------------
	InputSource* resolveEntity(const XMLCh* const publicId , const XMLCh* const systemId);
  private:
	static void to_qattrs(AttributeList& attrs, q_attributes_list& list);
	static q_name_pair to_q_name_pair(const XMLCh* name);

	static SAXParser::ValSchemes ambulant_val_scheme_2_xerces_ValSchemes(std::string v);

	SAXParser *m_saxparser;
	lib::logger *m_logger;
	sax_content_handler *m_content_handler;
	sax_error_handler *m_error_handler;
	bool m_parsing;
	char* m_buf;
	size_t m_size;
	const char* m_id;
};

} // namespace lib

} // namespace ambulant
#endif/*WITH_XERCES*/

#endif // AMBULANT_LIB_XERCES_PARSER_H
