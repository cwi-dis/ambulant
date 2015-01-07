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

#ifndef PLUGIN_FACTORY_H
#define PLUGIN_FACTORY_H

#include "ambulant/common/factory.h"
#include "ambulant/common/gui_player.h"

#define AMBULANT_PLUGIN_API_VERSION 15

namespace ambulant {

namespace common {

extern "C" {
	/// Pointer to the initialize function in the plugin. The initialize function
	/// should be a C function (not C++) and it should be called "initialize".
	typedef void (*initfuncptr)(int api_version, common::factories* factory, common::gui_player *player);

	/// Structure for extra information a plugin may want to expose to
	/// Ambulant or Ambulant extensions. If available it must be called
	/// "plugin_extra_data".
	struct plugin_extra_data {
		char *m_plugin_name;	///< Plugin name.
		void *m_plugin_extra;	///< Pointer to extra, plugin-dependent, data.
	};    
};
#ifdef WITH_STATIC_PLUGINS
void register_static_plugin(initfuncptr ifp);
#endif

/// Plugin loader.
/// This class, of which a singleton is instantiated, collects all plugins and
/// loads them. Subsequently, when a new player is created, it calls the init
/// routines of the plugins to all them to register themselves with the correct
/// global factories.
class AMBULANTAPI plugin_engine {
  public:

	~plugin_engine();

	/// Return the singleton plugin_engine object.
	static plugin_engine *get_plugin_engine();

	/// Add plugins to the given global factories.
	void add_plugins(common::factories *factory, common::gui_player *player = 0);

	/// Get extra-data for a named plugin, if available.
	void *get_extra_data(std::string name);

	/// Get list of plugins that need the Python plugin.
	const std::vector<std::string>& get_python_plugins() const;
  private:

	plugin_engine();

	/// Determine directories to search for plugins.
	void collect_plugin_directories();

	/// Load all plugins from directory dirname.
	void load_plugins(std::string dirname);

	/// Load a single plugin
	void load_plugin(const char *filename);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

	/// The list of directories to search for plugins.
	std::vector< std::string > m_plugindirs;

	/// The list of initialize functions to call.
	std::vector< initfuncptr > m_initfuncs;

	/// All available extra data.
	std::map< std::string, plugin_extra_data* > m_extra_data;
	static plugin_engine *s_singleton;

#ifdef WITH_PYTHON_PLUGIN
	/// The pathname of the engine for running the Python plugins
	std::string m_python_plugin_engine;

	/// The pathnames of the plugins written in Python
	std::vector<std::string> m_python_plugins;

#endif // WITH_PYTHON_PLUGIN

#ifdef _MSC_VER
#pragma warning(pop)
#endif

};

}
} //end namespaces


#endif /* _PLUGIN_FACTORY_H */
