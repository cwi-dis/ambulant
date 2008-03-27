// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
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
#include "ambulant/common/state.h"
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
class xpath_state_component : public common::state_component {
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
    
	/// Add a new variable to the state
	void new_value(const char *ref, const char *where, const char *name, const char *expr);
	
	/// Delete a variable from the state
	void del_value(const char *ref);
	
    /// Submit the state
    void send(const lib::node *submission);
    
    /// Calculate a string expression
    std::string string_expression(const char *expr);
	
	/// Get at the state_test_methods structure
	common::state_test_methods *get_state_test_methods() const { return m_state_test_methods; }
	
	/// Register interest in stateChange events
	void want_state_change(const char *ref, common::state_change_callback *cb);
  private:
  	void _check_state_change(xmlNodePtr changed);
  	
  	xmlDocPtr m_state;
  	xmlXPathContextPtr m_context;
	common::state_test_methods *m_state_test_methods;
	std::vector<std::pair<std::string, common::state_change_callback* > > m_state_change_callbacks;
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
class xpath_state_component_factory : public common::state_component_factory {
  public:
	virtual ~xpath_state_component_factory() {};
 
 	common::state_component *new_state_component(const char *uri);
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
		// Check for src= attribute for remote state
		if (state->get_attribute("src")) {
			net::url src_url = state->get_url("src");
			if (!src_url.is_local_file()) {
				lib::logger::get_logger()->trace("xpath_state_component: only file: scheme implemented for <state>");
				return;
			}
			std::string src_filename = src_url.get_file();
			m_state = xmlReadFile(src_filename.c_str(), NULL, 0);
			if (m_state) {
				// Finally we set up the XPath expression context
				m_context = xmlXPathNewContext(m_state);
				m_context->node = xmlDocGetRootElement(m_state);
				m_context->funcLookupFunc = smil_function_lookup;
				m_context->funcLookupData = (void *)this;
				assert(m_context);
				return;
			}
			// Otherwise we fall through and use the default (in-line) state.
			lib::logger::get_logger()->trace("xpath_state_component: xmlReadFile(%s) failed", src_filename.c_str());

		}
		aroot = state->down();
		while (aroot && aroot->is_data_node()) aroot = aroot->next();
		if (aroot == NULL) {
			// Second case: state element, but no content and no src URL.
			// Create default document.
			m_state = xmlReadDoc(BAD_CAST "<data/>\n", NULL, NULL, 0);
			if (m_state == NULL) {
				lib::logger::get_logger()->trace("xpath_state_component: xmlReadDoc(\"<data/>\") failed");
				return;
			}
			// Finally we set up the XPath expression context
			m_context = xmlXPathNewContext(m_state);
			m_context->node = xmlDocGetRootElement(m_state);
			m_context->funcLookupFunc = smil_function_lookup;
			m_context->funcLookupData = (void *)this;
			assert(m_context);
			return;
		}
			
	}
	assert(aroot);
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
	// If we now have a nodeset we try to cast to a string
	if (result->type == XPATH_NODESET) {
		result = xmlXPathConvertString(result);
		if (result == NULL) {
			lib::logger::get_logger()->trace("xpath_state_component: expr=\"%s\": cannot convert to string", expr);
			return true;
		}
	}
	if (result->type == XPATH_STRING) {
		result = xmlXPathConvertNumber(result);
		if (result == NULL) {
			lib::logger::get_logger()->trace("xpath_state_component: expr=\"%s\": cannot convert to number", expr);
			return true;
		}
	}
	bool rv = (bool)xmlXPathCastToBoolean(result);
	xmlXPathFreeObject(result);
	lib::logger::get_logger()->debug("xpath_state_component::bool_expression(%s) -> %d", expr, (int)rv);
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
	_check_state_change(nodeptr);
}

void
xpath_state_component::new_value(const char *ref, const char *where, const char *name, const char *expr)
{
	lib::logger::get_logger()->trace("xpath_state_component::new_value(ref=%s, where=%s, name=%s, expr=%s)",
		ref, where, name, expr);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		return;
	}
	// Evaluate the expression, get a string as result
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST expr, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: cannot evaluate expr=\"%s\"", expr);
		return;
	}
	xmlChar *result_str = xmlXPathCastToString(result);
	xmlXPathFreeObject(result);
	// Now compute the node-set expression and check that it begets a single node.
	// A missing ref is the current node.
	if (ref == NULL) ref = ".";
	result = xmlXPathEvalExpression(BAD_CAST ref, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: cannot evaluate ref=\"%s\"", ref);
		return;
	}
	if (result->type != XPATH_NODESET) {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: ref=\"%s\" is not a node-set", ref);
		return;
	}
	xmlNodeSetPtr nodeset = result->nodesetval;
	if (nodeset == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: ref=\"%s\" does not refer to an existing item", ref);
		return;
	}
	if (nodeset->nodeNr != 1) {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: ref=\"%s\" refers to %d items", ref, nodeset->nodeNr);
		return;
	}
	xmlNodePtr refnodeptr = *nodeset->nodeTab;
	// Create the new node
	xmlNodePtr newnodeptr = xmlNewNode(NULL, BAD_CAST name);
	assert(newnodeptr);
	xmlNodeSetContent(newnodeptr, result_str);
	xmlFree(result_str);

	// Insert in the right place	
	if (where && strcmp(where, "before") == 0) {
		xmlNodePtr rv = xmlAddPrevSibling(refnodeptr, newnodeptr);
		if (rv == NULL) {
			lib::logger::get_logger()->trace("xpath_state_component: <newvalue ref=\"%s\" name=\"%s\">: xmlAddPrevSibling failed", ref, name);
			return;
		}
	} else
	if (where && strcmp(where, "after") == 0) {
		xmlNodePtr rv = xmlAddNextSibling(refnodeptr, newnodeptr);
		if (rv == NULL) {
			lib::logger::get_logger()->trace("xpath_state_component: <newvalue ref=\"%s\" name=\"%s\">: xmlAddNextSibling failed", ref, name);
			return;
		}
	} else
	if (where == NULL || strcmp(where, "child") == 0) {
		xmlNodePtr rv = xmlAddChild(refnodeptr, newnodeptr);
		if (rv == NULL) {
			lib::logger::get_logger()->trace("xpath_state_component: <newvalue ref=\"%s\" name=\"%s\">: xmlAddChild failed", ref, name);
			return;
		}
	} else {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: where=\"%s\": unknown location", where);
		xmlFreeNode(newnodeptr);
		return;
	}
	// Re-set the context: it may have changed.
	m_context->node = xmlDocGetRootElement(m_state);
	_check_state_change(newnodeptr); // XXX Or refnodeptr? both?
}
	
void
xpath_state_component::del_value(const char *ref)
{
	lib::logger::get_logger()->trace("xpath_state_component::del_value(%s)", ref);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		return;
	}
	// Compute the node-set expression and check that it begets a single node
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST ref, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: delvalue: cannot evaluate ref=\"%s\"", ref);
		return;
	}
	if (result->type != XPATH_NODESET) {
		lib::logger::get_logger()->trace("xpath_state_component: delvalue: ref=\"%s\" is not a node-set", ref);
		return;
	}
	xmlNodeSetPtr nodeset = result->nodesetval;
	if (nodeset == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: delvalue: ref=\"%s\" does not refer to an existing item", ref);
		return;
	}
	if (nodeset->nodeNr != 1) {
		lib::logger::get_logger()->trace("xpath_state_component: delvalue: ref=\"%s\" refers to %d items", ref, nodeset->nodeNr);
		return;
	}
	xmlNodePtr refnodeptr = *nodeset->nodeTab;
	xmlUnlinkNode(refnodeptr);
	xmlFreeNode(refnodeptr);
	// Re-set the context: it may have changed.
	m_context->node = xmlDocGetRootElement(m_state);
}
	
void
xpath_state_component::send(const lib::node *submission)
{
	lib::logger::get_logger()->trace("xpath_state_component::send(%s)", submission->get_sig().c_str());
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		return;
	}
	assert(submission);
	const char *method = submission->get_attribute("method");
	if (method && strcmp(method, "put") != 0) {
		lib::logger::get_logger()->trace("xpath_state_component: only method=\"put\" implemented");
		return;
	}
	const char *replace = submission->get_attribute("replace");
	if (replace && strcmp(replace, "none") != 0) {
		lib::logger::get_logger()->trace("xpath_state_component: only replace=\"none\" implemented");
		return;
	}
	net::url dst_url = submission->get_url("action");
	if (dst_url.is_empty_path()) {
		lib::logger::get_logger()->trace("xpath_state_component: submission action attribute missing");
		return;
	}
	if (!dst_url.is_local_file()) {
		lib::logger::get_logger()->trace("xpath_state_component: only file: scheme implemented for <send>");
		return;
	}
	std::string dst_filename = dst_url.get_file();
	FILE *fp = fopen(dst_filename.c_str(), "w");
	xmlDocDump(fp, m_state);
	fclose(fp);
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
	lib::logger::get_logger()->debug("xpath_state_component::string_expression(%s) -> %s", expr, (void*)result_str);
	std::string rv((char *)result_str);
	xmlFree(result_str);
	return rv;
}

void
xpath_state_component::want_state_change(const char *ref, common::state_change_callback *cb)
{
	lib::logger::get_logger()->trace("xpath_state_component::want_state_change(%s)", ref);
	std::pair<std::string, common::state_change_callback*> item(ref, cb);
	m_state_change_callbacks.push_back(item);
}

void
xpath_state_component::_check_state_change(xmlNodePtr changed)
{
	std::vector<std::pair<std::string, common::state_change_callback* > >::iterator i;
	for (i=m_state_change_callbacks.begin(); i != m_state_change_callbacks.end(); i++) {
		std::string& ref = (*i).first;
		xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST ref.c_str(), m_context);
		if (result != NULL && result->type == XPATH_NODESET) {
			xmlNodeSetPtr nodeset = result->nodesetval;
			if (nodeset) {
				int  j;
				for (j=0; j<nodeset->nodeNr; j++) {
					if (nodeset->nodeTab[j] == changed) {
						AM_DBG lib::logger::get_logger()->debug("check_state_change: raising stateChange(%s)", ref.c_str());
						common::state_change_callback *cb = (*i).second;
						assert(cb);
						cb->on_state_change(ref.c_str());
						break;
					}
				}
			}
		}
	}
}

// -------------------
common::state_component *
xpath_state_component_factory::new_state_component(const char *uri)
{
	if (strcmp(uri, "http://www.w3.org/TR/1999/REC-xpath-19991116") == 0) {
		lib::logger::get_logger()->trace("xpath_state_component_factory::new_state_component: returned state_component");
		return new xpath_state_component();
	}
	lib::logger::get_logger()->trace("xpath_state_component_factory::new_state_component: no support for language %s", uri);
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
	common::global_state_component_factory *scf = factory->get_state_component_factory();
    if (scf) {
    	xpath_state_component_factory *dscf = new xpath_state_component_factory();
		scf->add_factory(dscf);
    	lib::logger::get_logger()->trace("xpath_state_plugin: registered");
    }
}
