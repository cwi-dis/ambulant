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

#ifndef __UNIX_PREFERENCES_H__
#define __UNIX_PREFERENCES_H__
#include "ambulant/common/preferences.h"
#include <vector>
#include <stdlib.h>
#include <stdio.h>

static std::string m_home = "";
static std::string m_ambulant_home = ".ambulant";
static std::string m_preferences_filename = "preferences";
static int m_bufsize = 1024;
enum datatype {STRING = 0, INT, BOOL};

class preference_entry {
  public:
	preference_entry(std::string name,
		datatype type,
		void* store,
		std::string* = NULL);
	std::string pref_name;
	std::string* pref_valid_val;
	datatype pref_type;
	void* pref_store;
};

class unix_preferences : public ambulant::common::preferences {

  public:
	unix_preferences();

	~unix_preferences();

	bool load_preferences();

	bool save_preferences();

  private:
	FILE* open_preferences_file(std::string mode);

	bool load_preference(std::string name, std::string value);

	bool load_preferences_from_environment();

	bool load_preferences_from_file();

	void get_preference(char* buf, char**namep, char** valuep);

	std::vector<preference_entry*>* s_preference_table;

	typedef std::vector<preference_entry*>::const_iterator
		preference_iterator;
};

#endif/*__UNIX_PREFERENCES_H__*/
