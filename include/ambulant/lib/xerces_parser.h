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

#include <string>

#include "ambulant/config/config.h"

#include "ambulant/common/preferences.h"

#include "ambulant/lib/sax_handler.h"

// temp for inline impl
#include "ambulant/lib/logger.h"

#ifdef	WITH_XERCES
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
	virtual ~xerces_sax_parser();
	
	bool parse(const char *filename);
	
	bool parse(const char *buf, size_t len, bool final);

	void set_content_handler(sax_content_handler *h);
       
	void set_error_handler(sax_error_handler *h);

	void startElement(const XMLCh* const name, AttributeList& attrs);

 	void endElement(const XMLCh* const name);   

	void characters(const XMLCh* const chars, const unsigned int length) {}
        
	void ignorableWhitespace(const XMLCh* const chars,
				 const unsigned int length) {}
    
	void resetDocument() {}

	void warning(const SAXParseException& exception);

	void error(const SAXParseException& exception);

	void fatalError(const SAXParseException& exception);
	
	void set_do_validating(bool b) { m_saxparser->setDoValidation(b);}
	void set_do_schema(bool b) { m_saxparser->setDoSchema(b);}
	
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
