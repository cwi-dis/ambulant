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
#include <libxml/xpathInternals.h>
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
	
	/// Get at the state_test_methods structure
	common::state_test_methods *get_state_test_methods() const { return m_state_test_methods; }
  private:
  	xmlDocPtr m_state;
  	xmlXPathContextPtr m_context;
	common::state_test_methods *m_state_test_methods;
};

// -------------------

extern "C" {
// Helper function that enables libxml2 to call our state_test methods.
// Picks up the requested function and arguments from the expression parser
// contents, calls the function, stores the return value.
void
smil_function_execute(xmlXPathParserContextPtr ctxt, int nargs)
{
	AM_DBG lib::logger::get_logger()->debug("smil_function_execute(0x%x, %d)", ctxt, nargs);
	assert(ctxt);
	xmlXPathContextPtr xmlcontext = ctxt->context;
	assert(xmlcontext);
	xpath_state_component *cmp = static_cast<xpath_state_component *>(xmlcontext->funcLookupData);
	assert(cmp);
	AM_DBG lib::logger::get_logger()->debug("xmlcontext=0x%x, component=0x%x", xmlcontext, cmp);
	common::state_test_methods *stm = cmp->get_state_test_methods();
	assert(stm);
	if (strcmp((char*)xmlcontext->function, "smil-audioDesc") == 0 ) {
		bool rv = stm->smil_audio_desc();
		valuePush(ctxt, xmlXPathNewBoolean(rv));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-bitrate") == 0 ) {
		int rv = stm->smil_bitrate();
		valuePush(ctxt, xmlXPathNewFloat(rv));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-captions") == 0 ) {
		bool rv = stm->smil_captions();
		valuePush(ctxt, xmlXPathNewBoolean(rv));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-component") == 0 ) {
		xmlChar *arg_str = xmlXPathPopString(ctxt);
		if (arg_str == NULL) goto badarg;
		std::string arg((char*)arg_str);
		xmlFree(arg_str);
		bool rv = stm->smil_component(arg);
		valuePush(ctxt, xmlXPathNewBoolean(rv));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-customTest") == 0 ) {
		xmlChar *arg_str = xmlXPathPopString(ctxt);
		if (arg_str == NULL) goto badarg;
		std::string arg((char*)arg_str);
		xmlFree(arg_str);
		bool rv = stm->smil_custom_test(arg);
		valuePush(ctxt, xmlXPathNewBoolean(rv));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-cpu") == 0 ) {
		std::string rv = stm->smil_cpu();
		valuePush(ctxt, xmlXPathNewString(BAD_CAST rv.c_str()));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-language") == 0 ) {
		xmlChar *arg_str = xmlXPathPopString(ctxt);
		if (arg_str == NULL) goto badarg;
		std::string arg((char*)arg_str);
		xmlFree(arg_str);
		bool rv = stm->smil_language(arg);
		valuePush(ctxt, xmlXPathNewBoolean(rv));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-operatingSystem") == 0 ) {
		std::string rv = stm->smil_operating_system();
		valuePush(ctxt, xmlXPathNewString(BAD_CAST rv.c_str()));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-overdubOrSubtitle") == 0 ) {
		std::string rv = stm->smil_overdub_or_subtitle();
		valuePush(ctxt, xmlXPathNewString(BAD_CAST rv.c_str()));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-required") == 0 ) {
		xmlChar *arg_str = xmlXPathPopString(ctxt);
		if (arg_str == NULL) goto badarg;
		std::string arg((char*)arg_str);
		xmlFree(arg_str);
		bool rv = stm->smil_required(arg);
		valuePush(ctxt, xmlXPathNewBoolean(rv));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-screenDepth") == 0 ) {
		int rv = stm->smil_screen_depth();
		valuePush(ctxt, xmlXPathNewFloat(rv));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-screenHeight") == 0 ) {
		int rv = stm->smil_screen_height();
		valuePush(ctxt, xmlXPathNewFloat(rv));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-screenWidth") == 0 ) {
		int rv = stm->smil_screen_width();
		valuePush(ctxt, xmlXPathNewFloat(rv));
	} else {
		assert(0);
	}
badarg:
	lib::logger::get_logger()->trace("xpath_state_component: argument error for function %s()", xmlcontext->function);
}

static char *smil_function_names[] = {
	"smil-audioDesc",
	"smil-bitrate",
	"smil-captions",
	"smil-component",
	"smil-customTest",
	"smil-cpu",
	"smil-language",
	"smil-operatingSystem",
	"smil-overdubOrSubtitle",
	"smil-required",
	"smil-screenDepth",
	"smil-screenHeight",
	"smil-screenWidth",
	NULL
};

// smil_function_lookup - Return the xmlXpathFunction for a given function name.
// Check that the function name is one of the supported functions, and, if it is,
// return a reference to a smil_function_execute which does the actual work.
xmlXPathFunction
smil_function_lookup(void *ctxt, const xmlChar *name, const xmlChar *nsuri)
{
	AM_DBG lib::logger::get_logger()->debug("smil_function_lookup(0x%x, %s, %s)", ctxt, name, nsuri);
	xpath_state_component *cmp = static_cast<xpath_state_component *>(ctxt);
	assert(cmp);
	char **namep;
	for(namep=smil_function_names; *namep; namep++) {
		if(strcmp(*namep, (const char*)name) == 0)
			return smil_function_execute;
	}
	return NULL;
}
} // extern "C"

// -------------------
class xpath_state_component_factory : public common::script_component_factory {
  public:
	virtual ~xpath_state_component_factory() {};
 
 	common::script_component *new_script_component(const char *uri);
};

// -------------------
xpath_state_component::xpath_state_component()
:	m_state(NULL),
	m_context(NULL),
	m_state_test_methods(NULL)
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
	m_state_test_methods = stm;
}

void
xpath_state_component::declare_state(const lib::node *state)
{
	lib::logger::get_logger()->trace("xpath_state_component::declare_state(%s)", state->get_sig().c_str());
	// First we free any old document and state.
	if (m_context) {
		xmlXPathFreeContext(m_context);
		m_context = NULL;
	}
	if (m_state) {
		xmlFreeDoc(m_state);
	}
	// Now we need to get the Ambulant-style DOM node for our state document
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
	// Create the libxml-style document
	m_state = xmlNewDoc(BAD_CAST "1.0");
	assert(m_state);
	xmlNodePtr xroot = NULL;
	xmlNodePtr xparent = NULL;
	// Iterate over the ambulant-style tree and create the libxml-style tree
	lib::node::const_iterator anp;
	for(anp=aroot->begin(); anp!=aroot->end(); anp++) {
		AM_DBG lib::logger::get_logger()->debug("declare_state: xparent=0x%x", xparent);
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
			xparent = xparent->parent;
		}
	}
	// Finally we set up the XPath expression context
	m_context = xmlXPathNewContext(m_state);
	m_context->node = xroot;
	m_context->funcLookupFunc = smil_function_lookup;
	m_context->funcLookupData = (void *)this;
	assert(m_context);
}

bool
xpath_state_component::bool_expression(const char *expr)
{
	lib::logger::get_logger()->trace("xpath_state_component::bool_expression(%s)", expr);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		return true;
	}
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST expr, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: cannot evaluate expr=\"%s\"", expr);
		return true;
	}
	bool rv = (bool)xmlXPathCastToBoolean(result);
	xmlXPathFreeObject(result);
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::bool_expression(%s) -> %d", expr, (int)rv);
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
	// Evaluate the expression, get a string as result
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST expr, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: setvalue: cannot evaluate expr=\"%s\"", expr);
		return;
	}
	xmlChar *result_str = xmlXPathCastToString(result);
	xmlXPathFreeObject(result);
	// Now compute the node-set expression and check that it begets a single node
	result = xmlXPathEvalExpression(BAD_CAST var, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: setvalue: cannot evaluate var=\"%s\"", var);
		return;
	}
	if (result->type != XPATH_NODESET) {
		lib::logger::get_logger()->trace("xpath_state_component: setvalue: var=\"%s\" is not a node-set", var);
		return;
	}
	xmlNodeSetPtr nodeset = result->nodesetval;
	if (nodeset == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: setvalue: var=\"%s\" does not refer to an existing item", var);
		return;
	}
	if (nodeset->nodeNr != 1) {
		lib::logger::get_logger()->trace("xpath_state_component: setvalue: var=\"%s\" refers to %d items", var, nodeset->nodeNr);
		return;
	}
	// Finally set the value
	xmlNodePtr nodeptr = *nodeset->nodeTab;
	xmlNodeSetContent(nodeptr, result_str);
	xmlFree(result_str);
}

void
xpath_state_component::send(const char *submission)
{
	lib::logger::get_logger()->trace("xpath_state_component::send(%s)", submission);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		return;
	}
	lib::logger::get_logger()->trace("xpath_state_component: <send> not yet implemented");
}

std::string
xpath_state_component::string_expression(const char *expr)
{
	lib::logger::get_logger()->trace("xpath_state_component::string_expression(%s)", expr);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		return "";
	}
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST expr, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: cannot evaluate \"{%s}\"", expr);
		return "";
	}
	xmlChar *result_str = xmlXPathCastToString(result);
	xmlXPathFreeObject(result);
	if (result_str == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: \"{%s}\" does not evaluate to a string", expr);
		return "";
	}
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::string_expression(%s) -> %s", expr, (int)result_str);
	std::string rv((char *)result_str);
	xmlFree(result_str);
	return rv;
}

// -------------------
common::script_component *
xpath_state_component_factory::new_script_component(const char *uri)
{
	if (strcmp(uri, "http://www.w3.org/TR/1999/REC-xpath-19991116") == 0) {
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
    AM_DBG lib::logger::get_logger()->debug("xpath_state_plugin: loaded.");
	common::global_script_component_factory *scf = factory->get_script_component_factory();
    if (scf) {
    	xpath_state_component_factory *dscf = new xpath_state_component_factory();
		scf->add_factory(dscf);
    	lib::logger::get_logger()->trace("xpath_state_plugin: registered");
    }
}