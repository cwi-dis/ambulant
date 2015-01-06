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

#ifndef AMBULANT_LIB_SAX_HANDLER_H
#define AMBULANT_LIB_SAX_HANDLER_H

#include "ambulant/config/config.h"

#include <string>

// attribute pair
#include <utility>

// return list of attrs
#include <list>

// std::runtime_error
#include <stdexcept>

#include "ambulant/lib/sax_types.h"

namespace ambulant {

namespace lib {

/// Interface for a consumer of SAX events.
/// An object with this interface is passed to a SAX XML parser,
/// and the parser then feeds this object with all information it
/// parses from the document.
class sax_content_handler {
  public:
	virtual ~sax_content_handler(){}

	/// Called at the beginning of the document.
	virtual void start_document() = 0;

	/// Called at the end of the document.
	virtual void end_document() = 0;

	/// Called when a new element starts, passing the tag name and attribute list.
	virtual void start_element(const q_name_pair& qn, const q_attributes_list& qattrs) = 0;

	/// Called when an element ends.
	virtual void end_element(const q_name_pair& qn) = 0;

	/// Called when an XML namespace declaration is encountered.
	virtual void start_prefix_mapping(const xml_string& prefix, const xml_string& uri) = 0;

	/// Called when an XML namespace declaration goes out of scope.
	virtual void end_prefix_mapping(const xml_string& prefix) = 0;

	/// Called when data is encountered.
	virtual void characters(const char *buf, size_t len) = 0;
};

/// Class that holds information on SAX parse errors.
/// The message is returned by the base class method
/// const char *what() const;
class sax_error : public std::runtime_error {
  public:
	/// Construct with given message and position information.
	sax_error(const std::string& message, int line, int column)
	:	std::runtime_error(message), m_line(line), m_column(column) {
	}


	/// Return the line at which the error was encountered.
	int get_line() const { return m_line;}
	/// Return the column at which the error was encountered.
	int get_column() const { return m_column;}

  private:
	int m_line;
	int m_column;
};

/// Interface for the SAX error handler.
/// An object with this interface is passed to a SAX XML parser,
/// and the parser calls it whenever errors are encountered in the document.
class sax_error_handler {
  public:
	virtual ~sax_error_handler(){}

	/// Called when an error occurs.
	virtual void error(const sax_error& error) = 0;
};

/// Interface for a SAX XML parser.
/// The XML parser should implement this interface.
class xml_parser {
  public:
	virtual ~xml_parser(){}

	/// Pass data to the parser. Pass final=true when this is the last
	/// data to be sent.
	virtual bool parse(const char *buf, size_t len, bool final) = 0;

	/// Install the SAX content handler.
	virtual void set_content_handler(sax_content_handler *h) = 0;

	/// Install the SAX error handler.
	virtual void set_error_handler(sax_error_handler *h) = 0;
};

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_SAX_HANDLER_H
