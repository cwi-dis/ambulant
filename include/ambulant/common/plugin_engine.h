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




#ifndef PLUGIN_FACTORY_H
#define PLUGIN_FACTORY_H


/* Disable plugins for Zaurus etc. */
#define	WITH_PLUGINS
//#undef  WITH_PLUGINS

#include "ambulant/common/factory.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/renderer.h"

 
namespace ambulant {

namespace net {
class datasource_factory;
};

namespace common {
class global_playable_factory;

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
    void add_plugins(common::factories *factory);
    
  private:
    
    plugin_engine();
    
    /// Determine directories to search for plugins.
    void collect_plugin_directories();
    
    /// Load all plugins from directory dirname.
    void load_plugins(std::string dirname);

	
	/// Pointer to the initialize function in the plugin.
	typedef void (*initfuncptr)(common::factories* factory);

	/// The list of directories to search for plugins.
  	std::vector< std::string > m_plugindirs;
  	
  	/// The list of initialize functions to call.
  	std::vector< initfuncptr > m_initfuncs;

    static plugin_engine *s_singleton;
};

}
} //end namespaces


#endif /* _PLUGIN_FACTORY_H */
