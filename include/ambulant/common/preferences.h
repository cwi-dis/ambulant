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
	
	// Use plugins or not
	bool m_use_plugins;
	
	// Where to find the plugins 
	std::string m_plugin_dir;

	/// Return the preferences singleton object.
	static preferences* get_preferences();

	/// Load preferences from OS preferred location.
	virtual bool load_preferences();

	/// Save preferences.
	virtual bool save_preferences();
	

  private:
	static preferences* s_preferences; // singleton

};// class preferences

} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_PREFERENCES_H
