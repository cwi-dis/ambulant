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

#ifndef AMBULANT_LIB_XERCES_PARSER_H
#define AMBULANT_LIB_XERCES_PARSER_H

#include "ambulant/config/config.h"

#include "ambulant/lib/sax_handler.h"

// temp for inline impl
#include "ambulant/lib/logger.h"

// Assuming "xml-xerces/c/src" of the distribution 
// is in the include path and bin directory in the lib path
#include "xercesc/parsers/SAXParser.hpp"
#include "xercesc/sax/HandlerBase.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/PlatformUtils.hpp"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

namespace lib {

///////////////////////////
// Adapter for xerces parser

using namespace xercesc;

class xerces_sax_parser : public HandlerBase, public xml_parser {
  public:
	enum {NS_SEP = '|'};

	xerces_sax_parser(sax_content_handler*,sax_error_handler*);
	~xerces_sax_parser();
	
	bool parse(const char *filename);
	
	bool parse(const char *buf, size_t len, bool final);

	void set_content_handler(sax_content_handler *h);
       
	void set_error_handler(sax_error_handler *h);

	void startElement(const XMLCh* const name,
			  AttributeList& attrs) {
		char *cname = XMLString::transcode(name);
		AM_DBG m_logger->trace("*** startElement %s", cname);
		q_name_pair qname = to_q_name_pair(name);
		q_attributes_list qattrs;
		to_qattrs(attrs, qattrs);
		m_content_handler->start_element(qname, qattrs);
		XMLString::release(&cname);
	}
   
	void endElement(const XMLCh* const name) {
		char *cname = XMLString::transcode(name);
		AM_DBG m_logger->trace("*** endElement %s", cname);
		q_name_pair qname = to_q_name_pair(name);
		m_content_handler->end_element(qname);
		XMLString::release(&cname);
	}
    
	void characters(const XMLCh* const chars, const unsigned int length) {}
        
	void ignorableWhitespace(const XMLCh* const chars, const unsigned int length) {}
    
	void resetDocument() {}

	void warning(const SAXParseException& exception) {
		m_logger->warn("*** Warning ");
		throw exception;
	}

	void error(const SAXParseException& exception) {
	        m_logger->error("*** Error ");
        	throw exception;
	}

	void fatalError(const SAXParseException& exception)  {
		m_logger->error("***** Fatal error ");
		throw exception;
	}
	
	void set_do_validating(bool b) { m_saxparser->setDoValidation(b);}
	void set_do_schema(bool b) { m_saxparser->setDoSchema(b);}
	static q_name_pair to_q_name_pair(const XMLCh*);
	static void to_qattrs(AttributeList&, q_attributes_list&);
	// ...
	
  private:
	SAXParser *m_saxparser;  
//XXXX	XML_Parser m_saxParser;
	lib::logger *m_logger;
	sax_content_handler *m_content_handler;
	sax_error_handler *m_error_handler;
	bool m_parsing;
	char* m_buf;
	size_t m_size;
	const char* m_id;
};

inline xerces_sax_parser::xerces_sax_parser(sax_content_handler *content_handler,
					    sax_error_handler *error_handler) 
:	m_content_handler(content_handler),
	m_error_handler(error_handler),
	m_parsing(false),
	m_saxparser(0), m_logger(0), m_buf((char*)malloc(1)), m_size(0),
	m_id("AmbulantXercesParser") {
	m_logger = lib::logger::get_logger();
        AM_DBG m_logger->trace("***  :xerces_sax_parser()");
	XMLPlatformUtils::Initialize();
	m_saxparser = new SAXParser();
	
	// Val_Never, Val_Always, Val_Auto
	m_saxparser->setValidationScheme(SAXParser::Val_Auto);
	
	// If set to true, namespace processing must also be turned on
	m_saxparser->setDoSchema(true);
	
	// True to turn on full schema constraint checking
	m_saxparser->setDoValidation(true);
	
	// True to turn on full schema constraint checking
	m_saxparser->setValidationSchemaFullChecking(true);
	
	// true: understand namespaces; false: otherwise
	m_saxparser->setDoNamespaces(true);
	
	m_saxparser->setDocumentHandler(this);
	m_saxparser->setErrorHandler(this);
}

inline xerces_sax_parser::~xerces_sax_parser() {
	delete m_saxparser;
	free (m_buf);
}

//static
inline void
xerces_sax_parser::to_qattrs(AttributeList& attrs, 
			     q_attributes_list& list) {
	if(attrs.getLength() == 0) return;
	for (int i = 0; i < attrs.getLength(); i++) {
		xml_string value = 
		  XMLString::transcode(attrs.getValue(i));
		q_attribute_pair qap (to_q_name_pair(attrs.getName(i)),
				      value);
		list.push_back(q_attribute_pair(qap));
	}
}
//static
inline q_name_pair 
xerces_sax_parser::to_q_name_pair(const XMLCh* name) {
	char *cname = XMLString::transcode(name);
	const char *p = cname;
	const char ns_sep = char(NS_SEP);
	while(*p != 0 && *p != ns_sep) p++;
	q_name_pair qn;
	if(*p == ns_sep) { 
		qn.first = std::string(cname, int(p-cname));
		qn.second = std::string(p+1);
	} else {
		qn.first = "";
		qn.second = cname;
	}
	XMLString::release(&cname);
	return  qn;
}

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_XERCES_PARSER_H
