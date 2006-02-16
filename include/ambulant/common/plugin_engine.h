/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

#define AMBULANT_PLUGIN_API_VERSION 2
 
namespace ambulant {

namespace common {

/// Pointer to the initialize function in the plugin.
extern "C" {
	typedef void (*initfuncptr)(int api_version, common::factories* factory, common::gui_player *player);
};

/// Plugin loader.
/// This class, of which a singleton is instantiated, collects all plugins and
/// loads them. Subsequently, when a new player is created, it calls the init
/// routines of the plugins to all them to register themselves with the correct
/// global factories.
class plugin_engine {
  public:

  	/// Return the singleton plugin_engine object.
    static plugin_engine *get_plugin_engine();
    
    /// Add plugins to the given global factories.
    void add_plugins(common::factories *factory, common::gui_player *player = 0);
    
  private:
    
    plugin_engine();
    
    /// Determine directories to search for plugins.
    void collect_plugin_directories();
    
    /// Load all plugins from directory dirname.
    void load_plugins(std::string dirname);

	/// The list of directories to search for plugins.
  	std::vector< std::string > m_plugindirs;
  	
  	/// The list of initialize functions to call.
  	std::vector< initfuncptr > m_initfuncs;

    static plugin_engine *s_singleton;
};

}
} //end namespaces


#endif /* _PLUGIN_FACTORY_H */
