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
#include"ambulant/common/plugin_init.h"

//#include<dlfcn.h>
#include<stdlib.h>
#include<dirent.h>
#include<string.h>
#include <ltdl.h>



#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;


int filter(const struct dirent* filen)
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




plugin::plugin_engine::plugin_engine(common::global_playable_factory* rf, net::datasource_factory* df)
{
	int nr_of_files;
	int errors;
	char filename[1024];
	typedef void (*initfunctype)(ambulant::common::global_playable_factory* rf, ambulant::net::datasource_factory* df);
	lt_dlhandle handle;
	
	initfunctype init;

	dirent **namelist;
	m_plugindir = getenv("AMB_PLUGIN_DIR");
	
	// Init libltdl
	errors = lt_dlinit ();
	
	
	AM_DBG lib::logger::get_logger()->trace("plugin_engine::Scaning plugin directory : %s", m_plugindir);

	if (m_plugindir != NULL) {
		nr_of_files = scandir(m_plugindir, &namelist, &filter , NULL);
		if (nr_of_files < 0) {
			lib::logger::get_logger()->error("plugin_playable_factory::Error reading plugin directory");
		} else {
			while (nr_of_files--)
   			{
      			//only normal files, not dots (. and ..)
      			if (strcmp(namelist[nr_of_files]->d_name, ".")  &&
	          		strcmp(namelist[nr_of_files]->d_name, "..")) { 
					strcpy(filename,m_plugindir);
					strcat(filename,namelist[nr_of_files]->d_name);
					handle = lt_dlopen(filename);
				  	if (handle) {
  						AM_DBG lib::logger::get_logger()->trace("plugin_playable_factory::reading plugin SUCCES [ %s ]",filename);
						AM_DBG lib::logger::get_logger()->trace("Registring test plugin's factory");
						init = (initfunctype) lt_dlsym(handle,"initialize");
						if (!init) {
							lib::logger::get_logger()->error("plugin_playable_factory: no initialize routine");
						} else {
							(*init)(rf,df);
						}
		  			} else {
						lib::logger::get_logger()->error("plugin_playable_factory::Error reading plugin %s",filename);
						lib::logger::get_logger()->error("Reading plugin failed because : %s\n\n", lt_dlerror());
					}
			}
			free(namelist[nr_of_files]);
			}
		free(namelist);
		}
	}
}
