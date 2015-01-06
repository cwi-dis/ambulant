// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/lib/expat_parser.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/logger.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

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

	if(m_expatParser == 0)
		throw std::runtime_error("XML_ParserCreateNS() failed");
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
	if(XML_Parse(m_expatParser, buf, int(len), (final?1:0)) != 1) {
		sax_error e(XML_ErrorString(XML_GetErrorCode(m_expatParser)),
			(int)XML_GetCurrentLineNumber(m_expatParser),
			(int)XML_GetCurrentColumnNumber(m_expatParser)
			);
		if(m_error_handler != 0)
			m_error_handler->error(e);
		else
			throw e;
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
