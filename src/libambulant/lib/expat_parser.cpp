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

#include "ambulant/lib/expat_parser.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/logger.h"

#define AM_DBG
#ifndef AM_DBG
#define AM_DB if(0)
#endif

using namespace ambulant;

#ifdef AMBULANT_PLATFORM_WIN32
// I hate this, but it seems to be the standard way to make sure
// libraries are included on Win32.
#pragma comment (lib,"libexpat.lib")
#endif

lib::xml_parser*
lib::expat_factory::new_parser(
		sax_content_handler* content_handler, 
		sax_error_handler* error_handler) 
{
	AM_DBG lib::logger::get_logger()->debug("expat_factory::new_parser(): expat parser returned");
	return new lib::expat_parser(content_handler, error_handler);
}

std::string 
lib::expat_factory::get_parser_name()
{
	return "expat";
}

lib::expat_parser::expat_parser(lib::sax_content_handler *content_handler, 
	lib::sax_error_handler *error_handler)
:	m_content_handler(content_handler),
	m_expatParser(0),
	m_error_handler(error_handler),
	m_parsing(false) {
#ifdef UNICODE
	m_expatParser = XML_ParserCreateNS(0, wchar_t(NS_SEP));
#else
	m_expatParser = XML_ParserCreateNS(0, char(NS_SEP));
#endif

#ifndef AMBULANT_PLATFORM_WIN32_WCE_3	
	if(m_expatParser == 0) 
		throw std::runtime_error("XML_ParserCreateNS() failed");
#else
	assert(m_expatParser != 0);
#endif 
	XML_SetUserData(m_expatParser, this);
	XML_SetElementHandler(m_expatParser, expat_parser::start_element, expat_parser::end_element);
	XML_SetCharacterDataHandler(m_expatParser, expat_parser::characters);
	XML_SetNamespaceDeclHandler(m_expatParser, expat_parser::start_prefix_mapping, 
		expat_parser::end_prefix_mapping);
}

lib::expat_parser::~expat_parser() {
	if(m_expatParser != 0)
		XML_ParserFree(m_expatParser);
}

// parses a chunk of data using expat parser
// throws a sax_error if the error handler is null
bool lib::expat_parser::parse(const char *buf, size_t len, bool final) {
	if(!m_parsing) {
		m_parsing = true;
		m_content_handler->start_document();
	}
	if(XML_Parse(m_expatParser, buf, int(len), (final?1:0)) != 1)  {
		sax_error e(XML_ErrorString(XML_GetErrorCode(m_expatParser)), 
			XML_GetCurrentLineNumber(m_expatParser),
			XML_GetCurrentColumnNumber(m_expatParser)
			);
		if(m_error_handler != 0)
			m_error_handler->error(e);
		else
#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
			throw e;
#else 
			assert(false);
#endif
		return false;
	}
	if(m_parsing && final) {
		m_content_handler->end_document();
		m_parsing = false;
	}
	return true;
}

//static 
void lib::expat_parser::start_element(void *usrptr, const char *name, const char **attrs) {
	expat_parser *p = static_cast<expat_parser*>(usrptr);
	q_name_pair qname = to_q_name_pair(name);
	q_attributes_list qattrs;
	to_qattrs(attrs, qattrs);
	p->m_content_handler->start_element(qname, qattrs);
}

// static 
void lib::expat_parser::end_element(void *usrptr, const char *name) {
	expat_parser *p = static_cast<expat_parser*>(usrptr);
	q_name_pair qname = to_q_name_pair(name);
	p->m_content_handler->end_element(qname);
}

// static 
void lib::expat_parser::characters(void *usrptr, const char *buf, int len) {
	expat_parser *p = static_cast<expat_parser*>(usrptr);
	p->m_content_handler->characters(buf, len);
}
	
// static 
void lib::expat_parser::start_prefix_mapping(void *usrptr, const char *prefix, const char *uri) {
	expat_parser *p = static_cast<expat_parser*>(usrptr);
	std::string prefix_str((prefix!=0)?prefix:"");
	std::string uri_str((uri!=0)?uri:"");
	p->m_content_handler->start_prefix_mapping(prefix_str, uri_str);
}
	
// static 
void lib::expat_parser::end_prefix_mapping(void *usrptr, const char *prefix) {
	expat_parser *p = static_cast<expat_parser*>(usrptr);
	std::string prefix_str((prefix!=0)?prefix:"");
	p->m_content_handler->end_prefix_mapping(prefix_str);
}
	
//static 
void lib::expat_parser::to_qattrs(const char **attrs, q_attributes_list& list) {
	if(attrs == 0) return;
	for(int i=0;attrs[i];i+=2) {
		list.push_back(q_attribute_pair(to_q_name_pair(attrs[i]), attrs[i+1]));
	}
}
	
// static 
lib::q_name_pair 
lib::expat_parser::to_q_name_pair(const char *name) {
	const char *p = name;
	const char ns_sep = char(NS_SEP);
	while(*p != 0 && *p != ns_sep) p++;
	q_name_pair qn;
	if(*p == ns_sep) { 
		qn.first = std::string(name, int(p-name));
		qn.second = std::string(p+1);
	} else {
		qn.first = "";
		qn.second = name;
	}
	return  qn;
}
