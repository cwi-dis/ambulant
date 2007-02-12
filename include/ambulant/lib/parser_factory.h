/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

 
#ifndef AMBULANT_COMMON_PARSER_FACTORY_H
#define AMBULANT_COMMON_PARSER_FACTORY_H

#include <vector>
#include <string>
#include "ambulant/lib/sax_handler.h"
#include "ambulant/lib/logger.h"

namespace ambulant {

namespace lib {

/// Interface to create XML parsers.
class parser_factory {
  public:
	  
	virtual ~parser_factory() {};
	/// Create an XML parser that will feed the given content and error handlers.
	virtual xml_parser* new_parser(
		sax_content_handler* content_handler,
		sax_error_handler* error_handler) = 0;
	virtual std::string get_parser_name() { return "none"; };
};

/// Implementation of parser_factory plus provider interface.
class global_parser_factory : public parser_factory {
  public:
    /// Returns (singleton?) global_parser_factory object.
  	static global_parser_factory* get_parser_factory();
    ~global_parser_factory();
    
    /// Provider interface: add new parser factory implementation.
    void add_factory(parser_factory *pf);
   
    xml_parser* new_parser(
		sax_content_handler* content_handler,
		sax_error_handler* error_handler);

  private:
	global_parser_factory();
    std::vector<parser_factory *> m_factories;
	bool m_warned;
    parser_factory *m_default_factory;
  	static global_parser_factory* s_singleton;
};

}
} // end namespaces

#endif
