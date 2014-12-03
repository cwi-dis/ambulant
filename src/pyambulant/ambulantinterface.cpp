
/* ================== Callbacks Module pyambulant =================== */


#define CPP_TO_PYTHON_BRIDGE 1

#include "ambulantinterface.h"
#include "ambulantutilities.h"
#include "ambulantmodule.h"


// The Python interface does not qualify strings with const, so we have to
// disable warnings about non-writeable strings (zillions of them)

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

extern PyObject *audio_format_choicesObj_New(const ambulant::net::audio_format_choices *itself);
extern int audio_format_choicesObj_Convert(PyObject *v, ambulant::net::audio_format_choices *p_itself);


/* ------------------------- Class ostream -------------------------- */

ostream::ostream(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "is_open")) PyErr_Warn(PyExc_Warning, "ostream: missing attribute: is_open");
		if (!PyObject_HasAttrString(itself, "close")) PyErr_Warn(PyExc_Warning, "ostream: missing attribute: close");
		if (!PyObject_HasAttrString(itself, "write")) PyErr_Warn(PyExc_Warning, "ostream: missing attribute: write");
		if (!PyObject_HasAttrString(itself, "write")) PyErr_Warn(PyExc_Warning, "ostream: missing attribute: write");
		if (!PyObject_HasAttrString(itself, "flush")) PyErr_Warn(PyExc_Warning, "ostream: missing attribute: flush");
	}
	if (itself == NULL) itself = Py_None;

	py_ostream = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

ostream::~ostream()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_ostream;
	py_ostream = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


bool ostream::is_open() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_ostream, "is_open", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during ostream::is_open() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during ostream::is_open() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void ostream::close()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_ostream, "close", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during ostream::close() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

int ostream::write(const unsigned char * buffer, int nbytes)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	int _rv;
	PyObject *py_buffer = Py_BuildValue("s", buffer);
	PyObject *py_nbytes = Py_BuildValue("i", nbytes);

	PyObject *py_rv = PyObject_CallMethod(py_ostream, "write", "(OO)", py_buffer, py_nbytes);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during ostream::write() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "i", &_rv))
	{
		PySys_WriteStderr("Python exception during ostream::write() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_buffer);
	Py_XDECREF(py_nbytes);

	PyGILState_Release(_GILState);
	return _rv;
}

int ostream::write(const char* cstr)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	int _rv;
	PyObject *py_cstr = Py_BuildValue("s", cstr);

	PyObject *py_rv = PyObject_CallMethod(py_ostream, "write", "(O)", py_cstr);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during ostream::write() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "i", &_rv))
	{
		PySys_WriteStderr("Python exception during ostream::write() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_cstr);

	PyGILState_Release(_GILState);
	return _rv;
}

void ostream::flush()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_ostream, "flush", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during ostream::flush() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

/* ----------------------- Class node_context ----------------------- */

node_context::node_context(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "set_prefix_mapping")) PyErr_Warn(PyExc_Warning, "node_context: missing attribute: set_prefix_mapping");
		if (!PyObject_HasAttrString(itself, "get_namespace_prefix")) PyErr_Warn(PyExc_Warning, "node_context: missing attribute: get_namespace_prefix");
		if (!PyObject_HasAttrString(itself, "is_supported_prefix")) PyErr_Warn(PyExc_Warning, "node_context: missing attribute: is_supported_prefix");
		if (!PyObject_HasAttrString(itself, "resolve_url")) PyErr_Warn(PyExc_Warning, "node_context: missing attribute: resolve_url");
		if (!PyObject_HasAttrString(itself, "get_root")) PyErr_Warn(PyExc_Warning, "node_context: missing attribute: get_root");
		if (!PyObject_HasAttrString(itself, "get_node")) PyErr_Warn(PyExc_Warning, "node_context: missing attribute: get_node");
		if (!PyObject_HasAttrString(itself, "get_state")) PyErr_Warn(PyExc_Warning, "node_context: missing attribute: get_state");
		if (!PyObject_HasAttrString(itself, "apply_avt")) PyErr_Warn(PyExc_Warning, "node_context: missing attribute: apply_avt");
	}
	if (itself == NULL) itself = Py_None;

	py_node_context = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

node_context::~node_context()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_node_context;
	py_node_context = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void node_context::set_prefix_mapping(const std::string& prefix, const std::string& uri)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_prefix = Py_BuildValue("s", prefix.c_str());
	PyObject *py_uri = Py_BuildValue("s", uri.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_node_context, "set_prefix_mapping", "(OO)", py_prefix, py_uri);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node_context::set_prefix_mapping() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_prefix);
	Py_XDECREF(py_uri);

	PyGILState_Release(_GILState);
}

const ambulant::lib::xml_string& node_context::get_namespace_prefix(const ambulant::lib::xml_string& uri) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::xml_string get_namespace_prefix;
	PyObject *py_uri = Py_BuildValue("s", uri.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_node_context, "get_namespace_prefix", "(O)", py_uri);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node_context::get_namespace_prefix() callback:\n");
		PyErr_Print();
	}

	char *get_namespace_prefix_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &get_namespace_prefix_cstr))
	{
		PySys_WriteStderr("Python exception during node_context::get_namespace_prefix() return:\n");
		PyErr_Print();
	}

	get_namespace_prefix = get_namespace_prefix_cstr;
	Py_XDECREF(py_rv);
	Py_XDECREF(py_uri);

	PyGILState_Release(_GILState);
	const_cast<node_context *>(this)->get_namespace_prefix_rvkeepref = get_namespace_prefix;
	return get_namespace_prefix_rvkeepref;
}

bool node_context::is_supported_prefix(const ambulant::lib::xml_string& prefix) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_prefix = Py_BuildValue("s", prefix.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_node_context, "is_supported_prefix", "(O)", py_prefix);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node_context::is_supported_prefix() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node_context::is_supported_prefix() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_prefix);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::net::url node_context::resolve_url(const ambulant::net::url& rurl) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::net::url _rv;
	PyObject *py_rurl = Py_BuildValue("O", ambulant_url_New(rurl));

	PyObject *py_rv = PyObject_CallMethod(py_node_context, "resolve_url", "(O)", py_rurl);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node_context::resolve_url() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_url_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node_context::resolve_url() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_rurl);

	PyGILState_Release(_GILState);
	return _rv;
}

const ambulant::lib::node* node_context::get_root() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node_context, "get_root", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node_context::get_root() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node_context::get_root() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

const ambulant::lib::node* node_context::get_node(const std::string& idd) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const ambulant::lib::node* _rv;
	PyObject *py_idd = Py_BuildValue("s", idd.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_node_context, "get_node", "(O)", py_idd);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node_context::get_node() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node_context::get_node() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_idd);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::state_component* node_context::get_state() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::state_component* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node_context, "get_state", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node_context::get_state() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", state_componentObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node_context::get_state() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

const ambulant::lib::xml_string& node_context::apply_avt(const ambulant::lib::node* n, const ambulant::lib::xml_string& attrname, const ambulant::lib::xml_string& attrvalue) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::xml_string apply_avt;
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);
	PyObject *py_attrname = Py_BuildValue("s", attrname.c_str());
	PyObject *py_attrvalue = Py_BuildValue("s", attrvalue.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_node_context, "apply_avt", "(OOO)", py_n, py_attrname, py_attrvalue);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node_context::apply_avt() callback:\n");
		PyErr_Print();
	}

	char *apply_avt_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &apply_avt_cstr))
	{
		PySys_WriteStderr("Python exception during node_context::apply_avt() return:\n");
		PyErr_Print();
	}

	apply_avt = apply_avt_cstr;
	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);
	Py_XDECREF(py_attrname);
	Py_XDECREF(py_attrvalue);

	PyGILState_Release(_GILState);
	const_cast<node_context *>(this)->apply_avt_rvkeepref = apply_avt;
	return apply_avt_rvkeepref;
}

/* --------------------------- Class node --------------------------- */

node::node(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "down")) PyErr_Warn(PyExc_Warning, "node: missing attribute: down");
		if (!PyObject_HasAttrString(itself, "up")) PyErr_Warn(PyExc_Warning, "node: missing attribute: up");
		if (!PyObject_HasAttrString(itself, "next")) PyErr_Warn(PyExc_Warning, "node: missing attribute: next");
		if (!PyObject_HasAttrString(itself, "down")) PyErr_Warn(PyExc_Warning, "node: missing attribute: down");
		if (!PyObject_HasAttrString(itself, "up")) PyErr_Warn(PyExc_Warning, "node: missing attribute: up");
		if (!PyObject_HasAttrString(itself, "next")) PyErr_Warn(PyExc_Warning, "node: missing attribute: next");
		if (!PyObject_HasAttrString(itself, "down")) PyErr_Warn(PyExc_Warning, "node: missing attribute: down");
		if (!PyObject_HasAttrString(itself, "up")) PyErr_Warn(PyExc_Warning, "node: missing attribute: up");
		if (!PyObject_HasAttrString(itself, "next")) PyErr_Warn(PyExc_Warning, "node: missing attribute: next");
		if (!PyObject_HasAttrString(itself, "previous")) PyErr_Warn(PyExc_Warning, "node: missing attribute: previous");
		if (!PyObject_HasAttrString(itself, "get_last_child")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_last_child");
		if (!PyObject_HasAttrString(itself, "locate_node")) PyErr_Warn(PyExc_Warning, "node: missing attribute: locate_node");
		if (!PyObject_HasAttrString(itself, "get_first_child")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_first_child");
		if (!PyObject_HasAttrString(itself, "get_first_child")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_first_child");
		if (!PyObject_HasAttrString(itself, "get_root")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_root");
		if (!PyObject_HasAttrString(itself, "get_container_attribute")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_container_attribute");
		if (!PyObject_HasAttrString(itself, "append_child")) PyErr_Warn(PyExc_Warning, "node: missing attribute: append_child");
		if (!PyObject_HasAttrString(itself, "append_child")) PyErr_Warn(PyExc_Warning, "node: missing attribute: append_child");
		if (!PyObject_HasAttrString(itself, "detach")) PyErr_Warn(PyExc_Warning, "node: missing attribute: detach");
		if (!PyObject_HasAttrString(itself, "clone")) PyErr_Warn(PyExc_Warning, "node: missing attribute: clone");
		if (!PyObject_HasAttrString(itself, "append_data")) PyErr_Warn(PyExc_Warning, "node: missing attribute: append_data");
		if (!PyObject_HasAttrString(itself, "append_data")) PyErr_Warn(PyExc_Warning, "node: missing attribute: append_data");
		if (!PyObject_HasAttrString(itself, "append_data")) PyErr_Warn(PyExc_Warning, "node: missing attribute: append_data");
		if (!PyObject_HasAttrString(itself, "set_attribute")) PyErr_Warn(PyExc_Warning, "node: missing attribute: set_attribute");
		if (!PyObject_HasAttrString(itself, "set_attribute")) PyErr_Warn(PyExc_Warning, "node: missing attribute: set_attribute");
		if (!PyObject_HasAttrString(itself, "set_prefix_mapping")) PyErr_Warn(PyExc_Warning, "node: missing attribute: set_prefix_mapping");
		if (!PyObject_HasAttrString(itself, "get_namespace")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_namespace");
		if (!PyObject_HasAttrString(itself, "get_local_name")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_local_name");
		if (!PyObject_HasAttrString(itself, "get_qname")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_qname");
		if (!PyObject_HasAttrString(itself, "get_numid")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_numid");
		if (!PyObject_HasAttrString(itself, "get_data")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_data");
		if (!PyObject_HasAttrString(itself, "is_data_node")) PyErr_Warn(PyExc_Warning, "node: missing attribute: is_data_node");
		if (!PyObject_HasAttrString(itself, "get_trimmed_data")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_trimmed_data");
		if (!PyObject_HasAttrString(itself, "get_attribute")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_attribute");
		if (!PyObject_HasAttrString(itself, "get_attribute")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_attribute");
		if (!PyObject_HasAttrString(itself, "del_attribute")) PyErr_Warn(PyExc_Warning, "node: missing attribute: del_attribute");
		if (!PyObject_HasAttrString(itself, "get_url")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_url");
		if (!PyObject_HasAttrString(itself, "size")) PyErr_Warn(PyExc_Warning, "node: missing attribute: size");
		if (!PyObject_HasAttrString(itself, "get_xpath")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_xpath");
		if (!PyObject_HasAttrString(itself, "get_sig")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_sig");
		if (!PyObject_HasAttrString(itself, "xmlrepr")) PyErr_Warn(PyExc_Warning, "node: missing attribute: xmlrepr");
		if (!PyObject_HasAttrString(itself, "get_context")) PyErr_Warn(PyExc_Warning, "node: missing attribute: get_context");
		if (!PyObject_HasAttrString(itself, "set_context")) PyErr_Warn(PyExc_Warning, "node: missing attribute: set_context");
		if (!PyObject_HasAttrString(itself, "has_debug")) PyErr_Warn(PyExc_Warning, "node: missing attribute: has_debug");
	}
	if (itself == NULL) itself = Py_None;

	py_node = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

node::~node()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_node;
	py_node = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


const ambulant::lib::node* node::down() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "down", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::down() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::down() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

const ambulant::lib::node* node::up() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "up", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::up() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::up() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

const ambulant::lib::node* node::next() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "next", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::next() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::next() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node::down()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "down", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::down() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::down() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node::up()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "up", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::up() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::up() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node::next()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "next", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::next() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::next() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void node::down(ambulant::lib::node* n)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);

	PyObject *py_rv = PyObject_CallMethod(py_node, "down", "(O)", py_n);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::down() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);

	PyGILState_Release(_GILState);
}

void node::up(ambulant::lib::node* n)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);

	PyObject *py_rv = PyObject_CallMethod(py_node, "up", "(O)", py_n);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::up() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);

	PyGILState_Release(_GILState);
}

void node::next(ambulant::lib::node* n)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);

	PyObject *py_rv = PyObject_CallMethod(py_node, "next", "(O)", py_n);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::next() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);

	PyGILState_Release(_GILState);
}

const ambulant::lib::node* node::previous() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "previous", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::previous() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::previous() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

const ambulant::lib::node* node::get_last_child() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_last_child", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_last_child() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::get_last_child() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node::locate_node(const char* path)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;
	PyObject *py_path = Py_BuildValue("s", path);

	PyObject *py_rv = PyObject_CallMethod(py_node, "locate_node", "(O)", py_path);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::locate_node() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::locate_node() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_path);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node::get_first_child(const char* name)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;
	PyObject *py_name = Py_BuildValue("s", name);

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_first_child", "(O)", py_name);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_first_child() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::get_first_child() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);

	PyGILState_Release(_GILState);
	return _rv;
}

const ambulant::lib::node* node::get_first_child(const char* name) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const ambulant::lib::node* _rv;
	PyObject *py_name = Py_BuildValue("s", name);

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_first_child", "(O)", py_name);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_first_child() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::get_first_child() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node::get_root()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_root", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_root() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::get_root() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

const char * node::get_container_attribute(const char* name) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const char * _rv;
	PyObject *py_name = Py_BuildValue("s", name);

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_container_attribute", "(O)", py_name);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_container_attribute() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "z", &_rv))
	{
		PySys_WriteStderr("Python exception during node::get_container_attribute() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node::append_child(ambulant::lib::node* child)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;
	PyObject *py_child = Py_BuildValue("O&", nodeObj_New, child);

	PyObject *py_rv = PyObject_CallMethod(py_node, "append_child", "(O)", py_child);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::append_child() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::append_child() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_child);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node::append_child(const char* name)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;
	PyObject *py_name = Py_BuildValue("s", name);

	PyObject *py_rv = PyObject_CallMethod(py_node, "append_child", "(O)", py_name);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::append_child() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::append_child() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node::detach()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "detach", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::detach() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::detach() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node::clone() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "clone", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::clone() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::clone() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void node::append_data(const char *data__in__, size_t data__len__)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_data = Py_BuildValue("s#", data__in__, (int)data__len__);

	PyObject *py_rv = PyObject_CallMethod(py_node, "append_data", "(O)", py_data);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::append_data() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_data);

	PyGILState_Release(_GILState);
}

void node::append_data(const char* c_str)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_c_str = Py_BuildValue("s", c_str);

	PyObject *py_rv = PyObject_CallMethod(py_node, "append_data", "(O)", py_c_str);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::append_data() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_c_str);

	PyGILState_Release(_GILState);
}

void node::append_data(const ambulant::lib::xml_string& str)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_str = Py_BuildValue("s", str.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_node, "append_data", "(O)", py_str);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::append_data() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_str);

	PyGILState_Release(_GILState);
}

void node::set_attribute(const char* name, const char* value)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_name = Py_BuildValue("s", name);
	PyObject *py_value = Py_BuildValue("s", value);

	PyObject *py_rv = PyObject_CallMethod(py_node, "set_attribute", "(OO)", py_name, py_value);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::set_attribute() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);
	Py_XDECREF(py_value);

	PyGILState_Release(_GILState);
}

void node::set_attribute(const char* name, const ambulant::lib::xml_string& value)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_name = Py_BuildValue("s", name);
	PyObject *py_value = Py_BuildValue("s", value.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_node, "set_attribute", "(OO)", py_name, py_value);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::set_attribute() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);
	Py_XDECREF(py_value);

	PyGILState_Release(_GILState);
}

void node::set_prefix_mapping(const std::string& prefix, const std::string& uri)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_prefix = Py_BuildValue("s", prefix.c_str());
	PyObject *py_uri = Py_BuildValue("s", uri.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_node, "set_prefix_mapping", "(OO)", py_prefix, py_uri);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::set_prefix_mapping() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_prefix);
	Py_XDECREF(py_uri);

	PyGILState_Release(_GILState);
}

const ambulant::lib::xml_string& node::get_namespace() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::xml_string get_namespace;

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_namespace", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_namespace() callback:\n");
		PyErr_Print();
	}

	char *get_namespace_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &get_namespace_cstr))
	{
		PySys_WriteStderr("Python exception during node::get_namespace() return:\n");
		PyErr_Print();
	}

	get_namespace = get_namespace_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	const_cast<node *>(this)->get_namespace_rvkeepref = get_namespace;
	return get_namespace_rvkeepref;
}

const ambulant::lib::xml_string& node::get_local_name() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::xml_string get_local_name;

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_local_name", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_local_name() callback:\n");
		PyErr_Print();
	}

	char *get_local_name_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &get_local_name_cstr))
	{
		PySys_WriteStderr("Python exception during node::get_local_name() return:\n");
		PyErr_Print();
	}

	get_local_name = get_local_name_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	const_cast<node *>(this)->get_local_name_rvkeepref = get_local_name;
	return get_local_name_rvkeepref;
}

const ambulant::lib::q_name_pair& node::get_qname() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::q_name_pair get_qname;

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_qname", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_qname() callback:\n");
		PyErr_Print();
	}

	ambulant::lib::xml_string get_qname_first;
	ambulant::lib::xml_string get_qname_second;
	char *get_qname_first_cstr="";
	char *get_qname_second_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "(ss)", &get_qname_first_cstr, &get_qname_second_cstr))
	{
		PySys_WriteStderr("Python exception during node::get_qname() return:\n");
		PyErr_Print();
	}

	get_qname_first = get_qname_first_cstr;
	get_qname_second = get_qname_second_cstr;
	get_qname = ambulant::lib::q_name_pair(get_qname_first, get_qname_second);
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	const_cast<node *>(this)->get_qname_rvkeepref = get_qname;
	return get_qname_rvkeepref;
}

int node::get_numid() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	int _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_numid", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_numid() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "i", &_rv))
	{
		PySys_WriteStderr("Python exception during node::get_numid() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

const ambulant::lib::xml_string& node::get_data() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::xml_string get_data;

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_data", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_data() callback:\n");
		PyErr_Print();
	}

	char *get_data_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &get_data_cstr))
	{
		PySys_WriteStderr("Python exception during node::get_data() return:\n");
		PyErr_Print();
	}

	get_data = get_data_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	const_cast<node *>(this)->get_data_rvkeepref = get_data;
	return get_data_rvkeepref;
}

bool node::is_data_node() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "is_data_node", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::is_data_node() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::is_data_node() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::xml_string node::get_trimmed_data() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::xml_string _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_trimmed_data", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_trimmed_data() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during node::get_trimmed_data() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

const char * node::get_attribute(const char* name) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const char * _rv;
	PyObject *py_name = Py_BuildValue("s", name);

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_attribute", "(O)", py_name);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_attribute() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "z", &_rv))
	{
		PySys_WriteStderr("Python exception during node::get_attribute() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);

	PyGILState_Release(_GILState);
	return _rv;
}

const char * node::get_attribute(const std::string& name) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const char * _rv;
	PyObject *py_name = Py_BuildValue("s", name.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_attribute", "(O)", py_name);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_attribute() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "z", &_rv))
	{
		PySys_WriteStderr("Python exception during node::get_attribute() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);

	PyGILState_Release(_GILState);
	return _rv;
}

void node::del_attribute(const char* name)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_name = Py_BuildValue("s", name);

	PyObject *py_rv = PyObject_CallMethod(py_node, "del_attribute", "(O)", py_name);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::del_attribute() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);

	PyGILState_Release(_GILState);
}

ambulant::net::url node::get_url(const char* attrname) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::net::url _rv;
	PyObject *py_attrname = Py_BuildValue("s", attrname);

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_url", "(O)", py_attrname);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_url() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_url_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::get_url() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_attrname);

	PyGILState_Release(_GILState);
	return _rv;
}

unsigned int node::size() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	unsigned int _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "size", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::size() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during node::size() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

std::string node::get_xpath() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	std::string _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_xpath", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_xpath() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during node::get_xpath() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

std::string node::get_sig() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	std::string _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_sig", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_sig() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during node::get_sig() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::xml_string node::xmlrepr() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::xml_string _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "xmlrepr", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::xmlrepr() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during node::xmlrepr() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

const ambulant::lib::node_context* node::get_context() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const ambulant::lib::node_context* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_node, "get_context", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::get_context() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", node_contextObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node::get_context() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void node::set_context(ambulant::lib::node_context* c)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_c = Py_BuildValue("O&", node_contextObj_New, c);

	PyObject *py_rv = PyObject_CallMethod(py_node, "set_context", "(O)", py_c);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node::set_context() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_c);

	PyGILState_Release(_GILState);
}

/* ----------------------- Class node_factory ----------------------- */

node_factory::node_factory(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_node")) PyErr_Warn(PyExc_Warning, "node_factory: missing attribute: new_node");
		if (!PyObject_HasAttrString(itself, "new_node")) PyErr_Warn(PyExc_Warning, "node_factory: missing attribute: new_node");
		if (!PyObject_HasAttrString(itself, "new_data_node")) PyErr_Warn(PyExc_Warning, "node_factory: missing attribute: new_data_node");
	}
	if (itself == NULL) itself = Py_None;

	py_node_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

node_factory::~node_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_node_factory;
	py_node_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::lib::node* node_factory::new_node(const ambulant::lib::q_name_pair& qn, const ambulant::lib::q_attributes_list& qattrs, const ambulant::lib::node_context* ctx)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;
	PyObject *py_qn = Py_BuildValue("(ss)", qn.first.c_str(), qn.second.c_str());
	PyObject *py_qattrs = Py_BuildValue("O", ambulant_attributes_list_New(qattrs));
	PyObject *py_ctx = Py_BuildValue("O&", node_contextObj_New, ctx);

	PyObject *py_rv = PyObject_CallMethod(py_node_factory, "new_node", "(OOO)", py_qn, py_qattrs, py_ctx);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node_factory::new_node() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node_factory::new_node() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_qn);
	Py_XDECREF(py_qattrs);
	Py_XDECREF(py_ctx);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node_factory::new_node(const ambulant::lib::node* other)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;
	PyObject *py_other = Py_BuildValue("O&", nodeObj_New, other);

	PyObject *py_rv = PyObject_CallMethod(py_node_factory, "new_node", "(O)", py_other);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node_factory::new_node() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node_factory::new_node() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_other);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node* node_factory::new_data_node(const char *data__in__, size_t data__len__, const ambulant::lib::node_context* ctx)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node* _rv;
	PyObject *py_data = Py_BuildValue("s#", data__in__, (int)data__len__);
	PyObject *py_ctx = Py_BuildValue("O&", node_contextObj_New, ctx);

	PyObject *py_rv = PyObject_CallMethod(py_node_factory, "new_data_node", "(OO)", py_data, py_ctx);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during node_factory::new_data_node() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", nodeObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during node_factory::new_data_node() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_data);
	Py_XDECREF(py_ctx);

	PyGILState_Release(_GILState);
	return _rv;
}

/* -------------------------- Class event --------------------------- */

event::event(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "fire")) PyErr_Warn(PyExc_Warning, "event: missing attribute: fire");
	}
	if (itself == NULL) itself = Py_None;

	py_event = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

event::~event()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_event;
	py_event = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void event::fire()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_event, "fire", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during event::fire() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

/* --------------------- Class event_processor ---------------------- */

event_processor::event_processor(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "add_event")) PyErr_Warn(PyExc_Warning, "event_processor: missing attribute: add_event");
		if (!PyObject_HasAttrString(itself, "cancel_all_events")) PyErr_Warn(PyExc_Warning, "event_processor: missing attribute: cancel_all_events");
		if (!PyObject_HasAttrString(itself, "cancel_event")) PyErr_Warn(PyExc_Warning, "event_processor: missing attribute: cancel_event");
		if (!PyObject_HasAttrString(itself, "get_timer")) PyErr_Warn(PyExc_Warning, "event_processor: missing attribute: get_timer");
	}
	if (itself == NULL) itself = Py_None;

	py_event_processor = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

event_processor::~event_processor()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_event_processor;
	py_event_processor = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void event_processor::add_event(ambulant::lib::event* pe, ambulant::lib::timer::time_type t, ambulant::lib::event_priority priority)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_pe = Py_BuildValue("O&", eventObj_New, pe);
	PyObject *py_t = Py_BuildValue("l", t);
	PyObject *py_priority = Py_BuildValue("l", priority);

	PyObject *py_rv = PyObject_CallMethod(py_event_processor, "add_event", "(OOO)", py_pe, py_t, py_priority);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during event_processor::add_event() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_pe);
	Py_XDECREF(py_t);
	Py_XDECREF(py_priority);

	PyGILState_Release(_GILState);
}

void event_processor::cancel_all_events()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_event_processor, "cancel_all_events", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during event_processor::cancel_all_events() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

bool event_processor::cancel_event(ambulant::lib::event* pe, ambulant::lib::event_priority priority)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_pe = Py_BuildValue("O&", eventObj_New, pe);
	PyObject *py_priority = Py_BuildValue("l", priority);

	PyObject *py_rv = PyObject_CallMethod(py_event_processor, "cancel_event", "(OO)", py_pe, py_priority);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during event_processor::cancel_event() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during event_processor::cancel_event() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_pe);
	Py_XDECREF(py_priority);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::timer* event_processor::get_timer() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::timer* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_event_processor, "get_timer", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during event_processor::get_timer() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", timerObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during event_processor::get_timer() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ---------------------- Class parser_factory ---------------------- */

parser_factory::parser_factory(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "get_parser_name")) PyErr_Warn(PyExc_Warning, "parser_factory: missing attribute: get_parser_name");
	}
	if (itself == NULL) itself = Py_None;

	py_parser_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

parser_factory::~parser_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_parser_factory;
	py_parser_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


std::string parser_factory::get_parser_name()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	std::string _rv;

	PyObject *py_rv = PyObject_CallMethod(py_parser_factory, "get_parser_name", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during parser_factory::get_parser_name() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during parser_factory::get_parser_name() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ------------------------ Class xml_parser ------------------------ */

xml_parser::xml_parser(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "parse")) PyErr_Warn(PyExc_Warning, "xml_parser: missing attribute: parse");
	}
	if (itself == NULL) itself = Py_None;

	py_xml_parser = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

xml_parser::~xml_parser()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_xml_parser;
	py_xml_parser = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


bool xml_parser::parse(const char *buf__in__, size_t buf__len__, bool final)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_buf = Py_BuildValue("s#", buf__in__, (int)buf__len__);
	PyObject *py_final = Py_BuildValue("O&", bool_New, final);

	PyObject *py_rv = PyObject_CallMethod(py_xml_parser, "parse", "(OO)", py_buf, py_final);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during xml_parser::parse() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during xml_parser::parse() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_buf);
	Py_XDECREF(py_final);

	PyGILState_Release(_GILState);
	return _rv;
}

/* --------------------- Class system_embedder ---------------------- */

system_embedder::system_embedder(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "show_file")) PyErr_Warn(PyExc_Warning, "system_embedder: missing attribute: show_file");
	}
	if (itself == NULL) itself = Py_None;

	py_system_embedder = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

system_embedder::~system_embedder()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_system_embedder;
	py_system_embedder = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void system_embedder::show_file(const ambulant::net::url& href)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_href = Py_BuildValue("O", ambulant_url_New(href));

	PyObject *py_rv = PyObject_CallMethod(py_system_embedder, "show_file", "(O)", py_href);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during system_embedder::show_file() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_href);

	PyGILState_Release(_GILState);
}

/* -------------------------- Class timer --------------------------- */

timer::timer(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "elapsed")) PyErr_Warn(PyExc_Warning, "timer: missing attribute: elapsed");
		if (!PyObject_HasAttrString(itself, "get_realtime_speed")) PyErr_Warn(PyExc_Warning, "timer: missing attribute: get_realtime_speed");
		if (!PyObject_HasAttrString(itself, "set_drift")) PyErr_Warn(PyExc_Warning, "timer: missing attribute: set_drift");
		if (!PyObject_HasAttrString(itself, "get_drift")) PyErr_Warn(PyExc_Warning, "timer: missing attribute: get_drift");
		if (!PyObject_HasAttrString(itself, "skew")) PyErr_Warn(PyExc_Warning, "timer: missing attribute: skew");
		if (!PyObject_HasAttrString(itself, "running")) PyErr_Warn(PyExc_Warning, "timer: missing attribute: running");
		if (!PyObject_HasAttrString(itself, "is_slaved")) PyErr_Warn(PyExc_Warning, "timer: missing attribute: is_slaved");
	}
	if (itself == NULL) itself = Py_None;

	py_timer = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

timer::~timer()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_timer;
	py_timer = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::lib::timer::time_type timer::elapsed() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::timer::time_type _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer, "elapsed", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer::elapsed() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during timer::elapsed() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

double timer::get_realtime_speed() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	double _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer, "get_realtime_speed", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer::get_realtime_speed() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "d", &_rv))
	{
		PySys_WriteStderr("Python exception during timer::get_realtime_speed() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::timer::signed_time_type timer::set_drift(ambulant::lib::timer::signed_time_type drift)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::timer::signed_time_type _rv;
	PyObject *py_drift = Py_BuildValue("l", drift);

	PyObject *py_rv = PyObject_CallMethod(py_timer, "set_drift", "(O)", py_drift);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer::set_drift() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during timer::set_drift() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_drift);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::timer::signed_time_type timer::get_drift() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::timer::signed_time_type _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer, "get_drift", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer::get_drift() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during timer::get_drift() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void timer::skew(ambulant::lib::timer::signed_time_type skew)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_skew = Py_BuildValue("l", skew);

	PyObject *py_rv = PyObject_CallMethod(py_timer, "skew", "(O)", py_skew);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer::skew() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_skew);

	PyGILState_Release(_GILState);
}

bool timer::running() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer, "running", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer::running() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during timer::running() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool timer::is_slaved() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer, "is_slaved", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer::is_slaved() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during timer::is_slaved() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ---------------------- Class timer_control ----------------------- */

timer_control::timer_control(PyObject *itself)
:	::timer(itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "elapsed")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: elapsed");
		if (!PyObject_HasAttrString(itself, "elapsed")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: elapsed");
		if (!PyObject_HasAttrString(itself, "start")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: start");
		if (!PyObject_HasAttrString(itself, "stop")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: stop");
		if (!PyObject_HasAttrString(itself, "pause")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: pause");
		if (!PyObject_HasAttrString(itself, "resume")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: resume");
		if (!PyObject_HasAttrString(itself, "set_speed")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: set_speed");
		if (!PyObject_HasAttrString(itself, "set_time")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: set_time");
		if (!PyObject_HasAttrString(itself, "get_speed")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: get_speed");
		if (!PyObject_HasAttrString(itself, "running")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: running");
		if (!PyObject_HasAttrString(itself, "get_realtime_speed")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: get_realtime_speed");
		if (!PyObject_HasAttrString(itself, "set_drift")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: set_drift");
		if (!PyObject_HasAttrString(itself, "get_drift")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: get_drift");
		if (!PyObject_HasAttrString(itself, "skew")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: skew");
		if (!PyObject_HasAttrString(itself, "set_observer")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: set_observer");
		if (!PyObject_HasAttrString(itself, "set_slaved")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: set_slaved");
		if (!PyObject_HasAttrString(itself, "is_slaved")) PyErr_Warn(PyExc_Warning, "timer_control: missing attribute: is_slaved");
	}
	if (itself == NULL) itself = Py_None;

	py_timer_control = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

timer_control::~timer_control()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_timer_control;
	py_timer_control = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::lib::timer::time_type timer_control::elapsed() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::timer::time_type _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "elapsed", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::elapsed() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during timer_control::elapsed() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::timer::time_type timer_control::elapsed(ambulant::lib::timer::time_type pt) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::timer::time_type _rv;
	PyObject *py_pt = Py_BuildValue("l", pt);

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "elapsed", "(O)", py_pt);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::elapsed() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during timer_control::elapsed() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_pt);

	PyGILState_Release(_GILState);
	return _rv;
}

void timer_control::start(ambulant::lib::timer::time_type t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_t = Py_BuildValue("l", t);

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "start", "(O)", py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::start() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}

void timer_control::stop()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "stop", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::stop() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void timer_control::pause()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "pause", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::pause() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void timer_control::resume()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "resume", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::resume() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void timer_control::set_speed(double speed)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_speed = Py_BuildValue("d", speed);

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "set_speed", "(O)", py_speed);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::set_speed() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_speed);

	PyGILState_Release(_GILState);
}

void timer_control::set_time(ambulant::lib::timer::time_type t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_t = Py_BuildValue("l", t);

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "set_time", "(O)", py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::set_time() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}

double timer_control::get_speed() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	double _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "get_speed", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::get_speed() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "d", &_rv))
	{
		PySys_WriteStderr("Python exception during timer_control::get_speed() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool timer_control::running() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "running", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::running() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during timer_control::running() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

double timer_control::get_realtime_speed() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	double _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "get_realtime_speed", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::get_realtime_speed() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "d", &_rv))
	{
		PySys_WriteStderr("Python exception during timer_control::get_realtime_speed() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::timer::signed_time_type timer_control::set_drift(ambulant::lib::timer::signed_time_type drift)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::timer::signed_time_type _rv;
	PyObject *py_drift = Py_BuildValue("l", drift);

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "set_drift", "(O)", py_drift);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::set_drift() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during timer_control::set_drift() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_drift);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::timer::signed_time_type timer_control::get_drift() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::timer::signed_time_type _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "get_drift", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::get_drift() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during timer_control::get_drift() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void timer_control::skew(ambulant::lib::timer::signed_time_type skew)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_skew = Py_BuildValue("l", skew);

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "skew", "(O)", py_skew);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::skew() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_skew);

	PyGILState_Release(_GILState);
}

void timer_control::set_observer(ambulant::lib::timer_observer* obs)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_obs = Py_BuildValue("O&", timer_observerObj_New, obs);

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "set_observer", "(O)", py_obs);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::set_observer() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_obs);

	PyGILState_Release(_GILState);
}

void timer_control::set_slaved(bool slaved)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_slaved = Py_BuildValue("O&", bool_New, slaved);

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "set_slaved", "(O)", py_slaved);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::set_slaved() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_slaved);

	PyGILState_Release(_GILState);
}

bool timer_control::is_slaved() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer_control, "is_slaved", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_control::is_slaved() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during timer_control::is_slaved() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ---------------------- Class timer_observer ---------------------- */

timer_observer::timer_observer(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "started")) PyErr_Warn(PyExc_Warning, "timer_observer: missing attribute: started");
		if (!PyObject_HasAttrString(itself, "stopped")) PyErr_Warn(PyExc_Warning, "timer_observer: missing attribute: stopped");
		if (!PyObject_HasAttrString(itself, "paused")) PyErr_Warn(PyExc_Warning, "timer_observer: missing attribute: paused");
		if (!PyObject_HasAttrString(itself, "resumed")) PyErr_Warn(PyExc_Warning, "timer_observer: missing attribute: resumed");
	}
	if (itself == NULL) itself = Py_None;

	py_timer_observer = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

timer_observer::~timer_observer()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_timer_observer;
	py_timer_observer = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void timer_observer::started()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_timer_observer, "started", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_observer::started() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void timer_observer::stopped()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_timer_observer, "stopped", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_observer::stopped() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void timer_observer::paused()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_timer_observer, "paused", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_observer::paused() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void timer_observer::resumed()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_timer_observer, "resumed", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_observer::resumed() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

/* ------------------------ Class timer_sync ------------------------ */

timer_sync::timer_sync(PyObject *itself)
:	::timer_observer(itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "initialize")) PyErr_Warn(PyExc_Warning, "timer_sync: missing attribute: initialize");
		if (!PyObject_HasAttrString(itself, "started")) PyErr_Warn(PyExc_Warning, "timer_sync: missing attribute: started");
		if (!PyObject_HasAttrString(itself, "stopped")) PyErr_Warn(PyExc_Warning, "timer_sync: missing attribute: stopped");
		if (!PyObject_HasAttrString(itself, "paused")) PyErr_Warn(PyExc_Warning, "timer_sync: missing attribute: paused");
		if (!PyObject_HasAttrString(itself, "resumed")) PyErr_Warn(PyExc_Warning, "timer_sync: missing attribute: resumed");
		if (!PyObject_HasAttrString(itself, "clicked")) PyErr_Warn(PyExc_Warning, "timer_sync: missing attribute: clicked");
		if (!PyObject_HasAttrString(itself, "uses_external_sync")) PyErr_Warn(PyExc_Warning, "timer_sync: missing attribute: uses_external_sync");
	}
	if (itself == NULL) itself = Py_None;

	py_timer_sync = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

timer_sync::~timer_sync()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_timer_sync;
	py_timer_sync = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void timer_sync::initialize(ambulant::lib::timer_control* timer)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_timer = Py_BuildValue("O&", timer_controlObj_New, timer);

	PyObject *py_rv = PyObject_CallMethod(py_timer_sync, "initialize", "(O)", py_timer);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_sync::initialize() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_timer);

	PyGILState_Release(_GILState);
}

void timer_sync::started()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_timer_sync, "started", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_sync::started() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void timer_sync::stopped()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_timer_sync, "stopped", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_sync::stopped() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void timer_sync::paused()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_timer_sync, "paused", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_sync::paused() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void timer_sync::resumed()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_timer_sync, "resumed", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_sync::resumed() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void timer_sync::clicked(const ambulant::lib::node* n, ambulant::lib::timer::time_type t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);
	PyObject *py_t = Py_BuildValue("l", t);

	PyObject *py_rv = PyObject_CallMethod(py_timer_sync, "clicked", "(OO)", py_n, py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_sync::clicked() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}

#ifdef WITH_REMOTE_SYNC
bool timer_sync::uses_external_sync()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_timer_sync, "uses_external_sync", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_sync::uses_external_sync() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during timer_sync::uses_external_sync() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}
#endif

/* -------------------- Class timer_sync_factory -------------------- */

timer_sync_factory::timer_sync_factory(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_timer_sync")) PyErr_Warn(PyExc_Warning, "timer_sync_factory: missing attribute: new_timer_sync");
	}
	if (itself == NULL) itself = Py_None;

	py_timer_sync_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

timer_sync_factory::~timer_sync_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_timer_sync_factory;
	py_timer_sync_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::lib::timer_sync* timer_sync_factory::new_timer_sync(ambulant::lib::document* doc)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::timer_sync* _rv;
	PyObject *py_doc = Py_BuildValue("O&", documentObj_New, doc);

	PyObject *py_rv = PyObject_CallMethod(py_timer_sync_factory, "new_timer_sync", "(O)", py_doc);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during timer_sync_factory::new_timer_sync() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", timer_syncObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during timer_sync_factory::new_timer_sync() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_doc);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ------------------------- Class embedder ------------------------- */

embedder::embedder(PyObject *itself)
:	::system_embedder(itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "close")) PyErr_Warn(PyExc_Warning, "embedder: missing attribute: close");
		if (!PyObject_HasAttrString(itself, "open")) PyErr_Warn(PyExc_Warning, "embedder: missing attribute: open");
		if (!PyObject_HasAttrString(itself, "done")) PyErr_Warn(PyExc_Warning, "embedder: missing attribute: done");
		if (!PyObject_HasAttrString(itself, "starting")) PyErr_Warn(PyExc_Warning, "embedder: missing attribute: starting");
		if (!PyObject_HasAttrString(itself, "aux_open")) PyErr_Warn(PyExc_Warning, "embedder: missing attribute: aux_open");
		if (!PyObject_HasAttrString(itself, "terminate")) PyErr_Warn(PyExc_Warning, "embedder: missing attribute: terminate");
	}
	if (itself == NULL) itself = Py_None;

	py_embedder = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

embedder::~embedder()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_embedder;
	py_embedder = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void embedder::close(ambulant::common::player* p)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playerObj_New, p);

	PyObject *py_rv = PyObject_CallMethod(py_embedder, "close", "(O)", py_p);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during embedder::close() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);

	PyGILState_Release(_GILState);
}

void embedder::open(ambulant::net::url newdoc, bool start, ambulant::common::player* old)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_newdoc = Py_BuildValue("O", ambulant_url_New(newdoc));
	PyObject *py_start = Py_BuildValue("O&", bool_New, start);
	PyObject *py_old = Py_BuildValue("O&", playerObj_New, old);

	PyObject *py_rv = PyObject_CallMethod(py_embedder, "open", "(OOO)", py_newdoc, py_start, py_old);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during embedder::open() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_newdoc);
	Py_XDECREF(py_start);
	Py_XDECREF(py_old);

	PyGILState_Release(_GILState);
}

void embedder::done(ambulant::common::player* p)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playerObj_New, p);

	PyObject *py_rv = PyObject_CallMethod(py_embedder, "done", "(O)", py_p);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during embedder::done() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);

	PyGILState_Release(_GILState);
}

void embedder::starting(ambulant::common::player* p)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playerObj_New, p);

	PyObject *py_rv = PyObject_CallMethod(py_embedder, "starting", "(O)", py_p);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during embedder::starting() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);

	PyGILState_Release(_GILState);
}

bool embedder::aux_open(const ambulant::net::url& href)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_href = Py_BuildValue("O", ambulant_url_New(href));

	PyObject *py_rv = PyObject_CallMethod(py_embedder, "aux_open", "(O)", py_href);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during embedder::aux_open() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during embedder::aux_open() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_href);

	PyGILState_Release(_GILState);
	return _rv;
}

void embedder::terminate()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_embedder, "terminate", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during embedder::terminate() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

/* ------------------------ Class factories ------------------------- */

factories::factories(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "init_factories")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: init_factories");
		if (!PyObject_HasAttrString(itself, "init_playable_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: init_playable_factory");
		if (!PyObject_HasAttrString(itself, "init_window_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: init_window_factory");
		if (!PyObject_HasAttrString(itself, "init_datasource_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: init_datasource_factory");
		if (!PyObject_HasAttrString(itself, "init_parser_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: init_parser_factory");
		if (!PyObject_HasAttrString(itself, "init_node_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: init_node_factory");
		if (!PyObject_HasAttrString(itself, "init_state_component_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: init_state_component_factory");
		if (!PyObject_HasAttrString(itself, "init_timer_sync_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: init_timer_sync_factory");
		if (!PyObject_HasAttrString(itself, "init_recorder_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: init_recorder_factory");
		if (!PyObject_HasAttrString(itself, "get_playable_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: get_playable_factory");
		if (!PyObject_HasAttrString(itself, "get_window_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: get_window_factory");
		if (!PyObject_HasAttrString(itself, "get_datasource_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: get_datasource_factory");
		if (!PyObject_HasAttrString(itself, "get_parser_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: get_parser_factory");
		if (!PyObject_HasAttrString(itself, "get_node_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: get_node_factory");
		if (!PyObject_HasAttrString(itself, "get_state_component_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: get_state_component_factory");
		if (!PyObject_HasAttrString(itself, "get_recorder_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: get_recorder_factory");
		if (!PyObject_HasAttrString(itself, "set_playable_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: set_playable_factory");
		if (!PyObject_HasAttrString(itself, "set_window_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: set_window_factory");
		if (!PyObject_HasAttrString(itself, "set_datasource_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: set_datasource_factory");
		if (!PyObject_HasAttrString(itself, "set_parser_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: set_parser_factory");
		if (!PyObject_HasAttrString(itself, "set_node_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: set_node_factory");
		if (!PyObject_HasAttrString(itself, "set_state_component_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: set_state_component_factory");
		if (!PyObject_HasAttrString(itself, "get_timer_sync_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: get_timer_sync_factory");
		if (!PyObject_HasAttrString(itself, "set_timer_sync_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: set_timer_sync_factory");
		if (!PyObject_HasAttrString(itself, "set_recorder_factory")) PyErr_Warn(PyExc_Warning, "factories: missing attribute: set_recorder_factory");
	}
	if (itself == NULL) itself = Py_None;

	py_factories = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

factories::~factories()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_factories;
	py_factories = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void factories::init_factories()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_factories, "init_factories", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::init_factories() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void factories::init_playable_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_factories, "init_playable_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::init_playable_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void factories::init_window_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_factories, "init_window_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::init_window_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void factories::init_datasource_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_factories, "init_datasource_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::init_datasource_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void factories::init_parser_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_factories, "init_parser_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::init_parser_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void factories::init_node_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_factories, "init_node_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::init_node_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void factories::init_state_component_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_factories, "init_state_component_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::init_state_component_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

#ifdef WITH_REMOTE_SYNC
void factories::init_timer_sync_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_factories, "init_timer_sync_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::init_timer_sync_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}
#endif

void factories::init_recorder_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_factories, "init_recorder_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::init_recorder_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

ambulant::common::global_playable_factory* factories::get_playable_factory() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::global_playable_factory* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_factories, "get_playable_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::get_playable_factory() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", global_playable_factoryObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during factories::get_playable_factory() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::window_factory* factories::get_window_factory() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::window_factory* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_factories, "get_window_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::get_window_factory() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", window_factoryObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during factories::get_window_factory() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::net::datasource_factory* factories::get_datasource_factory() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::net::datasource_factory* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_factories, "get_datasource_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::get_datasource_factory() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", datasource_factoryObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during factories::get_datasource_factory() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::global_parser_factory* factories::get_parser_factory() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::global_parser_factory* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_factories, "get_parser_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::get_parser_factory() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", global_parser_factoryObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during factories::get_parser_factory() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::node_factory* factories::get_node_factory() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::node_factory* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_factories, "get_node_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::get_node_factory() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", node_factoryObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during factories::get_node_factory() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::global_state_component_factory* factories::get_state_component_factory() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::global_state_component_factory* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_factories, "get_state_component_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::get_state_component_factory() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", global_state_component_factoryObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during factories::get_state_component_factory() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::recorder_factory* factories::get_recorder_factory() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::recorder_factory* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_factories, "get_recorder_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::get_recorder_factory() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", recorder_factoryObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during factories::get_recorder_factory() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void factories::set_playable_factory(ambulant::common::global_playable_factory* pf)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_pf = Py_BuildValue("O&", global_playable_factoryObj_New, pf);

	PyObject *py_rv = PyObject_CallMethod(py_factories, "set_playable_factory", "(O)", py_pf);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::set_playable_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_pf);

	PyGILState_Release(_GILState);
}

void factories::set_window_factory(ambulant::common::window_factory* wf)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_wf = Py_BuildValue("O&", window_factoryObj_New, wf);

	PyObject *py_rv = PyObject_CallMethod(py_factories, "set_window_factory", "(O)", py_wf);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::set_window_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_wf);

	PyGILState_Release(_GILState);
}

void factories::set_datasource_factory(ambulant::net::datasource_factory* df)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_df = Py_BuildValue("O&", datasource_factoryObj_New, df);

	PyObject *py_rv = PyObject_CallMethod(py_factories, "set_datasource_factory", "(O)", py_df);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::set_datasource_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_df);

	PyGILState_Release(_GILState);
}

void factories::set_parser_factory(ambulant::lib::global_parser_factory* pf)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_pf = Py_BuildValue("O&", global_parser_factoryObj_New, pf);

	PyObject *py_rv = PyObject_CallMethod(py_factories, "set_parser_factory", "(O)", py_pf);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::set_parser_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_pf);

	PyGILState_Release(_GILState);
}

void factories::set_node_factory(ambulant::lib::node_factory* nf)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_nf = Py_BuildValue("O&", node_factoryObj_New, nf);

	PyObject *py_rv = PyObject_CallMethod(py_factories, "set_node_factory", "(O)", py_nf);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::set_node_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_nf);

	PyGILState_Release(_GILState);
}

void factories::set_state_component_factory(ambulant::common::global_state_component_factory* sf)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_sf = Py_BuildValue("O&", global_state_component_factoryObj_New, sf);

	PyObject *py_rv = PyObject_CallMethod(py_factories, "set_state_component_factory", "(O)", py_sf);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::set_state_component_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_sf);

	PyGILState_Release(_GILState);
}

#ifdef WITH_REMOTE_SYNC
ambulant::lib::timer_sync_factory* factories::get_timer_sync_factory() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::timer_sync_factory* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_factories, "get_timer_sync_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::get_timer_sync_factory() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", timer_sync_factoryObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during factories::get_timer_sync_factory() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}
#endif

#ifdef WITH_REMOTE_SYNC
void factories::set_timer_sync_factory(ambulant::lib::timer_sync_factory* tsf)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_tsf = Py_BuildValue("O&", timer_sync_factoryObj_New, tsf);

	PyObject *py_rv = PyObject_CallMethod(py_factories, "set_timer_sync_factory", "(O)", py_tsf);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::set_timer_sync_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_tsf);

	PyGILState_Release(_GILState);
}
#endif

void factories::set_recorder_factory(ambulant::common::recorder_factory* rf)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rf = Py_BuildValue("O&", recorder_factoryObj_New, rf);

	PyObject *py_rv = PyObject_CallMethod(py_factories, "set_recorder_factory", "(O)", py_rf);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during factories::set_recorder_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_rf);

	PyGILState_Release(_GILState);
}

/* ------------------------ Class gui_screen ------------------------ */

gui_screen::gui_screen(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "get_size")) PyErr_Warn(PyExc_Warning, "gui_screen: missing attribute: get_size");
		if (!PyObject_HasAttrString(itself, "get_screenshot")) PyErr_Warn(PyExc_Warning, "gui_screen: missing attribute: get_screenshot");
	}
	if (itself == NULL) itself = Py_None;

	py_gui_screen = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

gui_screen::~gui_screen()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_gui_screen;
	py_gui_screen = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void gui_screen::get_size(int* width, int* height)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_screen, "get_size", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_screen::get_size() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "(ii)", &width, &height))
	{
		PySys_WriteStderr("Python exception during gui_screen::get_size() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

#ifndef CPP_TO_PYTHON_BRIDGE
bool gui_screen::get_screenshot(const char* type, char* *out_data__out__, size_t* out_data__len__)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_type = Py_BuildValue("s", type);
	PyObject *py_out_data = Py_BuildValue("z#", out_data__out__, (int)out_data__len__);

	PyObject *py_rv = PyObject_CallMethod(py_gui_screen, "get_screenshot", "(OO)", py_type, py_out_data);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_screen::get_screenshot() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_screen::get_screenshot() return:\n");
		PyErr_Print();
	}

	out_data__out__ = NULL;
	Py_XDECREF(py_rv);
	Py_XDECREF(py_type);
	Py_XDECREF(py_out_data);

	PyGILState_Release(_GILState);
	return _rv;
}
#endif

/* ------------------------ Class gui_player ------------------------ */

gui_player::gui_player(PyObject *itself)
:	::factories(itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "init_playable_factory")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: init_playable_factory");
		if (!PyObject_HasAttrString(itself, "init_window_factory")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: init_window_factory");
		if (!PyObject_HasAttrString(itself, "init_datasource_factory")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: init_datasource_factory");
		if (!PyObject_HasAttrString(itself, "init_parser_factory")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: init_parser_factory");
		if (!PyObject_HasAttrString(itself, "init_plugins")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: init_plugins");
		if (!PyObject_HasAttrString(itself, "play")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: play");
		if (!PyObject_HasAttrString(itself, "stop")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: stop");
		if (!PyObject_HasAttrString(itself, "pause")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: pause");
		if (!PyObject_HasAttrString(itself, "restart")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: restart");
		if (!PyObject_HasAttrString(itself, "goto_node")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: goto_node");
		if (!PyObject_HasAttrString(itself, "is_play_enabled")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: is_play_enabled");
		if (!PyObject_HasAttrString(itself, "is_stop_enabled")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: is_stop_enabled");
		if (!PyObject_HasAttrString(itself, "is_pause_enabled")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: is_pause_enabled");
		if (!PyObject_HasAttrString(itself, "is_play_active")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: is_play_active");
		if (!PyObject_HasAttrString(itself, "is_stop_active")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: is_stop_active");
		if (!PyObject_HasAttrString(itself, "is_pause_active")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: is_pause_active");
		if (!PyObject_HasAttrString(itself, "before_mousemove")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: before_mousemove");
		if (!PyObject_HasAttrString(itself, "after_mousemove")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: after_mousemove");
		if (!PyObject_HasAttrString(itself, "on_char")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: on_char");
		if (!PyObject_HasAttrString(itself, "on_focus_advance")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: on_focus_advance");
		if (!PyObject_HasAttrString(itself, "on_focus_activate")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: on_focus_activate");
		if (!PyObject_HasAttrString(itself, "get_document")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: get_document");
		if (!PyObject_HasAttrString(itself, "set_document")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: set_document");
		if (!PyObject_HasAttrString(itself, "get_embedder")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: get_embedder");
		if (!PyObject_HasAttrString(itself, "set_embedder")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: set_embedder");
		if (!PyObject_HasAttrString(itself, "get_player")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: get_player");
		if (!PyObject_HasAttrString(itself, "set_player")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: set_player");
		if (!PyObject_HasAttrString(itself, "get_url")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: get_url");
		if (!PyObject_HasAttrString(itself, "get_gui_screen")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: get_gui_screen");
		if (!PyObject_HasAttrString(itself, "clicked_external")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: clicked_external");
		if (!PyObject_HasAttrString(itself, "uses_external_sync")) PyErr_Warn(PyExc_Warning, "gui_player: missing attribute: uses_external_sync");
	}
	if (itself == NULL) itself = Py_None;

	py_gui_player = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

gui_player::~gui_player()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_gui_player;
	py_gui_player = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void gui_player::init_playable_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "init_playable_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::init_playable_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void gui_player::init_window_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "init_window_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::init_window_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void gui_player::init_datasource_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "init_datasource_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::init_datasource_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void gui_player::init_parser_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "init_parser_factory", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::init_parser_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void gui_player::init_plugins()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "init_plugins", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::init_plugins() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void gui_player::play()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "play", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::play() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void gui_player::stop()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "stop", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::stop() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void gui_player::pause()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "pause", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::pause() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void gui_player::restart(bool reparse)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_reparse = Py_BuildValue("O&", bool_New, reparse);

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "restart", "(O)", py_reparse);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::restart() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_reparse);

	PyGILState_Release(_GILState);
}

void gui_player::goto_node(const ambulant::lib::node* n)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "goto_node", "(O)", py_n);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::goto_node() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);

	PyGILState_Release(_GILState);
}

bool gui_player::is_play_enabled() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "is_play_enabled", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::is_play_enabled() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::is_play_enabled() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool gui_player::is_stop_enabled() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "is_stop_enabled", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::is_stop_enabled() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::is_stop_enabled() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool gui_player::is_pause_enabled() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "is_pause_enabled", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::is_pause_enabled() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::is_pause_enabled() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool gui_player::is_play_active() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "is_play_active", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::is_play_active() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::is_play_active() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool gui_player::is_stop_active() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "is_stop_active", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::is_stop_active() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::is_stop_active() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool gui_player::is_pause_active() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "is_pause_active", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::is_pause_active() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::is_pause_active() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void gui_player::before_mousemove(int cursor)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_cursor = Py_BuildValue("i", cursor);

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "before_mousemove", "(O)", py_cursor);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::before_mousemove() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_cursor);

	PyGILState_Release(_GILState);
}

int gui_player::after_mousemove()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	int _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "after_mousemove", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::after_mousemove() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "i", &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::after_mousemove() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void gui_player::on_char(int c)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_c = Py_BuildValue("i", c);

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "on_char", "(O)", py_c);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::on_char() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_c);

	PyGILState_Release(_GILState);
}

void gui_player::on_focus_advance()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "on_focus_advance", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::on_focus_advance() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void gui_player::on_focus_activate()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "on_focus_activate", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::on_focus_activate() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

ambulant::lib::document* gui_player::get_document() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::document* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "get_document", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::get_document() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", documentObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::get_document() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void gui_player::set_document(ambulant::lib::document* doc)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_doc = Py_BuildValue("O&", documentObj_New, doc);

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "set_document", "(O)", py_doc);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::set_document() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_doc);

	PyGILState_Release(_GILState);
}

ambulant::common::embedder* gui_player::get_embedder() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::embedder* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "get_embedder", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::get_embedder() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", embedderObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::get_embedder() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void gui_player::set_embedder(ambulant::common::embedder* em)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_em = Py_BuildValue("O&", embedderObj_New, em);

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "set_embedder", "(O)", py_em);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::set_embedder() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_em);

	PyGILState_Release(_GILState);
}

ambulant::common::player* gui_player::get_player() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::player* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "get_player", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::get_player() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", playerObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::get_player() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void gui_player::set_player(ambulant::common::player* pl)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_pl = Py_BuildValue("O&", playerObj_New, pl);

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "set_player", "(O)", py_pl);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::set_player() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_pl);

	PyGILState_Release(_GILState);
}

ambulant::net::url gui_player::get_url() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::net::url _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "get_url", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::get_url() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_url_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::get_url() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::gui_screen* gui_player::get_gui_screen()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::gui_screen* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "get_gui_screen", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::get_gui_screen() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", gui_screenObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_player::get_gui_screen() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

#ifdef WITH_REMOTE_SYNC
void gui_player::clicked_external(ambulant::lib::node* n, ambulant::lib::timer::time_type t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);
	PyObject *py_t = Py_BuildValue("l", t);

	PyObject *py_rv = PyObject_CallMethod(py_gui_player, "clicked_external", "(OO)", py_n, py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_player::clicked_external() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}
#endif

/* ------------------------ Class alignment ------------------------- */

alignment::alignment(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "get_image_fixpoint")) PyErr_Warn(PyExc_Warning, "alignment: missing attribute: get_image_fixpoint");
		if (!PyObject_HasAttrString(itself, "get_surface_fixpoint")) PyErr_Warn(PyExc_Warning, "alignment: missing attribute: get_surface_fixpoint");
	}
	if (itself == NULL) itself = Py_None;

	py_alignment = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

alignment::~alignment()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_alignment;
	py_alignment = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::lib::point alignment::get_image_fixpoint(ambulant::lib::size image_size) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::point _rv;
	PyObject *py_image_size = Py_BuildValue("O", ambulant_size_New(image_size));

	PyObject *py_rv = PyObject_CallMethod(py_alignment, "get_image_fixpoint", "(O)", py_image_size);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during alignment::get_image_fixpoint() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_point_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during alignment::get_image_fixpoint() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_image_size);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::point alignment::get_surface_fixpoint(ambulant::lib::size surface_size) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::point _rv;
	PyObject *py_surface_size = Py_BuildValue("O", ambulant_size_New(surface_size));

	PyObject *py_rv = PyObject_CallMethod(py_alignment, "get_surface_fixpoint", "(O)", py_surface_size);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during alignment::get_surface_fixpoint() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_point_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during alignment::get_surface_fixpoint() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_surface_size);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ------------------ Class animation_notification ------------------ */

animation_notification::animation_notification(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "animated")) PyErr_Warn(PyExc_Warning, "animation_notification: missing attribute: animated");
	}
	if (itself == NULL) itself = Py_None;

	py_animation_notification = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

animation_notification::~animation_notification()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_animation_notification;
	py_animation_notification = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void animation_notification::animated()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_animation_notification, "animated", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_notification::animated() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

/* ------------------------ Class gui_window ------------------------ */

gui_window::gui_window(PyObject *itself)
:	ambulant::common::gui_window(0)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "need_redraw")) PyErr_Warn(PyExc_Warning, "gui_window: missing attribute: need_redraw");
		if (!PyObject_HasAttrString(itself, "redraw_now")) PyErr_Warn(PyExc_Warning, "gui_window: missing attribute: redraw_now");
		if (!PyObject_HasAttrString(itself, "need_events")) PyErr_Warn(PyExc_Warning, "gui_window: missing attribute: need_events");
	}
	if (itself == NULL) itself = Py_None;

	py_gui_window = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

gui_window::~gui_window()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_gui_window;
	py_gui_window = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void gui_window::need_redraw(const ambulant::lib::rect& r)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_r = Py_BuildValue("O", ambulant_rect_New(r));

	PyObject *py_rv = PyObject_CallMethod(py_gui_window, "need_redraw", "(O)", py_r);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_window::need_redraw() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_r);

	PyGILState_Release(_GILState);
}

void gui_window::redraw_now()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_gui_window, "redraw_now", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_window::redraw_now() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void gui_window::need_events(bool want)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_want = Py_BuildValue("O&", bool_New, want);

	PyObject *py_rv = PyObject_CallMethod(py_gui_window, "need_events", "(O)", py_want);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_window::need_events() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_want);

	PyGILState_Release(_GILState);
}

/* ------------------------ Class gui_events ------------------------ */

gui_events::gui_events(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "redraw")) PyErr_Warn(PyExc_Warning, "gui_events: missing attribute: redraw");
		if (!PyObject_HasAttrString(itself, "user_event")) PyErr_Warn(PyExc_Warning, "gui_events: missing attribute: user_event");
		if (!PyObject_HasAttrString(itself, "transition_freeze_end")) PyErr_Warn(PyExc_Warning, "gui_events: missing attribute: transition_freeze_end");
	}
	if (itself == NULL) itself = Py_None;

	py_gui_events = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

gui_events::~gui_events()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_gui_events;
	py_gui_events = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void gui_events::redraw(const ambulant::lib::rect& dirty, ambulant::common::gui_window* window)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_dirty = Py_BuildValue("O", ambulant_rect_New(dirty));
	PyObject *py_window = Py_BuildValue("O&", gui_windowObj_New, window);

	PyObject *py_rv = PyObject_CallMethod(py_gui_events, "redraw", "(OO)", py_dirty, py_window);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_events::redraw() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_dirty);
	Py_XDECREF(py_window);

	PyGILState_Release(_GILState);
}

bool gui_events::user_event(const ambulant::lib::point& where, int what)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_where = Py_BuildValue("O", ambulant_point_New(where));
	PyObject *py_what = Py_BuildValue("i", what);

	PyObject *py_rv = PyObject_CallMethod(py_gui_events, "user_event", "(OO)", py_where, py_what);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_events::user_event() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during gui_events::user_event() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_where);
	Py_XDECREF(py_what);

	PyGILState_Release(_GILState);
	return _rv;
}

void gui_events::transition_freeze_end(ambulant::lib::rect area)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_area = Py_BuildValue("O", ambulant_rect_New(area));

	PyObject *py_rv = PyObject_CallMethod(py_gui_events, "transition_freeze_end", "(O)", py_area);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during gui_events::transition_freeze_end() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_area);

	PyGILState_Release(_GILState);
}

/* ------------------------- Class renderer ------------------------- */

renderer::renderer(PyObject *itself)
:	::gui_events(itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "set_surface")) PyErr_Warn(PyExc_Warning, "renderer: missing attribute: set_surface");
		if (!PyObject_HasAttrString(itself, "set_alignment")) PyErr_Warn(PyExc_Warning, "renderer: missing attribute: set_alignment");
		if (!PyObject_HasAttrString(itself, "set_intransition")) PyErr_Warn(PyExc_Warning, "renderer: missing attribute: set_intransition");
		if (!PyObject_HasAttrString(itself, "start_outtransition")) PyErr_Warn(PyExc_Warning, "renderer: missing attribute: start_outtransition");
		if (!PyObject_HasAttrString(itself, "get_surface")) PyErr_Warn(PyExc_Warning, "renderer: missing attribute: get_surface");
	}
	if (itself == NULL) itself = Py_None;

	py_renderer = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

renderer::~renderer()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_renderer;
	py_renderer = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void renderer::set_surface(ambulant::common::surface* destination)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_destination = Py_BuildValue("O&", surfaceObj_New, destination);

	PyObject *py_rv = PyObject_CallMethod(py_renderer, "set_surface", "(O)", py_destination);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during renderer::set_surface() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_destination);

	PyGILState_Release(_GILState);
}

void renderer::set_alignment(const ambulant::common::alignment* align)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_align = Py_BuildValue("O&", alignmentObj_New, align);

	PyObject *py_rv = PyObject_CallMethod(py_renderer, "set_alignment", "(O)", py_align);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during renderer::set_alignment() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_align);

	PyGILState_Release(_GILState);
}

void renderer::set_intransition(const ambulant::lib::transition_info* info)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_info = Py_BuildValue("O&", transition_infoObj_New, info);

	PyObject *py_rv = PyObject_CallMethod(py_renderer, "set_intransition", "(O)", py_info);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during renderer::set_intransition() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_info);

	PyGILState_Release(_GILState);
}

void renderer::start_outtransition(const ambulant::lib::transition_info* info)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_info = Py_BuildValue("O&", transition_infoObj_New, info);

	PyObject *py_rv = PyObject_CallMethod(py_renderer, "start_outtransition", "(O)", py_info);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during renderer::start_outtransition() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_info);

	PyGILState_Release(_GILState);
}

ambulant::common::surface* renderer::get_surface()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::surface* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_renderer, "get_surface", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during renderer::get_surface() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", surfaceObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during renderer::get_surface() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ------------------------ Class bgrenderer ------------------------ */

bgrenderer::bgrenderer(PyObject *itself)
:	::gui_events(itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "set_surface")) PyErr_Warn(PyExc_Warning, "bgrenderer: missing attribute: set_surface");
		if (!PyObject_HasAttrString(itself, "keep_as_background")) PyErr_Warn(PyExc_Warning, "bgrenderer: missing attribute: keep_as_background");
		if (!PyObject_HasAttrString(itself, "highlight")) PyErr_Warn(PyExc_Warning, "bgrenderer: missing attribute: highlight");
	}
	if (itself == NULL) itself = Py_None;

	py_bgrenderer = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

bgrenderer::~bgrenderer()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_bgrenderer;
	py_bgrenderer = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void bgrenderer::set_surface(ambulant::common::surface* destination)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_destination = Py_BuildValue("O&", surfaceObj_New, destination);

	PyObject *py_rv = PyObject_CallMethod(py_bgrenderer, "set_surface", "(O)", py_destination);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during bgrenderer::set_surface() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_destination);

	PyGILState_Release(_GILState);
}

void bgrenderer::keep_as_background()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_bgrenderer, "keep_as_background", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during bgrenderer::keep_as_background() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void bgrenderer::highlight(ambulant::common::gui_window* window)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_window = Py_BuildValue("O&", gui_windowObj_New, window);

	PyObject *py_rv = PyObject_CallMethod(py_bgrenderer, "highlight", "(O)", py_window);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during bgrenderer::highlight() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_window);

	PyGILState_Release(_GILState);
}

/* ------------------------- Class surface -------------------------- */

surface::surface(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "show")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: show");
		if (!PyObject_HasAttrString(itself, "renderer_done")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: renderer_done");
		if (!PyObject_HasAttrString(itself, "need_redraw")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: need_redraw");
		if (!PyObject_HasAttrString(itself, "need_redraw")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: need_redraw");
		if (!PyObject_HasAttrString(itself, "need_events")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: need_events");
		if (!PyObject_HasAttrString(itself, "transition_done")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: transition_done");
		if (!PyObject_HasAttrString(itself, "keep_as_background")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: keep_as_background");
		if (!PyObject_HasAttrString(itself, "get_rect")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: get_rect");
		if (!PyObject_HasAttrString(itself, "get_clipped_screen_rect")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: get_clipped_screen_rect");
		if (!PyObject_HasAttrString(itself, "get_global_topleft")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: get_global_topleft");
		if (!PyObject_HasAttrString(itself, "get_fit_rect")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: get_fit_rect");
		if (!PyObject_HasAttrString(itself, "get_fit_rect")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: get_fit_rect");
		if (!PyObject_HasAttrString(itself, "get_crop_rect")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: get_crop_rect");
		if (!PyObject_HasAttrString(itself, "get_info")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: get_info");
		if (!PyObject_HasAttrString(itself, "get_top_surface")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: get_top_surface");
		if (!PyObject_HasAttrString(itself, "is_tiled")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: is_tiled");
		if (!PyObject_HasAttrString(itself, "get_gui_window")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: get_gui_window");
		if (!PyObject_HasAttrString(itself, "set_renderer_private_data")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: set_renderer_private_data");
		if (!PyObject_HasAttrString(itself, "get_renderer_private_data")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: get_renderer_private_data");
		if (!PyObject_HasAttrString(itself, "highlight")) PyErr_Warn(PyExc_Warning, "surface: missing attribute: highlight");
	}
	if (itself == NULL) itself = Py_None;

	py_surface = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

surface::~surface()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_surface;
	py_surface = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void surface::show(ambulant::common::gui_events* renderer)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_renderer = Py_BuildValue("O&", gui_eventsObj_New, renderer);

	PyObject *py_rv = PyObject_CallMethod(py_surface, "show", "(O)", py_renderer);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::show() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_renderer);

	PyGILState_Release(_GILState);
}

void surface::renderer_done(ambulant::common::gui_events* renderer)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_renderer = Py_BuildValue("O&", gui_eventsObj_New, renderer);

	PyObject *py_rv = PyObject_CallMethod(py_surface, "renderer_done", "(O)", py_renderer);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::renderer_done() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_renderer);

	PyGILState_Release(_GILState);
}

void surface::need_redraw(const ambulant::lib::rect& r)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_r = Py_BuildValue("O", ambulant_rect_New(r));

	PyObject *py_rv = PyObject_CallMethod(py_surface, "need_redraw", "(O)", py_r);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::need_redraw() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_r);

	PyGILState_Release(_GILState);
}

void surface::need_redraw()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_surface, "need_redraw", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::need_redraw() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void surface::need_events(bool want)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_want = Py_BuildValue("O&", bool_New, want);

	PyObject *py_rv = PyObject_CallMethod(py_surface, "need_events", "(O)", py_want);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::need_events() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_want);

	PyGILState_Release(_GILState);
}

void surface::transition_done()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_surface, "transition_done", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::transition_done() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void surface::keep_as_background()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_surface, "keep_as_background", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::keep_as_background() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

const ambulant::lib::rect& surface::get_rect() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::rect get_rect;

	PyObject *py_rv = PyObject_CallMethod(py_surface, "get_rect", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::get_rect() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_rect_Convert, &get_rect))
	{
		PySys_WriteStderr("Python exception during surface::get_rect() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	const_cast<surface *>(this)->get_rect_rvkeepref = get_rect;
	return get_rect_rvkeepref;
}

const ambulant::lib::rect& surface::get_clipped_screen_rect() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::rect get_clipped_screen_rect;

	PyObject *py_rv = PyObject_CallMethod(py_surface, "get_clipped_screen_rect", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::get_clipped_screen_rect() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_rect_Convert, &get_clipped_screen_rect))
	{
		PySys_WriteStderr("Python exception during surface::get_clipped_screen_rect() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	const_cast<surface *>(this)->get_clipped_screen_rect_rvkeepref = get_clipped_screen_rect;
	return get_clipped_screen_rect_rvkeepref;
}

const ambulant::lib::point& surface::get_global_topleft() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::point get_global_topleft;

	PyObject *py_rv = PyObject_CallMethod(py_surface, "get_global_topleft", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::get_global_topleft() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_point_Convert, &get_global_topleft))
	{
		PySys_WriteStderr("Python exception during surface::get_global_topleft() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	const_cast<surface *>(this)->get_global_topleft_rvkeepref = get_global_topleft;
	return get_global_topleft_rvkeepref;
}

ambulant::lib::rect surface::get_fit_rect(const ambulant::lib::size& src_size, ambulant::lib::rect* out_src_rect, const ambulant::common::alignment* align) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::rect _rv;
	PyObject *py_src_size = Py_BuildValue("O", ambulant_size_New(src_size));
	PyObject *py_align = Py_BuildValue("O&", alignmentObj_New, align);

	PyObject *py_rv = PyObject_CallMethod(py_surface, "get_fit_rect", "(OO)", py_src_size, py_align);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::get_fit_rect() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&O&", ambulant_rect_Convert, &_rv, ambulant_rect_Convert, &out_src_rect))
	{
		PySys_WriteStderr("Python exception during surface::get_fit_rect() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_src_size);
	Py_XDECREF(py_align);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::rect surface::get_fit_rect(const ambulant::lib::rect& src_crop_rect, const ambulant::lib::size& src_size, ambulant::lib::rect* out_src_rect, const ambulant::common::alignment* align) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::rect _rv;
	PyObject *py_src_crop_rect = Py_BuildValue("O", ambulant_rect_New(src_crop_rect));
	PyObject *py_src_size = Py_BuildValue("O", ambulant_size_New(src_size));
	PyObject *py_align = Py_BuildValue("O&", alignmentObj_New, align);

	PyObject *py_rv = PyObject_CallMethod(py_surface, "get_fit_rect", "(OOO)", py_src_crop_rect, py_src_size, py_align);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::get_fit_rect() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&O&", ambulant_rect_Convert, &_rv, ambulant_rect_Convert, &out_src_rect))
	{
		PySys_WriteStderr("Python exception during surface::get_fit_rect() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_src_crop_rect);
	Py_XDECREF(py_src_size);
	Py_XDECREF(py_align);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::rect surface::get_crop_rect(const ambulant::lib::size& src_size) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::rect _rv;
	PyObject *py_src_size = Py_BuildValue("O", ambulant_size_New(src_size));

	PyObject *py_rv = PyObject_CallMethod(py_surface, "get_crop_rect", "(O)", py_src_size);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::get_crop_rect() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_rect_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during surface::get_crop_rect() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_src_size);

	PyGILState_Release(_GILState);
	return _rv;
}

const ambulant::common::region_info* surface::get_info() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const ambulant::common::region_info* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_surface, "get_info", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::get_info() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", region_infoObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during surface::get_info() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::surface* surface::get_top_surface()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::surface* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_surface, "get_top_surface", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::get_top_surface() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", surfaceObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during surface::get_top_surface() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool surface::is_tiled() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_surface, "is_tiled", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::is_tiled() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during surface::is_tiled() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::gui_window* surface::get_gui_window()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::gui_window* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_surface, "get_gui_window", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::get_gui_window() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", gui_windowObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during surface::get_gui_window() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void surface::set_renderer_private_data(ambulant::common::renderer_private_id idd, ambulant::common::renderer_private_data * data)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_idd = Py_BuildValue("l", idd);
	PyObject *py_data = Py_BuildValue("l", data);

	PyObject *py_rv = PyObject_CallMethod(py_surface, "set_renderer_private_data", "(OO)", py_idd, py_data);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::set_renderer_private_data() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_idd);
	Py_XDECREF(py_data);

	PyGILState_Release(_GILState);
}

ambulant::common::renderer_private_data * surface::get_renderer_private_data(ambulant::common::renderer_private_id idd)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::renderer_private_data * _rv;
	PyObject *py_idd = Py_BuildValue("l", idd);

	PyObject *py_rv = PyObject_CallMethod(py_surface, "get_renderer_private_data", "(O)", py_idd);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::get_renderer_private_data() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during surface::get_renderer_private_data() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_idd);

	PyGILState_Release(_GILState);
	return _rv;
}

void surface::highlight(bool on)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_on = Py_BuildValue("O&", bool_New, on);

	PyObject *py_rv = PyObject_CallMethod(py_surface, "highlight", "(O)", py_on);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface::highlight() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_on);

	PyGILState_Release(_GILState);
}

/* ---------------------- Class window_factory ---------------------- */

window_factory::window_factory(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "get_default_size")) PyErr_Warn(PyExc_Warning, "window_factory: missing attribute: get_default_size");
		if (!PyObject_HasAttrString(itself, "new_window")) PyErr_Warn(PyExc_Warning, "window_factory: missing attribute: new_window");
		if (!PyObject_HasAttrString(itself, "new_background_renderer")) PyErr_Warn(PyExc_Warning, "window_factory: missing attribute: new_background_renderer");
		if (!PyObject_HasAttrString(itself, "window_done")) PyErr_Warn(PyExc_Warning, "window_factory: missing attribute: window_done");
	}
	if (itself == NULL) itself = Py_None;

	py_window_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

window_factory::~window_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_window_factory;
	py_window_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::lib::size window_factory::get_default_size()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::size _rv;

	PyObject *py_rv = PyObject_CallMethod(py_window_factory, "get_default_size", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during window_factory::get_default_size() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_size_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during window_factory::get_default_size() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::gui_window* window_factory::new_window(const std::string& name, ambulant::lib::size bounds, ambulant::common::gui_events* handler)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::gui_window* _rv;
	PyObject *py_name = Py_BuildValue("s", name.c_str());
	PyObject *py_bounds = Py_BuildValue("O", ambulant_size_New(bounds));
	PyObject *py_handler = Py_BuildValue("O&", gui_eventsObj_New, handler);

	PyObject *py_rv = PyObject_CallMethod(py_window_factory, "new_window", "(OOO)", py_name, py_bounds, py_handler);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during window_factory::new_window() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", gui_windowObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during window_factory::new_window() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);
	Py_XDECREF(py_bounds);
	Py_XDECREF(py_handler);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::bgrenderer* window_factory::new_background_renderer(const ambulant::common::region_info* src)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::bgrenderer* _rv;
	PyObject *py_src = Py_BuildValue("O&", region_infoObj_New, src);

	PyObject *py_rv = PyObject_CallMethod(py_window_factory, "new_background_renderer", "(O)", py_src);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during window_factory::new_background_renderer() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bgrendererObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during window_factory::new_background_renderer() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_src);

	PyGILState_Release(_GILState);
	return _rv;
}

void window_factory::window_done(const std::string& name)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_name = Py_BuildValue("s", name.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_window_factory, "window_done", "(O)", py_name);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during window_factory::window_done() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);

	PyGILState_Release(_GILState);
}

/* --------------------- Class surface_template --------------------- */

surface_template::surface_template(PyObject *itself)
:	::animation_notification(itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_subsurface")) PyErr_Warn(PyExc_Warning, "surface_template: missing attribute: new_subsurface");
		if (!PyObject_HasAttrString(itself, "activate")) PyErr_Warn(PyExc_Warning, "surface_template: missing attribute: activate");
	}
	if (itself == NULL) itself = Py_None;

	py_surface_template = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

surface_template::~surface_template()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_surface_template;
	py_surface_template = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::common::surface_template* surface_template::new_subsurface(const ambulant::common::region_info* info, ambulant::common::bgrenderer* bgrend)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::surface_template* _rv;
	PyObject *py_info = Py_BuildValue("O&", region_infoObj_New, info);
	PyObject *py_bgrend = Py_BuildValue("O&", bgrendererObj_New, bgrend);

	PyObject *py_rv = PyObject_CallMethod(py_surface_template, "new_subsurface", "(OO)", py_info, py_bgrend);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface_template::new_subsurface() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", surface_templateObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during surface_template::new_subsurface() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_info);
	Py_XDECREF(py_bgrend);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::surface* surface_template::activate()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::surface* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_surface_template, "activate", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface_template::activate() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", surfaceObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during surface_template::activate() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

/* --------------------- Class surface_factory ---------------------- */

surface_factory::surface_factory(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_topsurface")) PyErr_Warn(PyExc_Warning, "surface_factory: missing attribute: new_topsurface");
	}
	if (itself == NULL) itself = Py_None;

	py_surface_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

surface_factory::~surface_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_surface_factory;
	py_surface_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::common::surface_template* surface_factory::new_topsurface(const ambulant::common::region_info* info, ambulant::common::bgrenderer* bgrend, ambulant::common::window_factory* wf)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::surface_template* _rv;
	PyObject *py_info = Py_BuildValue("O&", region_infoObj_New, info);
	PyObject *py_bgrend = Py_BuildValue("O&", bgrendererObj_New, bgrend);
	PyObject *py_wf = Py_BuildValue("O&", window_factoryObj_New, wf);

	PyObject *py_rv = PyObject_CallMethod(py_surface_factory, "new_topsurface", "(OOO)", py_info, py_bgrend, py_wf);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during surface_factory::new_topsurface() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", surface_templateObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during surface_factory::new_topsurface() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_info);
	Py_XDECREF(py_bgrend);
	Py_XDECREF(py_wf);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ---------------------- Class layout_manager ---------------------- */

layout_manager::layout_manager(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "get_surface")) PyErr_Warn(PyExc_Warning, "layout_manager: missing attribute: get_surface");
		if (!PyObject_HasAttrString(itself, "get_alignment")) PyErr_Warn(PyExc_Warning, "layout_manager: missing attribute: get_alignment");
		if (!PyObject_HasAttrString(itself, "get_animation_notification")) PyErr_Warn(PyExc_Warning, "layout_manager: missing attribute: get_animation_notification");
		if (!PyObject_HasAttrString(itself, "get_animation_destination")) PyErr_Warn(PyExc_Warning, "layout_manager: missing attribute: get_animation_destination");
	}
	if (itself == NULL) itself = Py_None;

	py_layout_manager = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

layout_manager::~layout_manager()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_layout_manager;
	py_layout_manager = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::common::surface* layout_manager::get_surface(const ambulant::lib::node* node)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::surface* _rv;
	PyObject *py_node = Py_BuildValue("O&", nodeObj_New, node);

	PyObject *py_rv = PyObject_CallMethod(py_layout_manager, "get_surface", "(O)", py_node);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during layout_manager::get_surface() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", surfaceObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during layout_manager::get_surface() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_node);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::alignment* layout_manager::get_alignment(const ambulant::lib::node* node)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::alignment* _rv;
	PyObject *py_node = Py_BuildValue("O&", nodeObj_New, node);

	PyObject *py_rv = PyObject_CallMethod(py_layout_manager, "get_alignment", "(O)", py_node);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during layout_manager::get_alignment() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", alignmentObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during layout_manager::get_alignment() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_node);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::animation_notification* layout_manager::get_animation_notification(const ambulant::lib::node* node)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::animation_notification* _rv;
	PyObject *py_node = Py_BuildValue("O&", nodeObj_New, node);

	PyObject *py_rv = PyObject_CallMethod(py_layout_manager, "get_animation_notification", "(O)", py_node);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during layout_manager::get_animation_notification() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", animation_notificationObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during layout_manager::get_animation_notification() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_node);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::animation_destination* layout_manager::get_animation_destination(const ambulant::lib::node* node)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::animation_destination* _rv;
	PyObject *py_node = Py_BuildValue("O&", nodeObj_New, node);

	PyObject *py_rv = PyObject_CallMethod(py_layout_manager, "get_animation_destination", "(O)", py_node);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during layout_manager::get_animation_destination() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", animation_destinationObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during layout_manager::get_animation_destination() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_node);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ------------------------- Class playable ------------------------- */

playable::playable(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "init_with_node")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: init_with_node");
		if (!PyObject_HasAttrString(itself, "start")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: start");
		if (!PyObject_HasAttrString(itself, "stop")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: stop");
		if (!PyObject_HasAttrString(itself, "post_stop")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: post_stop");
		if (!PyObject_HasAttrString(itself, "pause")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: pause");
		if (!PyObject_HasAttrString(itself, "resume")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: resume");
		if (!PyObject_HasAttrString(itself, "seek")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: seek");
		if (!PyObject_HasAttrString(itself, "wantclicks")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: wantclicks");
		if (!PyObject_HasAttrString(itself, "preroll")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: preroll");
		if (!PyObject_HasAttrString(itself, "get_dur")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: get_dur");
		if (!PyObject_HasAttrString(itself, "get_cookie")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: get_cookie");
		if (!PyObject_HasAttrString(itself, "get_renderer")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: get_renderer");
		if (!PyObject_HasAttrString(itself, "get_sig")) PyErr_Warn(PyExc_Warning, "playable: missing attribute: get_sig");
	}
	if (itself == NULL) itself = Py_None;

	py_playable = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

playable::~playable()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_playable;
	py_playable = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void playable::init_with_node(const ambulant::lib::node* n)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);

	PyObject *py_rv = PyObject_CallMethod(py_playable, "init_with_node", "(O)", py_n);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::init_with_node() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);

	PyGILState_Release(_GILState);
}

void playable::start(double t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_t = Py_BuildValue("d", t);

	PyObject *py_rv = PyObject_CallMethod(py_playable, "start", "(O)", py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::start() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}

bool playable::stop()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_playable, "stop", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::stop() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during playable::stop() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void playable::post_stop()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_playable, "post_stop", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::post_stop() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void playable::pause(ambulant::common::pause_display d)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_d = Py_BuildValue("l", d);

	PyObject *py_rv = PyObject_CallMethod(py_playable, "pause", "(O)", py_d);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::pause() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_d);

	PyGILState_Release(_GILState);
}

void playable::resume()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_playable, "resume", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::resume() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void playable::seek(double t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_t = Py_BuildValue("d", t);

	PyObject *py_rv = PyObject_CallMethod(py_playable, "seek", "(O)", py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::seek() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}

void playable::wantclicks(bool want)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_want = Py_BuildValue("O&", bool_New, want);

	PyObject *py_rv = PyObject_CallMethod(py_playable, "wantclicks", "(O)", py_want);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::wantclicks() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_want);

	PyGILState_Release(_GILState);
}

void playable::preroll(double when, double where, double how_much)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_when = Py_BuildValue("d", when);
	PyObject *py_where = Py_BuildValue("d", where);
	PyObject *py_how_much = Py_BuildValue("d", how_much);

	PyObject *py_rv = PyObject_CallMethod(py_playable, "preroll", "(OOO)", py_when, py_where, py_how_much);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::preroll() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_when);
	Py_XDECREF(py_where);
	Py_XDECREF(py_how_much);

	PyGILState_Release(_GILState);
}

ambulant::common::duration playable::get_dur()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::duration _rv;

	PyObject *py_rv = PyObject_CallMethod(py_playable, "get_dur", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::get_dur() callback:\n");
		PyErr_Print();
	}

	bool _rv_first;
	double _rv_second;
	if (py_rv && !PyArg_Parse(py_rv, "(O&d)", bool_Convert, &_rv_first, &_rv_second))
	{
		PySys_WriteStderr("Python exception during playable::get_dur() return:\n");
		PyErr_Print();
	}

	_rv = ambulant::common::duration(_rv_first, _rv_second);
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::playable::cookie_type playable::get_cookie() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::playable::cookie_type _rv;

	PyObject *py_rv = PyObject_CallMethod(py_playable, "get_cookie", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::get_cookie() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during playable::get_cookie() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::renderer* playable::get_renderer()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::renderer* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_playable, "get_renderer", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::get_renderer() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", rendererObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during playable::get_renderer() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

std::string playable::get_sig() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	std::string _rv;

	PyObject *py_rv = PyObject_CallMethod(py_playable, "get_sig", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable::get_sig() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during playable::get_sig() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ------------------ Class playable_notification ------------------- */

playable_notification::playable_notification(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "started")) PyErr_Warn(PyExc_Warning, "playable_notification: missing attribute: started");
		if (!PyObject_HasAttrString(itself, "stopped")) PyErr_Warn(PyExc_Warning, "playable_notification: missing attribute: stopped");
		if (!PyObject_HasAttrString(itself, "clicked")) PyErr_Warn(PyExc_Warning, "playable_notification: missing attribute: clicked");
		if (!PyObject_HasAttrString(itself, "pointed")) PyErr_Warn(PyExc_Warning, "playable_notification: missing attribute: pointed");
		if (!PyObject_HasAttrString(itself, "transitioned")) PyErr_Warn(PyExc_Warning, "playable_notification: missing attribute: transitioned");
		if (!PyObject_HasAttrString(itself, "marker_seen")) PyErr_Warn(PyExc_Warning, "playable_notification: missing attribute: marker_seen");
		if (!PyObject_HasAttrString(itself, "playable_stalled")) PyErr_Warn(PyExc_Warning, "playable_notification: missing attribute: playable_stalled");
		if (!PyObject_HasAttrString(itself, "playable_unstalled")) PyErr_Warn(PyExc_Warning, "playable_notification: missing attribute: playable_unstalled");
		if (!PyObject_HasAttrString(itself, "playable_started")) PyErr_Warn(PyExc_Warning, "playable_notification: missing attribute: playable_started");
		if (!PyObject_HasAttrString(itself, "playable_resource")) PyErr_Warn(PyExc_Warning, "playable_notification: missing attribute: playable_resource");
	}
	if (itself == NULL) itself = Py_None;

	py_playable_notification = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

playable_notification::~playable_notification()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_playable_notification;
	py_playable_notification = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void playable_notification::started(ambulant::common::playable::cookie_type n, double t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("l", n);
	PyObject *py_t = Py_BuildValue("d", t);

	PyObject *py_rv = PyObject_CallMethod(py_playable_notification, "started", "(OO)", py_n, py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_notification::started() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}

void playable_notification::stopped(ambulant::common::playable::cookie_type n, double t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("l", n);
	PyObject *py_t = Py_BuildValue("d", t);

	PyObject *py_rv = PyObject_CallMethod(py_playable_notification, "stopped", "(OO)", py_n, py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_notification::stopped() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}

void playable_notification::clicked(ambulant::common::playable::cookie_type n, double t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("l", n);
	PyObject *py_t = Py_BuildValue("d", t);

	PyObject *py_rv = PyObject_CallMethod(py_playable_notification, "clicked", "(OO)", py_n, py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_notification::clicked() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}

void playable_notification::pointed(ambulant::common::playable::cookie_type n, double t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("l", n);
	PyObject *py_t = Py_BuildValue("d", t);

	PyObject *py_rv = PyObject_CallMethod(py_playable_notification, "pointed", "(OO)", py_n, py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_notification::pointed() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}

void playable_notification::transitioned(ambulant::common::playable::cookie_type n, double t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("l", n);
	PyObject *py_t = Py_BuildValue("d", t);

	PyObject *py_rv = PyObject_CallMethod(py_playable_notification, "transitioned", "(OO)", py_n, py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_notification::transitioned() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}

void playable_notification::marker_seen(ambulant::common::playable::cookie_type n, const char* name, double t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("l", n);
	PyObject *py_name = Py_BuildValue("s", name);
	PyObject *py_t = Py_BuildValue("d", t);

	PyObject *py_rv = PyObject_CallMethod(py_playable_notification, "marker_seen", "(OOO)", py_n, py_name, py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_notification::marker_seen() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);
	Py_XDECREF(py_name);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}

void playable_notification::playable_stalled(const ambulant::common::playable* p, const char* reason)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playableObj_New, p);
	PyObject *py_reason = Py_BuildValue("s", reason);

	PyObject *py_rv = PyObject_CallMethod(py_playable_notification, "playable_stalled", "(OO)", py_p, py_reason);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_notification::playable_stalled() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);
	Py_XDECREF(py_reason);

	PyGILState_Release(_GILState);
}

void playable_notification::playable_unstalled(const ambulant::common::playable* p)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playableObj_New, p);

	PyObject *py_rv = PyObject_CallMethod(py_playable_notification, "playable_unstalled", "(O)", py_p);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_notification::playable_unstalled() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);

	PyGILState_Release(_GILState);
}

void playable_notification::playable_started(const ambulant::common::playable* p, const ambulant::lib::node* n, const char* comment)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playableObj_New, p);
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);
	PyObject *py_comment = Py_BuildValue("s", comment);

	PyObject *py_rv = PyObject_CallMethod(py_playable_notification, "playable_started", "(OOO)", py_p, py_n, py_comment);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_notification::playable_started() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);
	Py_XDECREF(py_n);
	Py_XDECREF(py_comment);

	PyGILState_Release(_GILState);
}

void playable_notification::playable_resource(const ambulant::common::playable* p, const char* resource, long amount)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playableObj_New, p);
	PyObject *py_resource = Py_BuildValue("s", resource);
	PyObject *py_amount = Py_BuildValue("l", amount);

	PyObject *py_rv = PyObject_CallMethod(py_playable_notification, "playable_resource", "(OOO)", py_p, py_resource, py_amount);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_notification::playable_resource() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);
	Py_XDECREF(py_resource);
	Py_XDECREF(py_amount);

	PyGILState_Release(_GILState);
}

/* --------------------- Class playable_factory --------------------- */

playable_factory::playable_factory(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "supports")) PyErr_Warn(PyExc_Warning, "playable_factory: missing attribute: supports");
		if (!PyObject_HasAttrString(itself, "new_playable")) PyErr_Warn(PyExc_Warning, "playable_factory: missing attribute: new_playable");
		if (!PyObject_HasAttrString(itself, "new_aux_audio_playable")) PyErr_Warn(PyExc_Warning, "playable_factory: missing attribute: new_aux_audio_playable");
	}
	if (itself == NULL) itself = Py_None;

	py_playable_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

playable_factory::~playable_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_playable_factory;
	py_playable_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


bool playable_factory::supports(ambulant::common::renderer_select* rs)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_rs = Py_BuildValue("O&", renderer_selectObj_New, rs);

	PyObject *py_rv = PyObject_CallMethod(py_playable_factory, "supports", "(O)", py_rs);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_factory::supports() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during playable_factory::supports() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_rs);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::playable* playable_factory::new_playable(ambulant::common::playable_notification* context, ambulant::common::playable::cookie_type cookie, const ambulant::lib::node* node, ambulant::lib::event_processor* evp)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::playable* _rv;
	PyObject *py_context = Py_BuildValue("O&", playable_notificationObj_New, context);
	PyObject *py_cookie = Py_BuildValue("l", cookie);
	PyObject *py_node = Py_BuildValue("O&", nodeObj_New, node);
	PyObject *py_evp = Py_BuildValue("O&", event_processorObj_New, evp);

	PyObject *py_rv = PyObject_CallMethod(py_playable_factory, "new_playable", "(OOOO)", py_context, py_cookie, py_node, py_evp);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_factory::new_playable() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", playableObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during playable_factory::new_playable() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_context);
	Py_XDECREF(py_cookie);
	Py_XDECREF(py_node);
	Py_XDECREF(py_evp);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::playable* playable_factory::new_aux_audio_playable(ambulant::common::playable_notification* context, ambulant::common::playable::cookie_type cookie, const ambulant::lib::node* node, ambulant::lib::event_processor* evp, ambulant::net::audio_datasource* src)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::playable* _rv;
	PyObject *py_context = Py_BuildValue("O&", playable_notificationObj_New, context);
	PyObject *py_cookie = Py_BuildValue("l", cookie);
	PyObject *py_node = Py_BuildValue("O&", nodeObj_New, node);
	PyObject *py_evp = Py_BuildValue("O&", event_processorObj_New, evp);
	PyObject *py_src = Py_BuildValue("O&", audio_datasourceObj_New, src);

	PyObject *py_rv = PyObject_CallMethod(py_playable_factory, "new_aux_audio_playable", "(OOOOO)", py_context, py_cookie, py_node, py_evp, py_src);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during playable_factory::new_aux_audio_playable() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", playableObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during playable_factory::new_aux_audio_playable() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_context);
	Py_XDECREF(py_cookie);
	Py_XDECREF(py_node);
	Py_XDECREF(py_evp);
	Py_XDECREF(py_src);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ----------------- Class global_playable_factory ------------------ */

global_playable_factory::global_playable_factory(PyObject *itself)
:	::playable_factory(itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "add_factory")) PyErr_Warn(PyExc_Warning, "global_playable_factory: missing attribute: add_factory");
		if (!PyObject_HasAttrString(itself, "preferred_renderer")) PyErr_Warn(PyExc_Warning, "global_playable_factory: missing attribute: preferred_renderer");
	}
	if (itself == NULL) itself = Py_None;

	py_global_playable_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

global_playable_factory::~global_playable_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_global_playable_factory;
	py_global_playable_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void global_playable_factory::add_factory(ambulant::common::playable_factory* rf)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rf = Py_BuildValue("O&", playable_factoryObj_New, rf);

	PyObject *py_rv = PyObject_CallMethod(py_global_playable_factory, "add_factory", "(O)", py_rf);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during global_playable_factory::add_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_rf);

	PyGILState_Release(_GILState);
}

void global_playable_factory::preferred_renderer(const char* name)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_name = Py_BuildValue("s", name);

	PyObject *py_rv = PyObject_CallMethod(py_global_playable_factory, "preferred_renderer", "(O)", py_name);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during global_playable_factory::preferred_renderer() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);

	PyGILState_Release(_GILState);
}

/* ------------------------- Class recorder ------------------------- */

recorder::recorder(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_video_data")) PyErr_Warn(PyExc_Warning, "recorder: missing attribute: new_video_data");
		if (!PyObject_HasAttrString(itself, "new_audio_data")) PyErr_Warn(PyExc_Warning, "recorder: missing attribute: new_audio_data");
	}
	if (itself == NULL) itself = Py_None;

	py_recorder = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

recorder::~recorder()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_recorder;
	py_recorder = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void recorder::new_video_data(const char *data__in__, size_t data__len__, ambulant::lib::timer::time_type documenttimestamp)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_data = Py_BuildValue("s#", data__in__, (int)data__len__);
	PyObject *py_documenttimestamp = Py_BuildValue("l", documenttimestamp);

	PyObject *py_rv = PyObject_CallMethod(py_recorder, "new_video_data", "(OO)", py_data, py_documenttimestamp);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during recorder::new_video_data() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_data);
	Py_XDECREF(py_documenttimestamp);

	PyGILState_Release(_GILState);
}

void recorder::new_audio_data(const char *data__in__, size_t data__len__, ambulant::lib::timer::time_type documenttimestamp)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_data = Py_BuildValue("s#", data__in__, (int)data__len__);
	PyObject *py_documenttimestamp = Py_BuildValue("l", documenttimestamp);

	PyObject *py_rv = PyObject_CallMethod(py_recorder, "new_audio_data", "(OO)", py_data, py_documenttimestamp);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during recorder::new_audio_data() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_data);
	Py_XDECREF(py_documenttimestamp);

	PyGILState_Release(_GILState);
}

/* --------------------- Class recorder_factory --------------------- */

recorder_factory::recorder_factory(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_recorder")) PyErr_Warn(PyExc_Warning, "recorder_factory: missing attribute: new_recorder");
	}
	if (itself == NULL) itself = Py_None;

	py_recorder_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

recorder_factory::~recorder_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_recorder_factory;
	py_recorder_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::common::recorder* recorder_factory::new_recorder(ambulant::net::pixel_order pixel_order, ambulant::lib::size window_size)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::recorder* _rv;
	PyObject *py_pixel_order = Py_BuildValue("l", pixel_order);
	PyObject *py_window_size = Py_BuildValue("O", ambulant_size_New(window_size));

	PyObject *py_rv = PyObject_CallMethod(py_recorder_factory, "new_recorder", "(OO)", py_pixel_order, py_window_size);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during recorder_factory::new_recorder() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", recorderObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during recorder_factory::new_recorder() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_pixel_order);
	Py_XDECREF(py_window_size);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ---------------------- Class focus_feedback ---------------------- */

focus_feedback::focus_feedback(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "node_focussed")) PyErr_Warn(PyExc_Warning, "focus_feedback: missing attribute: node_focussed");
	}
	if (itself == NULL) itself = Py_None;

	py_focus_feedback = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

focus_feedback::~focus_feedback()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_focus_feedback;
	py_focus_feedback = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void focus_feedback::node_focussed(const ambulant::lib::node* n)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);

	PyObject *py_rv = PyObject_CallMethod(py_focus_feedback, "node_focussed", "(O)", py_n);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during focus_feedback::node_focussed() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);

	PyGILState_Release(_GILState);
}

/* --------------------- Class player_feedback ---------------------- */

player_feedback::player_feedback(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "document_loaded")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: document_loaded");
		if (!PyObject_HasAttrString(itself, "document_started")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: document_started");
		if (!PyObject_HasAttrString(itself, "document_stopped")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: document_stopped");
		if (!PyObject_HasAttrString(itself, "node_started")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: node_started");
		if (!PyObject_HasAttrString(itself, "node_filled")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: node_filled");
		if (!PyObject_HasAttrString(itself, "node_stopped")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: node_stopped");
		if (!PyObject_HasAttrString(itself, "playable_started")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: playable_started");
		if (!PyObject_HasAttrString(itself, "playable_stalled")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: playable_stalled");
		if (!PyObject_HasAttrString(itself, "playable_unstalled")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: playable_unstalled");
		if (!PyObject_HasAttrString(itself, "playable_cached")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: playable_cached");
		if (!PyObject_HasAttrString(itself, "playable_deleted")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: playable_deleted");
		if (!PyObject_HasAttrString(itself, "playable_resource")) PyErr_Warn(PyExc_Warning, "player_feedback: missing attribute: playable_resource");
	}
	if (itself == NULL) itself = Py_None;

	py_player_feedback = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

player_feedback::~player_feedback()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_player_feedback;
	py_player_feedback = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void player_feedback::document_loaded(ambulant::lib::document* doc)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_doc = Py_BuildValue("O&", documentObj_New, doc);

	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "document_loaded", "(O)", py_doc);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::document_loaded() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_doc);

	PyGILState_Release(_GILState);
}

void player_feedback::document_started()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "document_started", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::document_started() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void player_feedback::document_stopped()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "document_stopped", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::document_stopped() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void player_feedback::node_started(const ambulant::lib::node* n)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);

	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "node_started", "(O)", py_n);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::node_started() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);

	PyGILState_Release(_GILState);
}

void player_feedback::node_filled(const ambulant::lib::node* n)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);

	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "node_filled", "(O)", py_n);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::node_filled() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);

	PyGILState_Release(_GILState);
}

void player_feedback::node_stopped(const ambulant::lib::node* n)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);

	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "node_stopped", "(O)", py_n);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::node_stopped() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);

	PyGILState_Release(_GILState);
}

void player_feedback::playable_started(const ambulant::common::playable* p, const ambulant::lib::node* n, const char* comment)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playableObj_New, p);
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);
	PyObject *py_comment = Py_BuildValue("s", comment);

	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "playable_started", "(OOO)", py_p, py_n, py_comment);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::playable_started() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);
	Py_XDECREF(py_n);
	Py_XDECREF(py_comment);

	PyGILState_Release(_GILState);
}

void player_feedback::playable_stalled(const ambulant::common::playable* p, const char* reason)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playableObj_New, p);
	PyObject *py_reason = Py_BuildValue("s", reason);

	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "playable_stalled", "(OO)", py_p, py_reason);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::playable_stalled() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);
	Py_XDECREF(py_reason);

	PyGILState_Release(_GILState);
}

void player_feedback::playable_unstalled(const ambulant::common::playable* p)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playableObj_New, p);

	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "playable_unstalled", "(O)", py_p);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::playable_unstalled() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);

	PyGILState_Release(_GILState);
}

void player_feedback::playable_cached(const ambulant::common::playable* p)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playableObj_New, p);

	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "playable_cached", "(O)", py_p);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::playable_cached() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);

	PyGILState_Release(_GILState);
}

void player_feedback::playable_deleted(const ambulant::common::playable* p)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playableObj_New, p);

	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "playable_deleted", "(O)", py_p);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::playable_deleted() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);

	PyGILState_Release(_GILState);
}

void player_feedback::playable_resource(const ambulant::common::playable* p, const char* resource, long amount)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_p = Py_BuildValue("O&", playableObj_New, p);
	PyObject *py_resource = Py_BuildValue("s", resource);
	PyObject *py_amount = Py_BuildValue("l", amount);

	PyObject *py_rv = PyObject_CallMethod(py_player_feedback, "playable_resource", "(OOO)", py_p, py_resource, py_amount);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player_feedback::playable_resource() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_p);
	Py_XDECREF(py_resource);
	Py_XDECREF(py_amount);

	PyGILState_Release(_GILState);
}

/* -------------------------- Class player -------------------------- */

player::player(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "initialize")) PyErr_Warn(PyExc_Warning, "player: missing attribute: initialize");
		if (!PyObject_HasAttrString(itself, "terminate")) PyErr_Warn(PyExc_Warning, "player: missing attribute: terminate");
		if (!PyObject_HasAttrString(itself, "get_timer")) PyErr_Warn(PyExc_Warning, "player: missing attribute: get_timer");
		if (!PyObject_HasAttrString(itself, "get_evp")) PyErr_Warn(PyExc_Warning, "player: missing attribute: get_evp");
		if (!PyObject_HasAttrString(itself, "start")) PyErr_Warn(PyExc_Warning, "player: missing attribute: start");
		if (!PyObject_HasAttrString(itself, "stop")) PyErr_Warn(PyExc_Warning, "player: missing attribute: stop");
		if (!PyObject_HasAttrString(itself, "pause")) PyErr_Warn(PyExc_Warning, "player: missing attribute: pause");
		if (!PyObject_HasAttrString(itself, "resume")) PyErr_Warn(PyExc_Warning, "player: missing attribute: resume");
		if (!PyObject_HasAttrString(itself, "is_playing")) PyErr_Warn(PyExc_Warning, "player: missing attribute: is_playing");
		if (!PyObject_HasAttrString(itself, "is_pausing")) PyErr_Warn(PyExc_Warning, "player: missing attribute: is_pausing");
		if (!PyObject_HasAttrString(itself, "is_done")) PyErr_Warn(PyExc_Warning, "player: missing attribute: is_done");
		if (!PyObject_HasAttrString(itself, "after_mousemove")) PyErr_Warn(PyExc_Warning, "player: missing attribute: after_mousemove");
		if (!PyObject_HasAttrString(itself, "before_mousemove")) PyErr_Warn(PyExc_Warning, "player: missing attribute: before_mousemove");
		if (!PyObject_HasAttrString(itself, "on_char")) PyErr_Warn(PyExc_Warning, "player: missing attribute: on_char");
		if (!PyObject_HasAttrString(itself, "on_state_change")) PyErr_Warn(PyExc_Warning, "player: missing attribute: on_state_change");
		if (!PyObject_HasAttrString(itself, "get_state_engine")) PyErr_Warn(PyExc_Warning, "player: missing attribute: get_state_engine");
		if (!PyObject_HasAttrString(itself, "on_focus_advance")) PyErr_Warn(PyExc_Warning, "player: missing attribute: on_focus_advance");
		if (!PyObject_HasAttrString(itself, "on_focus_activate")) PyErr_Warn(PyExc_Warning, "player: missing attribute: on_focus_activate");
		if (!PyObject_HasAttrString(itself, "set_focus_feedback")) PyErr_Warn(PyExc_Warning, "player: missing attribute: set_focus_feedback");
		if (!PyObject_HasAttrString(itself, "set_feedback")) PyErr_Warn(PyExc_Warning, "player: missing attribute: set_feedback");
		if (!PyObject_HasAttrString(itself, "get_feedback")) PyErr_Warn(PyExc_Warning, "player: missing attribute: get_feedback");
		if (!PyObject_HasAttrString(itself, "goto_node")) PyErr_Warn(PyExc_Warning, "player: missing attribute: goto_node");
		if (!PyObject_HasAttrString(itself, "highlight")) PyErr_Warn(PyExc_Warning, "player: missing attribute: highlight");
		if (!PyObject_HasAttrString(itself, "clicked_external")) PyErr_Warn(PyExc_Warning, "player: missing attribute: clicked_external");
		if (!PyObject_HasAttrString(itself, "uses_external_sync")) PyErr_Warn(PyExc_Warning, "player: missing attribute: uses_external_sync");
	}
	if (itself == NULL) itself = Py_None;

	py_player = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

player::~player()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_player;
	py_player = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void player::initialize()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_player, "initialize", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::initialize() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void player::terminate()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_player, "terminate", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::terminate() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

ambulant::lib::timer* player::get_timer()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::timer* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_player, "get_timer", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::get_timer() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", timerObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during player::get_timer() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::event_processor* player::get_evp()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::event_processor* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_player, "get_evp", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::get_evp() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", event_processorObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during player::get_evp() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void player::start()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_player, "start", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::start() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void player::stop()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_player, "stop", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::stop() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void player::pause()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_player, "pause", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::pause() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void player::resume()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_player, "resume", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::resume() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

bool player::is_playing() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_player, "is_playing", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::is_playing() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during player::is_playing() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool player::is_pausing() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_player, "is_pausing", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::is_pausing() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during player::is_pausing() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool player::is_done() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_player, "is_done", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::is_done() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during player::is_done() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

int player::after_mousemove()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	int _rv;

	PyObject *py_rv = PyObject_CallMethod(py_player, "after_mousemove", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::after_mousemove() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "i", &_rv))
	{
		PySys_WriteStderr("Python exception during player::after_mousemove() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void player::before_mousemove(int cursor)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_cursor = Py_BuildValue("i", cursor);

	PyObject *py_rv = PyObject_CallMethod(py_player, "before_mousemove", "(O)", py_cursor);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::before_mousemove() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_cursor);

	PyGILState_Release(_GILState);
}

void player::on_char(int ch)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_ch = Py_BuildValue("i", ch);

	PyObject *py_rv = PyObject_CallMethod(py_player, "on_char", "(O)", py_ch);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::on_char() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_ch);

	PyGILState_Release(_GILState);
}

void player::on_state_change(const char* ref)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_ref = Py_BuildValue("s", ref);

	PyObject *py_rv = PyObject_CallMethod(py_player, "on_state_change", "(O)", py_ref);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::on_state_change() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_ref);

	PyGILState_Release(_GILState);
}

ambulant::common::state_component* player::get_state_engine()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::state_component* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_player, "get_state_engine", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::get_state_engine() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", state_componentObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during player::get_state_engine() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void player::on_focus_advance()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_player, "on_focus_advance", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::on_focus_advance() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void player::on_focus_activate()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_player, "on_focus_activate", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::on_focus_activate() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

void player::set_focus_feedback(ambulant::common::focus_feedback* fb)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_fb = Py_BuildValue("O&", focus_feedbackObj_New, fb);

	PyObject *py_rv = PyObject_CallMethod(py_player, "set_focus_feedback", "(O)", py_fb);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::set_focus_feedback() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_fb);

	PyGILState_Release(_GILState);
}

void player::set_feedback(ambulant::common::player_feedback* fb)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_fb = Py_BuildValue("O&", player_feedbackObj_New, fb);

	PyObject *py_rv = PyObject_CallMethod(py_player, "set_feedback", "(O)", py_fb);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::set_feedback() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_fb);

	PyGILState_Release(_GILState);
}

ambulant::common::player_feedback* player::get_feedback()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::player_feedback* _rv;

	PyObject *py_rv = PyObject_CallMethod(py_player, "get_feedback", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::get_feedback() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", player_feedbackObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during player::get_feedback() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool player::goto_node(const ambulant::lib::node* n)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);

	PyObject *py_rv = PyObject_CallMethod(py_player, "goto_node", "(O)", py_n);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::goto_node() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during player::goto_node() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);

	PyGILState_Release(_GILState);
	return _rv;
}

bool player::highlight(const ambulant::lib::node* n, bool on)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);
	PyObject *py_on = Py_BuildValue("O&", bool_New, on);

	PyObject *py_rv = PyObject_CallMethod(py_player, "highlight", "(OO)", py_n, py_on);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::highlight() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during player::highlight() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);
	Py_XDECREF(py_on);

	PyGILState_Release(_GILState);
	return _rv;
}

#ifdef WITH_REMOTE_SYNC
void player::clicked_external(ambulant::lib::node* n, ambulant::lib::timer::time_type t)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_n = Py_BuildValue("O&", nodeObj_New, n);
	PyObject *py_t = Py_BuildValue("l", t);

	PyObject *py_rv = PyObject_CallMethod(py_player, "clicked_external", "(OO)", py_n, py_t);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::clicked_external() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_n);
	Py_XDECREF(py_t);

	PyGILState_Release(_GILState);
}
#endif

#ifdef WITH_REMOTE_SYNC
bool player::uses_external_sync() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_player, "uses_external_sync", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during player::uses_external_sync() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during player::uses_external_sync() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}
#endif

/* ----------------------- Class region_info ------------------------ */

region_info::region_info(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "get_name")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_name");
		if (!PyObject_HasAttrString(itself, "get_rect")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_rect");
		if (!PyObject_HasAttrString(itself, "get_fit")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_fit");
		if (!PyObject_HasAttrString(itself, "get_bgcolor")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_bgcolor");
		if (!PyObject_HasAttrString(itself, "get_bgopacity")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_bgopacity");
		if (!PyObject_HasAttrString(itself, "get_transparent")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_transparent");
		if (!PyObject_HasAttrString(itself, "get_zindex")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_zindex");
		if (!PyObject_HasAttrString(itself, "get_showbackground")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_showbackground");
		if (!PyObject_HasAttrString(itself, "is_subregion")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: is_subregion");
		if (!PyObject_HasAttrString(itself, "get_soundlevel")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_soundlevel");
		if (!PyObject_HasAttrString(itself, "get_soundalign")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_soundalign");
		if (!PyObject_HasAttrString(itself, "get_tiling")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_tiling");
		if (!PyObject_HasAttrString(itself, "get_bgimage")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_bgimage");
		if (!PyObject_HasAttrString(itself, "get_crop_rect")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_crop_rect");
		if (!PyObject_HasAttrString(itself, "get_mediaopacity")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_mediaopacity");
		if (!PyObject_HasAttrString(itself, "get_mediabgopacity")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_mediabgopacity");
		if (!PyObject_HasAttrString(itself, "is_chromakey_specified")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: is_chromakey_specified");
		if (!PyObject_HasAttrString(itself, "get_chromakey")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_chromakey");
		if (!PyObject_HasAttrString(itself, "get_chromakeytolerance")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_chromakeytolerance");
		if (!PyObject_HasAttrString(itself, "get_chromakeyopacity")) PyErr_Warn(PyExc_Warning, "region_info: missing attribute: get_chromakeyopacity");
	}
	if (itself == NULL) itself = Py_None;

	py_region_info = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

region_info::~region_info()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_region_info;
	py_region_info = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


std::string region_info::get_name() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	std::string _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_name", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_name() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during region_info::get_name() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::rect region_info::get_rect(const ambulant::lib::rect * default_rect) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::rect _rv;
	PyObject *py_default_rect = Py_BuildValue("O&", ambulant_rect_New, default_rect);

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_rect", "(O)", py_default_rect);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_rect() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_rect_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_rect() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_default_rect);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::fit_t region_info::get_fit() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::fit_t _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_fit", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_fit() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_fit() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::color_t region_info::get_bgcolor() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::color_t _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_bgcolor", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_bgcolor() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_bgcolor() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

double region_info::get_bgopacity() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	double _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_bgopacity", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_bgopacity() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "d", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_bgopacity() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool region_info::get_transparent() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_transparent", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_transparent() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_transparent() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::zindex_t region_info::get_zindex() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::zindex_t _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_zindex", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_zindex() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_zindex() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool region_info::get_showbackground() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_showbackground", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_showbackground() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_showbackground() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool region_info::is_subregion() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "is_subregion", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::is_subregion() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::is_subregion() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

double region_info::get_soundlevel() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	double _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_soundlevel", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_soundlevel() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "d", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_soundlevel() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::sound_alignment region_info::get_soundalign() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::sound_alignment _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_soundalign", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_soundalign() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_soundalign() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::tiling region_info::get_tiling() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::tiling _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_tiling", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_tiling() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_tiling() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

const char * region_info::get_bgimage() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	const char * _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_bgimage", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_bgimage() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "z", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_bgimage() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::rect region_info::get_crop_rect(const ambulant::lib::size& srcsize) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::rect _rv;
	PyObject *py_srcsize = Py_BuildValue("O", ambulant_size_New(srcsize));

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_crop_rect", "(O)", py_srcsize);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_crop_rect() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_rect_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_crop_rect() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_srcsize);

	PyGILState_Release(_GILState);
	return _rv;
}

double region_info::get_mediaopacity() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	double _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_mediaopacity", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_mediaopacity() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "d", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_mediaopacity() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

double region_info::get_mediabgopacity() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	double _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_mediabgopacity", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_mediabgopacity() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "d", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_mediabgopacity() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool region_info::is_chromakey_specified() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "is_chromakey_specified", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::is_chromakey_specified() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::is_chromakey_specified() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::color_t region_info::get_chromakey() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::color_t _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_chromakey", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_chromakey() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_chromakey() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::color_t region_info::get_chromakeytolerance() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::color_t _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_chromakeytolerance", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_chromakeytolerance() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_chromakeytolerance() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

double region_info::get_chromakeyopacity() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	double _rv;

	PyObject *py_rv = PyObject_CallMethod(py_region_info, "get_chromakeyopacity", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during region_info::get_chromakeyopacity() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "d", &_rv))
	{
		PySys_WriteStderr("Python exception during region_info::get_chromakeyopacity() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ------------------ Class animation_destination ------------------- */

animation_destination::animation_destination(PyObject *itself)
:	::region_info(itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "get_region_dim")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: get_region_dim");
		if (!PyObject_HasAttrString(itself, "get_region_color")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: get_region_color");
		if (!PyObject_HasAttrString(itself, "get_region_zindex")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: get_region_zindex");
		if (!PyObject_HasAttrString(itself, "get_region_soundlevel")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: get_region_soundlevel");
		if (!PyObject_HasAttrString(itself, "get_region_soundalign")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: get_region_soundalign");
		if (!PyObject_HasAttrString(itself, "get_region_opacity")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: get_region_opacity");
		if (!PyObject_HasAttrString(itself, "set_region_dim")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: set_region_dim");
		if (!PyObject_HasAttrString(itself, "set_region_color")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: set_region_color");
		if (!PyObject_HasAttrString(itself, "set_region_zindex")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: set_region_zindex");
		if (!PyObject_HasAttrString(itself, "set_region_soundlevel")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: set_region_soundlevel");
		if (!PyObject_HasAttrString(itself, "set_region_soundalign")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: set_region_soundalign");
		if (!PyObject_HasAttrString(itself, "set_region_opacity")) PyErr_Warn(PyExc_Warning, "animation_destination: missing attribute: set_region_opacity");
	}
	if (itself == NULL) itself = Py_None;

	py_animation_destination = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

animation_destination::~animation_destination()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_animation_destination;
	py_animation_destination = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::common::region_dim animation_destination::get_region_dim(const std::string& which, bool fromdom) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::region_dim _rv;
	PyObject *py_which = Py_BuildValue("s", which.c_str());
	PyObject *py_fromdom = Py_BuildValue("O&", bool_New, fromdom);

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "get_region_dim", "(OO)", py_which, py_fromdom);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_dim() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", ambulant_region_dim_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_dim() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_which);
	Py_XDECREF(py_fromdom);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::lib::color_t animation_destination::get_region_color(const std::string& which, bool fromdom) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::lib::color_t _rv;
	PyObject *py_which = Py_BuildValue("s", which.c_str());
	PyObject *py_fromdom = Py_BuildValue("O&", bool_New, fromdom);

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "get_region_color", "(OO)", py_which, py_fromdom);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_color() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_color() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_which);
	Py_XDECREF(py_fromdom);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::zindex_t animation_destination::get_region_zindex(bool fromdom) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::zindex_t _rv;
	PyObject *py_fromdom = Py_BuildValue("O&", bool_New, fromdom);

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "get_region_zindex", "(O)", py_fromdom);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_zindex() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_zindex() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_fromdom);

	PyGILState_Release(_GILState);
	return _rv;
}

double animation_destination::get_region_soundlevel(bool fromdom) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	double _rv;
	PyObject *py_fromdom = Py_BuildValue("O&", bool_New, fromdom);

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "get_region_soundlevel", "(O)", py_fromdom);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_soundlevel() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "d", &_rv))
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_soundlevel() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_fromdom);

	PyGILState_Release(_GILState);
	return _rv;
}

ambulant::common::sound_alignment animation_destination::get_region_soundalign(bool fromdom) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::sound_alignment _rv;
	PyObject *py_fromdom = Py_BuildValue("O&", bool_New, fromdom);

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "get_region_soundalign", "(O)", py_fromdom);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_soundalign() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_soundalign() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_fromdom);

	PyGILState_Release(_GILState);
	return _rv;
}

double animation_destination::get_region_opacity(const std::string& which, bool fromdom) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	double _rv;
	PyObject *py_which = Py_BuildValue("s", which.c_str());
	PyObject *py_fromdom = Py_BuildValue("O&", bool_New, fromdom);

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "get_region_opacity", "(OO)", py_which, py_fromdom);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_opacity() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "d", &_rv))
	{
		PySys_WriteStderr("Python exception during animation_destination::get_region_opacity() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_which);
	Py_XDECREF(py_fromdom);

	PyGILState_Release(_GILState);
	return _rv;
}

void animation_destination::set_region_dim(const std::string& which, const ambulant::common::region_dim& rd)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_which = Py_BuildValue("s", which.c_str());
	PyObject *py_rd = Py_BuildValue("O", ambulant_region_dim_New(rd));

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "set_region_dim", "(OO)", py_which, py_rd);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::set_region_dim() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_which);
	Py_XDECREF(py_rd);

	PyGILState_Release(_GILState);
}

void animation_destination::set_region_color(const std::string& which, ambulant::lib::color_t clr)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_which = Py_BuildValue("s", which.c_str());
	PyObject *py_clr = Py_BuildValue("l", clr);

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "set_region_color", "(OO)", py_which, py_clr);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::set_region_color() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_which);
	Py_XDECREF(py_clr);

	PyGILState_Release(_GILState);
}

void animation_destination::set_region_zindex(ambulant::common::zindex_t z)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_z = Py_BuildValue("l", z);

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "set_region_zindex", "(O)", py_z);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::set_region_zindex() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_z);

	PyGILState_Release(_GILState);
}

void animation_destination::set_region_soundlevel(double level)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_level = Py_BuildValue("d", level);

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "set_region_soundlevel", "(O)", py_level);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::set_region_soundlevel() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_level);

	PyGILState_Release(_GILState);
}

void animation_destination::set_region_soundalign(ambulant::common::sound_alignment sa)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_sa = Py_BuildValue("l", sa);

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "set_region_soundalign", "(O)", py_sa);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::set_region_soundalign() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_sa);

	PyGILState_Release(_GILState);
}

void animation_destination::set_region_opacity(const std::string& which, double level)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_which = Py_BuildValue("s", which.c_str());
	PyObject *py_level = Py_BuildValue("d", level);

	PyObject *py_rv = PyObject_CallMethod(py_animation_destination, "set_region_opacity", "(OO)", py_which, py_level);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during animation_destination::set_region_opacity() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_which);
	Py_XDECREF(py_level);

	PyGILState_Release(_GILState);
}

/* -------------------- Class state_test_methods -------------------- */

state_test_methods::state_test_methods(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "smil_audio_desc")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_audio_desc");
		if (!PyObject_HasAttrString(itself, "smil_bitrate")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_bitrate");
		if (!PyObject_HasAttrString(itself, "smil_captions")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_captions");
		if (!PyObject_HasAttrString(itself, "smil_component")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_component");
		if (!PyObject_HasAttrString(itself, "smil_custom_test")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_custom_test");
		if (!PyObject_HasAttrString(itself, "smil_cpu")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_cpu");
		if (!PyObject_HasAttrString(itself, "smil_language")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_language");
		if (!PyObject_HasAttrString(itself, "smil_operating_system")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_operating_system");
		if (!PyObject_HasAttrString(itself, "smil_overdub_or_subtitle")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_overdub_or_subtitle");
		if (!PyObject_HasAttrString(itself, "smil_required")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_required");
		if (!PyObject_HasAttrString(itself, "smil_screen_depth")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_screen_depth");
		if (!PyObject_HasAttrString(itself, "smil_screen_height")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_screen_height");
		if (!PyObject_HasAttrString(itself, "smil_screen_width")) PyErr_Warn(PyExc_Warning, "state_test_methods: missing attribute: smil_screen_width");
	}
	if (itself == NULL) itself = Py_None;

	py_state_test_methods = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

state_test_methods::~state_test_methods()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_state_test_methods;
	py_state_test_methods = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


bool state_test_methods::smil_audio_desc() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_audio_desc", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_audio_desc() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_audio_desc() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

int state_test_methods::smil_bitrate() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	int _rv;

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_bitrate", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_bitrate() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "i", &_rv))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_bitrate() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool state_test_methods::smil_captions() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_captions", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_captions() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_captions() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool state_test_methods::smil_component(std::string uri) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_uri = Py_BuildValue("s", uri.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_component", "(O)", py_uri);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_component() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_component() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_uri);

	PyGILState_Release(_GILState);
	return _rv;
}

bool state_test_methods::smil_custom_test(std::string name) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_name = Py_BuildValue("s", name.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_custom_test", "(O)", py_name);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_custom_test() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_custom_test() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_name);

	PyGILState_Release(_GILState);
	return _rv;
}

std::string state_test_methods::smil_cpu() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	std::string _rv;

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_cpu", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_cpu() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_cpu() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

float state_test_methods::smil_language(std::string lang) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	float _rv;
	PyObject *py_lang = Py_BuildValue("s", lang.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_language", "(O)", py_lang);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_language() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "f", &_rv))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_language() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_lang);

	PyGILState_Release(_GILState);
	return _rv;
}

std::string state_test_methods::smil_operating_system() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	std::string _rv;

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_operating_system", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_operating_system() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_operating_system() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

std::string state_test_methods::smil_overdub_or_subtitle() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	std::string _rv;

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_overdub_or_subtitle", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_overdub_or_subtitle() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_overdub_or_subtitle() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

bool state_test_methods::smil_required(std::string uri) const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_uri = Py_BuildValue("s", uri.c_str());

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_required", "(O)", py_uri);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_required() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_required() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_uri);

	PyGILState_Release(_GILState);
	return _rv;
}

int state_test_methods::smil_screen_depth() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	int _rv;

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_screen_depth", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_screen_depth() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "i", &_rv))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_screen_depth() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

int state_test_methods::smil_screen_height() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	int _rv;

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_screen_height", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_screen_height() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "i", &_rv))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_screen_height() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

int state_test_methods::smil_screen_width() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	int _rv;

	PyObject *py_rv = PyObject_CallMethod(py_state_test_methods, "smil_screen_width", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_screen_width() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "i", &_rv))
	{
		PySys_WriteStderr("Python exception during state_test_methods::smil_screen_width() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ------------------ Class state_change_callback ------------------- */

state_change_callback::state_change_callback(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "on_state_change")) PyErr_Warn(PyExc_Warning, "state_change_callback: missing attribute: on_state_change");
	}
	if (itself == NULL) itself = Py_None;

	py_state_change_callback = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

state_change_callback::~state_change_callback()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_state_change_callback;
	py_state_change_callback = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void state_change_callback::on_state_change(const char* ref)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_ref = Py_BuildValue("s", ref);

	PyObject *py_rv = PyObject_CallMethod(py_state_change_callback, "on_state_change", "(O)", py_ref);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_change_callback::on_state_change() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_ref);

	PyGILState_Release(_GILState);
}

/* --------------------- Class state_component ---------------------- */

state_component::state_component(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "register_state_test_methods")) PyErr_Warn(PyExc_Warning, "state_component: missing attribute: register_state_test_methods");
		if (!PyObject_HasAttrString(itself, "declare_state")) PyErr_Warn(PyExc_Warning, "state_component: missing attribute: declare_state");
		if (!PyObject_HasAttrString(itself, "bool_expression")) PyErr_Warn(PyExc_Warning, "state_component: missing attribute: bool_expression");
		if (!PyObject_HasAttrString(itself, "set_value")) PyErr_Warn(PyExc_Warning, "state_component: missing attribute: set_value");
		if (!PyObject_HasAttrString(itself, "new_value")) PyErr_Warn(PyExc_Warning, "state_component: missing attribute: new_value");
		if (!PyObject_HasAttrString(itself, "del_value")) PyErr_Warn(PyExc_Warning, "state_component: missing attribute: del_value");
		if (!PyObject_HasAttrString(itself, "send")) PyErr_Warn(PyExc_Warning, "state_component: missing attribute: send");
		if (!PyObject_HasAttrString(itself, "string_expression")) PyErr_Warn(PyExc_Warning, "state_component: missing attribute: string_expression");
		if (!PyObject_HasAttrString(itself, "want_state_change")) PyErr_Warn(PyExc_Warning, "state_component: missing attribute: want_state_change");
		if (!PyObject_HasAttrString(itself, "getsubtree")) PyErr_Warn(PyExc_Warning, "state_component: missing attribute: getsubtree");
	}
	if (itself == NULL) itself = Py_None;

	py_state_component = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

state_component::~state_component()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_state_component;
	py_state_component = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void state_component::register_state_test_methods(ambulant::common::state_test_methods* stm)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_stm = Py_BuildValue("O&", state_test_methodsObj_New, stm);

	PyObject *py_rv = PyObject_CallMethod(py_state_component, "register_state_test_methods", "(O)", py_stm);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_component::register_state_test_methods() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_stm);

	PyGILState_Release(_GILState);
}

void state_component::declare_state(const ambulant::lib::node* state)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_state = Py_BuildValue("O&", nodeObj_New, state);

	PyObject *py_rv = PyObject_CallMethod(py_state_component, "declare_state", "(O)", py_state);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_component::declare_state() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_state);

	PyGILState_Release(_GILState);
}

bool state_component::bool_expression(const char* expr)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;
	PyObject *py_expr = Py_BuildValue("s", expr);

	PyObject *py_rv = PyObject_CallMethod(py_state_component, "bool_expression", "(O)", py_expr);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_component::bool_expression() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during state_component::bool_expression() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_expr);

	PyGILState_Release(_GILState);
	return _rv;
}

void state_component::set_value(const char* var, const char* expr)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_var = Py_BuildValue("s", var);
	PyObject *py_expr = Py_BuildValue("s", expr);

	PyObject *py_rv = PyObject_CallMethod(py_state_component, "set_value", "(OO)", py_var, py_expr);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_component::set_value() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_var);
	Py_XDECREF(py_expr);

	PyGILState_Release(_GILState);
}

void state_component::new_value(const char* ref, const char* where, const char* name, const char* expr)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_ref = Py_BuildValue("s", ref);
	PyObject *py_where = Py_BuildValue("s", where);
	PyObject *py_name = Py_BuildValue("s", name);
	PyObject *py_expr = Py_BuildValue("s", expr);

	PyObject *py_rv = PyObject_CallMethod(py_state_component, "new_value", "(OOOO)", py_ref, py_where, py_name, py_expr);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_component::new_value() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_ref);
	Py_XDECREF(py_where);
	Py_XDECREF(py_name);
	Py_XDECREF(py_expr);

	PyGILState_Release(_GILState);
}

void state_component::del_value(const char* ref)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_ref = Py_BuildValue("s", ref);

	PyObject *py_rv = PyObject_CallMethod(py_state_component, "del_value", "(O)", py_ref);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_component::del_value() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_ref);

	PyGILState_Release(_GILState);
}

void state_component::send(const ambulant::lib::node* submission)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_submission = Py_BuildValue("O&", nodeObj_New, submission);

	PyObject *py_rv = PyObject_CallMethod(py_state_component, "send", "(O)", py_submission);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_component::send() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_submission);

	PyGILState_Release(_GILState);
}

std::string state_component::string_expression(const char* expr)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	std::string _rv;
	PyObject *py_expr = Py_BuildValue("s", expr);

	PyObject *py_rv = PyObject_CallMethod(py_state_component, "string_expression", "(O)", py_expr);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_component::string_expression() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during state_component::string_expression() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);
	Py_XDECREF(py_expr);

	PyGILState_Release(_GILState);
	return _rv;
}

void state_component::want_state_change(const char* ref, ambulant::common::state_change_callback* cb)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_ref = Py_BuildValue("s", ref);
	PyObject *py_cb = Py_BuildValue("O&", state_change_callbackObj_New, cb);

	PyObject *py_rv = PyObject_CallMethod(py_state_component, "want_state_change", "(OO)", py_ref, py_cb);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_component::want_state_change() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_ref);
	Py_XDECREF(py_cb);

	PyGILState_Release(_GILState);
}

std::string state_component::getsubtree(const char* ref, bool as_query)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	std::string _rv;
	PyObject *py_ref = Py_BuildValue("s", ref);
	PyObject *py_as_query = Py_BuildValue("O&", bool_New, as_query);

	PyObject *py_rv = PyObject_CallMethod(py_state_component, "getsubtree", "(OO)", py_ref, py_as_query);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_component::getsubtree() callback:\n");
		PyErr_Print();
	}

	char *_rv_cstr="";
	if (py_rv && !PyArg_Parse(py_rv, "s", &_rv_cstr))
	{
		PySys_WriteStderr("Python exception during state_component::getsubtree() return:\n");
		PyErr_Print();
	}

	_rv = _rv_cstr;
	Py_XDECREF(py_rv);
	Py_XDECREF(py_ref);
	Py_XDECREF(py_as_query);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ----------------- Class state_component_factory ------------------ */

state_component_factory::state_component_factory(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_state_component")) PyErr_Warn(PyExc_Warning, "state_component_factory: missing attribute: new_state_component");
	}
	if (itself == NULL) itself = Py_None;

	py_state_component_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

state_component_factory::~state_component_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_state_component_factory;
	py_state_component_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::common::state_component* state_component_factory::new_state_component(const char* uri)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::common::state_component* _rv;
	PyObject *py_uri = Py_BuildValue("s", uri);

	PyObject *py_rv = PyObject_CallMethod(py_state_component_factory, "new_state_component", "(O)", py_uri);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during state_component_factory::new_state_component() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", state_componentObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during state_component_factory::new_state_component() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_uri);

	PyGILState_Release(_GILState);
	return _rv;
}

/* -------------- Class global_state_component_factory -------------- */

global_state_component_factory::global_state_component_factory(PyObject *itself)
:	::state_component_factory(itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "add_factory")) PyErr_Warn(PyExc_Warning, "global_state_component_factory: missing attribute: add_factory");
	}
	if (itself == NULL) itself = Py_None;

	py_global_state_component_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

global_state_component_factory::~global_state_component_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_global_state_component_factory;
	py_global_state_component_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void global_state_component_factory::add_factory(ambulant::common::state_component_factory* sf)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_sf = Py_BuildValue("O&", state_component_factoryObj_New, sf);

	PyObject *py_rv = PyObject_CallMethod(py_global_state_component_factory, "add_factory", "(O)", py_sf);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during global_state_component_factory::add_factory() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_sf);

	PyGILState_Release(_GILState);
}

/* ------------------------ Class datasource ------------------------ */

datasource::datasource(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "start")) PyErr_Warn(PyExc_Warning, "datasource: missing attribute: start");
		if (!PyObject_HasAttrString(itself, "start_prefetch")) PyErr_Warn(PyExc_Warning, "datasource: missing attribute: start_prefetch");
		if (!PyObject_HasAttrString(itself, "stop")) PyErr_Warn(PyExc_Warning, "datasource: missing attribute: stop");
		if (!PyObject_HasAttrString(itself, "end_of_file")) PyErr_Warn(PyExc_Warning, "datasource: missing attribute: end_of_file");
		if (!PyObject_HasAttrString(itself, "size")) PyErr_Warn(PyExc_Warning, "datasource: missing attribute: size");
		if (!PyObject_HasAttrString(itself, "readdone")) PyErr_Warn(PyExc_Warning, "datasource: missing attribute: readdone");
	}
	if (itself == NULL) itself = Py_None;

	py_datasource = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

datasource::~datasource()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_datasource;
	py_datasource = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


void datasource::start(ambulant::lib::event_processor* evp, ambulant::lib::event* callback)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_evp = Py_BuildValue("O&", event_processorObj_New, evp);
	PyObject *py_callback = Py_BuildValue("O&", eventObj_New, callback);

	PyObject *py_rv = PyObject_CallMethod(py_datasource, "start", "(OO)", py_evp, py_callback);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during datasource::start() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_evp);
	Py_XDECREF(py_callback);

	PyGILState_Release(_GILState);
}

void datasource::start_prefetch(ambulant::lib::event_processor* evp)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_evp = Py_BuildValue("O&", event_processorObj_New, evp);

	PyObject *py_rv = PyObject_CallMethod(py_datasource, "start_prefetch", "(O)", py_evp);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during datasource::start_prefetch() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_evp);

	PyGILState_Release(_GILState);
}

void datasource::stop()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_rv = PyObject_CallMethod(py_datasource, "stop", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during datasource::stop() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
}

bool datasource::end_of_file()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	bool _rv;

	PyObject *py_rv = PyObject_CallMethod(py_datasource, "end_of_file", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during datasource::end_of_file() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", bool_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during datasource::end_of_file() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

size_t datasource::size() const
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	size_t _rv;

	PyObject *py_rv = PyObject_CallMethod(py_datasource, "size", "()");
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during datasource::size() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "l", &_rv))
	{
		PySys_WriteStderr("Python exception during datasource::size() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);

	PyGILState_Release(_GILState);
	return _rv;
}

void datasource::readdone(size_t len)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *py_len = Py_BuildValue("l", len);

	PyObject *py_rv = PyObject_CallMethod(py_datasource, "readdone", "(O)", py_len);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during datasource::readdone() callback:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_len);

	PyGILState_Release(_GILState);
}

/* ------------------ Class raw_datasource_factory ------------------ */

raw_datasource_factory::raw_datasource_factory(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_raw_datasource")) PyErr_Warn(PyExc_Warning, "raw_datasource_factory: missing attribute: new_raw_datasource");
	}
	if (itself == NULL) itself = Py_None;

	py_raw_datasource_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

raw_datasource_factory::~raw_datasource_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_raw_datasource_factory;
	py_raw_datasource_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::net::datasource* raw_datasource_factory::new_raw_datasource(const ambulant::net::url& url)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::net::datasource* _rv;
	PyObject *py_url = Py_BuildValue("O", ambulant_url_New(url));

	PyObject *py_rv = PyObject_CallMethod(py_raw_datasource_factory, "new_raw_datasource", "(O)", py_url);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during raw_datasource_factory::new_raw_datasource() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", datasourceObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during raw_datasource_factory::new_raw_datasource() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_url);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ----------------- Class audio_datasource_factory ----------------- */

audio_datasource_factory::audio_datasource_factory(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_audio_datasource")) PyErr_Warn(PyExc_Warning, "audio_datasource_factory: missing attribute: new_audio_datasource");
	}
	if (itself == NULL) itself = Py_None;

	py_audio_datasource_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

audio_datasource_factory::~audio_datasource_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_audio_datasource_factory;
	py_audio_datasource_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::net::audio_datasource* audio_datasource_factory::new_audio_datasource(const ambulant::net::url& url, const ambulant::net::audio_format_choices& fmt, ambulant::net::timestamp_t clip_begin, ambulant::net::timestamp_t clip_end)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::net::audio_datasource* _rv;
	PyObject *py_url = Py_BuildValue("O", ambulant_url_New(url));
	PyObject *py_fmt = Py_BuildValue("O", audio_format_choicesObj_New(&fmt));
	PyObject *py_clip_begin = Py_BuildValue("L", clip_begin);
	PyObject *py_clip_end = Py_BuildValue("L", clip_end);

	PyObject *py_rv = PyObject_CallMethod(py_audio_datasource_factory, "new_audio_datasource", "(OOOO)", py_url, py_fmt, py_clip_begin, py_clip_end);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during audio_datasource_factory::new_audio_datasource() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", audio_datasourceObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during audio_datasource_factory::new_audio_datasource() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_url);
	Py_XDECREF(py_fmt);
	Py_XDECREF(py_clip_begin);
	Py_XDECREF(py_clip_end);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ----------------- Class video_datasource_factory ----------------- */

video_datasource_factory::video_datasource_factory(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_video_datasource")) PyErr_Warn(PyExc_Warning, "video_datasource_factory: missing attribute: new_video_datasource");
	}
	if (itself == NULL) itself = Py_None;

	py_video_datasource_factory = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

video_datasource_factory::~video_datasource_factory()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_video_datasource_factory;
	py_video_datasource_factory = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::net::video_datasource* video_datasource_factory::new_video_datasource(const ambulant::net::url& url, ambulant::net::timestamp_t clip_begin, ambulant::net::timestamp_t clip_end)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::net::video_datasource* _rv;
	PyObject *py_url = Py_BuildValue("O", ambulant_url_New(url));
	PyObject *py_clip_begin = Py_BuildValue("L", clip_begin);
	PyObject *py_clip_end = Py_BuildValue("L", clip_end);

	PyObject *py_rv = PyObject_CallMethod(py_video_datasource_factory, "new_video_datasource", "(OOO)", py_url, py_clip_begin, py_clip_end);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during video_datasource_factory::new_video_datasource() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", video_datasourceObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during video_datasource_factory::new_video_datasource() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_url);
	Py_XDECREF(py_clip_begin);
	Py_XDECREF(py_clip_end);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ------------------- Class audio_parser_finder -------------------- */

audio_parser_finder::audio_parser_finder(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_audio_parser")) PyErr_Warn(PyExc_Warning, "audio_parser_finder: missing attribute: new_audio_parser");
	}
	if (itself == NULL) itself = Py_None;

	py_audio_parser_finder = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

audio_parser_finder::~audio_parser_finder()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_audio_parser_finder;
	py_audio_parser_finder = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::net::audio_datasource* audio_parser_finder::new_audio_parser(const ambulant::net::url& url, const ambulant::net::audio_format_choices& hint, ambulant::net::audio_datasource* src)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::net::audio_datasource* _rv;
	PyObject *py_url = Py_BuildValue("O", ambulant_url_New(url));
	PyObject *py_hint = Py_BuildValue("O", audio_format_choicesObj_New(&hint));
	PyObject *py_src = Py_BuildValue("O&", audio_datasourceObj_New, src);

	PyObject *py_rv = PyObject_CallMethod(py_audio_parser_finder, "new_audio_parser", "(OOO)", py_url, py_hint, py_src);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during audio_parser_finder::new_audio_parser() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", audio_datasourceObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during audio_parser_finder::new_audio_parser() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_url);
	Py_XDECREF(py_hint);
	Py_XDECREF(py_src);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ------------------- Class audio_filter_finder -------------------- */

audio_filter_finder::audio_filter_finder(PyObject *itself)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	if (itself)
	{
		if (!PyObject_HasAttrString(itself, "new_audio_filter")) PyErr_Warn(PyExc_Warning, "audio_filter_finder: missing attribute: new_audio_filter");
	}
	if (itself == NULL) itself = Py_None;

	py_audio_filter_finder = itself;
	Py_XINCREF(itself);
	PyGILState_Release(_GILState);
}

audio_filter_finder::~audio_filter_finder()
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	PyObject *itself = py_audio_filter_finder;
	py_audio_filter_finder = NULL;
	if (pycppbridge_Check(itself) && pycppbridge_getwrapper(itself) == this)
	{
		pycppbridge_setwrapper(itself, NULL);
	}
	Py_XDECREF(itself);
	PyGILState_Release(_GILState);
}


ambulant::net::audio_datasource* audio_filter_finder::new_audio_filter(ambulant::net::audio_datasource* src, const ambulant::net::audio_format_choices& fmts)
{
	PyGILState_STATE _GILState = PyGILState_Ensure();
	ambulant::net::audio_datasource* _rv;
	PyObject *py_src = Py_BuildValue("O&", audio_datasourceObj_New, src);
	PyObject *py_fmts = Py_BuildValue("O", audio_format_choicesObj_New(&fmts));

	PyObject *py_rv = PyObject_CallMethod(py_audio_filter_finder, "new_audio_filter", "(OO)", py_src, py_fmts);
	if (PyErr_Occurred())
	{
		PySys_WriteStderr("Python exception during audio_filter_finder::new_audio_filter() callback:\n");
		PyErr_Print();
	}

	if (py_rv && !PyArg_Parse(py_rv, "O&", audio_datasourceObj_Convert, &_rv))
	{
		PySys_WriteStderr("Python exception during audio_filter_finder::new_audio_filter() return:\n");
		PyErr_Print();
	}

	Py_XDECREF(py_rv);
	Py_XDECREF(py_src);
	Py_XDECREF(py_fmts);

	PyGILState_Release(_GILState);
	return _rv;
}

/* ================ End callbacks module pyambulant ================= */

