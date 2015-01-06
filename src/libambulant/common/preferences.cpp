// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#if defined(WITH_LTDL_PLUGINS) || defined(WITH_WINDOWS_PLUGINS)
#define WITH_PLUGINS 1
#endif

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
#if	defined(WITH_FFMPEG) || defined(WITH_PLUGINS)
	m_prefer_ffmpeg(true),
	m_strict_url_parsing(false),
	m_tabbed_links(false),
#else //WITH_FFMPEG
	m_prefer_ffmpeg(false),
#endif//WITH_FFMPEG
#ifdef	WITH_PLUGINS
	m_use_plugins(true),
#else //WITH_PLUGINS
	m_use_plugins(false),
#endif//WITH_PLUGINS
	m_plugin_path(""),
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

const std::string
preferences::repr() {
	std::string r = "";
	r += "parser: "+m_parser_id;
	r += ", validation_scheme: "+m_validation_scheme;
	r += ", do_namespaces: "; r += m_do_namespaces?"yes":"no";
	r += ", do_schema: "; r += m_do_schema?"yes":"no";
	r += ", validation_schema_full_checking: "; r += m_validation_schema_full_checking?"yes":"no";
	r += ", strict_url_parsing: "; r += m_strict_url_parsing?"yes":"no";
	r += ", m_tabbed_links: "; r += m_tabbed_links?"yes":"no";
	r += ", prefer_ffmpeg: "; r += m_prefer_ffmpeg?"yes":"no";
	r += ", use_plugins: "; r += m_use_plugins?"yes":"no";
	r += ", plugin_path: "; r += m_plugin_path;
	r += ", dynamic_content_control: "; r += m_dynamic_content_control?"yes":"no";
	r += ", fullscreen: "; r += m_fullscreen?"yes":"no";
	r += ", prefer_rtsp_tcp: "; r += m_prefer_rtsp_tcp?"yes":"no";
	
	return r;
}
