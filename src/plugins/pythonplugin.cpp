// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

#define AMPYTHON_MODULE_NAME "pyamplugin"
#define AMPYTHON_METHOD_NAME "initialize"

static ambulant::common::factories * 
bug_workaround(ambulant::common::factories* factory)
{
	return factory;
}

extern "C" void initialize(
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
    if (getenv("AMBULANT_ENABLE_PYTHON") == 0) {
        lib::logger::get_logger()->trace("python_plugin: skipped. Run with AMBULANT_ENABLE_PYTHON=1 to enable.");
        return;
    }
    
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
	
	PyObject *mod = PyImport_ImportModule("ambulant");
    if (mod == NULL) {
        PyErr_Print();
        lib::logger::get_logger()->debug("python_plugin: import ambulant failed.");
        return;
    }
    AM_DBG lib::logger::get_logger()->debug("python_plugin: imported ambulant.");
	
    mod = PyImport_ImportModule(AMPYTHON_MODULE_NAME);
    if (mod == NULL) {
        PyErr_Print();
        lib::logger::get_logger()->debug("python_plugin: import %s failed.", AMPYTHON_MODULE_NAME);
        return;
    }
    AM_DBG lib::logger::get_logger()->debug("python_plugin: imported %s.", AMPYTHON_MODULE_NAME);
    
    PyObject *rv = PyObject_CallMethod(mod, AMPYTHON_METHOD_NAME, "iO&O&", 
        api_version,
        factoriesObj_New, factory,
        gui_playerObj_New, player
        );
    if (rv == NULL) {
        PyErr_Print();
        lib::logger::get_logger()->debug("python_plugin: calling of %s failed.", AMPYTHON_METHOD_NAME);
        return;
    }
    AM_DBG lib::logger::get_logger()->debug("python_plugin: %s returned, about to release GIL", AMPYTHON_METHOD_NAME);
    PyGILState_Release(_GILState);
    Py_DECREF(rv);
    AM_DBG lib::logger::get_logger()->debug("python_plugin: returning.");
}
