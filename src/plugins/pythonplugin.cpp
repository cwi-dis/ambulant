// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#include "Python.h"

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
#include "ambulant/lib/timer_sync.h"
#include "ambulant/lib/transition_info.h"
#include "ambulant/common/embedder.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/player.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/state.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/stdio_datasource.h"
#include "ambulant/net/posix_datasource.h"
#endif
#include "ambulantmodule.h"

// The Python interface does not qualify strings with const, so we have to
// disable warnings about non-writeable strings (zillions of them)

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

extern "C" {

void initambulant();

#define WITH_OBJC_BRIDGE
#ifdef WITH_OBJC_BRIDGE
struct pyobjc_api {
	int api_version;	/* API version */
	size_t struct_len;	/* Length of this struct */
	PyTypeObject* class_type;	/* PyObjCClass_Type	   */
	PyTypeObject* object_type;	/* PyObjCObject_Type   */
	PyTypeObject* select_type;	/* PyObjCSelector_Type */

	/* PyObjC_RegisterMethodMapping */
	void *register_method_mapping;

	/* PyObjC_RegisterSignatureMapping */
	int (*register_signature_mapping)(
		char*,
		PyObject *(*)(PyObject*, PyObject*, PyObject*),
		void (*)(void*, void*, void**, void*));

	/* PyObjCObject_GetObject */
	void* (*obj_get_object)(PyObject*);

	/* PyObjCObject_ClearObject */
	void (*obj_clear_object)(PyObject*);

	/* PyObjCClass_GetClass */
	void* (*cls_get_class)(PyObject*);

	/* PyObjCClass_New */
	PyObject* (*cls_to_python)(void* cls);

	/* PyObjC_PythonToId */
	void* (*python_to_id)(PyObject*);

	/* PyObjC_IdToPython */
	PyObject* (*id_to_python)(void*);
};

static size_t extra_data;
struct ambulant::common::plugin_extra_data plugin_extra_data = {
	"python_extra_data",
	(void*)&extra_data
};
#endif // WITH_OBJC_BRIDGE
}; // extern "C"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

#define AMPYTHON_MODULE_NAME "pyamplugin_state"
#define AMPYTHON_METHOD_NAME "initialize"

// Ambulant can put anything into this extra_data pointer, which will
// be passed on to Python
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
		lib::logger::get_logger()->warn(gettext("%s: built for plugin-api version %d, current %d. Skipping."),"python_plugin", AMBULANT_PLUGIN_API_VERSION, api_version);
		return;
	}
	if ( !ambulant::check_version() ) {
		lib::logger::get_logger()->warn(gettext("%s: built for different Ambulant version (%s)"),"python_plugin", AMBULANT_VERSION);
	}
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
		size_t last_fsep = (*i).find_last_of("/\\");
		if (last_fsep == std::string::npos) {
			lib::logger::get_logger()->trace("python_plugin: cannot find dirpath for %s", (*i).c_str());
			continue;
		}
		std::string dirname = (*i).substr(0, last_fsep);
		std::string modname = (*i).substr(last_fsep+1);
		size_t last_dot = modname.find_last_of(".");
		if (last_dot != std::string::npos) {
			modname = modname.substr(0, last_dot);
		}
		// Step 4 - Add directory to sys.path
		PyObject *dirname_obj = PyString_FromString(dirname.c_str());
		if (!PySequence_Contains(sys_path, dirname_obj)) {
			if (PyList_Append(sys_path, dirname_obj) <= 0) {
				lib::logger::get_logger()->trace("python_plugin: could not append \"%s\" to sys.path", dirname.c_str());
			}
		}
		Py_DECREF(dirname_obj);

		// Step 5 - Import the module
		mod = PyImport_ImportModule(const_cast<char*>(modname.c_str()));
		if (mod == NULL) {
			PyErr_Print();
			lib::logger::get_logger()->trace("python_plugin: plugin file %s", (*i).c_str());
			lib::logger::get_logger()->debug("python_plugin: import %s failed.", modname.c_str());
			continue;
		}
		AM_DBG lib::logger::get_logger()->debug("python_plugin: imported %s.", modname.c_str());
#ifdef WITH_OBJC_BRIDGE
		// Step 6 - Communicate the extra_data, if needed and wanted
		if (extra_data /* plugin_extra_data.m_plugin_extra */) {
			AM_DBG lib::logger::get_logger()->debug("python_plugin: extra_data is 0x%x", extra_data);
			PyObject *objcmod = PyImport_ImportModule("objc");
			if (objcmod == NULL) {
				PyErr_Print();
				lib::logger::get_logger()->trace("python_plugin: extra_data: no objc module");
				continue;
			}
			PyObject *api_in_pyobj = PyObject_GetAttrString(objcmod, "__C_API__");
			if (api_in_pyobj == NULL) {
				PyErr_Print();
				lib::logger::get_logger()->trace("python_plugin: extra_data: no __C_API__ in objc module");
				continue;
			}
			struct pyobjc_api *api = (struct pyobjc_api *)PyCObject_AsVoidPtr(api_in_pyobj);
			if (api == NULL) {
				PyErr_Print();
				lib::logger::get_logger()->trace("python_plugin: extra_data: objc.__C_API__ not a cobject");
				continue;
			}
			PyObject *extra_data_obj = api->id_to_python((void*)extra_data);
			PyObject *rv = PyObject_CallMethod(mod, "set_extra_data", "O", extra_data_obj);
			if (rv == NULL) {
				lib::logger::get_logger()->debug("python_plugin: module %s does not want extra_data", modname.c_str());
				PyErr_Clear();
			}
			// ignore errors
			Py_XDECREF(rv);
		}
#endif
		lib::logger::get_logger()->trace("python_plugin: %s loaded", modname.c_str());
		// Step 7 - Call the initialization method
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
