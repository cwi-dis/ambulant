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


#pragma comment (lib,"xerces-c_2.lib")


#include "ambulant/lib/xerces_parser.h"
#include "ambulant/common/preferences.h"
#include "ambulant/lib/logger.h"

#ifdef	WITH_XERCES
#include <xercesc/framework/MemBufInputSource.hpp>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

using namespace lib;




lib::xml_parser*
lib::xerces_factory::new_parser(
		sax_content_handler* content_handler, 
		sax_error_handler* error_handler) 
{
	AM_DBG lib::logger::get_logger()->debug("xerces_factory::new_parser(): xerces parser returned");
	return new lib::xerces_sax_parser(content_handler, error_handler);
}


std::string 
lib::xerces_factory::get_parser_name()
{
	AM_DBG lib::logger::get_logger()->debug("xerces_factory::get_parser_name(): xerces parser");
	return "xerces";
}



xerces_sax_parser::xerces_sax_parser(sax_content_handler*content_handler,
				     sax_error_handler *error_handler) 
:	m_content_handler(content_handler),
	m_error_handler(error_handler),
	m_parsing(false),
	m_saxparser(0), m_logger(0), m_buf((char*)malloc(1)), m_size(0),
	m_id("AmbulantXercesParser") {
	m_logger = lib::logger::get_logger();
        AM_DBG m_logger->debug("***  :xerces_sax_parser()");
	XMLPlatformUtils::Initialize();
	m_saxparser = new SAXParser();
	
	common::preferences* prefs = common::preferences::get_preferences();
	// Val_Never, Val_Always, Val_Auto
	m_saxparser->setValidationScheme(ambulant_val_scheme_2_xerces_ValSchemes(prefs->m_validation_scheme));
	
	// If set to true, namespace processing must also be turned on
	m_saxparser->setDoSchema(prefs->m_do_schema);

	// True to turn on full schema constraint checking
	m_saxparser->setValidationSchemaFullChecking
		(prefs->m_validation_schema_full_checking);
	
	// true: understand namespaces; false: otherwise
	m_saxparser->setDoNamespaces(prefs->m_do_namespaces);
	
	m_saxparser->setDocumentHandler(this);
	m_saxparser->setErrorHandler(this);
}

xerces_sax_parser::~xerces_sax_parser() {
	AM_DBG m_logger->debug("xerces_sax_parser::~xerces_sax_parser()");
	XMLPlatformUtils::Terminate();
	free (m_buf);
}

bool
xerces_sax_parser::parse(const char *buf, size_t len, bool final) {
	bool succeeded = false;
	size_t old_size = m_size;
	m_buf = (char*) realloc(m_buf, m_size += len);
	if (m_buf == NULL)
		return false;
	memcpy(&m_buf[old_size], buf, len);
	if (final == false)
		return true;
	MemBufInputSource membuf((const XMLByte*) m_buf, m_size, m_id);
	try {
		m_saxparser->parse(membuf);
		succeeded = true;
	} catch (const XMLException& e) {
		char *exceptionMessage = XMLString::transcode(e.getMessage());
		int linenumber = e.getSrcLine();
		sax_error err(exceptionMessage, linenumber, -1);
		if(m_error_handler != 0)
			m_error_handler->error(err);
		else
			throw;
		XMLString::release(&exceptionMessage);
	} catch (const SAXParseException& e) {
		char *exceptionMessage = XMLString::transcode(e.getMessage());
		int linenumber = e.getLineNumber();
		int column = e.getColumnNumber();
		sax_error err(exceptionMessage, linenumber, column);
		if(m_error_handler != 0)
			m_error_handler->error(err);
		else
			throw e;
		XMLString::release(&exceptionMessage);
	} catch (...) {
		m_logger->error(gettext("%s: Unexpected exception during parsing"), m_id);
	}
	
	return succeeded;
}

void
xerces_sax_parser::set_content_handler(sax_content_handler *h) {
 	m_content_handler = h;
}

void
xerces_sax_parser::set_error_handler(sax_error_handler *h) {
		m_error_handler = h;
}

void
xerces_sax_parser::startElement(const XMLCh* const name,
				AttributeList& attrs) {
	char *cname = XMLString::transcode(name);
	AM_DBG m_logger->debug("*** startElement %s", cname);
	q_name_pair qname = to_q_name_pair(name);
	q_attributes_list qattrs;
	to_qattrs(attrs, qattrs);
	m_content_handler->start_element(qname, qattrs);
	XMLString::release(&cname);
}

void
xerces_sax_parser::endElement(const XMLCh* const name) {
	char *cname = XMLString::transcode(name);
	AM_DBG m_logger->debug("*** endElement %s", cname);
	q_name_pair qname = to_q_name_pair(name);
	m_content_handler->end_element(qname);
	XMLString::release(&cname);
}

void 
xerces_sax_parser::warning(const SAXParseException& exception) {
	throw exception;
}

void 
xerces_sax_parser::error(const SAXParseException& exception) {
	throw exception;
}

void
xerces_sax_parser::fatalError(const SAXParseException& exception)  {
	throw exception;
}
	
void
xerces_sax_parser::to_qattrs(AttributeList& attrs, 
			     q_attributes_list& list) {
	if (attrs.getLength() == 0) return;
	for (int i = 0; i < attrs.getLength(); i++) {
		char* value = XMLString::transcode(attrs.getValue(i));
		xml_string xmlvalue(value);
		q_attribute_pair qap (to_q_name_pair(attrs.getName(i)),
				      xmlvalue);
		list.push_back(q_attribute_pair(qap));
		XMLString::release(&value);
	}
}

q_name_pair 
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

SAXParser::ValSchemes
xerces_sax_parser::ambulant_val_scheme_2_xerces_ValSchemes(std::string v) {
	SAXParser::ValSchemes rv = SAXParser::Val_Never;

	if (v == "never")
		rv = SAXParser::Val_Never;
	else if (v == "always")
		rv = SAXParser::Val_Always;
	else if (v == "auto")
		rv = SAXParser::Val_Auto;

	return rv;
}
#endif/*WITH_XERCES*/
