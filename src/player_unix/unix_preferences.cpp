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
#include <fcntl.h>
 
// #define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

using namespace lib;
/*
preference_entry preference_table = {
  {"AMBULANT_USE_PARSER",
   { "any", "xerces", "expat" }, "any", STRING, &m_parser_id },
  {"AMBULANT_DO_VALIDATION",
   { "false", "true" }, "false", BOOL, & m_do_validation },
  NULL
};
*/
preference_entry::preference_entry(std::string name,
				   datatype type,
				   void* store,
				   std::string* valid_val) {
	pref_name  = name;
	pref_type  = type;
	pref_store = store;
	pref_valid_val = valid_val;
}

// valid values for preferences
// string arrays are terminated by the empty string
static std::string valid_parsers[] = {"any", "expat", "xerces", ""};
static std::string valid_schemes[] = {"never", "always", "auto",""};
static std::string valid_bools[] = {"false", "true", ""};

// ADD_PREF - add a new preference to the static preference table
#define ADD_PREF(X...) s_preference_table->push_back(new preference_entry(X))

unix_preferences::unix_preferences() {
	set_preferences_singleton(this);
	std::string id = "unix_preferences::unix_preferences";
	AM_DBG logger::get_logger()->debug("%s", id.c_str());
	s_preference_table = new std::vector<preference_entry*>();

	ADD_PREF("AMBULANT_WELCOME_SEEN", BOOL,
	     &m_welcome_seen, valid_bools);
	ADD_PREF("AMBULANT_LOG_LEVEL", INT, 
	     &m_log_level, NULL);
	ADD_PREF("AMBULANT_USE_PARSER", STRING, 
	     &m_parser_id, valid_parsers);
	ADD_PREF("AMBULANT_VALIDATION_SCHEME", STRING,
	     &m_validation_scheme, valid_schemes);
	ADD_PREF("AMBULANT_DO_VALIDATION", BOOL,
	     &m_do_validation, valid_bools);
	ADD_PREF("AMBULANT_DO_NAMESPACES", BOOL,
	     &m_do_namespaces, valid_bools);
	ADD_PREF("AMBULANT_DO_SCHEMA", BOOL,
	     &m_do_schema, valid_bools);
	ADD_PREF("AMBULANT_DO_VALIDATION_SCHEMA_FULL_CHECKING", BOOL, 
	     &m_validation_schema_full_checking, valid_bools);
}

unix_preferences::~unix_preferences() {
	for (preference_iterator pritr = s_preference_table->begin();
	     pritr != s_preference_table->end(); pritr++) {
		delete *pritr;
	}
	delete s_preference_table;
	s_preference_table = NULL;
}


bool
unix_preferences::load_preferences() {
	std::string id = "unix_preferences::load_preferences";
	AM_DBG logger::get_logger()->debug("%s", id.c_str());

	return load_preferences_from_file ()
	       || load_preferences_from_environment();
}

bool
unix_preferences::load_preference(std::string name, std::string value) {
	lib::logger* log =lib::logger::get_logger();
	std::string id = "unix_preferences::load_preference";
	bool found = false;
	AM_DBG log->debug("%s(%s=%s)", id.c_str(), 
			  name.c_str(), value.c_str());
	
	if (name == "")
		return false;
	for (preference_iterator pritr = s_preference_table->begin();
	     pritr != s_preference_table->end(); pritr++) {
		preference_entry* pe = *pritr;
		AM_DBG logger::get_logger()->debug("%s pe=%s", 
						   id.c_str(), 
						   pe->pref_name.c_str());
		if (name == pe->pref_name) {
			found = true;
			// optionally check value given
			if (pe->pref_valid_val != NULL) {
				std::string* vv = pe->pref_valid_val;
				while ( ! (*vv == "")) {
					if (*vv == value)
						break;
					vv++;
				}
				if (*vv == "") {
					log->error("%s %s=%s",
						   "Invalid value for",
						   pe->pref_name.c_str(),
						   value.c_str());
					log->error("Valid values are:");
					vv = pe->pref_valid_val;
					while ( ! vv->empty()) {
						log->error("%s",
							   vv->c_str());
						vv++;
					}
					break;
				}
			}
			switch (pe->pref_type) {
			case STRING:
				*(std::string*)pe->pref_store = value;
				break;
			case INT:
				*(int*)pe->pref_store = atoi(value.c_str());
				break;
			case BOOL:
				*(bool*)pe->pref_store =
					value == "true" ? true : false;
				break;
			default:
				break;
			}
			break; 
		}
	}
	if ( ! found ) {
		log->error("Invalid preference name %s.", name.c_str());
		log->error("Valid preference names  are:");
		for (preference_iterator pritr = s_preference_table->begin();
		     pritr != s_preference_table->end(); pritr++) {
			preference_entry* pe = *pritr;
			log->error("%s", pe->pref_name.c_str());
		}
	}
	return true;
}

bool
unix_preferences::load_preferences_from_environment() {
	// return true when no error
	std::string id = "unix_preferences::load_preferences_from_environment";
	AM_DBG logger::get_logger()->debug("%s()", id.c_str());
	bool rv = true;
	for (preference_iterator pritr = s_preference_table->begin();
	     pritr != s_preference_table->end(); pritr++) {
		preference_entry* pe = *pritr;
		char* v = getenv(pe->pref_name.c_str());
		if (v != NULL) {
			std::string value(v);
			rv &= load_preference(pe->pref_name, value);
		} else  {
			rv = false;
		}
	}
	return rv;
}

bool
unix_preferences::load_preferences_from_file() {
	// return true when no error
	std::string id = "unix_preferences::load_preferences_from_file";
	AM_DBG logger::get_logger()->debug("%s()", id.c_str());
	FILE* preferences_filep = open_preferences_file("r");
	char buf[m_bufsize], * s = buf;
	int lineno = 0;
	bool rv = true;
	
	if (preferences_filep == NULL)
		return false;
	
	while (s = fgets(buf, m_bufsize, preferences_filep)) {
		lineno++;
		if (*s == '#')
			continue;
		char* name; char* value;
		get_preference(s, &name, &value);
		if (name == NULL) {
			logger::get_logger()->error("%s(%s): %s %d - %s",
						    id.c_str(),
						    m_preferences_filename.c_str(),
						    "line", lineno,
						    "malformed");
			return false;
		}
		std::string s_name  = name;
		std::string s_value = value;
		rv &= load_preference(s_name, s_value);
	}
	return rv;
}
	
bool
unix_preferences::save_preferences() {
	std::string id = "unix_preferences::save_preferences";
	AM_DBG logger::get_logger()->debug("%s()", id.c_str());
	FILE* preferences_filep = open_preferences_file("w");
	
	if (preferences_filep == NULL) {
		return false;
	}

// OI - output int value
#define OI(N,I) (void) fprintf(preferences_filep,"%s=%d\n",N,I);
// OS - output atring value
#define OS(N,S) (void) fprintf(preferences_filep,"%s=%s\n",N,S);
	OS("# Ambulant Preferences", "Temporary format. DO NOT EDIT this file !");
	for (preference_iterator pritr = s_preference_table->begin();
	     pritr != s_preference_table->end(); pritr++) {
		preference_entry* pe = *pritr;
		switch(pe->pref_type) {
		case BOOL:
			OS(pe->pref_name.c_str(),
			    *(bool*)pe->pref_store == false ?
			    "false" : "true");
			break;
		case INT:
			OI(pe->pref_name.c_str(), *(int*)pe->pref_store);
			break;
		case STRING:
			OS(pe->pref_name.c_str(),
			   ((std::string*)pe->pref_store)->c_str());
			break;
		default:
			break;
		}
	}
	return true;
}
 
FILE* 
unix_preferences::open_preferences_file(std::string mode) {
	std::string id = "unix_preferences::open_preferences_file";
	if (m_home == "") {
		char* home_dir = getenv("HOME");
		if (home_dir != NULL) {
			m_home = std::string(home_dir);
			m_ambulant_home = m_home+"/"+m_ambulant_home;
			m_preferences_filename = 
			  m_ambulant_home+"/"+m_preferences_filename;
		} else {
			logger::get_logger()->error
			 	("HOME environment variable not set");
			return NULL;
		}
	}
	AM_DBG logger::get_logger()->debug("%s(%s,%s)",
					   id.c_str(),
					   m_preferences_filename.c_str(),
					   mode.c_str());
	FILE* preferences_filep = fopen(m_preferences_filename.c_str(),
					mode.c_str());
	if (preferences_filep == NULL) {
		struct stat statbuf;
		// create directory "$HOME/.ambulant" unless it exists
		if (stat(m_ambulant_home.c_str(), &statbuf) < 0
		    && mkdir(m_ambulant_home.c_str(), 0755) < 0) {
			logger::get_logger()->error
				("mkdir(\"%s\") failed: %s",
				 m_ambulant_home.c_str(),
				 strerror(errno));
			return NULL;
		}
		preferences_filep = fopen(m_preferences_filename.c_str(),
					  mode.c_str());
		if (preferences_filep == NULL) {
			logger::get_logger()->error
				("fopen(\"%s\",\"%s\") failed: %s",
				 m_preferences_filename.c_str(),
				 mode.c_str(),
				 strerror(errno));
			return NULL;
		}
	}
	return preferences_filep;
}

void 
unix_preferences::get_preference(char* buf, char**namep, char** valuep) {
	char* s = buf;
	while (*s && *s++ != '=');
	if (!*s)
		*namep = *valuep = NULL;
	else {
		*(s-1) = '\0';
		*namep = buf;
		*valuep = s;
	}
	while (strlen(s) && isspace(s[strlen(s)-1]))
	        s[strlen(s)-1] = '\0';
}
