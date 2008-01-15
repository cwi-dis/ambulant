// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include <stdlib.h>
#include "ambulant/lib/logger.h"
#include "ambulant/common/preferences.h"


//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

using namespace common;

preferences::preferences()
  :	m_welcome_seen(false),
	m_log_file(""),
#ifdef _DEBUG
	m_log_level(lib::logger::LEVEL_DEBUG),
#else
	m_log_level(lib::logger::LEVEL_SHOW),
#endif
	m_parser_id("any"),
	m_validation_scheme("auto"),
	m_do_namespaces(false),
	m_do_schema(false),
	m_validation_schema_full_checking(false),
	m_use_plugins(true),
	m_plugin_dir(""),
	m_prefer_ffmpeg(false),
	m_dynamic_content_control(false),
	m_fullscreen(false),
	m_prefer_rtsp_tcp(false)
{
	AM_DBG lib::logger::get_logger()->debug("preferences::preferences()");
	load_preferences();
}
	
preferences::~preferences()
	{}

preferences*  ambulant::common::preferences::s_preferences = 0;

void
ambulant::common::preferences::set_preferences_singleton(preferences *prefs) {
	if (s_preferences != 0) {
		ambulant::lib::logger::get_logger()->debug("Programmer error: preferences singleton already set");
		return;
	}
	s_preferences = prefs;
}

preferences* 
ambulant::common::preferences::get_preferences() {
	if (s_preferences == 0) {
		s_preferences =  new preferences;
	}
	return s_preferences;
}

bool 
preferences::load_preferences() {
	return false;
}

bool 
preferences::save_preferences() {
	return false;
}
