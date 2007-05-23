// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

#include "ambulant/lib/logger.h"
#include "ambulant/version.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/gui_player.h"

#if 1
// These are needed for ambulantmodule.h
#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/parser_factory.h"
#include "ambulant/lib/sax_handler.h"
#include "ambulant/lib/system.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/transition_info.h"
#include "ambulant/common/embedder.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/player.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/scripting.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/stdio_datasource.h"
#include "ambulant/net/posix_datasource.h"
#endif
#include "Python.h"
#include "ambulantmodule.h"
extern "C" {
void initambulant();
};

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

#define AMPYTHON_MODULE_NAME "pyamplugin_scripting"
#define AMPYTHON_METHOD_NAME "initialize"

static ambulant::common::factories * 
bug_workaround(ambulant::common::factories* factory)
{
	return factory;
}

extern "C"
#ifdef AMBULANT_PLATFORM_WIN32
__declspec(dllexport)
#endif
void initialize(
    int api_version,
    ambulant::common::factories* factory,
    ambulant::common::gui_player *player)
{
    if ( api_version != AMBULANT_PLUGIN_API_VERSION ) {
        lib::logger::get_logger()->warn("python_plugin: built for plugin-api version %d, current %d. Skipping.", 
            AMBULANT_PLUGIN_API_VERSION, api_version);
        return;
    }
    if ( !ambulant::check_version() )
        lib::logger::get_logger()->warn("python_plugin: built for different Ambulant version (%s)", AMBULANT_VERSION);
	factory = bug_workaround(factory);
    AM_DBG lib::logger::get_logger()->debug("python_plugin: loaded.");

    //
    // Step 1 - Initialize the Python engine and insert the "ambulant" module.
    //
    // Starting up Python is a bit difficult because we want to release the
    // lock before we return. So the first time we're here we initialze and then
    // release the GIL only to re-acquire it immediately.
    if (!PyEval_ThreadsInitialized()) {
    	PyImport_AppendInittab("ambulant", initambulant);
	    Py_Initialize();
	    PyEval_InitThreads();
	    PyEval_SaveThread();
	}
    AM_DBG lib::logger::get_logger()->debug("python_plugin: initialized Python.");
    
    PyGILState_STATE _GILState = PyGILState_Ensure();
	AM_DBG lib::logger::get_logger()->debug("python_plugin: acquired GIL.");
	
	// Step 2 - Check that we actually do have the ambulant module..
	PyObject *mod = PyImport_ImportModule("ambulant");
    if (mod == NULL) {
        PyErr_Print();
        lib::logger::get_logger()->debug("python_plugin: import ambulant failed.");
        PyGILState_Release(_GILState);
        return;
    }
    AM_DBG lib::logger::get_logger()->debug("python_plugin: imported ambulant.");
	
	// Now we loop over all the Python modules the plugin engine has found.
	PyObject *sys_path = PySys_GetObject("path");
	
	ambulant::common::plugin_engine *pe = ambulant::common::plugin_engine::get_plugin_engine();
	const std::vector<std::string>& all_modules = pe->get_python_plugins();
	std::vector<std::string>::const_iterator i;
    for(i=all_modules.begin(); i!=all_modules.end(); i++) {
    
        // Step 3 - Split into directory and module name
        int last_fsep = (*i).find_last_of("/\\");
        if (last_fsep == std::string::npos) {
            lib::logger::get_logger()->trace("python_plugin: cannot find dirpath for %s", (*i).c_str());
            continue;
        }
        std::string dirname = (*i).substr(0, last_fsep);
        std::string modname = (*i).substr(last_fsep+1);
        int last_dot = modname.find_last_of(".");
        if (last_dot != std::string::npos) {
            modname = modname.substr(0, last_dot);
        }
	// Step 4 - Add directory to sys.path
	PyObject *dirname_obj = PyString_FromString(dirname.c_str());
	if (!PySequence_Contains(sys_path, dirname_obj))
	    if (PyList_Append(sys_path, dirname_obj) <= 0)
	        lib::logger::get_logger()->trace("python_plugin: could not append \"%s\" to sys.path", dirname.c_str());
	Py_DECREF(dirname_obj);
	    
	// Step 5 - Import the module
	char* module_name = strdup(modname.c_str());
        mod = PyImport_ImportModule(module_name);
	free (module_name);
        if (mod == NULL) {
            PyErr_Print();
            lib::logger::get_logger()->trace("python_plugin: plugin file %s", (*i).c_str());
            lib::logger::get_logger()->debug("python_plugin: import %s failed.", modname.c_str());
            continue;
        }
        AM_DBG lib::logger::get_logger()->debug("python_plugin: imported %s.", modname.c_str());
        
        // Step 6 - Call the initialization method
        PyObject *rv = PyObject_CallMethod(mod, "initialize", "iO&O&", 
            api_version,
            factoriesObj_New, factory,
            gui_playerObj_New, player
            );
        if (rv == NULL) {
            PyErr_Print();
            lib::logger::get_logger()->trace("python_plugin: calling of %s.%s failed.", modname.c_str(), "initialize");
            continue;
        }
        Py_DECREF(rv);
    }
    AM_DBG lib::logger::get_logger()->debug("python_plugin: about to release GIL");
    PyGILState_Release(_GILState);
    AM_DBG lib::logger::get_logger()->debug("python_plugin: returning.");
}
