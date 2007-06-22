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
#include "ambulant/common/scripting.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

// -------------------
class dummy_state_component : public common::script_component {
  public:
    dummy_state_component() {};
	virtual ~dummy_state_component() {};
	
	/// Register the systemTest/customTest API
	void register_state_test_methods(common::state_test_methods *stm);
  
    /// Declare the state in the document
    void declare_state(const lib::node *state);
    
    /// Calculate a boolean expression
    bool bool_expression(const char *expr);
    
    /// Set a state variable to an expression
    void set_value(const char *var, const char *expr);
    
    /// Submit the state
    void send(const char *submission);
    
    /// Calculate a string expression
    std::string string_expression(const char *expr);
};

// -------------------
class dummy_state_component_factory : public common::script_component_factory {
  public:
	virtual ~dummy_state_component_factory() {};
 
 	common::script_component *new_script_component(const char *uri);
};

// -------------------
void
dummy_state_component::register_state_test_methods(common::state_test_methods *stm)
{
	lib::logger::get_logger()->trace("dummy_state_component::register_state_test_methods(0x%x)", stm);
}

void
dummy_state_component::declare_state(const lib::node *state)
{
	lib::logger::get_logger()->trace("dummy_state_component::declare_state(%s)", state->get_sig().c_str());
}

bool
dummy_state_component::bool_expression(const char *expr)
{
	lib::logger::get_logger()->trace("dummy_state_component::bool_expression(%s) -> false", expr);
	return false;
}

void
dummy_state_component::set_value(const char *var, const char *expr)
{
	lib::logger::get_logger()->trace("dummy_state_component::set_value(%s, %s)", var, expr);
}

void
dummy_state_component::send(const char *submission)
{
	lib::logger::get_logger()->trace("dummy_state_component::send(%s)", submission);
}

std::string
dummy_state_component::string_expression(const char *expr)
{
	lib::logger::get_logger()->trace("dummy_state_component::string_expression(%s) -> %s", expr, expr);
	return std::string(expr);
}

// -------------------
common::script_component *
dummy_state_component_factory::new_script_component(const char *uri)
{
	if (strcmp(uri, "http://www.ambulantplayer.org/components/dummy_state") == 0) {
		lib::logger::get_logger()->trace("dummy_state_component_factory::new_script_component: returned script_component");
		return new dummy_state_component();
	}
	lib::logger::get_logger()->trace("dummy_state_component_factory::new_script_component: no support for language %s", uri);
	return NULL;
}

// -------------------
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
        lib::logger::get_logger()->warn("state_dummy_plugin: built for plugin-api version %d, current %d. Skipping.", 
            AMBULANT_PLUGIN_API_VERSION, api_version);
        return;
    }
    if ( !ambulant::check_version() )
        lib::logger::get_logger()->warn("state_dummy_plugin: built for different Ambulant version (%s)", AMBULANT_VERSION);
	factory = bug_workaround(factory);
    lib::logger::get_logger()->debug("state_dummy_plugin: loaded.");
	common::global_script_component_factory *scf = factory->get_script_component_factory();
    if (scf) {
    	dummy_state_component_factory *dscf = new dummy_state_component_factory();
		scf->add_factory(dscf);
    	lib::logger::get_logger()->trace("state_dummy_plugin: registered");
    }
}
