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

class xml_parser {
  public:
	virtual ~xml_parser(){}
	virtual bool parse(const char *buf, size_t len, bool final) = 0;
	virtual void set_content_handler(sax_content_handler *h) = 0;
	virtual void set_error_handler(sax_error_handler *h) = 0;	
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_SAX_HANDLER_H
