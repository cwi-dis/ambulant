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

#include "ambulant/lib/xerces_parser.h"
#include "ambulant/common/factory.h"
#include "ambulant/lib/sax_handler.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

#ifdef WITH_XERCES

class xerces_plugin_factory : public lib::parser_factory {
  public:

	xerces_plugin_factory(common::factories* factory)
	:   m_factory(factory) {}
	~xerces_plugin_factory() {};
	
	lib::xml_parser* new_parser(
		lib::sax_content_handler* content_handler,
		lib::sax_error_handler* error_handler);
		
	std::string get_parser_name();
	
  private:
	common::factories* m_factory;
	
};


lib::xml_parser*
xerces_plugin_factory::new_parser(
		lib::sax_content_handler* content_handler, 
		lib::sax_error_handler* error_handler) 
{
	AM_DBG lib::logger::get_logger()->debug("xerces_factory::new_parser(): xerces parser returned");
	//return NULL;
	return new lib::xerces_sax_parser(content_handler, error_handler);	
}


std::string 
xerces_plugin_factory::get_parser_name()
{
	AM_DBG lib::logger::get_logger()->debug("xerces_factory::get_parser_name(): xerces parser");
	return "xerces";
}



extern "C" void initialize(ambulant::common::factories* factory)
{	
	AM_DBG lib::logger::get_logger()->debug("xerces_plugin::initialize registering factory function");
	if (factory->pf) {
		factory->pf->add_factory(new xerces_plugin_factory(factory));
		lib::logger::get_logger()->trace("xerces_plugin: registered");
	}
}

#endif
