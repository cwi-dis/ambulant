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

#include "ambulant/lib/logger.h"
#include "unix_preferences.h"
 
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

using namespace lib;

bool
unix_preferences::load_preferences() {
	AM_DBG logger::get_logger()->debug("unix_preferences::load_preferences()");
	set_preferences_singleton(this);

	char *parser = getenv("AMBULANT_USE_PARSER");
	if (parser) {
		if (strcmp(parser, "expat") == 0)
			m_parser_id = EXPAT;
		else if (strcmp(parser, "xerces") == 0)
			m_parser_id = XERCES;
		else
			lib::logger::get_logger()->error("Unknown parser in environment: AMBULANT_USE_PARSER=%s", parser);
	}
	char *validation_scheme = getenv("AMBULANT_VALIDATION_SCHEME");
	if (validation_scheme != NULL) {
		if (strcasecmp(validation_scheme,"always") == 0) {
			m_validation_scheme = ALWAYS;
		} else if (strcasecmp(validation_scheme,"auto") == 0) {
			m_validation_scheme = AUTO;
		} else if (strcasecmp(validation_scheme,"never") == 0) {
			m_validation_scheme = NEVER;
		}
	}
	char *do_namespaces = getenv("AMBULANT_DO_NAMESPACES");
	if (do_namespaces != NULL 
	    && strcasecmp(do_namespaces,"false") != 0)
		m_do_namespaces = true;
	char *do_schema = getenv("AMBULANT_DO_SCHEMA");
	if (do_schema != NULL && strcasecmp(do_schema,"false") != 0)
		m_do_schema = true;
	char *do_validation = getenv("AMBULANT_DO_VALIDATION");
	if (do_validation != NULL
	    && strcasecmp(do_validation,"false") != 0)
		m_do_validation = true;
	char *validation_schema_full_checking =
		getenv("AMBULANT_VALIDATION_SCHEMA_FULL_CHECKING");
	if (validation_schema_full_checking != NULL
	    && strcasecmp(validation_schema_full_checking,"false") != 0)
		m_validation_schema_full_checking = true;
	return true;
}

bool
unix_preferences::save_preferences() {
	return true;
}
 
