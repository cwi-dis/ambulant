
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_SAX_HANDLER_H
#define AMBULANT_LIB_SAX_HANDLER_H

#include <string>

// attribute pair
#include <utility>

// return list of attrs
#include <list>

// std::runtime_error
#include <stdexcept>

#include "sax_types.h"

namespace ambulant {

namespace lib {

class sax_content_handler {
  public:
	virtual ~sax_content_handler(){}
	virtual void start_document() = 0;
	virtual void end_document() = 0;
	virtual void start_element(const q_name_pair& qn, const q_attributes_list& qattrs) = 0;
	virtual void end_element(const q_name_pair& qn) = 0;
	virtual void start_prefix_mapping(const xml_string& prefix, const xml_string& uri) = 0;
	virtual void end_prefix_mapping(const xml_string& prefix) = 0;
	virtual void characters(const char *buf, size_t len) = 0;
};

class sax_error : public std::runtime_error {
  public:
	sax_error(const std::string& message, int line, int column)
	:	std::runtime_error(message), m_line(line), m_column(column) {
	}
		
	// The message is returned by the base function
	// const char *what() const;
	int get_line() const { return m_line;}
	int get_column() const { return m_column;}

  private:
	int m_line;
	int m_column;
};

class sax_error_handler {
  public:
	virtual ~sax_error_handler(){}
	virtual void error(const sax_error& error) = 0;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_SAX_HANDLER_H
