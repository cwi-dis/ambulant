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
	xpath_state_component(ambulant::common::factories* factory);
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

	/// Get subtree (or whole tree) either as XML or as query-string.
	std::string getsubtree(const char *ref, bool as_query);
  private:
  
	// Helper: call relevant state_change_callbacks.
    void _check_state_change(xmlNodePtr changed);

	// Helper: get a node or document, either as xml or query string
	bool _node_content(const char *ref, bool url_encoded, std::string& query);

    // Helper: construct query string for submission.
    std::string _node_as_form_urlencoded(xmlNodePtr node);

    ambulant::common::factories* m_factories;
	xmlDocPtr m_state;
	xmlXPathContextPtr m_context;
	common::state_test_methods *m_state_test_methods;
	std::vector<std::pair<std::string, common::state_change_callback* > > m_state_change_callbacks;
	lib::critical_section m_lock;
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
	if (strcmp((char*)xmlcontext->function, "smil-CPU") == 0 ) {
		std::string rv = stm->smil_cpu();
		valuePush(ctxt, xmlXPathNewString(BAD_CAST rv.c_str()));
	} else
	if (strcmp((char*)xmlcontext->function, "smil-language") == 0 ) {
		xmlChar *arg_str = xmlXPathPopString(ctxt);
		if (arg_str == NULL) goto badarg;
		std::string arg((char*)arg_str);
		xmlFree(arg_str);
		bool rv = (stm->smil_language(arg) > 0);
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
	return;
badarg:
	lib::logger::get_logger()->trace("xpath_state_component: argument error for function %s()", xmlcontext->function);
}

static const char *smil_function_names[] = {
	"smil-audioDesc",
	"smil-bitrate",
	"smil-captions",
	"smil-component",
	"smil-customTest",
	"smil-CPU",
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
	char const **namep;
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
    xpath_state_component_factory(ambulant::common::factories* factory)
    :   m_factories(factory)
    {};
    virtual ~xpath_state_component_factory() {}

	common::state_component *new_state_component(const char *uri);
  protected:
    ambulant::common::factories* m_factories;
};

// -------------------
xpath_state_component::xpath_state_component(ambulant::common::factories* factory)
:	m_factories(factory),
    m_state(NULL),
	m_context(NULL),
	m_state_test_methods(NULL)
{
}

xpath_state_component::~xpath_state_component()
{
	m_lock.enter();
	if (m_state) {
		xmlFreeDoc(m_state);
		m_state = NULL;
	}
	if (m_context) {
		xmlXPathFreeContext(m_context);
		m_context = NULL;
	}
	m_lock.leave();
}

void
xpath_state_component::register_state_test_methods(common::state_test_methods *stm)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::register_state_test_methods(0x%x)", stm);
	m_state_test_methods = stm;
	m_lock.leave();
}

void
xpath_state_component::declare_state(const lib::node *state)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::declare_state(%s)", state->get_sig().c_str());
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
			std::string src_filename = src_url.get_url();
			m_state = xmlReadFile(src_filename.c_str(), NULL, 0);
			if (m_state) {
				// Finally we set up the XPath expression context.
				// NOTE: m_context->node has to be re-set before every evaluation,
				// it seems. Found at <http://mail.gnome.org/archives/xml/2007-November/msg00013.html>.
				m_context = xmlXPathNewContext(m_state);
				m_context->node = xmlDocGetRootElement(m_state);
				m_context->funcLookupFunc = smil_function_lookup;
				m_context->funcLookupData = (void *)this;
				assert(m_context);
				m_lock.leave();
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
				m_lock.leave();
				return;
			}
			// Finally we set up the XPath expression context
			m_context = xmlXPathNewContext(m_state);
			m_context->node = xmlDocGetRootElement(m_state);
			m_context->funcLookupFunc = smil_function_lookup;
			m_context->funcLookupData = (void *)this;
			assert(m_context);
			m_lock.leave();
			return;
		}

	}
	assert(aroot);
	const lib::node *arootnext = aroot->next();
	while (arootnext) {
		if (!arootnext->is_data_node()) {
			lib::logger::get_logger()->trace("xpath_state_component: <state> must have exactly one child");
			m_lock.leave();
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
	m_lock.leave();
}

bool
xpath_state_component::bool_expression(const char *expr)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::bool_expression(%s)", expr);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		m_lock.leave();
		return true;
	}
	m_context->node = xmlDocGetRootElement(m_state);
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST expr, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: cannot evaluate expr=\"%s\"", expr);
		m_lock.leave();
		return true;
	}
	// If we now have a nodeset we try to cast to a string
	if (result->type == XPATH_NODESET) {
		result = xmlXPathConvertString(result);
		if (result == NULL) {
			lib::logger::get_logger()->trace("xpath_state_component: expr=\"%s\": cannot convert to string", expr);
			m_lock.leave();
			return true;
		}
	}
	if (result->type == XPATH_STRING) {
		result = xmlXPathConvertNumber(result);
		if (result == NULL) {
			lib::logger::get_logger()->trace("xpath_state_component: expr=\"%s\": cannot convert to number", expr);
			m_lock.leave();
			return true;
		}
	}
	bool rv = xmlXPathCastToBoolean(result) != 0;
	xmlXPathFreeObject(result);
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::bool_expression(%s) -> %d", expr, (int)rv);
	m_lock.leave();
	return rv;
}

void
xpath_state_component::set_value(const char *var, const char *expr)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::set_value(%s, %s)", var, expr);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		m_lock.leave();
		return;
	}
	// Evaluate the expression, get a string as result
	m_context->node = xmlDocGetRootElement(m_state);
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST expr, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: setvalue: cannot evaluate expr=\"%s\"", expr);
		m_lock.leave();
		return;
	}
	xmlChar *result_str = xmlXPathCastToString(result);
	xmlXPathFreeObject(result);
    result = NULL;
	// Now compute the node-set expression and check that it begets a single node
	m_context->node = xmlDocGetRootElement(m_state);
	result = xmlXPathEvalExpression(BAD_CAST var, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: setvalue: cannot evaluate var=\"%s\"", var);
        xmlFree(result_str);
		m_lock.leave();
		return;
	}
	if (result->type != XPATH_NODESET) {
		lib::logger::get_logger()->trace("xpath_state_component: setvalue: var=\"%s\" is not a node-set", var);
        xmlXPathFreeObject(result);
        xmlFree(result_str);
		m_lock.leave();
		return;
	}
	xmlNodeSetPtr nodeset = result->nodesetval;
	if (nodeset == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: setvalue: var=\"%s\" does not refer to an existing item", var);
        xmlXPathFreeObject(result);
        xmlFree(result_str);
		m_lock.leave();
		return;
	}
	if (nodeset->nodeNr != 1) {
		lib::logger::get_logger()->trace("xpath_state_component: setvalue: var=\"%s\" refers to %d items", var, nodeset->nodeNr);
        xmlXPathFreeObject(result);
        xmlFree(result_str);
		m_lock.leave();
		return;
	}
	// Finally set the value
	xmlNodePtr nodeptr = *nodeset->nodeTab;
	xmlNodeSetContent(nodeptr, result_str);
    xmlXPathFreeObject(result);
	xmlFree(result_str);
    
    // XXXJACK: this looks dangerous, but otherwise we get deadlocks...
	m_lock.leave();
	_check_state_change(nodeptr);
}

void
xpath_state_component::new_value(const char *ref, const char *where, const char *name, const char *expr)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::new_value(ref=%s, where=%s, name=%s, expr=%s)",
		ref, where, name, expr);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		m_lock.leave();
		return;
	}
	// Evaluate the expression, get a string as result
	m_context->node = xmlDocGetRootElement(m_state);
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST expr, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: cannot evaluate expr=\"%s\"", expr);
		m_lock.leave();
		return;
	}
	xmlChar *result_str = xmlXPathCastToString(result);
	xmlXPathFreeObject(result);
	// Now compute the node-set expression and check that it begets a single node.
	// A missing ref is the current node.
	if (ref == NULL) ref = ".";
	m_context->node = xmlDocGetRootElement(m_state);
	result = xmlXPathEvalExpression(BAD_CAST ref, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: cannot evaluate ref=\"%s\"", ref);
		m_lock.leave();
		return;
	}
	if (result->type != XPATH_NODESET) {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: ref=\"%s\" is not a node-set", ref);
		m_lock.leave();
		return;
	}
	xmlNodeSetPtr nodeset = result->nodesetval;
	if (nodeset == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: ref=\"%s\" does not refer to an existing item", ref);
		m_lock.leave();
		return;
	}
	if (nodeset->nodeNr != 1) {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: ref=\"%s\" refers to %d items", ref, nodeset->nodeNr);
		m_lock.leave();
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
			m_lock.leave();
			return;
		}
	} else
	if (where && strcmp(where, "after") == 0) {
		xmlNodePtr rv = xmlAddNextSibling(refnodeptr, newnodeptr);
		if (rv == NULL) {
			lib::logger::get_logger()->trace("xpath_state_component: <newvalue ref=\"%s\" name=\"%s\">: xmlAddNextSibling failed", ref, name);
			m_lock.leave();
			return;
		}
	} else
	if (where == NULL || strcmp(where, "child") == 0) {
		xmlNodePtr rv = xmlAddChild(refnodeptr, newnodeptr);
		if (rv == NULL) {
			lib::logger::get_logger()->trace("xpath_state_component: <newvalue ref=\"%s\" name=\"%s\">: xmlAddChild failed", ref, name);
			m_lock.leave();
			return;
		}
	} else {
		lib::logger::get_logger()->trace("xpath_state_component: newvalue: where=\"%s\": unknown location", where);
		xmlFreeNode(newnodeptr);
		m_lock.leave();
		return;
	}
	// Re-set the context: it may have changed.
    // XXXJACK: this looks dangerous, but otherwise we get deadlocks...
	m_lock.leave();
	_check_state_change(newnodeptr); // XXX Or refnodeptr? both?
}

void
xpath_state_component::del_value(const char *ref)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::del_value(%s)", ref);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		m_lock.leave();
		return;
	}
	// Compute the node-set expression and check that it begets a single node
	m_context->node = xmlDocGetRootElement(m_state);
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST ref, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: delvalue: cannot evaluate ref=\"%s\"", ref);
		m_lock.leave();
		return;
	}
	if (result->type != XPATH_NODESET) {
		lib::logger::get_logger()->trace("xpath_state_component: delvalue: ref=\"%s\" is not a node-set", ref);
		m_lock.leave();
		return;
	}
	xmlNodeSetPtr nodeset = result->nodesetval;
	if (nodeset == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: delvalue: ref=\"%s\" does not refer to an existing item", ref);
		m_lock.leave();
		return;
	}
	if (nodeset->nodeNr != 1) {
		lib::logger::get_logger()->trace("xpath_state_component: delvalue: ref=\"%s\" refers to %d items", ref, nodeset->nodeNr);
		m_lock.leave();
		return;
	}
	xmlNodePtr refnodeptr = *nodeset->nodeTab;
	xmlUnlinkNode(refnodeptr);
	xmlFreeNode(refnodeptr);
	m_lock.leave();
}

void
xpath_state_component::send(const lib::node *submission)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::send(%s)", submission->get_sig().c_str());
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: send: state not initialized");
		m_lock.leave();
		return;
	}
	m_context->node = xmlDocGetRootElement(m_state);
	assert(submission);

	const char *method = submission->get_attribute("method");
	bool is_get = false, is_put = false;
	if (method && strcmp(method, "put") == 0) {
		is_put = true;
	} else if (method && strcmp(method, "get") == 0) {
		is_get = true;
	} else {
		lib::logger::get_logger()->trace("xpath_state_component: send: unknown method=\"%s\"", method);
		m_lock.leave();
		return;
	}
	const char *replace = submission->get_attribute("replace");

	net::url dst_url = submission->get_url("action");
	if (dst_url.is_empty_path()) {
		lib::logger::get_logger()->trace("xpath_state_component: send: submission action attribute missing");
		m_lock.leave();
		return;
	}

	if (dst_url.is_local_file()) {
		if (!is_put) {
			lib::logger::get_logger()->trace("xpath_state_component: send: only method=\"put\" implemented for file: URLs");
			m_lock.leave();
			return;
		}
        if (replace && strcmp(replace, "none") != 0) {
            lib::logger::get_logger()->trace("xpath_state_component: send: only replace=\"none\" implemented");
            m_lock.leave();
            return;
        }
		std::string dst_filename = dst_url.get_file();
		FILE *fp = fopen(dst_filename.c_str(), "w");
		// XXXJACK: we're ignoring ref here...
		xmlDocDump(fp, m_state);
		fclose(fp);
	} else {
		const char *ref = submission->get_attribute("ref");
		std::string query;
		if ( !_node_content(ref, is_get, query)) {
			// No need for error: this is provided by _node_content().
			m_lock.leave();
			return;
		}
		net::url query_url = net::url::from_url(dst_url.get_url() + "?" + query);
        char *data = NULL;
        size_t datasize = 0;
        lib::logger::get_logger()->trace("xpath_state_component: submitting to URL <%s>", query_url.get_url().c_str());
        if (!net::read_data_from_url(query_url, m_factories->get_datasource_factory(), &data, &datasize)) {
            lib::logger::get_logger()->error("%s: Cannot open", query_url.get_url().c_str());
            m_lock.leave();
            return;
        }
        // Lazy programmer: here we could parse data and insert into the tree.
        if (replace && strcmp(replace, "none") != 0) {
            lib::logger::get_logger()->trace("xpath_state_component: send: only replace=\"none\" implemented");
            m_lock.leave();
            return;
        }
        m_lock.leave();
		return;
	}
	m_lock.leave();
}

bool
xpath_state_component::_node_content(const char *ref, bool url_encoded, std::string& query)
{
	// Special case: if we want the whole document as XML we make sure to create a document
	if ((ref == NULL || *ref == '\0') && !url_encoded) {
		xmlChar *data;
		int dataSize;
		xmlDocDumpFormatMemory(m_state, &data, &dataSize, 0);
		query = (char *)data;
		xmlFree(data);
		return true;
	}
	m_context->node = xmlDocGetRootElement(m_state);

	// Get the set of nodes that ref points to, or the root.
	xmlNodePtr refnode;
	if (ref == NULL || *ref == '\0') {
		refnode = m_context->node; // The root
	} else {
		xmlXPathObjectPtr refobj = xmlXPathEvalExpression(BAD_CAST ref, m_context);
		if (refobj == NULL) {
			lib::logger::get_logger()->trace("xpath_state_component: send: cannot evaluate ref=\"%s\"", ref);
			return false;
		}
		if (refobj->type != XPATH_NODESET) {
			lib::logger::get_logger()->trace("xpath_state_component: send: ref=\"%s\" is not a node-set", ref);
			return false;
		}
		xmlNodeSetPtr nodeset = refobj->nodesetval;
		if (nodeset == NULL) {
			lib::logger::get_logger()->trace("xpath_state_component: send: ref=\"%s\" does not refer to an existing item", ref);
			return false;
		}
		if (nodeset->nodeNr != 1) {
			lib::logger::get_logger()->trace("xpath_state_component: setvalue: var=\"%s\" refers to %d items", ref, nodeset->nodeNr);
			return false;
		}
		// Finally set the value
		refnode = *nodeset->nodeTab;
	}
	assert(refnode);

	if (url_encoded) {
		query = _node_as_form_urlencoded(refnode);
		return true;
	} else {
		xmlBufferPtr buf = xmlBufferCreate();
		xmlNodeDump(buf, m_state, refnode, 0, 0);
		query = (char *)xmlBufferContent(buf);
		xmlBufferFree(buf);
		return true;
	}
}

std::string
xpath_state_component::getsubtree(const char *ref, bool as_query)
{
	std::string rv;
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::getsubtree(%s, %d)", ref, as_query);
	(void)_node_content(ref, as_query, rv);
	m_lock.leave();
	return rv;
}

std::string
xpath_state_component::string_expression(const char *expr)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::string_expression(%s)", expr);
	if (m_state == NULL || m_context == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: state not initialized");
		m_lock.leave();
		return "";
	}
	m_context->node = xmlDocGetRootElement(m_state);
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST expr, m_context);
	if (result == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: cannot evaluate \"{%s}\"", expr);
		m_lock.leave();
		return "";
	}
	// Sanity check, for document authors
	if (result->type == XPATH_NODESET && xmlXPathNodeSetGetLength(result->nodesetval) != 1) {
		lib::logger::get_logger()->trace("xpath_state_component: \"{%s}\": evaluates to %d nodes", expr, xmlXPathNodeSetGetLength(result->nodesetval));
	}
	xmlChar *result_str = xmlXPathCastToString(result);
	xmlXPathFreeObject(result);
	if (result_str == NULL) {
		lib::logger::get_logger()->trace("xpath_state_component: \"{%s}\" does not evaluate to a string", expr);
		m_lock.leave();
		return "";
	}
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::string_expression(%s) -> %s", expr, (void*)result_str);
	std::string rv((char *)result_str);
	xmlFree(result_str);
	m_lock.leave();
	return rv;
}

void
xpath_state_component::want_state_change(const char *ref, common::state_change_callback *cb)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("xpath_state_component::want_state_change(%s)", ref);
	// Test it is actually a nodeset
	m_context->node = xmlDocGetRootElement(m_state);
	xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST ref, m_context);
	if (result == NULL || result->type != XPATH_NODESET) {
		lib::logger::get_logger()->trace("xpath_state: want_state_change: expression %s is not a nodeset", ref);
	} else {
		std::pair<std::string, common::state_change_callback*> item(ref, cb);
		m_state_change_callbacks.push_back(item);
	}
    xmlXPathFreeObject(result);
	m_lock.leave();
}

void
xpath_state_component::_check_state_change(xmlNodePtr changed)
{
	typedef std::pair<common::state_change_callback*, std::string> todo;
	typedef std::list<todo> todolisttype;
	todolisttype todolist;
	m_lock.enter();
	std::vector<std::pair<std::string, common::state_change_callback* > >::iterator i;
	AM_DBG lib::logger::get_logger()->debug("_check_state_change()");
	for (i=m_state_change_callbacks.begin(); i != m_state_change_callbacks.end(); i++) {
		std::string& ref = (*i).first;
		AM_DBG lib::logger::get_logger()->debug("check_state_change: check for \"%s\"", ref.c_str());
		m_context->node = xmlDocGetRootElement(m_state);
		xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST ref.c_str(), m_context);
		AM_DBG lib::logger::get_logger()->debug("... result=0x%x, type=%d", result, result?result->type:0);
		if (result == NULL || result->type != XPATH_NODESET) {
			/* nothing*/;
		}
		if (result != NULL && result->type == XPATH_NODESET) {
			xmlNodeSetPtr nodeset = result->nodesetval;
			AM_DBG lib::logger::get_logger()->debug("... nodeset=0x%x count=%d", nodeset, nodeset?nodeset->nodeNr:0);
			if (nodeset) {
				int	 j;
				for (j=0; j<nodeset->nodeNr; j++) {
					if (nodeset->nodeTab[j] == changed) {
						AM_DBG lib::logger::get_logger()->debug("check_state_change: raising stateChange(%s)", ref.c_str());
						common::state_change_callback *cb = (*i).second;
						assert(cb);
						todolist.push_back(todo(cb, ref));
						break;
					}
				}
			}
		}
        xmlXPathFreeObject(result);
	}
	m_lock.leave();
	todolisttype::iterator t;
	for(t=todolist.begin(); t != todolist.end(); t++) {
		((*t).first)->on_state_change((*t).second.c_str());
	}
}

// Helper function: get data from a subtree and return it as an application/x-www-form-urlencoded string
std::string
xpath_state_component::_node_as_form_urlencoded(xmlNodePtr node)
{
	std::string rv;
	std::string node_data;
    if (xmlChildElementCount(node) == 0)
        node_data = (char *)xmlNodeGetContent(node);
	std::string node_name = (char *)node->name;
	if (node_data != "") {
		rv = node_name + "=" + node_data;
	}
	xmlNodePtr child;
	for(child=xmlFirstElementChild(node); child; child=xmlNextElementSibling(child)) {
		std::string nextvalue = _node_as_form_urlencoded(child);
		if (nextvalue != "") {
			if (rv != "") {
				rv = rv + "&" + nextvalue;
			} else {
				rv = nextvalue;
			}
		}
	}
	return rv;
}

// -------------------
common::state_component *
xpath_state_component_factory::new_state_component(const char *uri)
{
	if (strcmp(uri, "http://www.w3.org/TR/1999/REC-xpath-19991116") == 0) {
		AM_DBG lib::logger::get_logger()->debug("xpath_state_component_factory::new_state_component: returned state_component");
		return new xpath_state_component(m_factories);
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
		lib::logger::get_logger()->warn(gettext("%s: built for plugin-api version %d, current %d. Skipping."),"xpath_state_plugin", AMBULANT_PLUGIN_API_VERSION, api_version);
		return;
	}
	if ( !ambulant::check_version() ) {
		lib::logger::get_logger()->warn(gettext("%s: built for different Ambulant version (%s)"),"xpath_state_plugin", AMBULANT_VERSION);
	}
	factory = bug_workaround(factory);
	AM_DBG lib::logger::get_logger()->debug("xpath_state_plugin: loaded.");
	common::global_state_component_factory *scf = factory->get_state_component_factory();
	if (scf) {
		xpath_state_component_factory *dscf = new xpath_state_component_factory(factory);
		scf->add_factory(dscf);
		lib::logger::get_logger()->trace("xpath_state_plugin: registered");
	}
}

#ifdef WITH_STATIC_PLUGINS
class regplugin {
  public:
    regplugin() {
        register_static_plugin(initialize);
    }
};

static regplugin regplugin;
#endif
