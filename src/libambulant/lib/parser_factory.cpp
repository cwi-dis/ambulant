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
 
#ifndef AM_DBG
#define AM_DBG if(0)
#endif



#include "ambulant/lib/parser_factory.h"
#include "ambulant/lib/expat_parser.h"  
#include "ambulant/common/preferences.h"

 using namespace ambulant;
 using namespace lib;

global_parser_factory* ambulant::lib::global_parser_factory::s_singleton = NULL;

 
 
global_parser_factory* 
global_parser_factory::get_parser_factory()
{
	AM_DBG lib::logger::get_logger()->debug("global_parser_factory::get_parser_factory() called");

	if (s_singleton == NULL) {
        s_singleton = new global_parser_factory();
		AM_DBG lib::logger::get_logger()->debug("global_parser_factory::get_parser_factory() returning 0x%x", (void*) s_singleton);
	}
    return s_singleton;
}




global_parser_factory::global_parser_factory()
:	m_warned(false),
	m_default_factory(NULL)
{
	
	m_default_factory = new lib::expat_factory();
}

global_parser_factory::~global_parser_factory()
{
    // XXXX Should I delete the factories in m_factories? I think
    // so, but I'm not sure...
    delete m_default_factory;
	m_default_factory = NULL;
}
    
void
global_parser_factory::add_factory(parser_factory *pf)
{
	AM_DBG lib::logger::get_logger()->debug("global_parser_factory::add_factory(0x%x) called", (void*) pf);
    m_factories.push_back(pf);
}
    
xml_parser*
global_parser_factory::new_parser(
	sax_content_handler* content_handler,
	sax_error_handler* error_handler)
{
	std::string& parser_id = common::preferences::get_preferences()->m_parser_id;
	AM_DBG lib::logger::get_logger()->debug("global_parser_factory::new_parser() called (pref = %s)",parser_id.c_str());
    xml_parser *pv;

	if ( parser_id == "any" && m_default_factory) {

   		pv = m_default_factory->new_parser(content_handler, error_handler);
		if (pv) {
			AM_DBG lib::logger::get_logger()->debug("global_parser_factory::new_parser(\"any\") returning parser (0x%x)", (void*) pv);
			return pv;
		}
	}
    std::vector<parser_factory*>::iterator i;
	pv = NULL;
    for(i=m_factories.begin(); i != m_factories.end(); i++) {
		if (( (*i)->get_parser_name() == parser_id ) || ( parser_id == "any" )) {
			AM_DBG lib::logger::get_logger()->debug("global_parser_factory::new_parser() trying parser %s", parser_id.c_str());
        	pv = (*i)->new_parser(content_handler, error_handler);
			
			if (pv){
				AM_DBG lib::logger::get_logger()->debug("global_parser_factory::new_parser() returning parser (0x%x)", (void*) pv);
				return pv;
			}
		} else {
			pv = NULL;
		}
      
    }
	if (m_default_factory) {
		if (!m_warned && parser_id != m_default_factory->get_parser_name()) {
			m_warned = true;
			lib::logger::get_logger()->warn(gettext("Parser \"%s\" not available, using \"%s\""),
				parser_id.c_str(), m_default_factory->get_parser_name().c_str());
		}
   		return m_default_factory->new_parser(content_handler, error_handler);
	} else {
		return NULL;
	}
}
