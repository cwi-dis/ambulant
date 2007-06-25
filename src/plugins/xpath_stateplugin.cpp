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
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <stdio.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

// -------------------
class xpath_state_component : public common::script_component {
  public:
    xpath_state_component();
	virtual ~xpath_state_component();
	
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
  private:
  	xmlDocPtr m_state;
  	xmlXPathContextPtr m_context;
};

// -------------------
class xpath_state_component_factory : public common::script_component_factory {
  public:
	virtual ~xpath_state_component_factory() {};
 
 	common::script_component *new_script_component(const char *uri);
};

// -------------------
xpath_state_component::xpath_state_component()
:	m_state(NULL),
	m_context(NULL)
{
}
xpath_state_component::~xpath_state_component()
{
	if (m_state) {
		xmlFreeDoc(m_state);
		m_state = NULL;
	}
	if (m_context) {
		xmlXPathFreeContext(m_context);
		m_context = NULL;
	}
}

void
xpath_state_component::register_state_test_methods(common::state_test_methods *stm)
{
	lib::logger::get_logger()->trace("xpath_state_component::register_state_test_methods(0x%x)", stm);
}

void
xpath_state_component::declare_state(const lib::node *state)
{
	lib::logger::get_logger()->trace("xpath_state_component::declare_state(%s)", state->get_sig().c_str());
	if (m_context) {
		xmlXPathFreeContext(m_context);
		m_context = NULL;
	}
	if (m_state) {
		xmlFreeDoc(m_state);
	}
	const lib::node *aroot = NULL;
	if (state != NULL) {
		// XXX Need to check for src= attribute for remote state
		aroot = state->down();
		while (aroot && aroot->is_data_node()) aroot = aroot->next();
	}
	if (aroot == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: empty <state> not allowed");
		return;
	}
	const lib::node *arootnext = aroot->next();
	while (arootnext) {
		if (!arootnext->is_data_node()) {
			lib::logger::get_logger()->trace("xpath_state_component: <state> must have exactly one child");
			return;
		}
		arootnext = arootnext->next();
	}
	m_state = xmlNewDoc(BAD_CAST "1.0");
	assert(m_state);
	xmlNodePtr xroot = NULL;
	xmlNodePtr xparent = NULL;
	lib::node::const_iterator anp;
	for(anp=aroot->begin(); anp!=aroot->end(); anp++) {
		/*AM_DBG*/ lib::logger::get_logger()->debug("declare_state: xparent=0x%x", xparent);
		if ((*anp).second == aroot) {
			// Root node
			if ((*anp).first) {
				assert(xroot == NULL);
				xroot = xmlNewNode(NULL, BAD_CAST (*anp).second->get_local_name().c_str());
				assert(xroot);
				xmlDocSetRootElement(m_state, xroot);
				xparent = xroot;
			}
			continue;
		}
		if ((*anp).first) {
			const lib::node *an = (*anp).second;
			xmlNodePtr xn;
			/*AM_DBG*/ lib::logger::get_logger()->debug("declare_state: examine %s", an->get_sig().c_str());
			assert(xparent);
			if (an->is_data_node()) {
				xn = xmlNewText(BAD_CAST an->get_data().c_str());
				assert(xn);
				xn = xmlAddChild(xparent, xn);
			} else {
				xn = xmlNewChild(xparent, NULL, BAD_CAST an->get_local_name().c_str(), NULL);
				assert(xn);
			}
			xparent = xn;
		} else {
			/*AM_DBG*/ lib::logger::get_logger()->debug("declare_state: out of %s", (*anp).second->get_sig().c_str());
			xparent = xparent->parent;
		}
	}
	/*AM_DBG*/ xmlDocDump(stdout, m_state); // WARNING: will crash Ambulant afterwards!
	m_context = xmlXPathNewContext(m_state);
	m_context->node = xroot;
	assert(m_context);
}

bool
xpath_state_component::bool_expression(const char *expr)
{
	lib::logger::get_logger()->trace("xpath_state_component::bool_expression(%s) -> false", expr);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		return false;
	}
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST expr, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: cannot evaluate expr=\"%s\"", expr);
		return false;
	}
	bool rv = (bool)xmlXPathCastToBoolean(result);
	xmlXPathFreeObject(result);
	/*AM_DBG*/ lib::logger::get_logger()->debug("xpath_state_component::bool_expression(%s) -> %d", expr, (int)rv);
	return rv;
}

void
xpath_state_component::set_value(const char *var, const char *expr)
{
	lib::logger::get_logger()->trace("xpath_state_component::set_value(%s, %s)", var, expr);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		return;
	}
}

void
xpath_state_component::send(const char *submission)
{
	lib::logger::get_logger()->trace("xpath_state_component::send(%s)", submission);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		return;
	}
}

std::string
xpath_state_component::string_expression(const char *expr)
{
	lib::logger::get_logger()->trace("xpath_state_component::string_expression(%s) -> %s", expr, expr);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		return "";
	}
	return std::string(expr);
}

// -------------------
common::script_component *
xpath_state_component_factory::new_script_component(const char *uri)
{
	if (strcmp(uri, "http://www.ambulantplayer.org/components/XPath1.0") == 0) {
		lib::logger::get_logger()->trace("xpath_state_component_factory::new_script_component: returned script_component");
		return new xpath_state_component();
	}
	lib::logger::get_logger()->trace("xpath_state_component_factory::new_script_component: no support for language %s", uri);
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
        lib::logger::get_logger()->warn("xpath_state_plugin: built for plugin-api version %d, current %d. Skipping.", 
            AMBULANT_PLUGIN_API_VERSION, api_version);
        return;
    }
    if ( !ambulant::check_version() )
        lib::logger::get_logger()->warn("xpath_state_plugin: built for different Ambulant version (%s)", AMBULANT_VERSION);
	factory = bug_workaround(factory);
    lib::logger::get_logger()->debug("xpath_state_plugin: loaded.");
	common::global_script_component_factory *scf = factory->get_script_component_factory();
    if (scf) {
    	xpath_state_component_factory *dscf = new xpath_state_component_factory();
		scf->add_factory(dscf);
    	lib::logger::get_logger()->trace("xpath_state_plugin: registered");
    }
}
