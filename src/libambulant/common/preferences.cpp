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
#include <stdlib.h>
#include "ambulant/lib/logger.h"
#include "ambulant/common/preferences.h"

#ifdef	WITH_XERCES
#include <xercesc/parsers/SAXParser.hpp>
#endif/*WITH_XERCES*/

using namespace ambulant;

using namespace common;

preferences::preferences()
#ifdef	WITH_XERCES
  :	m_parser_id(ANY),
	m_validation_scheme(AUTO),
	m_do_namespaces(false),
	m_do_schema(false),
	m_do_validation(false),
	m_validation_schema_full_checking(false)
#else /*WITH_XERCES*/
 :	m_parser_id(ANY)
#endif/*WITH_XERCES*/
{
	char *parser = getenv("AMBULANT_USE_PARSER");
	if (parser) {
		if (strcmp(parser, "expat") == 0)
			m_parser_id = EXPAT;
		else if (strcmp(parser, "xerces") == 0)
			m_parser_id = XERCES;
		else
			lib::logger::get_logger()->error("Unknown parser in environment: AMBULANT_USE_PARSER=%s", parser);
	}
#ifdef	WITH_XERCES
	char *validation_scheme = getenv("AMBULANT_VALIDATION_SCHEME");
	if (validation_scheme != NULL) {
		if (strcmp(validation_scheme,"always") == 0
		    || strcmp(validation_scheme,"ALWAYS") == 0) {
			m_validation_scheme = ALWAYS;
		} else if (strcmp(validation_scheme,"auto") == 0
			   || strcmp(validation_scheme,"AUTO") == 0) {
			m_validation_scheme = AUTO;
		} else if (strcmp(validation_scheme,"never") == 0
			   || strcmp(validation_scheme,"NEVER") == 0) {
			m_validation_scheme = NEVER;
		}
	}
	char *do_namespaces = getenv("AMBULANT_DO_NAMESPACES");
	if (do_namespaces != NULL && strcmp(do_namespaces,"false") != 0)
		m_do_namespaces = true;
	char *do_schema = getenv("AMBULANT_DO_SCHEMA");
	if (do_schema != NULL && strcmp(do_schema,"false") != 0)
		m_do_schema = true;
	char *do_validation = getenv("AMBULANT_DO_VALIDATION");
	if (do_validation != NULL && strcmp(do_validation,"false") != 0)
		m_do_validation = true;
	char *validation_schema_full_checking = getenv("AMBULANT_VALIDATION_SCHEMA_FULL_CHECKING");
	if (validation_schema_full_checking != NULL && strcmp(validation_schema_full_checking,"false") != 0)
		m_validation_schema_full_checking = true;
#endif/*WITH_XERCES*/
}
	
preferences::~preferences()
	{}

preferences*  ambulant::common::preferences::s_preferences = 0;

preferences* 
ambulant::common::preferences::get_preferences() {
	if (s_preferences == 0) {
		s_preferences =  new preferences;
	}
	return s_preferences;
}

preferences::parser_id
preferences::get_parser_id() {
	return m_parser_id;
}

bool 
preferences::load_preferences() {
	return false;
}

bool 
preferences::save_preferences() {
	return false;
}
