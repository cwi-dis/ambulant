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


#include"ambulant/common/plugin_engine.h"
#include "ambulant/lib/logger.h"
#include "ambulant/common/preferences.h"

//#include<dlfcn.h>
#include<stdlib.h>
#include<string.h>

#if defined(WITH_LTDL_PLUGINS) || defined(WITH_WINDOWS_PLUGINS)
#define WITH_PLUGINS 1
#endif

#ifdef WITH_LTDL_PLUGINS
#include<dirent.h>
#include <ltdl.h>

#ifdef AMBULANT_PLATFORM_MACOS
#include <CoreFoundation/CoreFoundation.h>
#define LIBRARY_PATH_ENVVAR "DYLD_LIBRARY_PATH"
#else
#define LIBRARY_PATH_ENVVAR "LD_LIBRARY_PATH"
#endif // AMBULANT_PLATFORM_MACOS

#endif // WITH_LTDL_PLUGINS

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#define PLUGIN_PREFIX "libamplugin_"


using namespace ambulant;
using namespace common;

plugin_engine *ambulant::common::plugin_engine::s_singleton = NULL;

plugin_engine *
plugin_engine::get_plugin_engine()
{
    if (s_singleton == NULL)
        s_singleton = new plugin_engine;
    return s_singleton;
}

plugin_engine::plugin_engine()
{
bool use_plugins = common::preferences::get_preferences()->m_use_plugins;

#ifdef WITH_PLUGINS
    collect_plugin_directories();
#ifdef WITH_LTDL_PLUGINS
	lib::logger::get_logger()->trace("plugin_engine: using LTDL plugin loader");
	int errors = lt_dlinit();
	if (errors) {
		lib::logger::get_logger()->error("LTDL plugin loader: Cannot initialize: %d error(s)", errors);
	    return;
	}
#endif
#ifdef WITH_WINDOWS_PLUGINS
	lib::logger::get_logger()->trace("plugin_engine: using LTDL plugin loader");
#endif
	if (use_plugins) {
		std::vector< std::string >::iterator i;
    	for (i=m_plugindirs.begin(); i!=m_plugindirs.end(); i++) {
        	load_plugins(*i);
    	}
	}
#else
	lib::logger::get_logger()->trace("plugin_engine: no plugin loader configured");
#endif
}

void
plugin_engine::collect_plugin_directories()
{
	// First dir to search is set per user preferences
	std::string& plugin_dir = common::preferences::get_preferences()->m_plugin_dir;
	if(plugin_dir != "")
		m_plugindirs.push_back(plugin_dir);
	
	// XXXX Need to add per-user plugin dir!
	
#ifdef AMBULANT_PLATFORM_MACOS
	// On MacOSX add the bundle's plugin dir
	CFBundleRef main_bundle = CFBundleGetMainBundle();
	if (main_bundle) {
		CFURLRef plugin_url = CFBundleCopyBuiltInPlugInsURL(main_bundle);
		char plugin_pathname[1024];
		if (plugin_url &&
				CFURLGetFileSystemRepresentation(plugin_url, true, (UInt8 *)plugin_pathname, sizeof(plugin_pathname))) {
			m_plugindirs.push_back(plugin_pathname);
		}
		if (plugin_url) CFRelease(plugin_url);
	}
#elif defined(AMBULANT_PLATFORM_UNIX)
	// On other unix platforms add the pkglibdir
	// XXXX Need to parameterize this!
	m_plugindirs.push_back("/usr/local/lib/ambulant");
#endif
#ifdef AMBULANT_PLATFORM_WIN32
	// XXXX Need to add application directory
#endif
}

#ifdef WITH_LTDL_PLUGINS
static int filter(const struct dirent* filen)
{
	int len;
	len = strlen(filen->d_name);
	if (!strncmp(filen->d_name+(len-3),".la",3) ) {
		return 1;
	} else {
		return 0;
	}
	return 0;
}

void
plugin_engine::load_plugins(std::string dirname)
{
	lib::logger::get_logger()->trace("plugin_engine: Scanning plugin directory: %s", dirname.c_str());
	char filename[1024];
	dirent **namelist;
	bool ldpath_added = false;
	
    int nr_of_files = scandir(dirname.c_str(), &namelist, &filter , NULL);
    if (nr_of_files < 0) {
        lib::logger::get_logger()->trace("Error reading plugin directory: %s: %s", dirname.c_str(), strerror(errno));
        return;
    } else {
        while (nr_of_files--) {
            //only normal files, not dots (. and ..)
            if (strcmp(namelist[nr_of_files]->d_name, ".")  &&
                    strcmp(namelist[nr_of_files]->d_name, "..")) {
                char *pluginname = namelist[nr_of_files]->d_name;
                
                // Check the name is valid
                if (strncmp(PLUGIN_PREFIX, pluginname, sizeof(PLUGIN_PREFIX)-1) != 0) {
                    lib::logger::get_logger()->trace("plugin_engine: skipping %s", pluginname);
                    continue;
                }
                
                // Construct the full pathname
                strncpy(filename, dirname.c_str(), sizeof(filename));
                strncat(filename, "/", sizeof(filename));
                strncat(filename, pluginname, sizeof(filename));
				
				// Add the plugin dir to the search path
				if (!ldpath_added) {
					setenv(LIBRARY_PATH_ENVVAR, dirname.c_str(), 1);
					ldpath_added = true;
				}

                // Load the plugin
                lib::logger::get_logger()->trace("plugin_engine: loading %s", pluginname);
				system("printenv");
 	            lt_dlhandle handle = lt_dlopen(filename);
                if (handle) {
                    AM_DBG lib::logger::get_logger()->debug("plugin_engine: reading plugin SUCCES [ %s ]",filename);
                    AM_DBG lib::logger::get_logger()->debug("Registering test plugin's factory");
                    initfuncptr init = (initfuncptr) lt_dlsym(handle,"initialize");
                    if (!init) {
                        lib::logger::get_logger()->error("plugin_engine: no initialize routine");
                    } else {
                        m_initfuncs.push_back(init);
                    }
                } else {
                    lib::logger::get_logger()->error("plugin_engine: Error reading plugin %s",filename);
                    lib::logger::get_logger()->error("plugin_engine: Reading plugin failed because : %s\n\n", lt_dlerror());
                }
            }
            free(namelist[nr_of_files]);
        }
        free(namelist);
    }
	lib::logger::get_logger()->trace("plugin_engine: Done with plugin directory: %s", dirname.c_str());
	if (ldpath_added)
		unsetenv(LIBRARY_PATH_ENVVAR);
}

#elif WITH_WINDOWS_PLUGINS
void
plugin_engine::load_plugins(std::string dirname)
{
	lib::logger::get_logger()->trace("plugin_engine: Scanning plugin directory: %s", dirname.c_str());
	char filepattern[1024];

	strncpy(filepattern, dirname.c_str(), sizeof(filepattern));
	strncpy(filepattern, "\\", sizeof(filepattern));
	strncpy(filepattern, PLUGIN_PREFIX, sizeof(filepattern));
	strncpy(filepattern, "_*.dll", sizeof(filepattern));

	WIN32_FIND_DATA dirData;
	HANDLE *dirHandle = FindFirstFile(filepattern, *dirData);
    if (dirHandle == NULL) {
        lib::logger::get_logger()->error("Error reading plugin directory: %s", filepattern);
        return;
    } else {
		do {
            // Construct the full pathname
		   	char filename[1024];
			strncpy(filename, dirname.c_str(), sizeof(filename));
 			strncpy(filename, "\\", sizeof(filename));
            strncat(filename, dirData.cFileName, sizeof(filename));

            // Load the plugin
            lib::logger::get_logger()->trace("plugin_engine: loading %s", filename);
 	        HMODULE handle = LoadLibrary(filename);
            if (handle) {
                AM_DBG lib::logger::get_logger()->debug("plugin_engine: reading plugin SUCCES [ %s ]",filename);
                AM_DBG lib::logger::get_logger()->debug("Registering test plugin's factory");
                initfuncptr init = (initfuncptr) GetProcAddress(handle,"initialize");
                if (!init) {
                    lib::logger::get_logger()->error("plugin_engine: no initialize routine");
                } else {
                    m_initfuncs.push_back(init);
                }
            } else {
                lib::logger::get_logger()->error("plugin_engine: Error reading plugin %s",filename);
                lib::logger::get_logger()->error("plugin_engine: Reading plugin failed because : %s\n\n", lt_dlerror());
            }

		} while(FindNextFile(dirHandle, &dirData);
	}
 	lib::logger::get_logger()->trace("plugin_engine: Done with plugin directory: %s", dirname.c_str());
}

#else
void
plugin_engine::load_plugins(std::string dirname)
{
}
#endif // WITH_XXXX_PLUGINS

void
plugin_engine::add_plugins(common::factories* factory)
{
    std::vector< initfuncptr >::iterator i;
    for(i=m_initfuncs.begin(); i!=m_initfuncs.end(); i++) {
        initfuncptr init;
        init = *i;
        (init)(factory);
    }
}
