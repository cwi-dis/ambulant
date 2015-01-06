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

#ifndef AMBULANT_COMMON_PREFERENCES_H
#define AMBULANT_COMMON_PREFERENCES_H

#include <string>
#include "ambulant/config/config.h"

namespace ambulant {

namespace common {

const unsigned int default_layout_width = 640;
const unsigned int default_layout_height = 480;

//const char* parser_ids[] = {"any", "expat", "xerces"};
//const char* val_schemes[] = {"never", "always", "auto"};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

/// Class providing access to user preference settings.
/// This is a singleton class, but it doesn't follow the corresponing
/// design pattern to the letter. It has a method whereby a subclass
/// can set the singleton. Usually the subclass is an OS-dependent
/// implementation that is responsible for saving and loading preferences.
class AMBULANTAPI preferences {

  protected:
	preferences();

	virtual ~preferences();

	/// Install the singleton.
	static void set_preferences_singleton(preferences *prefs);

  public:
	/// True if this user has seen the Welcome.smil document.
	bool m_welcome_seen;

	/// Where to log.
	std::string m_log_file;

	/// Determines how much output the user will see.
	int m_log_level;

	/// Determines the preferred XML parser.
	std::string m_parser_id;

	/// "never"/"auto"/"always": should we do validation?
	std::string m_validation_scheme;

	/// For an XML parser that supports it, determines namespace support.
	bool m_do_namespaces;

	/// When set, prefer validation with Schema over DTD.
	bool m_do_schema;

	/// Also do checking on the schema itself
	bool m_validation_schema_full_checking;

	/// Use plugins or not
	bool m_use_plugins;

	/// Where to find the plugins
	std::string m_plugin_path;

	/// Prefer ffmpeg/live555/sdl video/video over quicktime or directshow
	bool m_prefer_ffmpeg;

	/// Do we do strict URL parsing (i.e. complain about illegal characters
	/// in URLs)?
	bool m_strict_url_parsing;

	/// Do we allow selection/activation of anchors with tab/newline?
	bool m_tabbed_links;

	/// Do we want to do dynamic content control (systemTests and customTests)?
	bool m_dynamic_content_control;

	/// Was the player in fullscreen mode when the last document was closed?
	bool m_fullscreen;

	/// Should we use RTSP-over-TCP (in stead of RTSP-over-UDP)?
	bool m_prefer_rtsp_tcp;

	/// Return the preferences singleton object.
	static preferences* get_preferences();

	/// Load preferences from OS preferred location.
	virtual bool load_preferences();

	/// Save preferences.
	virtual bool save_preferences();

	/// Return a string repr. of all preference settings.
	virtual const std::string repr();

  private:
	static preferences* s_preferences; // singleton

};// class preferences

#ifdef _MSC_VER
#pragma warning(pop)
#endif


} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_PREFERENCES_H
