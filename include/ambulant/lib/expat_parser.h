
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

#include "ambulant/lib/sax_handler.h"

#include "expat.h"

// To execute samples depending on libexpat.dll assert 
// that the dll is reachable at runtime 
// (e.g. its in the path).


namespace ambulant {

namespace lib {

///////////////////////////
// Adapter for expat parser

class expat_parser {
  public:
	enum {NS_SEP = '|'};
	
	expat_parser(sax_content_handler *content_handler, sax_error_handler *error_handler);
	virtual ~expat_parser();

	// parses a chunk of data using expat parser
	// throws a sax_error if the error handler is null
	virtual bool parse(const char *buf, size_t len, bool final);
	
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
