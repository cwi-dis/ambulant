
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_EXPAT_PARSER_H
#define AMBULANT_LIB_EXPAT_PARSER_H

#ifndef AMBULANT_LIB_SAX_HANDLER_H
#include "sax_handler.h"
#endif

#include <string>

// Some of the following directives may go later to a config.h

#ifndef _WIN32
#include "expat.h"
#else
// third_party_packages directory
#define THIRD_PARTY_PACKAGES_DIR "D:/ufs/third_party_packages"

// The following include directive assumes that
// THIRD_PARTY_PACKAGES_DIR has been specified 
// as a project include directory.
#include "expat/lib/expat.h"

// libexpat path
#define LIBEXPAT_LIB THIRD_PARTY_PACKAGES_DIR "/expat/lib/Release/libexpat.lib"

// To execute samples depending on libexpat.dll assert 
// that the dll is reachable at runtime 
// (e.g. its in the path).

// For win32 we use the following MS extension to link with the import lib:
#ifdef _WIN32
#pragma comment (lib, LIBEXPAT_LIB)
#endif
#endif

namespace ambulant {

namespace lib {

///////////////////////////
// Adapter for expat parser

class expat_parser {
  public:
	enum {NS_SEP = '|'};
	
	expat_parser(sax_content_handler *content_handler, sax_error_handler *error_handler)
	:	m_content_handler(content_handler),
		m_error_handler(error_handler),
		m_expatParser(0),
		m_parsing(false) {
		m_expatParser = XML_ParserCreateNS(0, char(NS_SEP));
		if(m_expatParser == 0) 
			throw std::runtime_error("XML_ParserCreateNS() failed");
		XML_SetUserData(m_expatParser, this);
		XML_SetElementHandler(m_expatParser, expat_parser::start_element, expat_parser::end_element);
		XML_SetCharacterDataHandler(m_expatParser, expat_parser::characters);
		XML_SetNamespaceDeclHandler(m_expatParser, expat_parser::start_prefix_mapping, 
			expat_parser::end_prefix_mapping);
	}

	virtual ~expat_parser() {
		if(m_expatParser != 0)
			XML_ParserFree(m_expatParser);
	}

	// parses a chunk of data using expat parser
	// throws a sax_error if the error handler is null
	virtual bool parse(const char *buf, size_t len, bool final) {
		if(!m_parsing) {
			m_parsing = true;
			m_content_handler->start_document();
		}
		if(XML_Parse(m_expatParser, buf, int(len), (final?1:0)) != XML_STATUS_OK)  {
			sax_error e(XML_ErrorString(XML_GetErrorCode(m_expatParser)), 
				XML_GetCurrentLineNumber(m_expatParser),
				XML_GetCurrentColumnNumber(m_expatParser)
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
	
	private:
	static void start_element(void *usrptr, const char *name, const char **attrs) {
		expat_parser *p = static_cast<expat_parser*>(usrptr);
		q_name_pair qname = to_q_name_pair(name);
		q_attributes_list qattrs;
		to_qattrs(attrs, qattrs);
		p->m_content_handler->start_element(qname, qattrs);
	}

	static void end_element(void *usrptr, const char *name) {
		expat_parser *p = static_cast<expat_parser*>(usrptr);
		q_name_pair qname = to_q_name_pair(name);
		p->m_content_handler->end_element(qname);
	}

	static void characters(void *usrptr, const char *buf, int len) {
		expat_parser *p = static_cast<expat_parser*>(usrptr);
		p->m_content_handler->characters(buf, len);
	}
	
	static void start_prefix_mapping(void *usrptr, const char *prefix, const char *uri) {
		expat_parser *p = static_cast<expat_parser*>(usrptr);
		std::string prefix_str((prefix!=0)?prefix:"");
		std::string uri_str((uri!=0)?uri:"");
		p->m_content_handler->start_prefix_mapping(prefix_str, uri_str);
	}
	
	static void end_prefix_mapping(void *usrptr, const char *prefix) {
		expat_parser *p = static_cast<expat_parser*>(usrptr);
		std::string prefix_str((prefix!=0)?prefix:"");
		p->m_content_handler->end_prefix_mapping(prefix_str);
	}
	
	static void to_qattrs(const char **attrs, q_attributes_list& list) {
		if(attrs == 0) return;
		for(int i=0;attrs[i];i+=2) {
			list.push_back(q_attribute_pair(to_q_name_pair(attrs[i]), attrs[i+1]));
		}
	}
	
	static q_name_pair to_q_name_pair(const char *name) {
		const char *p = name;
		const char ns_sep = char(NS_SEP);
		while(*p && *p != ns_sep) p++;
		q_name_pair qn;
		if(*p == ns_sep) { 
			qn.first = std::string(name, int(p-name));
			qn.second = std::string(p+1);
		} else {
			qn.second = name;
		}
		return  qn;
	}
	
	XML_Parser m_expatParser;
	sax_content_handler *m_content_handler;
	sax_error_handler *m_error_handler;
	bool m_parsing;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_EXPAT_PARSER_H
