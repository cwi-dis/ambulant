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

namespace ambulant {

namespace lib {

///////////////////////////
// Adapter for xerces parser

using namespace xercesc;

class xerces_sax_parser : public HandlerBase {
  public:
	xerces_sax_parser();
	~xerces_sax_parser();
	
	bool parse(const char *filename);
	
	void startElement(const XMLCh* const name, AttributeList& attributes) {
		char *cname = XMLString::transcode(name);
		m_logger->trace("*** startElement %s", cname);
		XMLString::release(&cname);
	}
   
	void endElement(const XMLCh* const name) {
		char *cname = XMLString::transcode(name);
		m_logger->trace("*** endElement %s", cname);
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
	// ...
	
  private:
	SAXParser *m_saxparser;  
	lib::logger *m_logger;
};


inline xerces_sax_parser::xerces_sax_parser() 
:	m_saxparser(0), m_logger(0) {
	m_logger = lib::logger::get_logger();
	m_saxparser = new SAXParser();
	
	// Val_Never, Val_Always, Val_Auto
	m_saxparser->setValidationScheme(SAXParser::Val_Never);
	
	// If set to true, namespace processing must also be turned on
	m_saxparser->setDoSchema(false);
	
	// True to turn on full schema constraint checking
	m_saxparser->setDoValidation(false);
	
	// True to turn on full schema constraint checking
	m_saxparser->setValidationSchemaFullChecking(false);
	
	// true: understand namespaces; false: otherwise
	m_saxparser->setDoNamespaces(false);
	
	m_saxparser->setDocumentHandler(this);
	m_saxparser->setErrorHandler(this);

}

inline xerces_sax_parser::~xerces_sax_parser() {
	delete m_saxparser;
}

inline bool xerces_sax_parser::parse(const char *filename) {
	bool succeeded = false;
	try {
		m_saxparser->parse(filename);
		succeeded = true;
	} catch (const XMLException& e) {
        char *exceptionMessage = XMLString::transcode(e.getMessage());
        m_logger->error("During parsing: %s \n Exception message is: %s \n",
            filename, exceptionMessage);
        XMLString::release(&exceptionMessage);
    } catch (const SAXParseException& e) {
        char *exceptionMessage = XMLString::transcode(e.getMessage());
        m_logger->error("During parsing: %s \n Exception message is: %s \n",
			filename, exceptionMessage);
        XMLString::release(&exceptionMessage);
    } catch (...) {
         m_logger->error("Unexpected exception during parsing");
    }
	return succeeded;
}

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_XERCES_PARSER_H
