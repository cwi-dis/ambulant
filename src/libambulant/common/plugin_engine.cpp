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

//#include<dlfcn.h>
#include<stdlib.h>
#include<dirent.h>
#include<string.h>
#include <ltdl.h>

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
    collect_plugin_directories();
	int errors = lt_dlinit();
	if (errors) {
	    lib::logger::get_logger()->error("Cannot initialize plugin loader: %d error(s)", errors);
	    return;
	}
    std::vector< std::string >::iterator i;
    for (i=m_plugindirs.begin(); i!=m_plugindirs.end(); i++) {
        load_plugins(*i);
    }
}

void
plugin_engine::collect_plugin_directories()
{
    m_plugindirs.push_back("/Users/jack/src/ambulant/build-gcc3/src/plugins/.libs/");
}

static int filter(const struct dirent* filen)
{
	int len;
	len = strlen(filen->d_name);
	if (!strncmp(filen->d_name+(len-3),".so",3) ) {
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
	int errors;
	char filename[1024];
	dirent **namelist;
	
    int nr_of_files = scandir(dirname.c_str(), &namelist, &filter , NULL);
    if (nr_of_files < 0) {
        lib::logger::get_logger()->error("Error reading plugin directory: %s", dirname.c_str());
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
                strncat(filename, pluginname, sizeof(filename));

                // Load the plugin
                lib::logger::get_logger()->trace("plugin_engine: loading %s", pluginname);
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
}

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

#if 0
plugin::plugin_engine::plugin_engine(common::global_playable_factory* rf, net::datasource_factory* df)
{
	int nr_of_files;
	int errors;
	char filename[1024];
	lt_dlhandle handle;
	
	initfuncptr init;

	dirent **namelist;
//	m_plugindir = getenv("AMB_PLUGIN_DIR");
	m_plugindir = "/Users/jack/src/ambulant/build-gcc3/src/plugins/.libs/";
	
	// Init libltdl
	errors = lt_dlinit ();
	
	
	lib::logger::get_logger()->trace("plugin_engine: Scanning plugin directory: %s", m_plugindir);

	if (m_plugindir != NULL) {
		nr_of_files = scandir(m_plugindir, &namelist, &filter , NULL);
		if (nr_of_files < 0) {
			lib::logger::get_logger()->error("plugin_engine: Error reading plugin directory");
		} else {
			while (nr_of_files--)
   			{
      			//only normal files, not dots (. and ..)
      			if (strcmp(namelist[nr_of_files]->d_name, ".")  &&
	          		strcmp(namelist[nr_of_files]->d_name, "..")) { 
					strcpy(filename,m_plugindir);
					char *pluginname = namelist[nr_of_files]->d_name;
					strcat(filename, pluginname);
					if (strncmp(PLUGIN_PREFIX, pluginname, sizeof(PLUGIN_PREFIX)-1) != 0) {
						lib::logger::get_logger()->trace("plugin_engine: skipping %s", pluginname);
						continue;
					}
					lib::logger::get_logger()->trace("plugin_engine: loading %s", pluginname);
					handle = lt_dlopen(filename);
				  	if (handle) {
  						AM_DBG lib::logger::get_logger()->debug("plugin_engine: reading plugin SUCCES [ %s ]",filename);
						AM_DBG lib::logger::get_logger()->debug("Registering test plugin's factory");
						init = (initfuncptr) lt_dlsym(handle,"initialize");
						if (!init) {
							lib::logger::get_logger()->error("plugin_engine: no initialize routine");
						} else {
							(*init)(factory);
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
	}
}
#endif
