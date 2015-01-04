
/* ======================== Module ambulant ========================= */

#include "Python.h"



/*AMBULANT_FOREIGN_INDENT_RULES*/
#include "ambulant/config/config.h"
#include "ambulant/version.h"
#include "ambulant/lib/amstream.h"
#include "ambulant/lib/logger.h"
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
#include "ambulant/common/gui_player.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/recorder.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/common/player.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/state.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/stdio_datasource.h"
#include "ambulant/net/posix_datasource.h"
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/net/ffmpeg_factory.h"

// Should have been included through genobj.py but that caused problems
#ifdef WITH_GTK
#include "ambulant/gui/gtk/gtk_factory.h"
#include <pygobject.h>
#include <pygtk/pygtk.h>
#if GTK_MAJOR_VERSION < 3
#define WITH_GTK2
#endif
#endif
#include "ambulantinterface.h"
#include "ambulantutilities.h"
#include "ambulantmodule.h"

// The Python interface does not qualify strings with const, so we have to
// disable warnings about non-writeable strings (zillions of them)

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

extern PyObject *audio_format_choicesObj_New(ambulant::net::audio_format_choices *itself);
extern int audio_format_choicesObj_Convert(PyObject *v, ambulant::net::audio_format_choices *p_itself);
extern int cobject_Convert(PyObject *v, void **p_itself);

/* Workaround for "const" added in Python 2.5. But removed before 2.5a1? */
#if PY_VERSION_HEX >= 0x02050000 && PY_VERSION_HEX < 0x020500a1
# define Py_KEYWORDS_STRING_TYPE const char
#else
# define Py_KEYWORDS_STRING_TYPE char
#endif

#ifdef WITH_GTK
static PyTypeObject *PyGObject_Type = NULL;
int
PyGObjectAsVoidPtr_Convert(PyObject *v, void **p_itself)
{
    static bool pygtk_initialized = false;
#if 0
    if (v == NULL || v->ob_type != PyGObject_Type) {
        PyErr_SetString(PyExc_TypeError, "PyGObject expected");
        return 0;
    }
#endif
    *p_itself = (void*)GTK_WIDGET(((PyGObject *)v)->obj);
    return 1;
}
#endif

static PyObject *PyAm_Error;

/* -------------------- Object type pycppbridge --------------------- */

typedef struct pycppbridgeObject {
	PyObject_HEAD
	cpppybridge *ob_wrapper;
} pycppbridgeObject;

static void pycppbridge_dealloc(pycppbridgeObject *self)
{
	delete self->ob_wrapper;
	self->ob_wrapper = NULL;
	self->ob_type->tp_free((PyObject *)self);
}

static PyMethodDef pycppbridge_methods[] = {
	{NULL, NULL, 0}
};

#define pycppbridge_getsetlist NULL


#define pycppbridge_compare NULL

#define pycppbridge_repr NULL

#define pycppbridge_hash NULL
#define pycppbridge_tp_init 0

#define pycppbridge_tp_alloc PyType_GenericAlloc

#define pycppbridge_tp_new PyType_GenericNew
#define pycppbridge_tp_free PyObject_Del


PyTypeObject pycppbridge_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.pycppbridge", /*tp_name*/
	sizeof(pycppbridgeObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) pycppbridge_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) pycppbridge_compare, /*tp_compare*/
	(reprfunc) pycppbridge_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) pycppbridge_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	pycppbridge_methods, /* tp_methods */
	0, /*tp_members*/
	pycppbridge_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	pycppbridge_tp_init, /* tp_init */
	pycppbridge_tp_alloc, /* tp_alloc */
	pycppbridge_tp_new, /* tp_new */
	pycppbridge_tp_free, /* tp_free */
};

/* ------------------ End object type pycppbridge ------------------- */


/* ---------------------- Object type ostream ----------------------- */

extern PyTypeObject ostream_Type;

inline bool ostreamObj_Check(PyObject *x)
{
	return ((x)->ob_type == &ostream_Type);
}

typedef struct ostreamObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::ostream* ob_itself;
} ostreamObject;

PyObject *ostreamObj_New(ambulant::lib::ostream* itself)
{
	ostreamObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_ostream
	ostream *encaps_itself = dynamic_cast<ostream *>(itself);
	if (encaps_itself && encaps_itself->py_ostream)
	{
		Py_INCREF(encaps_itself->py_ostream);
		return encaps_itself->py_ostream;
	}
#endif
	it = PyObject_NEW(ostreamObject, &ostream_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int ostreamObj_Convert(PyObject *v, ambulant::lib::ostream* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_ostream
	if (!ostreamObj_Check(v))
	{
		*p_itself = Py_WrapAs_ostream(v);
		if (*p_itself) return 1;
	}
#endif
	if (!ostreamObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "ostream required");
		return 0;
	}
	*p_itself = ((ostreamObject *)v)->ob_itself;
	return 1;
}

static void ostreamObj_dealloc(ostreamObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *ostreamObj_is_open(ostreamObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_open();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *ostreamObj_close(ostreamObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->close();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *ostreamObj_write_1(ostreamObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	unsigned char * buffer;
	int nbytes;
	if (!PyArg_ParseTuple(_args, "si",
	                      &buffer,
	                      &nbytes))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	int _rv = _self->ob_itself->write(buffer,
	                                  nbytes);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("i",
	                     _rv);
	return _res;
}

static PyObject *ostreamObj_write_2(ostreamObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* cstr;
	if (!PyArg_ParseTuple(_args, "s",
	                      &cstr))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	int _rv = _self->ob_itself->write(cstr);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("i",
	                     _rv);
	return _res;
}

static PyObject *ostreamObj_flush(ostreamObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->flush();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef ostreamObj_methods[] = {
	{"is_open", (PyCFunction)ostreamObj_is_open, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"close", (PyCFunction)ostreamObj_close, 1,
	 PyDoc_STR("() -> None")},
	{"write_1", (PyCFunction)ostreamObj_write_1, 1,
	 PyDoc_STR("(unsigned char * buffer, int nbytes) -> (int _rv)")},
	{"write_2", (PyCFunction)ostreamObj_write_2, 1,
	 PyDoc_STR("(char* cstr) -> (int _rv)")},
	{"flush", (PyCFunction)ostreamObj_flush, 1,
	 PyDoc_STR("() -> None")},
	{NULL, NULL, 0}
};

#define ostreamObj_getsetlist NULL


static int ostreamObj_compare(ostreamObject *self, ostreamObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define ostreamObj_repr NULL

static long ostreamObj_hash(ostreamObject *self)
{
	return (long)self->ob_itself;
}
static int ostreamObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::ostream* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, ostreamObj_Convert, &itself))
	{
		((ostreamObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define ostreamObj_tp_alloc PyType_GenericAlloc

static PyObject *ostreamObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((ostreamObject *)_self)->ob_itself = NULL;
	return _self;
}

#define ostreamObj_tp_free PyObject_Del


PyTypeObject ostream_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.ostream", /*tp_name*/
	sizeof(ostreamObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) ostreamObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) ostreamObj_compare, /*tp_compare*/
	(reprfunc) ostreamObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) ostreamObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	ostreamObj_methods, /* tp_methods */
	0, /*tp_members*/
	ostreamObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	ostreamObj_tp_init, /* tp_init */
	ostreamObj_tp_alloc, /* tp_alloc */
	ostreamObj_tp_new, /* tp_new */
	ostreamObj_tp_free, /* tp_free */
};

/* -------------------- End object type ostream --------------------- */


/* ----------------------- Object type logger ----------------------- */

extern PyTypeObject logger_Type;

inline bool loggerObj_Check(PyObject *x)
{
	return ((x)->ob_type == &logger_Type);
}

typedef struct loggerObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::logger* ob_itself;
} loggerObject;

PyObject *loggerObj_New(ambulant::lib::logger* itself)
{
	loggerObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_logger
	logger *encaps_itself = dynamic_cast<logger *>(itself);
	if (encaps_itself && encaps_itself->py_logger)
	{
		Py_INCREF(encaps_itself->py_logger);
		return encaps_itself->py_logger;
	}
#endif
	it = PyObject_NEW(loggerObject, &logger_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int loggerObj_Convert(PyObject *v, ambulant::lib::logger* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_logger
	if (!loggerObj_Check(v))
	{
		*p_itself = Py_WrapAs_logger(v);
		if (*p_itself) return 1;
	}
#endif
	if (!loggerObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "logger required");
		return 0;
	}
	*p_itself = ((loggerObject *)v)->ob_itself;
	return 1;
}

static void loggerObj_dealloc(loggerObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *loggerObj_debug(loggerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string s;
	char *s_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &s_cstr))
		return NULL;
	s = s_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->debug(s);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *loggerObj_trace(loggerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string s;
	char *s_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &s_cstr))
		return NULL;
	s = s_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->trace(s);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *loggerObj_show(loggerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string s;
	char *s_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &s_cstr))
		return NULL;
	s = s_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->show(s);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *loggerObj_warn(loggerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string s;
	char *s_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &s_cstr))
		return NULL;
	s = s_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->warn(s);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *loggerObj_error(loggerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string s;
	char *s_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &s_cstr))
		return NULL;
	s = s_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->error(s);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *loggerObj_fatal(loggerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string s;
	char *s_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &s_cstr))
		return NULL;
	s = s_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->fatal(s);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *loggerObj_log_cstr(loggerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int level;
	char* buf;
	if (!PyArg_ParseTuple(_args, "is",
	                      &level,
	                      &buf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->log_cstr(level,
	                           buf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *loggerObj_suppressed(loggerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int level;
	if (!PyArg_ParseTuple(_args, "i",
	                      &level))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->suppressed(level);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *loggerObj_set_level(loggerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int level;
	if (!PyArg_ParseTuple(_args, "i",
	                      &level))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_level(level);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *loggerObj_set_ostream(loggerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::ostream* pos;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ostreamObj_Convert, &pos))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_ostream(pos);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef loggerObj_methods[] = {
	{"debug", (PyCFunction)loggerObj_debug, 1,
	 PyDoc_STR("(std::string s) -> None")},
	{"trace", (PyCFunction)loggerObj_trace, 1,
	 PyDoc_STR("(std::string s) -> None")},
	{"show", (PyCFunction)loggerObj_show, 1,
	 PyDoc_STR("(std::string s) -> None")},
	{"warn", (PyCFunction)loggerObj_warn, 1,
	 PyDoc_STR("(std::string s) -> None")},
	{"error", (PyCFunction)loggerObj_error, 1,
	 PyDoc_STR("(std::string s) -> None")},
	{"fatal", (PyCFunction)loggerObj_fatal, 1,
	 PyDoc_STR("(std::string s) -> None")},
	{"log_cstr", (PyCFunction)loggerObj_log_cstr, 1,
	 PyDoc_STR("(int level, char* buf) -> None")},
	{"suppressed", (PyCFunction)loggerObj_suppressed, 1,
	 PyDoc_STR("(int level) -> (bool _rv)")},
	{"set_level", (PyCFunction)loggerObj_set_level, 1,
	 PyDoc_STR("(int level) -> None")},
	{"set_ostream", (PyCFunction)loggerObj_set_ostream, 1,
	 PyDoc_STR("(ambulant::lib::ostream* pos) -> None")},
	{NULL, NULL, 0}
};

#define loggerObj_getsetlist NULL


static int loggerObj_compare(loggerObject *self, loggerObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define loggerObj_repr NULL

static long loggerObj_hash(loggerObject *self)
{
	return (long)self->ob_itself;
}
static int loggerObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::logger* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	{
		std::string name;
		char *name_cstr="";
		if (PyArg_ParseTuple(_args, "s",
		                     &name_cstr))
		{
			name = name_cstr;
			((loggerObject *)_self)->ob_itself = new ambulant::lib::logger(name);
			return 0;
		}
	}

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, loggerObj_Convert, &itself))
	{
		((loggerObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define loggerObj_tp_alloc PyType_GenericAlloc

static PyObject *loggerObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((loggerObject *)_self)->ob_itself = NULL;
	return _self;
}

#define loggerObj_tp_free PyObject_Del


PyTypeObject logger_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.logger", /*tp_name*/
	sizeof(loggerObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) loggerObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) loggerObj_compare, /*tp_compare*/
	(reprfunc) loggerObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) loggerObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	loggerObj_methods, /* tp_methods */
	0, /*tp_members*/
	loggerObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	loggerObj_tp_init, /* tp_init */
	loggerObj_tp_alloc, /* tp_alloc */
	loggerObj_tp_new, /* tp_new */
	loggerObj_tp_free, /* tp_free */
};

/* --------------------- End object type logger --------------------- */


/* -------------------- Object type node_context -------------------- */

extern PyTypeObject node_context_Type;

inline bool node_contextObj_Check(PyObject *x)
{
	return ((x)->ob_type == &node_context_Type);
}

typedef struct node_contextObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::node_context* ob_itself;
} node_contextObject;

PyObject *node_contextObj_New(ambulant::lib::node_context* itself)
{
	node_contextObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_node_context
	node_context *encaps_itself = dynamic_cast<node_context *>(itself);
	if (encaps_itself && encaps_itself->py_node_context)
	{
		Py_INCREF(encaps_itself->py_node_context);
		return encaps_itself->py_node_context;
	}
#endif
	it = PyObject_NEW(node_contextObject, &node_context_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int node_contextObj_Convert(PyObject *v, ambulant::lib::node_context* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_node_context
	if (!node_contextObj_Check(v))
	{
		*p_itself = Py_WrapAs_node_context(v);
		if (*p_itself) return 1;
	}
#endif
	if (!node_contextObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "node_context required");
		return 0;
	}
	*p_itself = ((node_contextObject *)v)->ob_itself;
	return 1;
}

static void node_contextObj_dealloc(node_contextObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *node_contextObj_set_prefix_mapping(node_contextObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string prefix;
	std::string uri;
	char *prefix_cstr="";
	char *uri_cstr="";
	if (!PyArg_ParseTuple(_args, "ss",
	                      &prefix_cstr,
	                      &uri_cstr))
		return NULL;
	prefix = prefix_cstr;
	uri = uri_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_prefix_mapping(prefix,
	                                     uri);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *node_contextObj_get_namespace_prefix(node_contextObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::xml_string uri;
	char *uri_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &uri_cstr))
		return NULL;
	uri = uri_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::xml_string& _rv = _self->ob_itself->get_namespace_prefix(uri);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *node_contextObj_is_supported_prefix(node_contextObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::xml_string prefix;
	char *prefix_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &prefix_cstr))
		return NULL;
	prefix = prefix_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_supported_prefix(prefix);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *node_contextObj_resolve_url(node_contextObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url rurl;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_url_Convert, &rurl))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::url _rv = _self->ob_itself->resolve_url(rurl);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_url_New(_rv));
	return _res;
}

static PyObject *node_contextObj_get_root(node_contextObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node* _rv = _self->ob_itself->get_root();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *node_contextObj_get_node(node_contextObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string idd;
	char *idd_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &idd_cstr))
		return NULL;
	idd = idd_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node* _rv = _self->ob_itself->get_node(idd);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *node_contextObj_get_state(node_contextObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::state_component* _rv = _self->ob_itself->get_state();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     state_componentObj_New, _rv);
	return _res;
}

static PyObject *node_contextObj_apply_avt(node_contextObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	ambulant::lib::xml_string attrname;
	ambulant::lib::xml_string attrvalue;
	char *attrname_cstr="";
	char *attrvalue_cstr="";
	if (!PyArg_ParseTuple(_args, "O&ss",
	                      nodeObj_Convert, &n,
	                      &attrname_cstr,
	                      &attrvalue_cstr))
		return NULL;
	attrname = attrname_cstr;
	attrvalue = attrvalue_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::xml_string& _rv = _self->ob_itself->apply_avt(n,
	                                                                   attrname,
	                                                                   attrvalue);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyMethodDef node_contextObj_methods[] = {
	{"set_prefix_mapping", (PyCFunction)node_contextObj_set_prefix_mapping, 1,
	 PyDoc_STR("(std::string prefix, std::string uri) -> None")},
	{"get_namespace_prefix", (PyCFunction)node_contextObj_get_namespace_prefix, 1,
	 PyDoc_STR("(ambulant::lib::xml_string uri) -> (const ambulant::lib::xml_string& _rv)")},
	{"is_supported_prefix", (PyCFunction)node_contextObj_is_supported_prefix, 1,
	 PyDoc_STR("(ambulant::lib::xml_string prefix) -> (bool _rv)")},
	{"resolve_url", (PyCFunction)node_contextObj_resolve_url, 1,
	 PyDoc_STR("(ambulant::net::url rurl) -> (ambulant::net::url _rv)")},
	{"get_root", (PyCFunction)node_contextObj_get_root, 1,
	 PyDoc_STR("() -> (const ambulant::lib::node* _rv)")},
	{"get_node", (PyCFunction)node_contextObj_get_node, 1,
	 PyDoc_STR("(std::string idd) -> (const ambulant::lib::node* _rv)")},
	{"get_state", (PyCFunction)node_contextObj_get_state, 1,
	 PyDoc_STR("() -> (ambulant::common::state_component* _rv)")},
	{"apply_avt", (PyCFunction)node_contextObj_apply_avt, 1,
	 PyDoc_STR("(ambulant::lib::node* n, ambulant::lib::xml_string attrname, ambulant::lib::xml_string attrvalue) -> (const ambulant::lib::xml_string& _rv)")},
	{NULL, NULL, 0}
};

#define node_contextObj_getsetlist NULL


static int node_contextObj_compare(node_contextObject *self, node_contextObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define node_contextObj_repr NULL

static long node_contextObj_hash(node_contextObject *self)
{
	return (long)self->ob_itself;
}
static int node_contextObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::node_context* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, node_contextObj_Convert, &itself))
	{
		((node_contextObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define node_contextObj_tp_alloc PyType_GenericAlloc

static PyObject *node_contextObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((node_contextObject *)_self)->ob_itself = NULL;
	return _self;
}

#define node_contextObj_tp_free PyObject_Del


PyTypeObject node_context_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.node_context", /*tp_name*/
	sizeof(node_contextObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) node_contextObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) node_contextObj_compare, /*tp_compare*/
	(reprfunc) node_contextObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) node_contextObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	node_contextObj_methods, /* tp_methods */
	0, /*tp_members*/
	node_contextObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	node_contextObj_tp_init, /* tp_init */
	node_contextObj_tp_alloc, /* tp_alloc */
	node_contextObj_tp_new, /* tp_new */
	node_contextObj_tp_free, /* tp_free */
};

/* ------------------ End object type node_context ------------------ */


/* ------------------------ Object type node ------------------------ */

extern PyTypeObject node_Type;

inline bool nodeObj_Check(PyObject *x)
{
	return ((x)->ob_type == &node_Type);
}

typedef struct nodeObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::node* ob_itself;
} nodeObject;

PyObject *nodeObj_New(ambulant::lib::node* itself)
{
	nodeObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_node
	node *encaps_itself = dynamic_cast<node *>(itself);
	if (encaps_itself && encaps_itself->py_node)
	{
		Py_INCREF(encaps_itself->py_node);
		return encaps_itself->py_node;
	}
#endif
	it = PyObject_NEW(nodeObject, &node_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int nodeObj_Convert(PyObject *v, ambulant::lib::node* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_node
	if (!nodeObj_Check(v))
	{
		*p_itself = Py_WrapAs_node(v);
		if (*p_itself) return 1;
	}
#endif
	if (!nodeObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "node required");
		return 0;
	}
	*p_itself = ((nodeObject *)v)->ob_itself;
	return 1;
}

static void nodeObj_dealloc(nodeObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *nodeObj_down_1(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node* _rv = _self->ob_itself->down();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_up_1(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node* _rv = _self->ob_itself->up();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_next_1(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node* _rv = _self->ob_itself->next();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_down_2(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->down();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_up_2(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->up();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_next_2(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->next();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_down_3(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &n))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->down(n);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *nodeObj_up_3(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &n))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->up(n);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *nodeObj_next_3(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &n))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->next(n);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *nodeObj_previous(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node* _rv = _self->ob_itself->previous();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_get_last_child(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node* _rv = _self->ob_itself->get_last_child();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_locate_node(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* path;
	if (!PyArg_ParseTuple(_args, "s",
	                      &path))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->locate_node(path);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_get_first_child_1(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* name;
	if (!PyArg_ParseTuple(_args, "s",
	                      &name))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->get_first_child(name);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_get_first_child_2(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* name;
	if (!PyArg_ParseTuple(_args, "s",
	                      &name))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node* _rv = _self->ob_itself->get_first_child(name);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_get_root(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->get_root();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_get_container_attribute(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* name;
	if (!PyArg_ParseTuple(_args, "s",
	                      &name))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const char * _rv = _self->ob_itself->get_container_attribute(name);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("z",
	                     _rv);
	return _res;
}

static PyObject *nodeObj_append_child_1(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* child;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &child))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->append_child(child);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_append_child_2(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* name;
	if (!PyArg_ParseTuple(_args, "s",
	                      &name))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->append_child(name);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_detach(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->detach();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_clone(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->clone();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_append_data_1(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char *data__in__;
	size_t data__len__;
	int data__in_len__;
	if (!PyArg_ParseTuple(_args, "s#",
	                      &data__in__, &data__in_len__))
		return NULL;
	data__len__ = data__in_len__;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->append_data(data__in__, data__len__);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *nodeObj_append_data_2(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* c_str;
	if (!PyArg_ParseTuple(_args, "s",
	                      &c_str))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->append_data(c_str);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *nodeObj_append_data_3(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::xml_string str;
	char *str_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &str_cstr))
		return NULL;
	str = str_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->append_data(str);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *nodeObj_set_attribute_1(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* name;
	char* value;
	if (!PyArg_ParseTuple(_args, "ss",
	                      &name,
	                      &value))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_attribute(name,
	                                value);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *nodeObj_set_attribute_2(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* name;
	ambulant::lib::xml_string value;
	char *value_cstr="";
	if (!PyArg_ParseTuple(_args, "ss",
	                      &name,
	                      &value_cstr))
		return NULL;
	value = value_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_attribute(name,
	                                value);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *nodeObj_set_prefix_mapping(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string prefix;
	std::string uri;
	char *prefix_cstr="";
	char *uri_cstr="";
	if (!PyArg_ParseTuple(_args, "ss",
	                      &prefix_cstr,
	                      &uri_cstr))
		return NULL;
	prefix = prefix_cstr;
	uri = uri_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_prefix_mapping(prefix,
	                                     uri);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *nodeObj_get_namespace(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::xml_string& _rv = _self->ob_itself->get_namespace();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *nodeObj_get_local_name(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::xml_string& _rv = _self->ob_itself->get_local_name();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *nodeObj_get_qname(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::q_name_pair& _rv = _self->ob_itself->get_qname();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("(ss)",
	                     _rv.first.c_str(), _rv.second.c_str());
	return _res;
}

static PyObject *nodeObj_get_numid(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	int _rv = _self->ob_itself->get_numid();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("i",
	                     _rv);
	return _res;
}

static PyObject *nodeObj_get_data(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::xml_string& _rv = _self->ob_itself->get_data();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *nodeObj_is_data_node(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_data_node();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *nodeObj_get_trimmed_data(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::xml_string _rv = _self->ob_itself->get_trimmed_data();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *nodeObj_get_attribute_1(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* name;
	if (!PyArg_ParseTuple(_args, "s",
	                      &name))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const char * _rv = _self->ob_itself->get_attribute(name);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("z",
	                     _rv);
	return _res;
}

static PyObject *nodeObj_get_attribute_2(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string name;
	char *name_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &name_cstr))
		return NULL;
	name = name_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	const char * _rv = _self->ob_itself->get_attribute(name);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("z",
	                     _rv);
	return _res;
}

static PyObject *nodeObj_del_attribute(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* name;
	if (!PyArg_ParseTuple(_args, "s",
	                      &name))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->del_attribute(name);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *nodeObj_get_url(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* attrname;
	if (!PyArg_ParseTuple(_args, "s",
	                      &attrname))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::url _rv = _self->ob_itself->get_url(attrname);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_url_New(_rv));
	return _res;
}

static PyObject *nodeObj_size(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	unsigned int _rv = _self->ob_itself->size();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *nodeObj_get_xpath(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	std::string _rv = _self->ob_itself->get_xpath();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *nodeObj_get_sig(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	std::string _rv = _self->ob_itself->get_sig();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *nodeObj_xmlrepr(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::xml_string _rv = _self->ob_itself->xmlrepr();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *nodeObj_get_context(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node_context* _rv = _self->ob_itself->get_context();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     node_contextObj_New, _rv);
	return _res;
}

static PyObject *nodeObj_set_context(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node_context* c;
	if (!PyArg_ParseTuple(_args, "O&",
	                      node_contextObj_Convert, &c))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_context(c);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *nodeObj_has_debug(nodeObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* attrvalue;
	if (!PyArg_ParseTuple(_args, "s",
	                      &attrvalue))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->has_debug(attrvalue);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyMethodDef nodeObj_methods[] = {
	{"down_1", (PyCFunction)nodeObj_down_1, 1,
	 PyDoc_STR("() -> (const ambulant::lib::node* _rv)")},
	{"up_1", (PyCFunction)nodeObj_up_1, 1,
	 PyDoc_STR("() -> (const ambulant::lib::node* _rv)")},
	{"next_1", (PyCFunction)nodeObj_next_1, 1,
	 PyDoc_STR("() -> (const ambulant::lib::node* _rv)")},
	{"down_2", (PyCFunction)nodeObj_down_2, 1,
	 PyDoc_STR("() -> (ambulant::lib::node* _rv)")},
	{"up_2", (PyCFunction)nodeObj_up_2, 1,
	 PyDoc_STR("() -> (ambulant::lib::node* _rv)")},
	{"next_2", (PyCFunction)nodeObj_next_2, 1,
	 PyDoc_STR("() -> (ambulant::lib::node* _rv)")},
	{"down_3", (PyCFunction)nodeObj_down_3, 1,
	 PyDoc_STR("(ambulant::lib::node* n) -> None")},
	{"up_3", (PyCFunction)nodeObj_up_3, 1,
	 PyDoc_STR("(ambulant::lib::node* n) -> None")},
	{"next_3", (PyCFunction)nodeObj_next_3, 1,
	 PyDoc_STR("(ambulant::lib::node* n) -> None")},
	{"previous", (PyCFunction)nodeObj_previous, 1,
	 PyDoc_STR("() -> (const ambulant::lib::node* _rv)")},
	{"get_last_child", (PyCFunction)nodeObj_get_last_child, 1,
	 PyDoc_STR("() -> (const ambulant::lib::node* _rv)")},
	{"locate_node", (PyCFunction)nodeObj_locate_node, 1,
	 PyDoc_STR("(char* path) -> (ambulant::lib::node* _rv)")},
	{"get_first_child_1", (PyCFunction)nodeObj_get_first_child_1, 1,
	 PyDoc_STR("(char* name) -> (ambulant::lib::node* _rv)")},
	{"get_first_child_2", (PyCFunction)nodeObj_get_first_child_2, 1,
	 PyDoc_STR("(char* name) -> (const ambulant::lib::node* _rv)")},
	{"get_root", (PyCFunction)nodeObj_get_root, 1,
	 PyDoc_STR("() -> (ambulant::lib::node* _rv)")},
	{"get_container_attribute", (PyCFunction)nodeObj_get_container_attribute, 1,
	 PyDoc_STR("(char* name) -> (const char * _rv)")},
	{"append_child_1", (PyCFunction)nodeObj_append_child_1, 1,
	 PyDoc_STR("(ambulant::lib::node* child) -> (ambulant::lib::node* _rv)")},
	{"append_child_2", (PyCFunction)nodeObj_append_child_2, 1,
	 PyDoc_STR("(char* name) -> (ambulant::lib::node* _rv)")},
	{"detach", (PyCFunction)nodeObj_detach, 1,
	 PyDoc_STR("() -> (ambulant::lib::node* _rv)")},
	{"clone", (PyCFunction)nodeObj_clone, 1,
	 PyDoc_STR("() -> (ambulant::lib::node* _rv)")},
	{"append_data_1", (PyCFunction)nodeObj_append_data_1, 1,
	 PyDoc_STR("(Buffer data) -> None")},
	{"append_data_2", (PyCFunction)nodeObj_append_data_2, 1,
	 PyDoc_STR("(char* c_str) -> None")},
	{"append_data_3", (PyCFunction)nodeObj_append_data_3, 1,
	 PyDoc_STR("(ambulant::lib::xml_string str) -> None")},
	{"set_attribute_1", (PyCFunction)nodeObj_set_attribute_1, 1,
	 PyDoc_STR("(char* name, char* value) -> None")},
	{"set_attribute_2", (PyCFunction)nodeObj_set_attribute_2, 1,
	 PyDoc_STR("(char* name, ambulant::lib::xml_string value) -> None")},
	{"set_prefix_mapping", (PyCFunction)nodeObj_set_prefix_mapping, 1,
	 PyDoc_STR("(std::string prefix, std::string uri) -> None")},
	{"get_namespace", (PyCFunction)nodeObj_get_namespace, 1,
	 PyDoc_STR("() -> (const ambulant::lib::xml_string& _rv)")},
	{"get_local_name", (PyCFunction)nodeObj_get_local_name, 1,
	 PyDoc_STR("() -> (const ambulant::lib::xml_string& _rv)")},
	{"get_qname", (PyCFunction)nodeObj_get_qname, 1,
	 PyDoc_STR("() -> (const ambulant::lib::q_name_pair& _rv)")},
	{"get_numid", (PyCFunction)nodeObj_get_numid, 1,
	 PyDoc_STR("() -> (int _rv)")},
	{"get_data", (PyCFunction)nodeObj_get_data, 1,
	 PyDoc_STR("() -> (const ambulant::lib::xml_string& _rv)")},
	{"is_data_node", (PyCFunction)nodeObj_is_data_node, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"get_trimmed_data", (PyCFunction)nodeObj_get_trimmed_data, 1,
	 PyDoc_STR("() -> (ambulant::lib::xml_string _rv)")},
	{"get_attribute_1", (PyCFunction)nodeObj_get_attribute_1, 1,
	 PyDoc_STR("(char* name) -> (const char * _rv)")},
	{"get_attribute_2", (PyCFunction)nodeObj_get_attribute_2, 1,
	 PyDoc_STR("(std::string name) -> (const char * _rv)")},
	{"del_attribute", (PyCFunction)nodeObj_del_attribute, 1,
	 PyDoc_STR("(char* name) -> None")},
	{"get_url", (PyCFunction)nodeObj_get_url, 1,
	 PyDoc_STR("(char* attrname) -> (ambulant::net::url _rv)")},
	{"size", (PyCFunction)nodeObj_size, 1,
	 PyDoc_STR("() -> (unsigned int _rv)")},
	{"get_xpath", (PyCFunction)nodeObj_get_xpath, 1,
	 PyDoc_STR("() -> (std::string _rv)")},
	{"get_sig", (PyCFunction)nodeObj_get_sig, 1,
	 PyDoc_STR("() -> (std::string _rv)")},
	{"xmlrepr", (PyCFunction)nodeObj_xmlrepr, 1,
	 PyDoc_STR("() -> (ambulant::lib::xml_string _rv)")},
	{"get_context", (PyCFunction)nodeObj_get_context, 1,
	 PyDoc_STR("() -> (const ambulant::lib::node_context* _rv)")},
	{"set_context", (PyCFunction)nodeObj_set_context, 1,
	 PyDoc_STR("(ambulant::lib::node_context* c) -> None")},
	{"has_debug", (PyCFunction)nodeObj_has_debug, 1,
	 PyDoc_STR("(char* attrvalue) -> (bool _rv)")},
	{NULL, NULL, 0}
};

#define nodeObj_getsetlist NULL


static int nodeObj_compare(nodeObject *self, nodeObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define nodeObj_repr NULL

static long nodeObj_hash(nodeObject *self)
{
	return (long)self->ob_itself;
}
static int nodeObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::node* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, nodeObj_Convert, &itself))
	{
		((nodeObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define nodeObj_tp_alloc PyType_GenericAlloc

static PyObject *nodeObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((nodeObject *)_self)->ob_itself = NULL;
	return _self;
}

#define nodeObj_tp_free PyObject_Del


PyTypeObject node_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.node", /*tp_name*/
	sizeof(nodeObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) nodeObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) nodeObj_compare, /*tp_compare*/
	(reprfunc) nodeObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) nodeObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	nodeObj_methods, /* tp_methods */
	0, /*tp_members*/
	nodeObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	nodeObj_tp_init, /* tp_init */
	nodeObj_tp_alloc, /* tp_alloc */
	nodeObj_tp_new, /* tp_new */
	nodeObj_tp_free, /* tp_free */
};

/* ---------------------- End object type node ---------------------- */


/* -------------------- Object type node_factory -------------------- */

extern PyTypeObject node_factory_Type;

inline bool node_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &node_factory_Type);
}

typedef struct node_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::node_factory* ob_itself;
} node_factoryObject;

PyObject *node_factoryObj_New(ambulant::lib::node_factory* itself)
{
	node_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_node_factory
	node_factory *encaps_itself = dynamic_cast<node_factory *>(itself);
	if (encaps_itself && encaps_itself->py_node_factory)
	{
		Py_INCREF(encaps_itself->py_node_factory);
		return encaps_itself->py_node_factory;
	}
#endif
	it = PyObject_NEW(node_factoryObject, &node_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int node_factoryObj_Convert(PyObject *v, ambulant::lib::node_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_node_factory
	if (!node_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_node_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!node_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "node_factory required");
		return 0;
	}
	*p_itself = ((node_factoryObject *)v)->ob_itself;
	return 1;
}

static void node_factoryObj_dealloc(node_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *node_factoryObj_new_node_1(node_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::q_name_pair qn;
	ambulant::lib::q_attributes_list qattrs;
	ambulant::lib::node_context* ctx;
	ambulant::lib::xml_string qn_first;
	ambulant::lib::xml_string qn_second;
	char *qn_first_cstr="";
	char *qn_second_cstr="";
	if (!PyArg_ParseTuple(_args, "(ss)O&O&",
	                      &qn_first_cstr, &qn_second_cstr,
	                      ambulant_attributes_list_Convert, &qattrs,
	                      node_contextObj_Convert, &ctx))
		return NULL;
	qn_first = qn_first_cstr;
	qn_second = qn_second_cstr;
	qn = ambulant::lib::q_name_pair(qn_first, qn_second);
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->new_node(qn,
	                                                      qattrs,
	                                                      ctx);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *node_factoryObj_new_node_2(node_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* other;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &other))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->new_node(other);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *node_factoryObj_new_data_node(node_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char *data__in__;
	size_t data__len__;
	int data__in_len__;
	ambulant::lib::node_context* ctx;
	if (!PyArg_ParseTuple(_args, "s#O&",
	                      &data__in__, &data__in_len__,
	                      node_contextObj_Convert, &ctx))
		return NULL;
	data__len__ = data__in_len__;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->new_data_node(data__in__, data__len__,
	                                                           ctx);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyMethodDef node_factoryObj_methods[] = {
	{"new_node_1", (PyCFunction)node_factoryObj_new_node_1, 1,
	 PyDoc_STR("(ambulant::lib::q_name_pair qn, ambulant::lib::q_attributes_list qattrs, ambulant::lib::node_context* ctx) -> (ambulant::lib::node* _rv)")},
	{"new_node_2", (PyCFunction)node_factoryObj_new_node_2, 1,
	 PyDoc_STR("(ambulant::lib::node* other) -> (ambulant::lib::node* _rv)")},
	{"new_data_node", (PyCFunction)node_factoryObj_new_data_node, 1,
	 PyDoc_STR("(Buffer data, ambulant::lib::node_context* ctx) -> (ambulant::lib::node* _rv)")},
	{NULL, NULL, 0}
};

#define node_factoryObj_getsetlist NULL


static int node_factoryObj_compare(node_factoryObject *self, node_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define node_factoryObj_repr NULL

static long node_factoryObj_hash(node_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int node_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::node_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, node_factoryObj_Convert, &itself))
	{
		((node_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define node_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *node_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((node_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define node_factoryObj_tp_free PyObject_Del


PyTypeObject node_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.node_factory", /*tp_name*/
	sizeof(node_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) node_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) node_factoryObj_compare, /*tp_compare*/
	(reprfunc) node_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) node_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	node_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	node_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	node_factoryObj_tp_init, /* tp_init */
	node_factoryObj_tp_alloc, /* tp_alloc */
	node_factoryObj_tp_new, /* tp_new */
	node_factoryObj_tp_free, /* tp_free */
};

/* ------------------ End object type node_factory ------------------ */


/* ---------------------- Object type document ---------------------- */

extern PyTypeObject document_Type;

inline bool documentObj_Check(PyObject *x)
{
	return ((x)->ob_type == &document_Type);
}

typedef struct documentObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::document* ob_itself;
} documentObject;

PyObject *documentObj_New(ambulant::lib::document* itself)
{
	documentObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_document
	document *encaps_itself = dynamic_cast<document *>(itself);
	if (encaps_itself && encaps_itself->py_document)
	{
		Py_INCREF(encaps_itself->py_document);
		return encaps_itself->py_document;
	}
#endif
	it = PyObject_NEW(documentObject, &document_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int documentObj_Convert(PyObject *v, ambulant::lib::document* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_document
	if (!documentObj_Check(v))
	{
		*p_itself = Py_WrapAs_document(v);
		if (*p_itself) return 1;
	}
#endif
	if (!documentObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "document required");
		return 0;
	}
	*p_itself = ((documentObject *)v)->ob_itself;
	return 1;
}

static void documentObj_dealloc(documentObject *self)
{
	node_context_Type.tp_dealloc((PyObject *)self);
}

static PyObject *documentObj_get_root_1(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool detach;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &detach))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->get_root(detach);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *documentObj_get_root_2(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node* _rv = _self->ob_itself->get_root();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *documentObj_tree_changed(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->tree_changed();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *documentObj_locate_node_1(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* path;
	if (!PyArg_ParseTuple(_args, "s",
	                      &path))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node* _rv = _self->ob_itself->locate_node(path);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *documentObj_locate_node_2(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* path;
	if (!PyArg_ParseTuple(_args, "s",
	                      &path))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node* _rv = _self->ob_itself->locate_node(path);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *documentObj_get_src_url(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::url _rv = _self->ob_itself->get_src_url();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_url_New(_rv));
	return _res;
}

static PyObject *documentObj_set_prefix_mapping(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string prefix;
	std::string uri;
	char *prefix_cstr="";
	char *uri_cstr="";
	if (!PyArg_ParseTuple(_args, "ss",
	                      &prefix_cstr,
	                      &uri_cstr))
		return NULL;
	prefix = prefix_cstr;
	uri = uri_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_prefix_mapping(prefix,
	                                     uri);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *documentObj_get_namespace_prefix(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::xml_string uri;
	char *uri_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &uri_cstr))
		return NULL;
	uri = uri_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::xml_string& _rv = _self->ob_itself->get_namespace_prefix(uri);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *documentObj_is_supported_prefix(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::xml_string prefix;
	char *prefix_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &prefix_cstr))
		return NULL;
	prefix = prefix_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_supported_prefix(prefix);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *documentObj_is_supported_namespace(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::xml_string uri;
	char *uri_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &uri_cstr))
		return NULL;
	uri = uri_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_supported_namespace(uri);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *documentObj_resolve_url(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url rurl;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_url_Convert, &rurl))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::url _rv = _self->ob_itself->resolve_url(rurl);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_url_New(_rv));
	return _res;
}

static PyObject *documentObj_get_node(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string idd;
	char *idd_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &idd_cstr))
		return NULL;
	idd = idd_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::node* _rv = _self->ob_itself->get_node(idd);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     nodeObj_New, _rv);
	return _res;
}

static PyObject *documentObj_set_src_url(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url u;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_url_Convert, &u))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_src_url(u);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *documentObj_get_state(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::state_component* _rv = _self->ob_itself->get_state();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     state_componentObj_New, _rv);
	return _res;
}

static PyObject *documentObj_set_state(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::state_component* state;
	if (!PyArg_ParseTuple(_args, "O&",
	                      state_componentObj_Convert, &state))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_state(state);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *documentObj_apply_avt(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	ambulant::lib::xml_string attrname;
	ambulant::lib::xml_string attrvalue;
	char *attrname_cstr="";
	char *attrvalue_cstr="";
	if (!PyArg_ParseTuple(_args, "O&ss",
	                      nodeObj_Convert, &n,
	                      &attrname_cstr,
	                      &attrvalue_cstr))
		return NULL;
	attrname = attrname_cstr;
	attrvalue = attrvalue_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::xml_string& _rv = _self->ob_itself->apply_avt(n,
	                                                                   attrname,
	                                                                   attrvalue);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *documentObj_on_state_change(documentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* ref;
	if (!PyArg_ParseTuple(_args, "s",
	                      &ref))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->on_state_change(ref);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef documentObj_methods[] = {
	{"get_root_1", (PyCFunction)documentObj_get_root_1, 1,
	 PyDoc_STR("(bool detach) -> (ambulant::lib::node* _rv)")},
	{"get_root_2", (PyCFunction)documentObj_get_root_2, 1,
	 PyDoc_STR("() -> (const ambulant::lib::node* _rv)")},
	{"tree_changed", (PyCFunction)documentObj_tree_changed, 1,
	 PyDoc_STR("() -> None")},
	{"locate_node_1", (PyCFunction)documentObj_locate_node_1, 1,
	 PyDoc_STR("(char* path) -> (ambulant::lib::node* _rv)")},
	{"locate_node_2", (PyCFunction)documentObj_locate_node_2, 1,
	 PyDoc_STR("(char* path) -> (const ambulant::lib::node* _rv)")},
	{"get_src_url", (PyCFunction)documentObj_get_src_url, 1,
	 PyDoc_STR("() -> (ambulant::net::url _rv)")},
	{"set_prefix_mapping", (PyCFunction)documentObj_set_prefix_mapping, 1,
	 PyDoc_STR("(std::string prefix, std::string uri) -> None")},
	{"get_namespace_prefix", (PyCFunction)documentObj_get_namespace_prefix, 1,
	 PyDoc_STR("(ambulant::lib::xml_string uri) -> (const ambulant::lib::xml_string& _rv)")},
	{"is_supported_prefix", (PyCFunction)documentObj_is_supported_prefix, 1,
	 PyDoc_STR("(ambulant::lib::xml_string prefix) -> (bool _rv)")},
	{"is_supported_namespace", (PyCFunction)documentObj_is_supported_namespace, 1,
	 PyDoc_STR("(ambulant::lib::xml_string uri) -> (bool _rv)")},
	{"resolve_url", (PyCFunction)documentObj_resolve_url, 1,
	 PyDoc_STR("(ambulant::net::url rurl) -> (ambulant::net::url _rv)")},
	{"get_node", (PyCFunction)documentObj_get_node, 1,
	 PyDoc_STR("(std::string idd) -> (const ambulant::lib::node* _rv)")},
	{"set_src_url", (PyCFunction)documentObj_set_src_url, 1,
	 PyDoc_STR("(ambulant::net::url u) -> None")},
	{"get_state", (PyCFunction)documentObj_get_state, 1,
	 PyDoc_STR("() -> (ambulant::common::state_component* _rv)")},
	{"set_state", (PyCFunction)documentObj_set_state, 1,
	 PyDoc_STR("(ambulant::common::state_component* state) -> None")},
	{"apply_avt", (PyCFunction)documentObj_apply_avt, 1,
	 PyDoc_STR("(ambulant::lib::node* n, ambulant::lib::xml_string attrname, ambulant::lib::xml_string attrvalue) -> (const ambulant::lib::xml_string& _rv)")},
	{"on_state_change", (PyCFunction)documentObj_on_state_change, 1,
	 PyDoc_STR("(char* ref) -> None")},
	{NULL, NULL, 0}
};

#define documentObj_getsetlist NULL


static int documentObj_compare(documentObject *self, documentObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define documentObj_repr NULL

static long documentObj_hash(documentObject *self)
{
	return (long)self->ob_itself;
}
static int documentObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::document* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, documentObj_Convert, &itself))
	{
		((documentObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define documentObj_tp_alloc PyType_GenericAlloc

static PyObject *documentObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((documentObject *)_self)->ob_itself = NULL;
	return _self;
}

#define documentObj_tp_free PyObject_Del


PyTypeObject document_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.document", /*tp_name*/
	sizeof(documentObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) documentObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) documentObj_compare, /*tp_compare*/
	(reprfunc) documentObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) documentObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	documentObj_methods, /* tp_methods */
	0, /*tp_members*/
	documentObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	documentObj_tp_init, /* tp_init */
	documentObj_tp_alloc, /* tp_alloc */
	documentObj_tp_new, /* tp_new */
	documentObj_tp_free, /* tp_free */
};

/* -------------------- End object type document -------------------- */


/* ----------------------- Object type event ------------------------ */

extern PyTypeObject event_Type;

inline bool eventObj_Check(PyObject *x)
{
	return ((x)->ob_type == &event_Type);
}

typedef struct eventObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::event* ob_itself;
} eventObject;

PyObject *eventObj_New(ambulant::lib::event* itself)
{
	eventObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_event
	event *encaps_itself = dynamic_cast<event *>(itself);
	if (encaps_itself && encaps_itself->py_event)
	{
		Py_INCREF(encaps_itself->py_event);
		return encaps_itself->py_event;
	}
#endif
	it = PyObject_NEW(eventObject, &event_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int eventObj_Convert(PyObject *v, ambulant::lib::event* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_event
	if (!eventObj_Check(v))
	{
		*p_itself = Py_WrapAs_event(v);
		if (*p_itself) return 1;
	}
#endif
	if (!eventObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "event required");
		return 0;
	}
	*p_itself = ((eventObject *)v)->ob_itself;
	return 1;
}

static void eventObj_dealloc(eventObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *eventObj_fire(eventObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->fire();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef eventObj_methods[] = {
	{"fire", (PyCFunction)eventObj_fire, 1,
	 PyDoc_STR("() -> None")},
	{NULL, NULL, 0}
};

#define eventObj_getsetlist NULL


static int eventObj_compare(eventObject *self, eventObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define eventObj_repr NULL

static long eventObj_hash(eventObject *self)
{
	return (long)self->ob_itself;
}
static int eventObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::event* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, eventObj_Convert, &itself))
	{
		((eventObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define eventObj_tp_alloc PyType_GenericAlloc

static PyObject *eventObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((eventObject *)_self)->ob_itself = NULL;
	return _self;
}

#define eventObj_tp_free PyObject_Del


PyTypeObject event_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.event", /*tp_name*/
	sizeof(eventObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) eventObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) eventObj_compare, /*tp_compare*/
	(reprfunc) eventObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) eventObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	eventObj_methods, /* tp_methods */
	0, /*tp_members*/
	eventObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	eventObj_tp_init, /* tp_init */
	eventObj_tp_alloc, /* tp_alloc */
	eventObj_tp_new, /* tp_new */
	eventObj_tp_free, /* tp_free */
};

/* --------------------- End object type event ---------------------- */


/* ------------------ Object type event_processor ------------------- */

extern PyTypeObject event_processor_Type;

inline bool event_processorObj_Check(PyObject *x)
{
	return ((x)->ob_type == &event_processor_Type);
}

typedef struct event_processorObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::event_processor* ob_itself;
} event_processorObject;

PyObject *event_processorObj_New(ambulant::lib::event_processor* itself)
{
	event_processorObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_event_processor
	event_processor *encaps_itself = dynamic_cast<event_processor *>(itself);
	if (encaps_itself && encaps_itself->py_event_processor)
	{
		Py_INCREF(encaps_itself->py_event_processor);
		return encaps_itself->py_event_processor;
	}
#endif
	it = PyObject_NEW(event_processorObject, &event_processor_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int event_processorObj_Convert(PyObject *v, ambulant::lib::event_processor* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_event_processor
	if (!event_processorObj_Check(v))
	{
		*p_itself = Py_WrapAs_event_processor(v);
		if (*p_itself) return 1;
	}
#endif
	if (!event_processorObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "event_processor required");
		return 0;
	}
	*p_itself = ((event_processorObject *)v)->ob_itself;
	return 1;
}

static void event_processorObj_dealloc(event_processorObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *event_processorObj_add_event(event_processorObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::event* pe;
	ambulant::lib::timer::time_type t;
	ambulant::lib::event_priority priority;
	if (!PyArg_ParseTuple(_args, "O&ll",
	                      eventObj_Convert, &pe,
	                      &t,
	                      &priority))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->add_event(pe,
	                            t,
	                            priority);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *event_processorObj_cancel_all_events(event_processorObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->cancel_all_events();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *event_processorObj_cancel_event(event_processorObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::event* pe;
	ambulant::lib::event_priority priority;
	if (!PyArg_ParseTuple(_args, "O&l",
	                      eventObj_Convert, &pe,
	                      &priority))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->cancel_event(pe,
	                                          priority);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *event_processorObj_get_timer(event_processorObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer* _rv = _self->ob_itself->get_timer();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     timerObj_New, _rv);
	return _res;
}

static PyMethodDef event_processorObj_methods[] = {
	{"add_event", (PyCFunction)event_processorObj_add_event, 1,
	 PyDoc_STR("(ambulant::lib::event* pe, ambulant::lib::timer::time_type t, ambulant::lib::event_priority priority) -> None")},
	{"cancel_all_events", (PyCFunction)event_processorObj_cancel_all_events, 1,
	 PyDoc_STR("() -> None")},
	{"cancel_event", (PyCFunction)event_processorObj_cancel_event, 1,
	 PyDoc_STR("(ambulant::lib::event* pe, ambulant::lib::event_priority priority) -> (bool _rv)")},
	{"get_timer", (PyCFunction)event_processorObj_get_timer, 1,
	 PyDoc_STR("() -> (ambulant::lib::timer* _rv)")},
	{NULL, NULL, 0}
};

#define event_processorObj_getsetlist NULL


static int event_processorObj_compare(event_processorObject *self, event_processorObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define event_processorObj_repr NULL

static long event_processorObj_hash(event_processorObject *self)
{
	return (long)self->ob_itself;
}
static int event_processorObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::event_processor* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, event_processorObj_Convert, &itself))
	{
		((event_processorObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define event_processorObj_tp_alloc PyType_GenericAlloc

static PyObject *event_processorObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((event_processorObject *)_self)->ob_itself = NULL;
	return _self;
}

#define event_processorObj_tp_free PyObject_Del


PyTypeObject event_processor_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.event_processor", /*tp_name*/
	sizeof(event_processorObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) event_processorObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) event_processorObj_compare, /*tp_compare*/
	(reprfunc) event_processorObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) event_processorObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	event_processorObj_methods, /* tp_methods */
	0, /*tp_members*/
	event_processorObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	event_processorObj_tp_init, /* tp_init */
	event_processorObj_tp_alloc, /* tp_alloc */
	event_processorObj_tp_new, /* tp_new */
	event_processorObj_tp_free, /* tp_free */
};

/* ---------------- End object type event_processor ----------------- */


/* ------------------- Object type parser_factory ------------------- */

extern PyTypeObject parser_factory_Type;

inline bool parser_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &parser_factory_Type);
}

typedef struct parser_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::parser_factory* ob_itself;
} parser_factoryObject;

PyObject *parser_factoryObj_New(ambulant::lib::parser_factory* itself)
{
	parser_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_parser_factory
	parser_factory *encaps_itself = dynamic_cast<parser_factory *>(itself);
	if (encaps_itself && encaps_itself->py_parser_factory)
	{
		Py_INCREF(encaps_itself->py_parser_factory);
		return encaps_itself->py_parser_factory;
	}
#endif
	it = PyObject_NEW(parser_factoryObject, &parser_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int parser_factoryObj_Convert(PyObject *v, ambulant::lib::parser_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_parser_factory
	if (!parser_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_parser_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!parser_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "parser_factory required");
		return 0;
	}
	*p_itself = ((parser_factoryObject *)v)->ob_itself;
	return 1;
}

static void parser_factoryObj_dealloc(parser_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *parser_factoryObj_get_parser_name(parser_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	std::string _rv = _self->ob_itself->get_parser_name();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyMethodDef parser_factoryObj_methods[] = {
	{"get_parser_name", (PyCFunction)parser_factoryObj_get_parser_name, 1,
	 PyDoc_STR("() -> (std::string _rv)")},
	{NULL, NULL, 0}
};

#define parser_factoryObj_getsetlist NULL


static int parser_factoryObj_compare(parser_factoryObject *self, parser_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define parser_factoryObj_repr NULL

static long parser_factoryObj_hash(parser_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int parser_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::parser_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, parser_factoryObj_Convert, &itself))
	{
		((parser_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define parser_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *parser_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((parser_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define parser_factoryObj_tp_free PyObject_Del


PyTypeObject parser_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.parser_factory", /*tp_name*/
	sizeof(parser_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) parser_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) parser_factoryObj_compare, /*tp_compare*/
	(reprfunc) parser_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) parser_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	parser_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	parser_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	parser_factoryObj_tp_init, /* tp_init */
	parser_factoryObj_tp_alloc, /* tp_alloc */
	parser_factoryObj_tp_new, /* tp_new */
	parser_factoryObj_tp_free, /* tp_free */
};

/* ----------------- End object type parser_factory ----------------- */


/* --------------- Object type global_parser_factory ---------------- */

extern PyTypeObject global_parser_factory_Type;

inline bool global_parser_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &global_parser_factory_Type);
}

typedef struct global_parser_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::global_parser_factory* ob_itself;
} global_parser_factoryObject;

PyObject *global_parser_factoryObj_New(ambulant::lib::global_parser_factory* itself)
{
	global_parser_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_global_parser_factory
	global_parser_factory *encaps_itself = dynamic_cast<global_parser_factory *>(itself);
	if (encaps_itself && encaps_itself->py_global_parser_factory)
	{
		Py_INCREF(encaps_itself->py_global_parser_factory);
		return encaps_itself->py_global_parser_factory;
	}
#endif
	it = PyObject_NEW(global_parser_factoryObject, &global_parser_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int global_parser_factoryObj_Convert(PyObject *v, ambulant::lib::global_parser_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_global_parser_factory
	if (!global_parser_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_global_parser_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!global_parser_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "global_parser_factory required");
		return 0;
	}
	*p_itself = ((global_parser_factoryObject *)v)->ob_itself;
	return 1;
}

static void global_parser_factoryObj_dealloc(global_parser_factoryObject *self)
{
	parser_factory_Type.tp_dealloc((PyObject *)self);
}

static PyObject *global_parser_factoryObj_add_factory(global_parser_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::parser_factory* pf;
	if (!PyArg_ParseTuple(_args, "O&",
	                      parser_factoryObj_Convert, &pf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->add_factory(pf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef global_parser_factoryObj_methods[] = {
	{"add_factory", (PyCFunction)global_parser_factoryObj_add_factory, 1,
	 PyDoc_STR("(ambulant::lib::parser_factory* pf) -> None")},
	{NULL, NULL, 0}
};

#define global_parser_factoryObj_getsetlist NULL


static int global_parser_factoryObj_compare(global_parser_factoryObject *self, global_parser_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define global_parser_factoryObj_repr NULL

static long global_parser_factoryObj_hash(global_parser_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int global_parser_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::global_parser_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, global_parser_factoryObj_Convert, &itself))
	{
		((global_parser_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define global_parser_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *global_parser_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((global_parser_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define global_parser_factoryObj_tp_free PyObject_Del


PyTypeObject global_parser_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.global_parser_factory", /*tp_name*/
	sizeof(global_parser_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) global_parser_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) global_parser_factoryObj_compare, /*tp_compare*/
	(reprfunc) global_parser_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) global_parser_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	global_parser_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	global_parser_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	global_parser_factoryObj_tp_init, /* tp_init */
	global_parser_factoryObj_tp_alloc, /* tp_alloc */
	global_parser_factoryObj_tp_new, /* tp_new */
	global_parser_factoryObj_tp_free, /* tp_free */
};

/* ------------- End object type global_parser_factory -------------- */


/* --------------------- Object type xml_parser --------------------- */

extern PyTypeObject xml_parser_Type;

inline bool xml_parserObj_Check(PyObject *x)
{
	return ((x)->ob_type == &xml_parser_Type);
}

typedef struct xml_parserObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::xml_parser* ob_itself;
} xml_parserObject;

PyObject *xml_parserObj_New(ambulant::lib::xml_parser* itself)
{
	xml_parserObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_xml_parser
	xml_parser *encaps_itself = dynamic_cast<xml_parser *>(itself);
	if (encaps_itself && encaps_itself->py_xml_parser)
	{
		Py_INCREF(encaps_itself->py_xml_parser);
		return encaps_itself->py_xml_parser;
	}
#endif
	it = PyObject_NEW(xml_parserObject, &xml_parser_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int xml_parserObj_Convert(PyObject *v, ambulant::lib::xml_parser* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_xml_parser
	if (!xml_parserObj_Check(v))
	{
		*p_itself = Py_WrapAs_xml_parser(v);
		if (*p_itself) return 1;
	}
#endif
	if (!xml_parserObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "xml_parser required");
		return 0;
	}
	*p_itself = ((xml_parserObject *)v)->ob_itself;
	return 1;
}

static void xml_parserObj_dealloc(xml_parserObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *xml_parserObj_parse(xml_parserObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char *buf__in__;
	size_t buf__len__;
	int buf__in_len__;
	bool final;
	if (!PyArg_ParseTuple(_args, "s#O&",
	                      &buf__in__, &buf__in_len__,
	                      bool_Convert, &final))
		return NULL;
	buf__len__ = buf__in_len__;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->parse(buf__in__, buf__len__,
	                                   final);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyMethodDef xml_parserObj_methods[] = {
	{"parse", (PyCFunction)xml_parserObj_parse, 1,
	 PyDoc_STR("(Buffer buf, bool final) -> (bool _rv)")},
	{NULL, NULL, 0}
};

#define xml_parserObj_getsetlist NULL


static int xml_parserObj_compare(xml_parserObject *self, xml_parserObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define xml_parserObj_repr NULL

static long xml_parserObj_hash(xml_parserObject *self)
{
	return (long)self->ob_itself;
}
static int xml_parserObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::xml_parser* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, xml_parserObj_Convert, &itself))
	{
		((xml_parserObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define xml_parserObj_tp_alloc PyType_GenericAlloc

static PyObject *xml_parserObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((xml_parserObject *)_self)->ob_itself = NULL;
	return _self;
}

#define xml_parserObj_tp_free PyObject_Del


PyTypeObject xml_parser_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.xml_parser", /*tp_name*/
	sizeof(xml_parserObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) xml_parserObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) xml_parserObj_compare, /*tp_compare*/
	(reprfunc) xml_parserObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) xml_parserObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	xml_parserObj_methods, /* tp_methods */
	0, /*tp_members*/
	xml_parserObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	xml_parserObj_tp_init, /* tp_init */
	xml_parserObj_tp_alloc, /* tp_alloc */
	xml_parserObj_tp_new, /* tp_new */
	xml_parserObj_tp_free, /* tp_free */
};

/* ------------------- End object type xml_parser ------------------- */


/* ------------------ Object type system_embedder ------------------- */

extern PyTypeObject system_embedder_Type;

inline bool system_embedderObj_Check(PyObject *x)
{
	return ((x)->ob_type == &system_embedder_Type);
}

typedef struct system_embedderObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::system_embedder* ob_itself;
} system_embedderObject;

PyObject *system_embedderObj_New(ambulant::lib::system_embedder* itself)
{
	system_embedderObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_system_embedder
	system_embedder *encaps_itself = dynamic_cast<system_embedder *>(itself);
	if (encaps_itself && encaps_itself->py_system_embedder)
	{
		Py_INCREF(encaps_itself->py_system_embedder);
		return encaps_itself->py_system_embedder;
	}
#endif
	it = PyObject_NEW(system_embedderObject, &system_embedder_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int system_embedderObj_Convert(PyObject *v, ambulant::lib::system_embedder* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_system_embedder
	if (!system_embedderObj_Check(v))
	{
		*p_itself = Py_WrapAs_system_embedder(v);
		if (*p_itself) return 1;
	}
#endif
	if (!system_embedderObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "system_embedder required");
		return 0;
	}
	*p_itself = ((system_embedderObject *)v)->ob_itself;
	return 1;
}

static void system_embedderObj_dealloc(system_embedderObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *system_embedderObj_show_file(system_embedderObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url href;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_url_Convert, &href))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->show_file(href);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef system_embedderObj_methods[] = {
	{"show_file", (PyCFunction)system_embedderObj_show_file, 1,
	 PyDoc_STR("(ambulant::net::url href) -> None")},
	{NULL, NULL, 0}
};

#define system_embedderObj_getsetlist NULL


static int system_embedderObj_compare(system_embedderObject *self, system_embedderObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define system_embedderObj_repr NULL

static long system_embedderObj_hash(system_embedderObject *self)
{
	return (long)self->ob_itself;
}
static int system_embedderObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::system_embedder* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, system_embedderObj_Convert, &itself))
	{
		((system_embedderObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define system_embedderObj_tp_alloc PyType_GenericAlloc

static PyObject *system_embedderObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((system_embedderObject *)_self)->ob_itself = NULL;
	return _self;
}

#define system_embedderObj_tp_free PyObject_Del


PyTypeObject system_embedder_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.system_embedder", /*tp_name*/
	sizeof(system_embedderObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) system_embedderObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) system_embedderObj_compare, /*tp_compare*/
	(reprfunc) system_embedderObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) system_embedderObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	system_embedderObj_methods, /* tp_methods */
	0, /*tp_members*/
	system_embedderObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	system_embedderObj_tp_init, /* tp_init */
	system_embedderObj_tp_alloc, /* tp_alloc */
	system_embedderObj_tp_new, /* tp_new */
	system_embedderObj_tp_free, /* tp_free */
};

/* ---------------- End object type system_embedder ----------------- */


/* ----------------------- Object type timer ------------------------ */

extern PyTypeObject timer_Type;

inline bool timerObj_Check(PyObject *x)
{
	return ((x)->ob_type == &timer_Type);
}

typedef struct timerObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::timer* ob_itself;
} timerObject;

PyObject *timerObj_New(ambulant::lib::timer* itself)
{
	timerObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_timer
	timer *encaps_itself = dynamic_cast<timer *>(itself);
	if (encaps_itself && encaps_itself->py_timer)
	{
		Py_INCREF(encaps_itself->py_timer);
		return encaps_itself->py_timer;
	}
#endif
	it = PyObject_NEW(timerObject, &timer_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int timerObj_Convert(PyObject *v, ambulant::lib::timer* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_timer
	if (!timerObj_Check(v))
	{
		*p_itself = Py_WrapAs_timer(v);
		if (*p_itself) return 1;
	}
#endif
	if (!timerObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "timer required");
		return 0;
	}
	*p_itself = ((timerObject *)v)->ob_itself;
	return 1;
}

static void timerObj_dealloc(timerObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *timerObj_elapsed(timerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer::time_type _rv = _self->ob_itself->elapsed();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *timerObj_get_realtime_speed(timerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_realtime_speed();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyObject *timerObj_set_drift(timerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::signed_time_type drift;
	if (!PyArg_ParseTuple(_args, "l",
	                      &drift))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer::signed_time_type _rv = _self->ob_itself->set_drift(drift);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *timerObj_get_drift(timerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer::signed_time_type _rv = _self->ob_itself->get_drift();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *timerObj_skew(timerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::signed_time_type skew;
	if (!PyArg_ParseTuple(_args, "l",
	                      &skew))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->skew(skew);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timerObj_running(timerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->running();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *timerObj_is_slaved(timerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_slaved();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyMethodDef timerObj_methods[] = {
	{"elapsed", (PyCFunction)timerObj_elapsed, 1,
	 PyDoc_STR("() -> (ambulant::lib::timer::time_type _rv)")},
	{"get_realtime_speed", (PyCFunction)timerObj_get_realtime_speed, 1,
	 PyDoc_STR("() -> (double _rv)")},
	{"set_drift", (PyCFunction)timerObj_set_drift, 1,
	 PyDoc_STR("(ambulant::lib::timer::signed_time_type drift) -> (ambulant::lib::timer::signed_time_type _rv)")},
	{"get_drift", (PyCFunction)timerObj_get_drift, 1,
	 PyDoc_STR("() -> (ambulant::lib::timer::signed_time_type _rv)")},
	{"skew", (PyCFunction)timerObj_skew, 1,
	 PyDoc_STR("(ambulant::lib::timer::signed_time_type skew) -> None")},
	{"running", (PyCFunction)timerObj_running, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"is_slaved", (PyCFunction)timerObj_is_slaved, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{NULL, NULL, 0}
};

#define timerObj_getsetlist NULL


static int timerObj_compare(timerObject *self, timerObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define timerObj_repr NULL

static long timerObj_hash(timerObject *self)
{
	return (long)self->ob_itself;
}
static int timerObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::timer* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, timerObj_Convert, &itself))
	{
		((timerObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define timerObj_tp_alloc PyType_GenericAlloc

static PyObject *timerObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((timerObject *)_self)->ob_itself = NULL;
	return _self;
}

#define timerObj_tp_free PyObject_Del


PyTypeObject timer_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.timer", /*tp_name*/
	sizeof(timerObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) timerObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) timerObj_compare, /*tp_compare*/
	(reprfunc) timerObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) timerObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	timerObj_methods, /* tp_methods */
	0, /*tp_members*/
	timerObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	timerObj_tp_init, /* tp_init */
	timerObj_tp_alloc, /* tp_alloc */
	timerObj_tp_new, /* tp_new */
	timerObj_tp_free, /* tp_free */
};

/* --------------------- End object type timer ---------------------- */


/* ------------------- Object type timer_control -------------------- */

extern PyTypeObject timer_control_Type;

inline bool timer_controlObj_Check(PyObject *x)
{
	return ((x)->ob_type == &timer_control_Type);
}

typedef struct timer_controlObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::timer_control* ob_itself;
} timer_controlObject;

PyObject *timer_controlObj_New(ambulant::lib::timer_control* itself)
{
	timer_controlObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_timer_control
	timer_control *encaps_itself = dynamic_cast<timer_control *>(itself);
	if (encaps_itself && encaps_itself->py_timer_control)
	{
		Py_INCREF(encaps_itself->py_timer_control);
		return encaps_itself->py_timer_control;
	}
#endif
	it = PyObject_NEW(timer_controlObject, &timer_control_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int timer_controlObj_Convert(PyObject *v, ambulant::lib::timer_control* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_timer_control
	if (!timer_controlObj_Check(v))
	{
		*p_itself = Py_WrapAs_timer_control(v);
		if (*p_itself) return 1;
	}
#endif
	if (!timer_controlObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "timer_control required");
		return 0;
	}
	*p_itself = ((timer_controlObject *)v)->ob_itself;
	return 1;
}

static void timer_controlObj_dealloc(timer_controlObject *self)
{
	timer_Type.tp_dealloc((PyObject *)self);
}

static PyObject *timer_controlObj_elapsed_1(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer::time_type _rv = _self->ob_itself->elapsed();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *timer_controlObj_elapsed_2(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::time_type pt;
	if (!PyArg_ParseTuple(_args, "l",
	                      &pt))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer::time_type _rv = _self->ob_itself->elapsed(pt);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *timer_controlObj_start(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::time_type t;
	if (!PyArg_ParseTuple(_args, "l",
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->start(t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_controlObj_stop(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->stop();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_controlObj_pause(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->pause();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_controlObj_resume(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->resume();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_controlObj_set_speed(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	double speed;
	if (!PyArg_ParseTuple(_args, "d",
	                      &speed))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_speed(speed);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_controlObj_set_time(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::time_type t;
	if (!PyArg_ParseTuple(_args, "l",
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_time(t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_controlObj_get_speed(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_speed();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyObject *timer_controlObj_running(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->running();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *timer_controlObj_get_realtime_speed(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_realtime_speed();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyObject *timer_controlObj_set_drift(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::signed_time_type drift;
	if (!PyArg_ParseTuple(_args, "l",
	                      &drift))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer::signed_time_type _rv = _self->ob_itself->set_drift(drift);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *timer_controlObj_get_drift(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer::signed_time_type _rv = _self->ob_itself->get_drift();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *timer_controlObj_skew(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::signed_time_type skew;
	if (!PyArg_ParseTuple(_args, "l",
	                      &skew))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->skew(skew);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_controlObj_set_observer(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer_observer* obs;
	if (!PyArg_ParseTuple(_args, "O&",
	                      timer_observerObj_Convert, &obs))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_observer(obs);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_controlObj_set_slaved(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool slaved;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &slaved))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_slaved(slaved);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_controlObj_is_slaved(timer_controlObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_slaved();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyMethodDef timer_controlObj_methods[] = {
	{"elapsed_1", (PyCFunction)timer_controlObj_elapsed_1, 1,
	 PyDoc_STR("() -> (ambulant::lib::timer::time_type _rv)")},
	{"elapsed_2", (PyCFunction)timer_controlObj_elapsed_2, 1,
	 PyDoc_STR("(ambulant::lib::timer::time_type pt) -> (ambulant::lib::timer::time_type _rv)")},
	{"start", (PyCFunction)timer_controlObj_start, 1,
	 PyDoc_STR("(ambulant::lib::timer::time_type t) -> None")},
	{"stop", (PyCFunction)timer_controlObj_stop, 1,
	 PyDoc_STR("() -> None")},
	{"pause", (PyCFunction)timer_controlObj_pause, 1,
	 PyDoc_STR("() -> None")},
	{"resume", (PyCFunction)timer_controlObj_resume, 1,
	 PyDoc_STR("() -> None")},
	{"set_speed", (PyCFunction)timer_controlObj_set_speed, 1,
	 PyDoc_STR("(double speed) -> None")},
	{"set_time", (PyCFunction)timer_controlObj_set_time, 1,
	 PyDoc_STR("(ambulant::lib::timer::time_type t) -> None")},
	{"get_speed", (PyCFunction)timer_controlObj_get_speed, 1,
	 PyDoc_STR("() -> (double _rv)")},
	{"running", (PyCFunction)timer_controlObj_running, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"get_realtime_speed", (PyCFunction)timer_controlObj_get_realtime_speed, 1,
	 PyDoc_STR("() -> (double _rv)")},
	{"set_drift", (PyCFunction)timer_controlObj_set_drift, 1,
	 PyDoc_STR("(ambulant::lib::timer::signed_time_type drift) -> (ambulant::lib::timer::signed_time_type _rv)")},
	{"get_drift", (PyCFunction)timer_controlObj_get_drift, 1,
	 PyDoc_STR("() -> (ambulant::lib::timer::signed_time_type _rv)")},
	{"skew", (PyCFunction)timer_controlObj_skew, 1,
	 PyDoc_STR("(ambulant::lib::timer::signed_time_type skew) -> None")},
	{"set_observer", (PyCFunction)timer_controlObj_set_observer, 1,
	 PyDoc_STR("(ambulant::lib::timer_observer* obs) -> None")},
	{"set_slaved", (PyCFunction)timer_controlObj_set_slaved, 1,
	 PyDoc_STR("(bool slaved) -> None")},
	{"is_slaved", (PyCFunction)timer_controlObj_is_slaved, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{NULL, NULL, 0}
};

#define timer_controlObj_getsetlist NULL


static int timer_controlObj_compare(timer_controlObject *self, timer_controlObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define timer_controlObj_repr NULL

static long timer_controlObj_hash(timer_controlObject *self)
{
	return (long)self->ob_itself;
}
static int timer_controlObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::timer_control* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, timer_controlObj_Convert, &itself))
	{
		((timer_controlObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define timer_controlObj_tp_alloc PyType_GenericAlloc

static PyObject *timer_controlObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((timer_controlObject *)_self)->ob_itself = NULL;
	return _self;
}

#define timer_controlObj_tp_free PyObject_Del


PyTypeObject timer_control_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.timer_control", /*tp_name*/
	sizeof(timer_controlObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) timer_controlObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) timer_controlObj_compare, /*tp_compare*/
	(reprfunc) timer_controlObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) timer_controlObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	timer_controlObj_methods, /* tp_methods */
	0, /*tp_members*/
	timer_controlObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	timer_controlObj_tp_init, /* tp_init */
	timer_controlObj_tp_alloc, /* tp_alloc */
	timer_controlObj_tp_new, /* tp_new */
	timer_controlObj_tp_free, /* tp_free */
};

/* ----------------- End object type timer_control ------------------ */


/* ----------------- Object type timer_control_impl ----------------- */

extern PyTypeObject timer_control_impl_Type;

inline bool timer_control_implObj_Check(PyObject *x)
{
	return ((x)->ob_type == &timer_control_impl_Type);
}

typedef struct timer_control_implObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::timer_control_impl* ob_itself;
} timer_control_implObject;

PyObject *timer_control_implObj_New(ambulant::lib::timer_control_impl* itself)
{
	timer_control_implObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_timer_control_impl
	timer_control_impl *encaps_itself = dynamic_cast<timer_control_impl *>(itself);
	if (encaps_itself && encaps_itself->py_timer_control_impl)
	{
		Py_INCREF(encaps_itself->py_timer_control_impl);
		return encaps_itself->py_timer_control_impl;
	}
#endif
	it = PyObject_NEW(timer_control_implObject, &timer_control_impl_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int timer_control_implObj_Convert(PyObject *v, ambulant::lib::timer_control_impl* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_timer_control_impl
	if (!timer_control_implObj_Check(v))
	{
		*p_itself = Py_WrapAs_timer_control_impl(v);
		if (*p_itself) return 1;
	}
#endif
	if (!timer_control_implObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "timer_control_impl required");
		return 0;
	}
	*p_itself = ((timer_control_implObject *)v)->ob_itself;
	return 1;
}

static void timer_control_implObj_dealloc(timer_control_implObject *self)
{
	timer_control_Type.tp_dealloc((PyObject *)self);
}

static PyObject *timer_control_implObj_elapsed_1(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer::time_type _rv = _self->ob_itself->elapsed();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *timer_control_implObj_elapsed_2(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::time_type pet;
	if (!PyArg_ParseTuple(_args, "l",
	                      &pet))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer::time_type _rv = _self->ob_itself->elapsed(pet);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *timer_control_implObj_start(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::time_type t;
	if (!PyArg_ParseTuple(_args, "l",
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->start(t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_control_implObj_stop(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->stop();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_control_implObj_pause(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->pause();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_control_implObj_resume(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->resume();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_control_implObj_set_speed(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	double speed;
	if (!PyArg_ParseTuple(_args, "d",
	                      &speed))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_speed(speed);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_control_implObj_set_time(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::time_type t;
	if (!PyArg_ParseTuple(_args, "l",
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_time(t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_control_implObj_get_speed(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_speed();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyObject *timer_control_implObj_running(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->running();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *timer_control_implObj_get_realtime_speed(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_realtime_speed();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyObject *timer_control_implObj_set_drift(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::signed_time_type drift;
	if (!PyArg_ParseTuple(_args, "l",
	                      &drift))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer::signed_time_type _rv = _self->ob_itself->set_drift(drift);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *timer_control_implObj_get_drift(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer::signed_time_type _rv = _self->ob_itself->get_drift();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *timer_control_implObj_skew(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer::signed_time_type skew_;
	if (!PyArg_ParseTuple(_args, "l",
	                      &skew_))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->skew(skew_);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_control_implObj_set_observer(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer_observer* obs;
	if (!PyArg_ParseTuple(_args, "O&",
	                      timer_observerObj_Convert, &obs))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_observer(obs);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_control_implObj_set_slaved(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool slaved;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &slaved))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_slaved(slaved);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_control_implObj_is_slaved(timer_control_implObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_slaved();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyMethodDef timer_control_implObj_methods[] = {
	{"elapsed_1", (PyCFunction)timer_control_implObj_elapsed_1, 1,
	 PyDoc_STR("() -> (ambulant::lib::timer::time_type _rv)")},
	{"elapsed_2", (PyCFunction)timer_control_implObj_elapsed_2, 1,
	 PyDoc_STR("(ambulant::lib::timer::time_type pet) -> (ambulant::lib::timer::time_type _rv)")},
	{"start", (PyCFunction)timer_control_implObj_start, 1,
	 PyDoc_STR("(ambulant::lib::timer::time_type t) -> None")},
	{"stop", (PyCFunction)timer_control_implObj_stop, 1,
	 PyDoc_STR("() -> None")},
	{"pause", (PyCFunction)timer_control_implObj_pause, 1,
	 PyDoc_STR("() -> None")},
	{"resume", (PyCFunction)timer_control_implObj_resume, 1,
	 PyDoc_STR("() -> None")},
	{"set_speed", (PyCFunction)timer_control_implObj_set_speed, 1,
	 PyDoc_STR("(double speed) -> None")},
	{"set_time", (PyCFunction)timer_control_implObj_set_time, 1,
	 PyDoc_STR("(ambulant::lib::timer::time_type t) -> None")},
	{"get_speed", (PyCFunction)timer_control_implObj_get_speed, 1,
	 PyDoc_STR("() -> (double _rv)")},
	{"running", (PyCFunction)timer_control_implObj_running, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"get_realtime_speed", (PyCFunction)timer_control_implObj_get_realtime_speed, 1,
	 PyDoc_STR("() -> (double _rv)")},
	{"set_drift", (PyCFunction)timer_control_implObj_set_drift, 1,
	 PyDoc_STR("(ambulant::lib::timer::signed_time_type drift) -> (ambulant::lib::timer::signed_time_type _rv)")},
	{"get_drift", (PyCFunction)timer_control_implObj_get_drift, 1,
	 PyDoc_STR("() -> (ambulant::lib::timer::signed_time_type _rv)")},
	{"skew", (PyCFunction)timer_control_implObj_skew, 1,
	 PyDoc_STR("(ambulant::lib::timer::signed_time_type skew_) -> None")},
	{"set_observer", (PyCFunction)timer_control_implObj_set_observer, 1,
	 PyDoc_STR("(ambulant::lib::timer_observer* obs) -> None")},
	{"set_slaved", (PyCFunction)timer_control_implObj_set_slaved, 1,
	 PyDoc_STR("(bool slaved) -> None")},
	{"is_slaved", (PyCFunction)timer_control_implObj_is_slaved, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{NULL, NULL, 0}
};

#define timer_control_implObj_getsetlist NULL


static int timer_control_implObj_compare(timer_control_implObject *self, timer_control_implObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define timer_control_implObj_repr NULL

static long timer_control_implObj_hash(timer_control_implObject *self)
{
	return (long)self->ob_itself;
}
static int timer_control_implObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::timer_control_impl* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, timer_control_implObj_Convert, &itself))
	{
		((timer_control_implObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define timer_control_implObj_tp_alloc PyType_GenericAlloc

static PyObject *timer_control_implObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((timer_control_implObject *)_self)->ob_itself = NULL;
	return _self;
}

#define timer_control_implObj_tp_free PyObject_Del


PyTypeObject timer_control_impl_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.timer_control_impl", /*tp_name*/
	sizeof(timer_control_implObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) timer_control_implObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) timer_control_implObj_compare, /*tp_compare*/
	(reprfunc) timer_control_implObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) timer_control_implObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	timer_control_implObj_methods, /* tp_methods */
	0, /*tp_members*/
	timer_control_implObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	timer_control_implObj_tp_init, /* tp_init */
	timer_control_implObj_tp_alloc, /* tp_alloc */
	timer_control_implObj_tp_new, /* tp_new */
	timer_control_implObj_tp_free, /* tp_free */
};

/* --------------- End object type timer_control_impl --------------- */


/* ------------------- Object type timer_observer ------------------- */

extern PyTypeObject timer_observer_Type;

inline bool timer_observerObj_Check(PyObject *x)
{
	return ((x)->ob_type == &timer_observer_Type);
}

typedef struct timer_observerObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::timer_observer* ob_itself;
} timer_observerObject;

PyObject *timer_observerObj_New(ambulant::lib::timer_observer* itself)
{
	timer_observerObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_timer_observer
	timer_observer *encaps_itself = dynamic_cast<timer_observer *>(itself);
	if (encaps_itself && encaps_itself->py_timer_observer)
	{
		Py_INCREF(encaps_itself->py_timer_observer);
		return encaps_itself->py_timer_observer;
	}
#endif
	it = PyObject_NEW(timer_observerObject, &timer_observer_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int timer_observerObj_Convert(PyObject *v, ambulant::lib::timer_observer* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_timer_observer
	if (!timer_observerObj_Check(v))
	{
		*p_itself = Py_WrapAs_timer_observer(v);
		if (*p_itself) return 1;
	}
#endif
	if (!timer_observerObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "timer_observer required");
		return 0;
	}
	*p_itself = ((timer_observerObject *)v)->ob_itself;
	return 1;
}

static void timer_observerObj_dealloc(timer_observerObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *timer_observerObj_started(timer_observerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->started();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_observerObj_stopped(timer_observerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->stopped();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_observerObj_paused(timer_observerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->paused();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_observerObj_resumed(timer_observerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->resumed();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef timer_observerObj_methods[] = {
	{"started", (PyCFunction)timer_observerObj_started, 1,
	 PyDoc_STR("() -> None")},
	{"stopped", (PyCFunction)timer_observerObj_stopped, 1,
	 PyDoc_STR("() -> None")},
	{"paused", (PyCFunction)timer_observerObj_paused, 1,
	 PyDoc_STR("() -> None")},
	{"resumed", (PyCFunction)timer_observerObj_resumed, 1,
	 PyDoc_STR("() -> None")},
	{NULL, NULL, 0}
};

#define timer_observerObj_getsetlist NULL


static int timer_observerObj_compare(timer_observerObject *self, timer_observerObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define timer_observerObj_repr NULL

static long timer_observerObj_hash(timer_observerObject *self)
{
	return (long)self->ob_itself;
}
static int timer_observerObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::timer_observer* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, timer_observerObj_Convert, &itself))
	{
		((timer_observerObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define timer_observerObj_tp_alloc PyType_GenericAlloc

static PyObject *timer_observerObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((timer_observerObject *)_self)->ob_itself = NULL;
	return _self;
}

#define timer_observerObj_tp_free PyObject_Del


PyTypeObject timer_observer_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.timer_observer", /*tp_name*/
	sizeof(timer_observerObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) timer_observerObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) timer_observerObj_compare, /*tp_compare*/
	(reprfunc) timer_observerObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) timer_observerObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	timer_observerObj_methods, /* tp_methods */
	0, /*tp_members*/
	timer_observerObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	timer_observerObj_tp_init, /* tp_init */
	timer_observerObj_tp_alloc, /* tp_alloc */
	timer_observerObj_tp_new, /* tp_new */
	timer_observerObj_tp_free, /* tp_free */
};

/* ----------------- End object type timer_observer ----------------- */


/* --------------------- Object type timer_sync --------------------- */

extern PyTypeObject timer_sync_Type;

inline bool timer_syncObj_Check(PyObject *x)
{
	return ((x)->ob_type == &timer_sync_Type);
}

typedef struct timer_syncObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::timer_sync* ob_itself;
} timer_syncObject;

PyObject *timer_syncObj_New(ambulant::lib::timer_sync* itself)
{
	timer_syncObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_timer_sync
	timer_sync *encaps_itself = dynamic_cast<timer_sync *>(itself);
	if (encaps_itself && encaps_itself->py_timer_sync)
	{
		Py_INCREF(encaps_itself->py_timer_sync);
		return encaps_itself->py_timer_sync;
	}
#endif
	it = PyObject_NEW(timer_syncObject, &timer_sync_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int timer_syncObj_Convert(PyObject *v, ambulant::lib::timer_sync* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_timer_sync
	if (!timer_syncObj_Check(v))
	{
		*p_itself = Py_WrapAs_timer_sync(v);
		if (*p_itself) return 1;
	}
#endif
	if (!timer_syncObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "timer_sync required");
		return 0;
	}
	*p_itself = ((timer_syncObject *)v)->ob_itself;
	return 1;
}

static void timer_syncObj_dealloc(timer_syncObject *self)
{
	timer_observer_Type.tp_dealloc((PyObject *)self);
}

static PyObject *timer_syncObj_initialize(timer_syncObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer_control* timer;
	if (!PyArg_ParseTuple(_args, "O&",
	                      timer_controlObj_Convert, &timer))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->initialize(timer);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_syncObj_started(timer_syncObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->started();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_syncObj_stopped(timer_syncObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->stopped();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_syncObj_paused(timer_syncObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->paused();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_syncObj_resumed(timer_syncObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->resumed();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *timer_syncObj_clicked(timer_syncObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	ambulant::lib::timer::time_type t;
	if (!PyArg_ParseTuple(_args, "O&l",
	                      nodeObj_Convert, &n,
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->clicked(n,
	                          t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

#ifdef WITH_REMOTE_SYNC

static PyObject *timer_syncObj_uses_external_sync(timer_syncObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->uses_external_sync();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}
#endif

static PyMethodDef timer_syncObj_methods[] = {
	{"initialize", (PyCFunction)timer_syncObj_initialize, 1,
	 PyDoc_STR("(ambulant::lib::timer_control* timer) -> None")},
	{"started", (PyCFunction)timer_syncObj_started, 1,
	 PyDoc_STR("() -> None")},
	{"stopped", (PyCFunction)timer_syncObj_stopped, 1,
	 PyDoc_STR("() -> None")},
	{"paused", (PyCFunction)timer_syncObj_paused, 1,
	 PyDoc_STR("() -> None")},
	{"resumed", (PyCFunction)timer_syncObj_resumed, 1,
	 PyDoc_STR("() -> None")},
	{"clicked", (PyCFunction)timer_syncObj_clicked, 1,
	 PyDoc_STR("(ambulant::lib::node* n, ambulant::lib::timer::time_type t) -> None")},

#ifdef WITH_REMOTE_SYNC
	{"uses_external_sync", (PyCFunction)timer_syncObj_uses_external_sync, 1,
	 PyDoc_STR("() -> (bool _rv)")},
#endif
	{NULL, NULL, 0}
};

#define timer_syncObj_getsetlist NULL


static int timer_syncObj_compare(timer_syncObject *self, timer_syncObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define timer_syncObj_repr NULL

static long timer_syncObj_hash(timer_syncObject *self)
{
	return (long)self->ob_itself;
}
static int timer_syncObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::timer_sync* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, timer_syncObj_Convert, &itself))
	{
		((timer_syncObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define timer_syncObj_tp_alloc PyType_GenericAlloc

static PyObject *timer_syncObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((timer_syncObject *)_self)->ob_itself = NULL;
	return _self;
}

#define timer_syncObj_tp_free PyObject_Del


PyTypeObject timer_sync_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.timer_sync", /*tp_name*/
	sizeof(timer_syncObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) timer_syncObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) timer_syncObj_compare, /*tp_compare*/
	(reprfunc) timer_syncObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) timer_syncObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	timer_syncObj_methods, /* tp_methods */
	0, /*tp_members*/
	timer_syncObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	timer_syncObj_tp_init, /* tp_init */
	timer_syncObj_tp_alloc, /* tp_alloc */
	timer_syncObj_tp_new, /* tp_new */
	timer_syncObj_tp_free, /* tp_free */
};

/* ------------------- End object type timer_sync ------------------- */


/* ----------------- Object type timer_sync_factory ----------------- */

extern PyTypeObject timer_sync_factory_Type;

inline bool timer_sync_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &timer_sync_factory_Type);
}

typedef struct timer_sync_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::timer_sync_factory* ob_itself;
} timer_sync_factoryObject;

PyObject *timer_sync_factoryObj_New(ambulant::lib::timer_sync_factory* itself)
{
	timer_sync_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_timer_sync_factory
	timer_sync_factory *encaps_itself = dynamic_cast<timer_sync_factory *>(itself);
	if (encaps_itself && encaps_itself->py_timer_sync_factory)
	{
		Py_INCREF(encaps_itself->py_timer_sync_factory);
		return encaps_itself->py_timer_sync_factory;
	}
#endif
	it = PyObject_NEW(timer_sync_factoryObject, &timer_sync_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int timer_sync_factoryObj_Convert(PyObject *v, ambulant::lib::timer_sync_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_timer_sync_factory
	if (!timer_sync_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_timer_sync_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!timer_sync_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "timer_sync_factory required");
		return 0;
	}
	*p_itself = ((timer_sync_factoryObject *)v)->ob_itself;
	return 1;
}

static void timer_sync_factoryObj_dealloc(timer_sync_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *timer_sync_factoryObj_new_timer_sync(timer_sync_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::document* doc;
	if (!PyArg_ParseTuple(_args, "O&",
	                      documentObj_Convert, &doc))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer_sync* _rv = _self->ob_itself->new_timer_sync(doc);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     timer_syncObj_New, _rv);
	return _res;
}

static PyMethodDef timer_sync_factoryObj_methods[] = {
	{"new_timer_sync", (PyCFunction)timer_sync_factoryObj_new_timer_sync, 1,
	 PyDoc_STR("(ambulant::lib::document* doc) -> (ambulant::lib::timer_sync* _rv)")},
	{NULL, NULL, 0}
};

#define timer_sync_factoryObj_getsetlist NULL


static int timer_sync_factoryObj_compare(timer_sync_factoryObject *self, timer_sync_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define timer_sync_factoryObj_repr NULL

static long timer_sync_factoryObj_hash(timer_sync_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int timer_sync_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::timer_sync_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, timer_sync_factoryObj_Convert, &itself))
	{
		((timer_sync_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define timer_sync_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *timer_sync_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((timer_sync_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define timer_sync_factoryObj_tp_free PyObject_Del


PyTypeObject timer_sync_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.timer_sync_factory", /*tp_name*/
	sizeof(timer_sync_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) timer_sync_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) timer_sync_factoryObj_compare, /*tp_compare*/
	(reprfunc) timer_sync_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) timer_sync_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	timer_sync_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	timer_sync_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	timer_sync_factoryObj_tp_init, /* tp_init */
	timer_sync_factoryObj_tp_alloc, /* tp_alloc */
	timer_sync_factoryObj_tp_new, /* tp_new */
	timer_sync_factoryObj_tp_free, /* tp_free */
};

/* --------------- End object type timer_sync_factory --------------- */


/* ------------------ Object type transition_info ------------------- */

extern PyTypeObject transition_info_Type;

inline bool transition_infoObj_Check(PyObject *x)
{
	return ((x)->ob_type == &transition_info_Type);
}

typedef struct transition_infoObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::lib::transition_info* ob_itself;
} transition_infoObject;

PyObject *transition_infoObj_New(ambulant::lib::transition_info* itself)
{
	transition_infoObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_transition_info
	transition_info *encaps_itself = dynamic_cast<transition_info *>(itself);
	if (encaps_itself && encaps_itself->py_transition_info)
	{
		Py_INCREF(encaps_itself->py_transition_info);
		return encaps_itself->py_transition_info;
	}
#endif
	it = PyObject_NEW(transition_infoObject, &transition_info_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int transition_infoObj_Convert(PyObject *v, ambulant::lib::transition_info* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_transition_info
	if (!transition_infoObj_Check(v))
	{
		*p_itself = Py_WrapAs_transition_info(v);
		if (*p_itself) return 1;
	}
#endif
	if (!transition_infoObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "transition_info required");
		return 0;
	}
	*p_itself = ((transition_infoObject *)v)->ob_itself;
	return 1;
}

static void transition_infoObj_dealloc(transition_infoObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyMethodDef transition_infoObj_methods[] = {
	{NULL, NULL, 0}
};

#define transition_infoObj_getsetlist NULL


static int transition_infoObj_compare(transition_infoObject *self, transition_infoObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define transition_infoObj_repr NULL

static long transition_infoObj_hash(transition_infoObject *self)
{
	return (long)self->ob_itself;
}
static int transition_infoObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::lib::transition_info* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	{
		if (PyArg_ParseTuple(_args, ""))
		{
			((transition_infoObject *)_self)->ob_itself = new ambulant::lib::transition_info();
			return 0;
		}
	}

	{
		ambulant::lib::transition_info* info;
		if (PyArg_ParseTuple(_args, "O&",
		                     transition_infoObj_Convert, &info))
		{
			((transition_infoObject *)_self)->ob_itself = new ambulant::lib::transition_info(info);
			return 0;
		}
	}

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, transition_infoObj_Convert, &itself))
	{
		((transition_infoObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define transition_infoObj_tp_alloc PyType_GenericAlloc

static PyObject *transition_infoObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((transition_infoObject *)_self)->ob_itself = NULL;
	return _self;
}

#define transition_infoObj_tp_free PyObject_Del


PyTypeObject transition_info_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.transition_info", /*tp_name*/
	sizeof(transition_infoObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) transition_infoObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) transition_infoObj_compare, /*tp_compare*/
	(reprfunc) transition_infoObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) transition_infoObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	transition_infoObj_methods, /* tp_methods */
	0, /*tp_members*/
	transition_infoObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	transition_infoObj_tp_init, /* tp_init */
	transition_infoObj_tp_alloc, /* tp_alloc */
	transition_infoObj_tp_new, /* tp_new */
	transition_infoObj_tp_free, /* tp_free */
};

/* ---------------- End object type transition_info ----------------- */


/* ---------------------- Object type embedder ---------------------- */

extern PyTypeObject embedder_Type;

inline bool embedderObj_Check(PyObject *x)
{
	return ((x)->ob_type == &embedder_Type);
}

typedef struct embedderObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::embedder* ob_itself;
} embedderObject;

PyObject *embedderObj_New(ambulant::common::embedder* itself)
{
	embedderObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_embedder
	embedder *encaps_itself = dynamic_cast<embedder *>(itself);
	if (encaps_itself && encaps_itself->py_embedder)
	{
		Py_INCREF(encaps_itself->py_embedder);
		return encaps_itself->py_embedder;
	}
#endif
	it = PyObject_NEW(embedderObject, &embedder_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int embedderObj_Convert(PyObject *v, ambulant::common::embedder* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_embedder
	if (!embedderObj_Check(v))
	{
		*p_itself = Py_WrapAs_embedder(v);
		if (*p_itself) return 1;
	}
#endif
	if (!embedderObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "embedder required");
		return 0;
	}
	*p_itself = ((embedderObject *)v)->ob_itself;
	return 1;
}

static void embedderObj_dealloc(embedderObject *self)
{
	system_embedder_Type.tp_dealloc((PyObject *)self);
}

static PyObject *embedderObj_close(embedderObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::player* p;
	if (!PyArg_ParseTuple(_args, "O&",
	                      playerObj_Convert, &p))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->close(p);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *embedderObj_open(embedderObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url newdoc;
	bool start;
	ambulant::common::player* old;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ambulant_url_Convert, &newdoc,
	                      bool_Convert, &start,
	                      playerObj_Convert, &old))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->open(newdoc,
	                       start,
	                       old);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *embedderObj_done(embedderObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::player* p;
	if (!PyArg_ParseTuple(_args, "O&",
	                      playerObj_Convert, &p))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->done(p);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *embedderObj_starting(embedderObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::player* p;
	if (!PyArg_ParseTuple(_args, "O&",
	                      playerObj_Convert, &p))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->starting(p);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *embedderObj_aux_open(embedderObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url href;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_url_Convert, &href))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->aux_open(href);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *embedderObj_terminate(embedderObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->terminate();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef embedderObj_methods[] = {
	{"close", (PyCFunction)embedderObj_close, 1,
	 PyDoc_STR("(ambulant::common::player* p) -> None")},
	{"open", (PyCFunction)embedderObj_open, 1,
	 PyDoc_STR("(ambulant::net::url newdoc, bool start, ambulant::common::player* old) -> None")},
	{"done", (PyCFunction)embedderObj_done, 1,
	 PyDoc_STR("(ambulant::common::player* p) -> None")},
	{"starting", (PyCFunction)embedderObj_starting, 1,
	 PyDoc_STR("(ambulant::common::player* p) -> None")},
	{"aux_open", (PyCFunction)embedderObj_aux_open, 1,
	 PyDoc_STR("(ambulant::net::url href) -> (bool _rv)")},
	{"terminate", (PyCFunction)embedderObj_terminate, 1,
	 PyDoc_STR("() -> None")},
	{NULL, NULL, 0}
};

#define embedderObj_getsetlist NULL


static int embedderObj_compare(embedderObject *self, embedderObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define embedderObj_repr NULL

static long embedderObj_hash(embedderObject *self)
{
	return (long)self->ob_itself;
}
static int embedderObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::embedder* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, embedderObj_Convert, &itself))
	{
		((embedderObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define embedderObj_tp_alloc PyType_GenericAlloc

static PyObject *embedderObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((embedderObject *)_self)->ob_itself = NULL;
	return _self;
}

#define embedderObj_tp_free PyObject_Del


PyTypeObject embedder_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.embedder", /*tp_name*/
	sizeof(embedderObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) embedderObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) embedderObj_compare, /*tp_compare*/
	(reprfunc) embedderObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) embedderObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	embedderObj_methods, /* tp_methods */
	0, /*tp_members*/
	embedderObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	embedderObj_tp_init, /* tp_init */
	embedderObj_tp_alloc, /* tp_alloc */
	embedderObj_tp_new, /* tp_new */
	embedderObj_tp_free, /* tp_free */
};

/* -------------------- End object type embedder -------------------- */


/* --------------------- Object type factories ---------------------- */

extern PyTypeObject factories_Type;

inline bool factoriesObj_Check(PyObject *x)
{
	return ((x)->ob_type == &factories_Type);
}

typedef struct factoriesObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::factories* ob_itself;
} factoriesObject;

PyObject *factoriesObj_New(ambulant::common::factories* itself)
{
	factoriesObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_factories
	factories *encaps_itself = dynamic_cast<factories *>(itself);
	if (encaps_itself && encaps_itself->py_factories)
	{
		Py_INCREF(encaps_itself->py_factories);
		return encaps_itself->py_factories;
	}
#endif
	it = PyObject_NEW(factoriesObject, &factories_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int factoriesObj_Convert(PyObject *v, ambulant::common::factories* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_factories
	if (!factoriesObj_Check(v))
	{
		*p_itself = Py_WrapAs_factories(v);
		if (*p_itself) return 1;
	}
#endif
	if (!factoriesObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "factories required");
		return 0;
	}
	*p_itself = ((factoriesObject *)v)->ob_itself;
	return 1;
}

static void factoriesObj_dealloc(factoriesObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *factoriesObj_init_factories(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_factories();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_init_playable_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_playable_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_init_window_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_window_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_init_datasource_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_datasource_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_init_parser_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_parser_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_init_node_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_node_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_init_state_component_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_state_component_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

#ifdef WITH_REMOTE_SYNC

static PyObject *factoriesObj_init_timer_sync_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_timer_sync_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}
#endif

static PyObject *factoriesObj_init_recorder_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_recorder_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_get_playable_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::global_playable_factory* _rv = _self->ob_itself->get_playable_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     global_playable_factoryObj_New, _rv);
	return _res;
}

static PyObject *factoriesObj_get_window_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::window_factory* _rv = _self->ob_itself->get_window_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     window_factoryObj_New, _rv);
	return _res;
}

static PyObject *factoriesObj_get_datasource_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::datasource_factory* _rv = _self->ob_itself->get_datasource_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     datasource_factoryObj_New, _rv);
	return _res;
}

static PyObject *factoriesObj_get_parser_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::global_parser_factory* _rv = _self->ob_itself->get_parser_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     global_parser_factoryObj_New, _rv);
	return _res;
}

static PyObject *factoriesObj_get_node_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::node_factory* _rv = _self->ob_itself->get_node_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     node_factoryObj_New, _rv);
	return _res;
}

static PyObject *factoriesObj_get_state_component_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::global_state_component_factory* _rv = _self->ob_itself->get_state_component_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     global_state_component_factoryObj_New, _rv);
	return _res;
}

static PyObject *factoriesObj_get_recorder_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::recorder_factory* _rv = _self->ob_itself->get_recorder_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     recorder_factoryObj_New, _rv);
	return _res;
}

static PyObject *factoriesObj_set_playable_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::global_playable_factory* pf;
	if (!PyArg_ParseTuple(_args, "O&",
	                      global_playable_factoryObj_Convert, &pf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_playable_factory(pf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_set_window_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::window_factory* wf;
	if (!PyArg_ParseTuple(_args, "O&",
	                      window_factoryObj_Convert, &wf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_window_factory(wf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_set_datasource_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::datasource_factory* df;
	if (!PyArg_ParseTuple(_args, "O&",
	                      datasource_factoryObj_Convert, &df))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_datasource_factory(df);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_set_parser_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::global_parser_factory* pf;
	if (!PyArg_ParseTuple(_args, "O&",
	                      global_parser_factoryObj_Convert, &pf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_parser_factory(pf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_set_node_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node_factory* nf;
	if (!PyArg_ParseTuple(_args, "O&",
	                      node_factoryObj_Convert, &nf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_node_factory(nf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *factoriesObj_set_state_component_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::global_state_component_factory* sf;
	if (!PyArg_ParseTuple(_args, "O&",
	                      global_state_component_factoryObj_Convert, &sf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_state_component_factory(sf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

#ifdef WITH_REMOTE_SYNC

static PyObject *factoriesObj_get_timer_sync_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer_sync_factory* _rv = _self->ob_itself->get_timer_sync_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     timer_sync_factoryObj_New, _rv);
	return _res;
}
#endif

#ifdef WITH_REMOTE_SYNC

static PyObject *factoriesObj_set_timer_sync_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer_sync_factory* tsf;
	if (!PyArg_ParseTuple(_args, "O&",
	                      timer_sync_factoryObj_Convert, &tsf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_timer_sync_factory(tsf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}
#endif

static PyObject *factoriesObj_set_recorder_factory(factoriesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::recorder_factory* rf;
	if (!PyArg_ParseTuple(_args, "O&",
	                      recorder_factoryObj_Convert, &rf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_recorder_factory(rf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef factoriesObj_methods[] = {
	{"init_factories", (PyCFunction)factoriesObj_init_factories, 1,
	 PyDoc_STR("() -> None")},
	{"init_playable_factory", (PyCFunction)factoriesObj_init_playable_factory, 1,
	 PyDoc_STR("() -> None")},
	{"init_window_factory", (PyCFunction)factoriesObj_init_window_factory, 1,
	 PyDoc_STR("() -> None")},
	{"init_datasource_factory", (PyCFunction)factoriesObj_init_datasource_factory, 1,
	 PyDoc_STR("() -> None")},
	{"init_parser_factory", (PyCFunction)factoriesObj_init_parser_factory, 1,
	 PyDoc_STR("() -> None")},
	{"init_node_factory", (PyCFunction)factoriesObj_init_node_factory, 1,
	 PyDoc_STR("() -> None")},
	{"init_state_component_factory", (PyCFunction)factoriesObj_init_state_component_factory, 1,
	 PyDoc_STR("() -> None")},

#ifdef WITH_REMOTE_SYNC
	{"init_timer_sync_factory", (PyCFunction)factoriesObj_init_timer_sync_factory, 1,
	 PyDoc_STR("() -> None")},
#endif
	{"init_recorder_factory", (PyCFunction)factoriesObj_init_recorder_factory, 1,
	 PyDoc_STR("() -> None")},
	{"get_playable_factory", (PyCFunction)factoriesObj_get_playable_factory, 1,
	 PyDoc_STR("() -> (ambulant::common::global_playable_factory* _rv)")},
	{"get_window_factory", (PyCFunction)factoriesObj_get_window_factory, 1,
	 PyDoc_STR("() -> (ambulant::common::window_factory* _rv)")},
	{"get_datasource_factory", (PyCFunction)factoriesObj_get_datasource_factory, 1,
	 PyDoc_STR("() -> (ambulant::net::datasource_factory* _rv)")},
	{"get_parser_factory", (PyCFunction)factoriesObj_get_parser_factory, 1,
	 PyDoc_STR("() -> (ambulant::lib::global_parser_factory* _rv)")},
	{"get_node_factory", (PyCFunction)factoriesObj_get_node_factory, 1,
	 PyDoc_STR("() -> (ambulant::lib::node_factory* _rv)")},
	{"get_state_component_factory", (PyCFunction)factoriesObj_get_state_component_factory, 1,
	 PyDoc_STR("() -> (ambulant::common::global_state_component_factory* _rv)")},
	{"get_recorder_factory", (PyCFunction)factoriesObj_get_recorder_factory, 1,
	 PyDoc_STR("() -> (ambulant::common::recorder_factory* _rv)")},
	{"set_playable_factory", (PyCFunction)factoriesObj_set_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::global_playable_factory* pf) -> None")},
	{"set_window_factory", (PyCFunction)factoriesObj_set_window_factory, 1,
	 PyDoc_STR("(ambulant::common::window_factory* wf) -> None")},
	{"set_datasource_factory", (PyCFunction)factoriesObj_set_datasource_factory, 1,
	 PyDoc_STR("(ambulant::net::datasource_factory* df) -> None")},
	{"set_parser_factory", (PyCFunction)factoriesObj_set_parser_factory, 1,
	 PyDoc_STR("(ambulant::lib::global_parser_factory* pf) -> None")},
	{"set_node_factory", (PyCFunction)factoriesObj_set_node_factory, 1,
	 PyDoc_STR("(ambulant::lib::node_factory* nf) -> None")},
	{"set_state_component_factory", (PyCFunction)factoriesObj_set_state_component_factory, 1,
	 PyDoc_STR("(ambulant::common::global_state_component_factory* sf) -> None")},

#ifdef WITH_REMOTE_SYNC
	{"get_timer_sync_factory", (PyCFunction)factoriesObj_get_timer_sync_factory, 1,
	 PyDoc_STR("() -> (ambulant::lib::timer_sync_factory* _rv)")},
#endif

#ifdef WITH_REMOTE_SYNC
	{"set_timer_sync_factory", (PyCFunction)factoriesObj_set_timer_sync_factory, 1,
	 PyDoc_STR("(ambulant::lib::timer_sync_factory* tsf) -> None")},
#endif
	{"set_recorder_factory", (PyCFunction)factoriesObj_set_recorder_factory, 1,
	 PyDoc_STR("(ambulant::common::recorder_factory* rf) -> None")},
	{NULL, NULL, 0}
};

#define factoriesObj_getsetlist NULL


static int factoriesObj_compare(factoriesObject *self, factoriesObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define factoriesObj_repr NULL

static long factoriesObj_hash(factoriesObject *self)
{
	return (long)self->ob_itself;
}
static int factoriesObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::factories* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	{
		if (PyArg_ParseTuple(_args, ""))
		{
			((factoriesObject *)_self)->ob_itself = new ambulant::common::factories();
			return 0;
		}
	}

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, factoriesObj_Convert, &itself))
	{
		((factoriesObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define factoriesObj_tp_alloc PyType_GenericAlloc

static PyObject *factoriesObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((factoriesObject *)_self)->ob_itself = NULL;
	return _self;
}

#define factoriesObj_tp_free PyObject_Del


PyTypeObject factories_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.factories", /*tp_name*/
	sizeof(factoriesObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) factoriesObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) factoriesObj_compare, /*tp_compare*/
	(reprfunc) factoriesObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) factoriesObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	factoriesObj_methods, /* tp_methods */
	0, /*tp_members*/
	factoriesObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	factoriesObj_tp_init, /* tp_init */
	factoriesObj_tp_alloc, /* tp_alloc */
	factoriesObj_tp_new, /* tp_new */
	factoriesObj_tp_free, /* tp_free */
};

/* ------------------- End object type factories -------------------- */


/* --------------------- Object type gui_screen --------------------- */

extern PyTypeObject gui_screen_Type;

inline bool gui_screenObj_Check(PyObject *x)
{
	return ((x)->ob_type == &gui_screen_Type);
}

typedef struct gui_screenObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::gui_screen* ob_itself;
} gui_screenObject;

PyObject *gui_screenObj_New(ambulant::common::gui_screen* itself)
{
	gui_screenObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_gui_screen
	gui_screen *encaps_itself = dynamic_cast<gui_screen *>(itself);
	if (encaps_itself && encaps_itself->py_gui_screen)
	{
		Py_INCREF(encaps_itself->py_gui_screen);
		return encaps_itself->py_gui_screen;
	}
#endif
	it = PyObject_NEW(gui_screenObject, &gui_screen_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int gui_screenObj_Convert(PyObject *v, ambulant::common::gui_screen* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_gui_screen
	if (!gui_screenObj_Check(v))
	{
		*p_itself = Py_WrapAs_gui_screen(v);
		if (*p_itself) return 1;
	}
#endif
	if (!gui_screenObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "gui_screen required");
		return 0;
	}
	*p_itself = ((gui_screenObject *)v)->ob_itself;
	return 1;
}

static void gui_screenObj_dealloc(gui_screenObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *gui_screenObj_get_size(gui_screenObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int width;
	int height;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->get_size(&width,
	                           &height);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("ii",
	                     width,
	                     height);
	return _res;
}

#ifndef CPP_TO_PYTHON_BRIDGE

static PyObject *gui_screenObj_get_screenshot(gui_screenObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* type;
	char *out_data__out__;
	size_t out_data__len__;
	if (!PyArg_ParseTuple(_args, "s",
	                      &type))
		return NULL;
	out_data__out__ = NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->get_screenshot(type,
	                                            &out_data__out__, &out_data__len__);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&z#",
	                     bool_New, _rv,
	                     out_data__out__, (int)out_data__len__);
	if( out_data__out__ ) free(out_data__out__);
	return _res;
}
#endif

static PyMethodDef gui_screenObj_methods[] = {
	{"get_size", (PyCFunction)gui_screenObj_get_size, 1,
	 PyDoc_STR("() -> (int width, int height)")},

#ifndef CPP_TO_PYTHON_BRIDGE
	{"get_screenshot", (PyCFunction)gui_screenObj_get_screenshot, 1,
	 PyDoc_STR("(char* type, Buffer out_data) -> (bool _rv, Buffer out_data)")},
#endif
	{NULL, NULL, 0}
};

#define gui_screenObj_getsetlist NULL


static int gui_screenObj_compare(gui_screenObject *self, gui_screenObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define gui_screenObj_repr NULL

static long gui_screenObj_hash(gui_screenObject *self)
{
	return (long)self->ob_itself;
}
static int gui_screenObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::gui_screen* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, gui_screenObj_Convert, &itself))
	{
		((gui_screenObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define gui_screenObj_tp_alloc PyType_GenericAlloc

static PyObject *gui_screenObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((gui_screenObject *)_self)->ob_itself = NULL;
	return _self;
}

#define gui_screenObj_tp_free PyObject_Del


PyTypeObject gui_screen_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.gui_screen", /*tp_name*/
	sizeof(gui_screenObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) gui_screenObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) gui_screenObj_compare, /*tp_compare*/
	(reprfunc) gui_screenObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) gui_screenObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	gui_screenObj_methods, /* tp_methods */
	0, /*tp_members*/
	gui_screenObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	gui_screenObj_tp_init, /* tp_init */
	gui_screenObj_tp_alloc, /* tp_alloc */
	gui_screenObj_tp_new, /* tp_new */
	gui_screenObj_tp_free, /* tp_free */
};

/* ------------------- End object type gui_screen ------------------- */


/* --------------------- Object type gui_player --------------------- */

extern PyTypeObject gui_player_Type;

inline bool gui_playerObj_Check(PyObject *x)
{
	return ((x)->ob_type == &gui_player_Type);
}

typedef struct gui_playerObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::gui_player* ob_itself;
} gui_playerObject;

PyObject *gui_playerObj_New(ambulant::common::gui_player* itself)
{
	gui_playerObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_gui_player
	gui_player *encaps_itself = dynamic_cast<gui_player *>(itself);
	if (encaps_itself && encaps_itself->py_gui_player)
	{
		Py_INCREF(encaps_itself->py_gui_player);
		return encaps_itself->py_gui_player;
	}
#endif
	it = PyObject_NEW(gui_playerObject, &gui_player_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int gui_playerObj_Convert(PyObject *v, ambulant::common::gui_player* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_gui_player
	if (!gui_playerObj_Check(v))
	{
		*p_itself = Py_WrapAs_gui_player(v);
		if (*p_itself) return 1;
	}
#endif
	if (!gui_playerObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "gui_player required");
		return 0;
	}
	*p_itself = ((gui_playerObject *)v)->ob_itself;
	return 1;
}

static void gui_playerObj_dealloc(gui_playerObject *self)
{
	factories_Type.tp_dealloc((PyObject *)self);
}

static PyObject *gui_playerObj_init_playable_factory(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_playable_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_init_window_factory(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_window_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_init_datasource_factory(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_datasource_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_init_parser_factory(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_parser_factory();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_init_plugins(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_plugins();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_play(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->play();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_stop(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->stop();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_pause(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->pause();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_restart(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool reparse;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &reparse))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->restart(reparse);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_goto_node(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &n))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->goto_node(n);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_is_play_enabled(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_play_enabled();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *gui_playerObj_is_stop_enabled(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_stop_enabled();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *gui_playerObj_is_pause_enabled(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_pause_enabled();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *gui_playerObj_is_play_active(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_play_active();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *gui_playerObj_is_stop_active(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_stop_active();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *gui_playerObj_is_pause_active(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_pause_active();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *gui_playerObj_before_mousemove(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int cursor;
	if (!PyArg_ParseTuple(_args, "i",
	                      &cursor))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->before_mousemove(cursor);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_after_mousemove(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	int _rv = _self->ob_itself->after_mousemove();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("i",
	                     _rv);
	return _res;
}

static PyObject *gui_playerObj_on_char(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int c;
	if (!PyArg_ParseTuple(_args, "i",
	                      &c))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->on_char(c);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_on_focus_advance(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->on_focus_advance();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_on_focus_activate(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->on_focus_activate();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_get_document(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::document* _rv = _self->ob_itself->get_document();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     documentObj_New, _rv);
	return _res;
}

static PyObject *gui_playerObj_set_document(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::document* doc;
	if (!PyArg_ParseTuple(_args, "O&",
	                      documentObj_Convert, &doc))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_document(doc);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_get_embedder(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::embedder* _rv = _self->ob_itself->get_embedder();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     embedderObj_New, _rv);
	return _res;
}

static PyObject *gui_playerObj_set_embedder(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::embedder* em;
	if (!PyArg_ParseTuple(_args, "O&",
	                      embedderObj_Convert, &em))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_embedder(em);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_get_player(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::player* _rv = _self->ob_itself->get_player();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playerObj_New, _rv);
	return _res;
}

static PyObject *gui_playerObj_set_player(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::player* pl;
	if (!PyArg_ParseTuple(_args, "O&",
	                      playerObj_Convert, &pl))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_player(pl);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_playerObj_get_url(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::url _rv = _self->ob_itself->get_url();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_url_New(_rv));
	return _res;
}

static PyObject *gui_playerObj_get_gui_screen(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::gui_screen* _rv = _self->ob_itself->get_gui_screen();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     gui_screenObj_New, _rv);
	return _res;
}

#ifdef WITH_REMOTE_SYNC

static PyObject *gui_playerObj_clicked_external(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	ambulant::lib::timer::time_type t;
	if (!PyArg_ParseTuple(_args, "O&l",
	                      nodeObj_Convert, &n,
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->clicked_external(n,
	                                   t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}
#endif

#ifdef WITH_REMOTE_SYNC

static PyObject *gui_playerObj_uses_external_sync(gui_playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->uses_external_sync();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}
#endif

static PyMethodDef gui_playerObj_methods[] = {
	{"init_playable_factory", (PyCFunction)gui_playerObj_init_playable_factory, 1,
	 PyDoc_STR("() -> None")},
	{"init_window_factory", (PyCFunction)gui_playerObj_init_window_factory, 1,
	 PyDoc_STR("() -> None")},
	{"init_datasource_factory", (PyCFunction)gui_playerObj_init_datasource_factory, 1,
	 PyDoc_STR("() -> None")},
	{"init_parser_factory", (PyCFunction)gui_playerObj_init_parser_factory, 1,
	 PyDoc_STR("() -> None")},
	{"init_plugins", (PyCFunction)gui_playerObj_init_plugins, 1,
	 PyDoc_STR("() -> None")},
	{"play", (PyCFunction)gui_playerObj_play, 1,
	 PyDoc_STR("() -> None")},
	{"stop", (PyCFunction)gui_playerObj_stop, 1,
	 PyDoc_STR("() -> None")},
	{"pause", (PyCFunction)gui_playerObj_pause, 1,
	 PyDoc_STR("() -> None")},
	{"restart", (PyCFunction)gui_playerObj_restart, 1,
	 PyDoc_STR("(bool reparse) -> None")},
	{"goto_node", (PyCFunction)gui_playerObj_goto_node, 1,
	 PyDoc_STR("(ambulant::lib::node* n) -> None")},
	{"is_play_enabled", (PyCFunction)gui_playerObj_is_play_enabled, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"is_stop_enabled", (PyCFunction)gui_playerObj_is_stop_enabled, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"is_pause_enabled", (PyCFunction)gui_playerObj_is_pause_enabled, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"is_play_active", (PyCFunction)gui_playerObj_is_play_active, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"is_stop_active", (PyCFunction)gui_playerObj_is_stop_active, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"is_pause_active", (PyCFunction)gui_playerObj_is_pause_active, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"before_mousemove", (PyCFunction)gui_playerObj_before_mousemove, 1,
	 PyDoc_STR("(int cursor) -> None")},
	{"after_mousemove", (PyCFunction)gui_playerObj_after_mousemove, 1,
	 PyDoc_STR("() -> (int _rv)")},
	{"on_char", (PyCFunction)gui_playerObj_on_char, 1,
	 PyDoc_STR("(int c) -> None")},
	{"on_focus_advance", (PyCFunction)gui_playerObj_on_focus_advance, 1,
	 PyDoc_STR("() -> None")},
	{"on_focus_activate", (PyCFunction)gui_playerObj_on_focus_activate, 1,
	 PyDoc_STR("() -> None")},
	{"get_document", (PyCFunction)gui_playerObj_get_document, 1,
	 PyDoc_STR("() -> (ambulant::lib::document* _rv)")},
	{"set_document", (PyCFunction)gui_playerObj_set_document, 1,
	 PyDoc_STR("(ambulant::lib::document* doc) -> None")},
	{"get_embedder", (PyCFunction)gui_playerObj_get_embedder, 1,
	 PyDoc_STR("() -> (ambulant::common::embedder* _rv)")},
	{"set_embedder", (PyCFunction)gui_playerObj_set_embedder, 1,
	 PyDoc_STR("(ambulant::common::embedder* em) -> None")},
	{"get_player", (PyCFunction)gui_playerObj_get_player, 1,
	 PyDoc_STR("() -> (ambulant::common::player* _rv)")},
	{"set_player", (PyCFunction)gui_playerObj_set_player, 1,
	 PyDoc_STR("(ambulant::common::player* pl) -> None")},
	{"get_url", (PyCFunction)gui_playerObj_get_url, 1,
	 PyDoc_STR("() -> (ambulant::net::url _rv)")},
	{"get_gui_screen", (PyCFunction)gui_playerObj_get_gui_screen, 1,
	 PyDoc_STR("() -> (ambulant::common::gui_screen* _rv)")},

#ifdef WITH_REMOTE_SYNC
	{"clicked_external", (PyCFunction)gui_playerObj_clicked_external, 1,
	 PyDoc_STR("(ambulant::lib::node* n, ambulant::lib::timer::time_type t) -> None")},
#endif

#ifdef WITH_REMOTE_SYNC
	{"uses_external_sync", (PyCFunction)gui_playerObj_uses_external_sync, 1,
	 PyDoc_STR("() -> (bool _rv)")},
#endif
	{NULL, NULL, 0}
};

#define gui_playerObj_getsetlist NULL


static int gui_playerObj_compare(gui_playerObject *self, gui_playerObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define gui_playerObj_repr NULL

static long gui_playerObj_hash(gui_playerObject *self)
{
	return (long)self->ob_itself;
}
static int gui_playerObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::gui_player* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	{
		if (PyArg_ParseTuple(_args, ""))
		{
			((gui_playerObject *)_self)->ob_itself = new ambulant::common::gui_player();
			return 0;
		}
	}

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, gui_playerObj_Convert, &itself))
	{
		((gui_playerObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define gui_playerObj_tp_alloc PyType_GenericAlloc

static PyObject *gui_playerObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((gui_playerObject *)_self)->ob_itself = NULL;
	return _self;
}

#define gui_playerObj_tp_free PyObject_Del


PyTypeObject gui_player_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.gui_player", /*tp_name*/
	sizeof(gui_playerObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) gui_playerObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) gui_playerObj_compare, /*tp_compare*/
	(reprfunc) gui_playerObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) gui_playerObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	gui_playerObj_methods, /* tp_methods */
	0, /*tp_members*/
	gui_playerObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	gui_playerObj_tp_init, /* tp_init */
	gui_playerObj_tp_alloc, /* tp_alloc */
	gui_playerObj_tp_new, /* tp_new */
	gui_playerObj_tp_free, /* tp_free */
};

/* ------------------- End object type gui_player ------------------- */


/* --------------------- Object type alignment ---------------------- */

extern PyTypeObject alignment_Type;

inline bool alignmentObj_Check(PyObject *x)
{
	return ((x)->ob_type == &alignment_Type);
}

typedef struct alignmentObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::alignment* ob_itself;
} alignmentObject;

PyObject *alignmentObj_New(ambulant::common::alignment* itself)
{
	alignmentObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_alignment
	alignment *encaps_itself = dynamic_cast<alignment *>(itself);
	if (encaps_itself && encaps_itself->py_alignment)
	{
		Py_INCREF(encaps_itself->py_alignment);
		return encaps_itself->py_alignment;
	}
#endif
	it = PyObject_NEW(alignmentObject, &alignment_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int alignmentObj_Convert(PyObject *v, ambulant::common::alignment* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_alignment
	if (!alignmentObj_Check(v))
	{
		*p_itself = Py_WrapAs_alignment(v);
		if (*p_itself) return 1;
	}
#endif
	if (!alignmentObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "alignment required");
		return 0;
	}
	*p_itself = ((alignmentObject *)v)->ob_itself;
	return 1;
}

static void alignmentObj_dealloc(alignmentObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *alignmentObj_get_image_fixpoint(alignmentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::size image_size;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_size_Convert, &image_size))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::point _rv = _self->ob_itself->get_image_fixpoint(image_size);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_point_New(_rv));
	return _res;
}

static PyObject *alignmentObj_get_surface_fixpoint(alignmentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::size surface_size;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_size_Convert, &surface_size))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::point _rv = _self->ob_itself->get_surface_fixpoint(surface_size);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_point_New(_rv));
	return _res;
}

static PyMethodDef alignmentObj_methods[] = {
	{"get_image_fixpoint", (PyCFunction)alignmentObj_get_image_fixpoint, 1,
	 PyDoc_STR("(ambulant::lib::size image_size) -> (ambulant::lib::point _rv)")},
	{"get_surface_fixpoint", (PyCFunction)alignmentObj_get_surface_fixpoint, 1,
	 PyDoc_STR("(ambulant::lib::size surface_size) -> (ambulant::lib::point _rv)")},
	{NULL, NULL, 0}
};

#define alignmentObj_getsetlist NULL


static int alignmentObj_compare(alignmentObject *self, alignmentObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define alignmentObj_repr NULL

static long alignmentObj_hash(alignmentObject *self)
{
	return (long)self->ob_itself;
}
static int alignmentObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::alignment* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, alignmentObj_Convert, &itself))
	{
		((alignmentObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define alignmentObj_tp_alloc PyType_GenericAlloc

static PyObject *alignmentObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((alignmentObject *)_self)->ob_itself = NULL;
	return _self;
}

#define alignmentObj_tp_free PyObject_Del


PyTypeObject alignment_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.alignment", /*tp_name*/
	sizeof(alignmentObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) alignmentObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) alignmentObj_compare, /*tp_compare*/
	(reprfunc) alignmentObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) alignmentObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	alignmentObj_methods, /* tp_methods */
	0, /*tp_members*/
	alignmentObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	alignmentObj_tp_init, /* tp_init */
	alignmentObj_tp_alloc, /* tp_alloc */
	alignmentObj_tp_new, /* tp_new */
	alignmentObj_tp_free, /* tp_free */
};

/* ------------------- End object type alignment -------------------- */


/* --------------- Object type animation_notification --------------- */

extern PyTypeObject animation_notification_Type;

inline bool animation_notificationObj_Check(PyObject *x)
{
	return ((x)->ob_type == &animation_notification_Type);
}

typedef struct animation_notificationObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::animation_notification* ob_itself;
} animation_notificationObject;

PyObject *animation_notificationObj_New(ambulant::common::animation_notification* itself)
{
	animation_notificationObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_animation_notification
	animation_notification *encaps_itself = dynamic_cast<animation_notification *>(itself);
	if (encaps_itself && encaps_itself->py_animation_notification)
	{
		Py_INCREF(encaps_itself->py_animation_notification);
		return encaps_itself->py_animation_notification;
	}
#endif
	it = PyObject_NEW(animation_notificationObject, &animation_notification_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int animation_notificationObj_Convert(PyObject *v, ambulant::common::animation_notification* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_animation_notification
	if (!animation_notificationObj_Check(v))
	{
		*p_itself = Py_WrapAs_animation_notification(v);
		if (*p_itself) return 1;
	}
#endif
	if (!animation_notificationObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "animation_notification required");
		return 0;
	}
	*p_itself = ((animation_notificationObject *)v)->ob_itself;
	return 1;
}

static void animation_notificationObj_dealloc(animation_notificationObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *animation_notificationObj_animated(animation_notificationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->animated();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef animation_notificationObj_methods[] = {
	{"animated", (PyCFunction)animation_notificationObj_animated, 1,
	 PyDoc_STR("() -> None")},
	{NULL, NULL, 0}
};

#define animation_notificationObj_getsetlist NULL


static int animation_notificationObj_compare(animation_notificationObject *self, animation_notificationObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define animation_notificationObj_repr NULL

static long animation_notificationObj_hash(animation_notificationObject *self)
{
	return (long)self->ob_itself;
}
static int animation_notificationObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::animation_notification* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, animation_notificationObj_Convert, &itself))
	{
		((animation_notificationObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define animation_notificationObj_tp_alloc PyType_GenericAlloc

static PyObject *animation_notificationObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((animation_notificationObject *)_self)->ob_itself = NULL;
	return _self;
}

#define animation_notificationObj_tp_free PyObject_Del


PyTypeObject animation_notification_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.animation_notification", /*tp_name*/
	sizeof(animation_notificationObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) animation_notificationObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) animation_notificationObj_compare, /*tp_compare*/
	(reprfunc) animation_notificationObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) animation_notificationObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	animation_notificationObj_methods, /* tp_methods */
	0, /*tp_members*/
	animation_notificationObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	animation_notificationObj_tp_init, /* tp_init */
	animation_notificationObj_tp_alloc, /* tp_alloc */
	animation_notificationObj_tp_new, /* tp_new */
	animation_notificationObj_tp_free, /* tp_free */
};

/* ------------- End object type animation_notification ------------- */


/* --------------------- Object type gui_window --------------------- */

extern PyTypeObject gui_window_Type;

inline bool gui_windowObj_Check(PyObject *x)
{
	return ((x)->ob_type == &gui_window_Type);
}

typedef struct gui_windowObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::gui_window* ob_itself;
} gui_windowObject;

PyObject *gui_windowObj_New(ambulant::common::gui_window* itself)
{
	gui_windowObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_gui_window
	gui_window *encaps_itself = dynamic_cast<gui_window *>(itself);
	if (encaps_itself && encaps_itself->py_gui_window)
	{
		Py_INCREF(encaps_itself->py_gui_window);
		return encaps_itself->py_gui_window;
	}
#endif
	it = PyObject_NEW(gui_windowObject, &gui_window_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int gui_windowObj_Convert(PyObject *v, ambulant::common::gui_window* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_gui_window
	if (!gui_windowObj_Check(v))
	{
		*p_itself = Py_WrapAs_gui_window(v);
		if (*p_itself) return 1;
	}
#endif
	if (!gui_windowObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "gui_window required");
		return 0;
	}
	*p_itself = ((gui_windowObject *)v)->ob_itself;
	return 1;
}

static void gui_windowObj_dealloc(gui_windowObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *gui_windowObj_need_redraw(gui_windowObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_rect_Convert, &r))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->need_redraw(r);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_windowObj_redraw_now(gui_windowObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->redraw_now();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_windowObj_need_events(gui_windowObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool want;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &want))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->need_events(want);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef gui_windowObj_methods[] = {
	{"need_redraw", (PyCFunction)gui_windowObj_need_redraw, 1,
	 PyDoc_STR("(ambulant::lib::rect r) -> None")},
	{"redraw_now", (PyCFunction)gui_windowObj_redraw_now, 1,
	 PyDoc_STR("() -> None")},
	{"need_events", (PyCFunction)gui_windowObj_need_events, 1,
	 PyDoc_STR("(bool want) -> None")},
	{NULL, NULL, 0}
};

#define gui_windowObj_getsetlist NULL


static int gui_windowObj_compare(gui_windowObject *self, gui_windowObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define gui_windowObj_repr NULL

static long gui_windowObj_hash(gui_windowObject *self)
{
	return (long)self->ob_itself;
}
static int gui_windowObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::gui_window* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, gui_windowObj_Convert, &itself))
	{
		((gui_windowObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define gui_windowObj_tp_alloc PyType_GenericAlloc

static PyObject *gui_windowObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((gui_windowObject *)_self)->ob_itself = NULL;
	return _self;
}

#define gui_windowObj_tp_free PyObject_Del


PyTypeObject gui_window_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.gui_window", /*tp_name*/
	sizeof(gui_windowObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) gui_windowObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) gui_windowObj_compare, /*tp_compare*/
	(reprfunc) gui_windowObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) gui_windowObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	gui_windowObj_methods, /* tp_methods */
	0, /*tp_members*/
	gui_windowObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	gui_windowObj_tp_init, /* tp_init */
	gui_windowObj_tp_alloc, /* tp_alloc */
	gui_windowObj_tp_new, /* tp_new */
	gui_windowObj_tp_free, /* tp_free */
};

/* ------------------- End object type gui_window ------------------- */


/* --------------------- Object type gui_events --------------------- */

extern PyTypeObject gui_events_Type;

inline bool gui_eventsObj_Check(PyObject *x)
{
	return ((x)->ob_type == &gui_events_Type);
}

typedef struct gui_eventsObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::gui_events* ob_itself;
} gui_eventsObject;

PyObject *gui_eventsObj_New(ambulant::common::gui_events* itself)
{
	gui_eventsObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_gui_events
	gui_events *encaps_itself = dynamic_cast<gui_events *>(itself);
	if (encaps_itself && encaps_itself->py_gui_events)
	{
		Py_INCREF(encaps_itself->py_gui_events);
		return encaps_itself->py_gui_events;
	}
#endif
	it = PyObject_NEW(gui_eventsObject, &gui_events_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int gui_eventsObj_Convert(PyObject *v, ambulant::common::gui_events* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_gui_events
	if (!gui_eventsObj_Check(v))
	{
		*p_itself = Py_WrapAs_gui_events(v);
		if (*p_itself) return 1;
	}
#endif
	if (!gui_eventsObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "gui_events required");
		return 0;
	}
	*p_itself = ((gui_eventsObject *)v)->ob_itself;
	return 1;
}

static void gui_eventsObj_dealloc(gui_eventsObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *gui_eventsObj_redraw(gui_eventsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::rect dirty;
	ambulant::common::gui_window* window;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ambulant_rect_Convert, &dirty,
	                      gui_windowObj_Convert, &window))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->redraw(dirty,
	                         window);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *gui_eventsObj_user_event(gui_eventsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::point where;
	int what;
	if (!PyArg_ParseTuple(_args, "O&i",
	                      ambulant_point_Convert, &where,
	                      &what))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->user_event(where,
	                                        what);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *gui_eventsObj_transition_freeze_end(gui_eventsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::rect area;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_rect_Convert, &area))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->transition_freeze_end(area);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef gui_eventsObj_methods[] = {
	{"redraw", (PyCFunction)gui_eventsObj_redraw, 1,
	 PyDoc_STR("(ambulant::lib::rect dirty, ambulant::common::gui_window* window) -> None")},
	{"user_event", (PyCFunction)gui_eventsObj_user_event, 1,
	 PyDoc_STR("(ambulant::lib::point where, int what) -> (bool _rv)")},
	{"transition_freeze_end", (PyCFunction)gui_eventsObj_transition_freeze_end, 1,
	 PyDoc_STR("(ambulant::lib::rect area) -> None")},
	{NULL, NULL, 0}
};

#define gui_eventsObj_getsetlist NULL


static int gui_eventsObj_compare(gui_eventsObject *self, gui_eventsObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define gui_eventsObj_repr NULL

static long gui_eventsObj_hash(gui_eventsObject *self)
{
	return (long)self->ob_itself;
}
static int gui_eventsObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::gui_events* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, gui_eventsObj_Convert, &itself))
	{
		((gui_eventsObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define gui_eventsObj_tp_alloc PyType_GenericAlloc

static PyObject *gui_eventsObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((gui_eventsObject *)_self)->ob_itself = NULL;
	return _self;
}

#define gui_eventsObj_tp_free PyObject_Del


PyTypeObject gui_events_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.gui_events", /*tp_name*/
	sizeof(gui_eventsObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) gui_eventsObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) gui_eventsObj_compare, /*tp_compare*/
	(reprfunc) gui_eventsObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) gui_eventsObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	gui_eventsObj_methods, /* tp_methods */
	0, /*tp_members*/
	gui_eventsObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	gui_eventsObj_tp_init, /* tp_init */
	gui_eventsObj_tp_alloc, /* tp_alloc */
	gui_eventsObj_tp_new, /* tp_new */
	gui_eventsObj_tp_free, /* tp_free */
};

/* ------------------- End object type gui_events ------------------- */


/* ---------------------- Object type renderer ---------------------- */

extern PyTypeObject renderer_Type;

inline bool rendererObj_Check(PyObject *x)
{
	return ((x)->ob_type == &renderer_Type);
}

typedef struct rendererObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::renderer* ob_itself;
} rendererObject;

PyObject *rendererObj_New(ambulant::common::renderer* itself)
{
	rendererObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_renderer
	renderer *encaps_itself = dynamic_cast<renderer *>(itself);
	if (encaps_itself && encaps_itself->py_renderer)
	{
		Py_INCREF(encaps_itself->py_renderer);
		return encaps_itself->py_renderer;
	}
#endif
	it = PyObject_NEW(rendererObject, &renderer_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int rendererObj_Convert(PyObject *v, ambulant::common::renderer* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_renderer
	if (!rendererObj_Check(v))
	{
		*p_itself = Py_WrapAs_renderer(v);
		if (*p_itself) return 1;
	}
#endif
	if (!rendererObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "renderer required");
		return 0;
	}
	*p_itself = ((rendererObject *)v)->ob_itself;
	return 1;
}

static void rendererObj_dealloc(rendererObject *self)
{
	gui_events_Type.tp_dealloc((PyObject *)self);
}

static PyObject *rendererObj_set_surface(rendererObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::surface* destination;
	if (!PyArg_ParseTuple(_args, "O&",
	                      surfaceObj_Convert, &destination))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_surface(destination);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *rendererObj_set_alignment(rendererObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::alignment* align;
	if (!PyArg_ParseTuple(_args, "O&",
	                      alignmentObj_Convert, &align))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_alignment(align);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *rendererObj_set_intransition(rendererObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::transition_info* info;
	if (!PyArg_ParseTuple(_args, "O&",
	                      transition_infoObj_Convert, &info))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_intransition(info);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *rendererObj_start_outtransition(rendererObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::transition_info* info;
	if (!PyArg_ParseTuple(_args, "O&",
	                      transition_infoObj_Convert, &info))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->start_outtransition(info);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *rendererObj_get_surface(rendererObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::surface* _rv = _self->ob_itself->get_surface();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     surfaceObj_New, _rv);
	return _res;
}

static PyMethodDef rendererObj_methods[] = {
	{"set_surface", (PyCFunction)rendererObj_set_surface, 1,
	 PyDoc_STR("(ambulant::common::surface* destination) -> None")},
	{"set_alignment", (PyCFunction)rendererObj_set_alignment, 1,
	 PyDoc_STR("(ambulant::common::alignment* align) -> None")},
	{"set_intransition", (PyCFunction)rendererObj_set_intransition, 1,
	 PyDoc_STR("(ambulant::lib::transition_info* info) -> None")},
	{"start_outtransition", (PyCFunction)rendererObj_start_outtransition, 1,
	 PyDoc_STR("(ambulant::lib::transition_info* info) -> None")},
	{"get_surface", (PyCFunction)rendererObj_get_surface, 1,
	 PyDoc_STR("() -> (ambulant::common::surface* _rv)")},
	{NULL, NULL, 0}
};

#define rendererObj_getsetlist NULL


static int rendererObj_compare(rendererObject *self, rendererObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define rendererObj_repr NULL

static long rendererObj_hash(rendererObject *self)
{
	return (long)self->ob_itself;
}
static int rendererObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::renderer* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, rendererObj_Convert, &itself))
	{
		((rendererObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define rendererObj_tp_alloc PyType_GenericAlloc

static PyObject *rendererObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((rendererObject *)_self)->ob_itself = NULL;
	return _self;
}

#define rendererObj_tp_free PyObject_Del


PyTypeObject renderer_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.renderer", /*tp_name*/
	sizeof(rendererObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) rendererObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) rendererObj_compare, /*tp_compare*/
	(reprfunc) rendererObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) rendererObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	rendererObj_methods, /* tp_methods */
	0, /*tp_members*/
	rendererObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	rendererObj_tp_init, /* tp_init */
	rendererObj_tp_alloc, /* tp_alloc */
	rendererObj_tp_new, /* tp_new */
	rendererObj_tp_free, /* tp_free */
};

/* -------------------- End object type renderer -------------------- */


/* --------------------- Object type bgrenderer --------------------- */

extern PyTypeObject bgrenderer_Type;

inline bool bgrendererObj_Check(PyObject *x)
{
	return ((x)->ob_type == &bgrenderer_Type);
}

typedef struct bgrendererObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::bgrenderer* ob_itself;
} bgrendererObject;

PyObject *bgrendererObj_New(ambulant::common::bgrenderer* itself)
{
	bgrendererObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_bgrenderer
	bgrenderer *encaps_itself = dynamic_cast<bgrenderer *>(itself);
	if (encaps_itself && encaps_itself->py_bgrenderer)
	{
		Py_INCREF(encaps_itself->py_bgrenderer);
		return encaps_itself->py_bgrenderer;
	}
#endif
	it = PyObject_NEW(bgrendererObject, &bgrenderer_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int bgrendererObj_Convert(PyObject *v, ambulant::common::bgrenderer* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_bgrenderer
	if (!bgrendererObj_Check(v))
	{
		*p_itself = Py_WrapAs_bgrenderer(v);
		if (*p_itself) return 1;
	}
#endif
	if (!bgrendererObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "bgrenderer required");
		return 0;
	}
	*p_itself = ((bgrendererObject *)v)->ob_itself;
	return 1;
}

static void bgrendererObj_dealloc(bgrendererObject *self)
{
	gui_events_Type.tp_dealloc((PyObject *)self);
}

static PyObject *bgrendererObj_set_surface(bgrendererObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::surface* destination;
	if (!PyArg_ParseTuple(_args, "O&",
	                      surfaceObj_Convert, &destination))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_surface(destination);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *bgrendererObj_keep_as_background(bgrendererObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->keep_as_background();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *bgrendererObj_highlight(bgrendererObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::gui_window* window;
	if (!PyArg_ParseTuple(_args, "O&",
	                      gui_windowObj_Convert, &window))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->highlight(window);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef bgrendererObj_methods[] = {
	{"set_surface", (PyCFunction)bgrendererObj_set_surface, 1,
	 PyDoc_STR("(ambulant::common::surface* destination) -> None")},
	{"keep_as_background", (PyCFunction)bgrendererObj_keep_as_background, 1,
	 PyDoc_STR("() -> None")},
	{"highlight", (PyCFunction)bgrendererObj_highlight, 1,
	 PyDoc_STR("(ambulant::common::gui_window* window) -> None")},
	{NULL, NULL, 0}
};

#define bgrendererObj_getsetlist NULL


static int bgrendererObj_compare(bgrendererObject *self, bgrendererObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define bgrendererObj_repr NULL

static long bgrendererObj_hash(bgrendererObject *self)
{
	return (long)self->ob_itself;
}
static int bgrendererObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::bgrenderer* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, bgrendererObj_Convert, &itself))
	{
		((bgrendererObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define bgrendererObj_tp_alloc PyType_GenericAlloc

static PyObject *bgrendererObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((bgrendererObject *)_self)->ob_itself = NULL;
	return _self;
}

#define bgrendererObj_tp_free PyObject_Del


PyTypeObject bgrenderer_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.bgrenderer", /*tp_name*/
	sizeof(bgrendererObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) bgrendererObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) bgrendererObj_compare, /*tp_compare*/
	(reprfunc) bgrendererObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) bgrendererObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	bgrendererObj_methods, /* tp_methods */
	0, /*tp_members*/
	bgrendererObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	bgrendererObj_tp_init, /* tp_init */
	bgrendererObj_tp_alloc, /* tp_alloc */
	bgrendererObj_tp_new, /* tp_new */
	bgrendererObj_tp_free, /* tp_free */
};

/* ------------------- End object type bgrenderer ------------------- */


/* ---------------------- Object type surface ----------------------- */

extern PyTypeObject surface_Type;

inline bool surfaceObj_Check(PyObject *x)
{
	return ((x)->ob_type == &surface_Type);
}

typedef struct surfaceObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::surface* ob_itself;
} surfaceObject;

PyObject *surfaceObj_New(ambulant::common::surface* itself)
{
	surfaceObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_surface
	surface *encaps_itself = dynamic_cast<surface *>(itself);
	if (encaps_itself && encaps_itself->py_surface)
	{
		Py_INCREF(encaps_itself->py_surface);
		return encaps_itself->py_surface;
	}
#endif
	it = PyObject_NEW(surfaceObject, &surface_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int surfaceObj_Convert(PyObject *v, ambulant::common::surface* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_surface
	if (!surfaceObj_Check(v))
	{
		*p_itself = Py_WrapAs_surface(v);
		if (*p_itself) return 1;
	}
#endif
	if (!surfaceObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "surface required");
		return 0;
	}
	*p_itself = ((surfaceObject *)v)->ob_itself;
	return 1;
}

static void surfaceObj_dealloc(surfaceObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *surfaceObj_show(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::gui_events* renderer;
	if (!PyArg_ParseTuple(_args, "O&",
	                      gui_eventsObj_Convert, &renderer))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->show(renderer);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *surfaceObj_renderer_done(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::gui_events* renderer;
	if (!PyArg_ParseTuple(_args, "O&",
	                      gui_eventsObj_Convert, &renderer))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->renderer_done(renderer);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *surfaceObj_need_redraw_1(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_rect_Convert, &r))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->need_redraw(r);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *surfaceObj_need_redraw_2(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->need_redraw();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *surfaceObj_need_events(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool want;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &want))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->need_events(want);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *surfaceObj_transition_done(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->transition_done();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *surfaceObj_keep_as_background(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->keep_as_background();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *surfaceObj_get_rect(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::rect& _rv = _self->ob_itself->get_rect();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_rect_New(_rv));
	return _res;
}

static PyObject *surfaceObj_get_clipped_screen_rect(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::rect& _rv = _self->ob_itself->get_clipped_screen_rect();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_rect_New(_rv));
	return _res;
}

static PyObject *surfaceObj_get_global_topleft(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::point& _rv = _self->ob_itself->get_global_topleft();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_point_New(_rv));
	return _res;
}

static PyObject *surfaceObj_get_fit_rect_1(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::size src_size;
	ambulant::lib::rect out_src_rect;
	ambulant::common::alignment* align;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ambulant_size_Convert, &src_size,
	                      alignmentObj_Convert, &align))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::rect _rv = _self->ob_itself->get_fit_rect(src_size,
	                                                         &out_src_rect,
	                                                         align);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("OO",
	                     ambulant_rect_New(_rv),
	                     ambulant_rect_New(out_src_rect));
	return _res;
}

static PyObject *surfaceObj_get_fit_rect_2(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::rect src_crop_rect;
	ambulant::lib::size src_size;
	ambulant::lib::rect out_src_rect;
	ambulant::common::alignment* align;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ambulant_rect_Convert, &src_crop_rect,
	                      ambulant_size_Convert, &src_size,
	                      alignmentObj_Convert, &align))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::rect _rv = _self->ob_itself->get_fit_rect(src_crop_rect,
	                                                         src_size,
	                                                         &out_src_rect,
	                                                         align);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("OO",
	                     ambulant_rect_New(_rv),
	                     ambulant_rect_New(out_src_rect));
	return _res;
}

static PyObject *surfaceObj_get_crop_rect(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::size src_size;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_size_Convert, &src_size))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::rect _rv = _self->ob_itself->get_crop_rect(src_size);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_rect_New(_rv));
	return _res;
}

static PyObject *surfaceObj_get_info(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::common::region_info* _rv = _self->ob_itself->get_info();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     region_infoObj_New, _rv);
	return _res;
}

static PyObject *surfaceObj_get_top_surface(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::surface* _rv = _self->ob_itself->get_top_surface();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     surfaceObj_New, _rv);
	return _res;
}

static PyObject *surfaceObj_is_tiled(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_tiled();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *surfaceObj_get_gui_window(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::gui_window* _rv = _self->ob_itself->get_gui_window();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     gui_windowObj_New, _rv);
	return _res;
}

static PyObject *surfaceObj_set_renderer_private_data(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::renderer_private_id idd;
	ambulant::common::renderer_private_data * data;
	if (!PyArg_ParseTuple(_args, "ll",
	                      &idd,
	                      &data))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_renderer_private_data(idd,
	                                            data);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *surfaceObj_get_renderer_private_data(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::renderer_private_id idd;
	if (!PyArg_ParseTuple(_args, "l",
	                      &idd))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::renderer_private_data * _rv = _self->ob_itself->get_renderer_private_data(idd);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *surfaceObj_highlight(surfaceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool on;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &on))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->highlight(on);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef surfaceObj_methods[] = {
	{"show", (PyCFunction)surfaceObj_show, 1,
	 PyDoc_STR("(ambulant::common::gui_events* renderer) -> None")},
	{"renderer_done", (PyCFunction)surfaceObj_renderer_done, 1,
	 PyDoc_STR("(ambulant::common::gui_events* renderer) -> None")},
	{"need_redraw_1", (PyCFunction)surfaceObj_need_redraw_1, 1,
	 PyDoc_STR("(ambulant::lib::rect r) -> None")},
	{"need_redraw_2", (PyCFunction)surfaceObj_need_redraw_2, 1,
	 PyDoc_STR("() -> None")},
	{"need_events", (PyCFunction)surfaceObj_need_events, 1,
	 PyDoc_STR("(bool want) -> None")},
	{"transition_done", (PyCFunction)surfaceObj_transition_done, 1,
	 PyDoc_STR("() -> None")},
	{"keep_as_background", (PyCFunction)surfaceObj_keep_as_background, 1,
	 PyDoc_STR("() -> None")},
	{"get_rect", (PyCFunction)surfaceObj_get_rect, 1,
	 PyDoc_STR("() -> (const ambulant::lib::rect& _rv)")},
	{"get_clipped_screen_rect", (PyCFunction)surfaceObj_get_clipped_screen_rect, 1,
	 PyDoc_STR("() -> (const ambulant::lib::rect& _rv)")},
	{"get_global_topleft", (PyCFunction)surfaceObj_get_global_topleft, 1,
	 PyDoc_STR("() -> (const ambulant::lib::point& _rv)")},
	{"get_fit_rect_1", (PyCFunction)surfaceObj_get_fit_rect_1, 1,
	 PyDoc_STR("(ambulant::lib::size src_size, ambulant::common::alignment* align) -> (ambulant::lib::rect _rv, ambulant::lib::rect out_src_rect)")},
	{"get_fit_rect_2", (PyCFunction)surfaceObj_get_fit_rect_2, 1,
	 PyDoc_STR("(ambulant::lib::rect src_crop_rect, ambulant::lib::size src_size, ambulant::common::alignment* align) -> (ambulant::lib::rect _rv, ambulant::lib::rect out_src_rect)")},
	{"get_crop_rect", (PyCFunction)surfaceObj_get_crop_rect, 1,
	 PyDoc_STR("(ambulant::lib::size src_size) -> (ambulant::lib::rect _rv)")},
	{"get_info", (PyCFunction)surfaceObj_get_info, 1,
	 PyDoc_STR("() -> (const ambulant::common::region_info* _rv)")},
	{"get_top_surface", (PyCFunction)surfaceObj_get_top_surface, 1,
	 PyDoc_STR("() -> (ambulant::common::surface* _rv)")},
	{"is_tiled", (PyCFunction)surfaceObj_is_tiled, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"get_gui_window", (PyCFunction)surfaceObj_get_gui_window, 1,
	 PyDoc_STR("() -> (ambulant::common::gui_window* _rv)")},
	{"set_renderer_private_data", (PyCFunction)surfaceObj_set_renderer_private_data, 1,
	 PyDoc_STR("(ambulant::common::renderer_private_id idd, ambulant::common::renderer_private_data * data) -> None")},
	{"get_renderer_private_data", (PyCFunction)surfaceObj_get_renderer_private_data, 1,
	 PyDoc_STR("(ambulant::common::renderer_private_id idd) -> (ambulant::common::renderer_private_data * _rv)")},
	{"highlight", (PyCFunction)surfaceObj_highlight, 1,
	 PyDoc_STR("(bool on) -> None")},
	{NULL, NULL, 0}
};

#define surfaceObj_getsetlist NULL


static int surfaceObj_compare(surfaceObject *self, surfaceObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define surfaceObj_repr NULL

static long surfaceObj_hash(surfaceObject *self)
{
	return (long)self->ob_itself;
}
static int surfaceObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::surface* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, surfaceObj_Convert, &itself))
	{
		((surfaceObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define surfaceObj_tp_alloc PyType_GenericAlloc

static PyObject *surfaceObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((surfaceObject *)_self)->ob_itself = NULL;
	return _self;
}

#define surfaceObj_tp_free PyObject_Del


PyTypeObject surface_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.surface", /*tp_name*/
	sizeof(surfaceObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) surfaceObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) surfaceObj_compare, /*tp_compare*/
	(reprfunc) surfaceObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) surfaceObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	surfaceObj_methods, /* tp_methods */
	0, /*tp_members*/
	surfaceObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	surfaceObj_tp_init, /* tp_init */
	surfaceObj_tp_alloc, /* tp_alloc */
	surfaceObj_tp_new, /* tp_new */
	surfaceObj_tp_free, /* tp_free */
};

/* -------------------- End object type surface --------------------- */


/* ------------------- Object type window_factory ------------------- */

extern PyTypeObject window_factory_Type;

inline bool window_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &window_factory_Type);
}

typedef struct window_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::window_factory* ob_itself;
} window_factoryObject;

PyObject *window_factoryObj_New(ambulant::common::window_factory* itself)
{
	window_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_window_factory
	window_factory *encaps_itself = dynamic_cast<window_factory *>(itself);
	if (encaps_itself && encaps_itself->py_window_factory)
	{
		Py_INCREF(encaps_itself->py_window_factory);
		return encaps_itself->py_window_factory;
	}
#endif
	it = PyObject_NEW(window_factoryObject, &window_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int window_factoryObj_Convert(PyObject *v, ambulant::common::window_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_window_factory
	if (!window_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_window_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!window_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "window_factory required");
		return 0;
	}
	*p_itself = ((window_factoryObject *)v)->ob_itself;
	return 1;
}

static void window_factoryObj_dealloc(window_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *window_factoryObj_get_default_size(window_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::size _rv = _self->ob_itself->get_default_size();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_size_New(_rv));
	return _res;
}

static PyObject *window_factoryObj_new_window(window_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string name;
	ambulant::lib::size bounds;
	ambulant::common::gui_events* handler;
	char *name_cstr="";
	if (!PyArg_ParseTuple(_args, "sO&O&",
	                      &name_cstr,
	                      ambulant_size_Convert, &bounds,
	                      gui_eventsObj_Convert, &handler))
		return NULL;
	name = name_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::gui_window* _rv = _self->ob_itself->new_window(name,
	                                                                 bounds,
	                                                                 handler);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     gui_windowObj_New, _rv);
	return _res;
}

static PyObject *window_factoryObj_new_background_renderer(window_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::region_info* src;
	if (!PyArg_ParseTuple(_args, "O&",
	                      region_infoObj_Convert, &src))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::bgrenderer* _rv = _self->ob_itself->new_background_renderer(src);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bgrendererObj_New, _rv);
	return _res;
}

static PyObject *window_factoryObj_window_done(window_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string name;
	char *name_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &name_cstr))
		return NULL;
	name = name_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->window_done(name);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef window_factoryObj_methods[] = {
	{"get_default_size", (PyCFunction)window_factoryObj_get_default_size, 1,
	 PyDoc_STR("() -> (ambulant::lib::size _rv)")},
	{"new_window", (PyCFunction)window_factoryObj_new_window, 1,
	 PyDoc_STR("(std::string name, ambulant::lib::size bounds, ambulant::common::gui_events* handler) -> (ambulant::common::gui_window* _rv)")},
	{"new_background_renderer", (PyCFunction)window_factoryObj_new_background_renderer, 1,
	 PyDoc_STR("(ambulant::common::region_info* src) -> (ambulant::common::bgrenderer* _rv)")},
	{"window_done", (PyCFunction)window_factoryObj_window_done, 1,
	 PyDoc_STR("(std::string name) -> None")},
	{NULL, NULL, 0}
};

#define window_factoryObj_getsetlist NULL


static int window_factoryObj_compare(window_factoryObject *self, window_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define window_factoryObj_repr NULL

static long window_factoryObj_hash(window_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int window_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::window_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, window_factoryObj_Convert, &itself))
	{
		((window_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define window_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *window_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((window_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define window_factoryObj_tp_free PyObject_Del


PyTypeObject window_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.window_factory", /*tp_name*/
	sizeof(window_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) window_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) window_factoryObj_compare, /*tp_compare*/
	(reprfunc) window_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) window_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	window_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	window_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	window_factoryObj_tp_init, /* tp_init */
	window_factoryObj_tp_alloc, /* tp_alloc */
	window_factoryObj_tp_new, /* tp_new */
	window_factoryObj_tp_free, /* tp_free */
};

/* ----------------- End object type window_factory ----------------- */


/* ------------------ Object type surface_template ------------------ */

extern PyTypeObject surface_template_Type;

inline bool surface_templateObj_Check(PyObject *x)
{
	return ((x)->ob_type == &surface_template_Type);
}

typedef struct surface_templateObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::surface_template* ob_itself;
} surface_templateObject;

PyObject *surface_templateObj_New(ambulant::common::surface_template* itself)
{
	surface_templateObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_surface_template
	surface_template *encaps_itself = dynamic_cast<surface_template *>(itself);
	if (encaps_itself && encaps_itself->py_surface_template)
	{
		Py_INCREF(encaps_itself->py_surface_template);
		return encaps_itself->py_surface_template;
	}
#endif
	it = PyObject_NEW(surface_templateObject, &surface_template_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int surface_templateObj_Convert(PyObject *v, ambulant::common::surface_template* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_surface_template
	if (!surface_templateObj_Check(v))
	{
		*p_itself = Py_WrapAs_surface_template(v);
		if (*p_itself) return 1;
	}
#endif
	if (!surface_templateObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "surface_template required");
		return 0;
	}
	*p_itself = ((surface_templateObject *)v)->ob_itself;
	return 1;
}

static void surface_templateObj_dealloc(surface_templateObject *self)
{
	animation_notification_Type.tp_dealloc((PyObject *)self);
}

static PyObject *surface_templateObj_new_subsurface(surface_templateObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::region_info* info;
	ambulant::common::bgrenderer* bgrend;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      region_infoObj_Convert, &info,
	                      bgrendererObj_Convert, &bgrend))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::surface_template* _rv = _self->ob_itself->new_subsurface(info,
	                                                                           bgrend);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     surface_templateObj_New, _rv);
	return _res;
}

static PyObject *surface_templateObj_activate(surface_templateObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::surface* _rv = _self->ob_itself->activate();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     surfaceObj_New, _rv);
	return _res;
}

static PyMethodDef surface_templateObj_methods[] = {
	{"new_subsurface", (PyCFunction)surface_templateObj_new_subsurface, 1,
	 PyDoc_STR("(ambulant::common::region_info* info, ambulant::common::bgrenderer* bgrend) -> (ambulant::common::surface_template* _rv)")},
	{"activate", (PyCFunction)surface_templateObj_activate, 1,
	 PyDoc_STR("() -> (ambulant::common::surface* _rv)")},
	{NULL, NULL, 0}
};

#define surface_templateObj_getsetlist NULL


static int surface_templateObj_compare(surface_templateObject *self, surface_templateObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define surface_templateObj_repr NULL

static long surface_templateObj_hash(surface_templateObject *self)
{
	return (long)self->ob_itself;
}
static int surface_templateObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::surface_template* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, surface_templateObj_Convert, &itself))
	{
		((surface_templateObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define surface_templateObj_tp_alloc PyType_GenericAlloc

static PyObject *surface_templateObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((surface_templateObject *)_self)->ob_itself = NULL;
	return _self;
}

#define surface_templateObj_tp_free PyObject_Del


PyTypeObject surface_template_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.surface_template", /*tp_name*/
	sizeof(surface_templateObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) surface_templateObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) surface_templateObj_compare, /*tp_compare*/
	(reprfunc) surface_templateObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) surface_templateObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	surface_templateObj_methods, /* tp_methods */
	0, /*tp_members*/
	surface_templateObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	surface_templateObj_tp_init, /* tp_init */
	surface_templateObj_tp_alloc, /* tp_alloc */
	surface_templateObj_tp_new, /* tp_new */
	surface_templateObj_tp_free, /* tp_free */
};

/* ---------------- End object type surface_template ---------------- */


/* ------------------ Object type surface_factory ------------------- */

extern PyTypeObject surface_factory_Type;

inline bool surface_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &surface_factory_Type);
}

typedef struct surface_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::surface_factory* ob_itself;
} surface_factoryObject;

PyObject *surface_factoryObj_New(ambulant::common::surface_factory* itself)
{
	surface_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_surface_factory
	surface_factory *encaps_itself = dynamic_cast<surface_factory *>(itself);
	if (encaps_itself && encaps_itself->py_surface_factory)
	{
		Py_INCREF(encaps_itself->py_surface_factory);
		return encaps_itself->py_surface_factory;
	}
#endif
	it = PyObject_NEW(surface_factoryObject, &surface_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int surface_factoryObj_Convert(PyObject *v, ambulant::common::surface_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_surface_factory
	if (!surface_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_surface_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!surface_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "surface_factory required");
		return 0;
	}
	*p_itself = ((surface_factoryObject *)v)->ob_itself;
	return 1;
}

static void surface_factoryObj_dealloc(surface_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *surface_factoryObj_new_topsurface(surface_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::region_info* info;
	ambulant::common::bgrenderer* bgrend;
	ambulant::common::window_factory* wf;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      region_infoObj_Convert, &info,
	                      bgrendererObj_Convert, &bgrend,
	                      window_factoryObj_Convert, &wf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::surface_template* _rv = _self->ob_itself->new_topsurface(info,
	                                                                           bgrend,
	                                                                           wf);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     surface_templateObj_New, _rv);
	return _res;
}

static PyMethodDef surface_factoryObj_methods[] = {
	{"new_topsurface", (PyCFunction)surface_factoryObj_new_topsurface, 1,
	 PyDoc_STR("(ambulant::common::region_info* info, ambulant::common::bgrenderer* bgrend, ambulant::common::window_factory* wf) -> (ambulant::common::surface_template* _rv)")},
	{NULL, NULL, 0}
};

#define surface_factoryObj_getsetlist NULL


static int surface_factoryObj_compare(surface_factoryObject *self, surface_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define surface_factoryObj_repr NULL

static long surface_factoryObj_hash(surface_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int surface_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::surface_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, surface_factoryObj_Convert, &itself))
	{
		((surface_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define surface_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *surface_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((surface_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define surface_factoryObj_tp_free PyObject_Del


PyTypeObject surface_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.surface_factory", /*tp_name*/
	sizeof(surface_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) surface_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) surface_factoryObj_compare, /*tp_compare*/
	(reprfunc) surface_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) surface_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	surface_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	surface_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	surface_factoryObj_tp_init, /* tp_init */
	surface_factoryObj_tp_alloc, /* tp_alloc */
	surface_factoryObj_tp_new, /* tp_new */
	surface_factoryObj_tp_free, /* tp_free */
};

/* ---------------- End object type surface_factory ----------------- */


/* ------------------- Object type layout_manager ------------------- */

extern PyTypeObject layout_manager_Type;

inline bool layout_managerObj_Check(PyObject *x)
{
	return ((x)->ob_type == &layout_manager_Type);
}

typedef struct layout_managerObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::layout_manager* ob_itself;
} layout_managerObject;

PyObject *layout_managerObj_New(ambulant::common::layout_manager* itself)
{
	layout_managerObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_layout_manager
	layout_manager *encaps_itself = dynamic_cast<layout_manager *>(itself);
	if (encaps_itself && encaps_itself->py_layout_manager)
	{
		Py_INCREF(encaps_itself->py_layout_manager);
		return encaps_itself->py_layout_manager;
	}
#endif
	it = PyObject_NEW(layout_managerObject, &layout_manager_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int layout_managerObj_Convert(PyObject *v, ambulant::common::layout_manager* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_layout_manager
	if (!layout_managerObj_Check(v))
	{
		*p_itself = Py_WrapAs_layout_manager(v);
		if (*p_itself) return 1;
	}
#endif
	if (!layout_managerObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "layout_manager required");
		return 0;
	}
	*p_itself = ((layout_managerObject *)v)->ob_itself;
	return 1;
}

static void layout_managerObj_dealloc(layout_managerObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *layout_managerObj_get_surface(layout_managerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* node;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &node))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::surface* _rv = _self->ob_itself->get_surface(node);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     surfaceObj_New, _rv);
	return _res;
}

static PyObject *layout_managerObj_get_alignment(layout_managerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* node;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &node))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::alignment* _rv = _self->ob_itself->get_alignment(node);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     alignmentObj_New, _rv);
	return _res;
}

static PyObject *layout_managerObj_get_animation_notification(layout_managerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* node;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &node))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::animation_notification* _rv = _self->ob_itself->get_animation_notification(node);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     animation_notificationObj_New, _rv);
	return _res;
}

static PyObject *layout_managerObj_get_animation_destination(layout_managerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* node;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &node))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::animation_destination* _rv = _self->ob_itself->get_animation_destination(node);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     animation_destinationObj_New, _rv);
	return _res;
}

static PyMethodDef layout_managerObj_methods[] = {
	{"get_surface", (PyCFunction)layout_managerObj_get_surface, 1,
	 PyDoc_STR("(ambulant::lib::node* node) -> (ambulant::common::surface* _rv)")},
	{"get_alignment", (PyCFunction)layout_managerObj_get_alignment, 1,
	 PyDoc_STR("(ambulant::lib::node* node) -> (ambulant::common::alignment* _rv)")},
	{"get_animation_notification", (PyCFunction)layout_managerObj_get_animation_notification, 1,
	 PyDoc_STR("(ambulant::lib::node* node) -> (ambulant::common::animation_notification* _rv)")},
	{"get_animation_destination", (PyCFunction)layout_managerObj_get_animation_destination, 1,
	 PyDoc_STR("(ambulant::lib::node* node) -> (ambulant::common::animation_destination* _rv)")},
	{NULL, NULL, 0}
};

#define layout_managerObj_getsetlist NULL


static int layout_managerObj_compare(layout_managerObject *self, layout_managerObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define layout_managerObj_repr NULL

static long layout_managerObj_hash(layout_managerObject *self)
{
	return (long)self->ob_itself;
}
static int layout_managerObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::layout_manager* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, layout_managerObj_Convert, &itself))
	{
		((layout_managerObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define layout_managerObj_tp_alloc PyType_GenericAlloc

static PyObject *layout_managerObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((layout_managerObject *)_self)->ob_itself = NULL;
	return _self;
}

#define layout_managerObj_tp_free PyObject_Del


PyTypeObject layout_manager_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.layout_manager", /*tp_name*/
	sizeof(layout_managerObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) layout_managerObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) layout_managerObj_compare, /*tp_compare*/
	(reprfunc) layout_managerObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) layout_managerObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	layout_managerObj_methods, /* tp_methods */
	0, /*tp_members*/
	layout_managerObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	layout_managerObj_tp_init, /* tp_init */
	layout_managerObj_tp_alloc, /* tp_alloc */
	layout_managerObj_tp_new, /* tp_new */
	layout_managerObj_tp_free, /* tp_free */
};

/* ----------------- End object type layout_manager ----------------- */


/* ---------------------- Object type playable ---------------------- */

extern PyTypeObject playable_Type;

inline bool playableObj_Check(PyObject *x)
{
	return ((x)->ob_type == &playable_Type);
}

typedef struct playableObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::playable* ob_itself;
} playableObject;

PyObject *playableObj_New(ambulant::common::playable* itself)
{
	playableObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_playable
	playable *encaps_itself = dynamic_cast<playable *>(itself);
	if (encaps_itself && encaps_itself->py_playable)
	{
		Py_INCREF(encaps_itself->py_playable);
		return encaps_itself->py_playable;
	}
#endif
	it = PyObject_NEW(playableObject, &playable_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int playableObj_Convert(PyObject *v, ambulant::common::playable* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_playable
	if (!playableObj_Check(v))
	{
		*p_itself = Py_WrapAs_playable(v);
		if (*p_itself) return 1;
	}
#endif
	if (!playableObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "playable required");
		return 0;
	}
	*p_itself = ((playableObject *)v)->ob_itself;
	return 1;
}

static void playableObj_dealloc(playableObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *playableObj_init_with_node(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &n))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->init_with_node(n);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playableObj_start(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	double t;
	if (!PyArg_ParseTuple(_args, "d",
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->start(t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playableObj_stop(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->stop();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *playableObj_post_stop(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->post_stop();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playableObj_pause(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::pause_display d;
	if (!PyArg_ParseTuple(_args, "l",
	                      &d))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->pause(d);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playableObj_resume(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->resume();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playableObj_seek(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	double t;
	if (!PyArg_ParseTuple(_args, "d",
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->seek(t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playableObj_wantclicks(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool want;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &want))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->wantclicks(want);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playableObj_preroll(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	double when;
	double where;
	double how_much;
	if (!PyArg_ParseTuple(_args, "ddd",
	                      &when,
	                      &where,
	                      &how_much))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->preroll(when,
	                          where,
	                          how_much);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playableObj_get_dur(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::duration _rv = _self->ob_itself->get_dur();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("(O&d)",
	                     bool_New, _rv.first, _rv.second);
	return _res;
}

static PyObject *playableObj_get_cookie(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::playable::cookie_type _rv = _self->ob_itself->get_cookie();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *playableObj_get_renderer(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::renderer* _rv = _self->ob_itself->get_renderer();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     rendererObj_New, _rv);
	return _res;
}

static PyObject *playableObj_get_sig(playableObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	std::string _rv = _self->ob_itself->get_sig();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyMethodDef playableObj_methods[] = {
	{"init_with_node", (PyCFunction)playableObj_init_with_node, 1,
	 PyDoc_STR("(ambulant::lib::node* n) -> None")},
	{"start", (PyCFunction)playableObj_start, 1,
	 PyDoc_STR("(double t) -> None")},
	{"stop", (PyCFunction)playableObj_stop, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"post_stop", (PyCFunction)playableObj_post_stop, 1,
	 PyDoc_STR("() -> None")},
	{"pause", (PyCFunction)playableObj_pause, 1,
	 PyDoc_STR("(ambulant::common::pause_display d) -> None")},
	{"resume", (PyCFunction)playableObj_resume, 1,
	 PyDoc_STR("() -> None")},
	{"seek", (PyCFunction)playableObj_seek, 1,
	 PyDoc_STR("(double t) -> None")},
	{"wantclicks", (PyCFunction)playableObj_wantclicks, 1,
	 PyDoc_STR("(bool want) -> None")},
	{"preroll", (PyCFunction)playableObj_preroll, 1,
	 PyDoc_STR("(double when, double where, double how_much) -> None")},
	{"get_dur", (PyCFunction)playableObj_get_dur, 1,
	 PyDoc_STR("() -> (ambulant::common::duration _rv)")},
	{"get_cookie", (PyCFunction)playableObj_get_cookie, 1,
	 PyDoc_STR("() -> (ambulant::common::playable::cookie_type _rv)")},
	{"get_renderer", (PyCFunction)playableObj_get_renderer, 1,
	 PyDoc_STR("() -> (ambulant::common::renderer* _rv)")},
	{"get_sig", (PyCFunction)playableObj_get_sig, 1,
	 PyDoc_STR("() -> (std::string _rv)")},
	{NULL, NULL, 0}
};

#define playableObj_getsetlist NULL


static int playableObj_compare(playableObject *self, playableObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define playableObj_repr NULL

static long playableObj_hash(playableObject *self)
{
	return (long)self->ob_itself;
}
static int playableObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::playable* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, playableObj_Convert, &itself))
	{
		((playableObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define playableObj_tp_alloc PyType_GenericAlloc

static PyObject *playableObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((playableObject *)_self)->ob_itself = NULL;
	return _self;
}

#define playableObj_tp_free PyObject_Del


PyTypeObject playable_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.playable", /*tp_name*/
	sizeof(playableObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) playableObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) playableObj_compare, /*tp_compare*/
	(reprfunc) playableObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) playableObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	playableObj_methods, /* tp_methods */
	0, /*tp_members*/
	playableObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	playableObj_tp_init, /* tp_init */
	playableObj_tp_alloc, /* tp_alloc */
	playableObj_tp_new, /* tp_new */
	playableObj_tp_free, /* tp_free */
};

/* -------------------- End object type playable -------------------- */


/* --------------- Object type playable_notification ---------------- */

extern PyTypeObject playable_notification_Type;

inline bool playable_notificationObj_Check(PyObject *x)
{
	return ((x)->ob_type == &playable_notification_Type);
}

typedef struct playable_notificationObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::playable_notification* ob_itself;
} playable_notificationObject;

PyObject *playable_notificationObj_New(ambulant::common::playable_notification* itself)
{
	playable_notificationObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_playable_notification
	playable_notification *encaps_itself = dynamic_cast<playable_notification *>(itself);
	if (encaps_itself && encaps_itself->py_playable_notification)
	{
		Py_INCREF(encaps_itself->py_playable_notification);
		return encaps_itself->py_playable_notification;
	}
#endif
	it = PyObject_NEW(playable_notificationObject, &playable_notification_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int playable_notificationObj_Convert(PyObject *v, ambulant::common::playable_notification* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_playable_notification
	if (!playable_notificationObj_Check(v))
	{
		*p_itself = Py_WrapAs_playable_notification(v);
		if (*p_itself) return 1;
	}
#endif
	if (!playable_notificationObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "playable_notification required");
		return 0;
	}
	*p_itself = ((playable_notificationObject *)v)->ob_itself;
	return 1;
}

static void playable_notificationObj_dealloc(playable_notificationObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *playable_notificationObj_started(playable_notificationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable::cookie_type n;
	double t;
	if (!PyArg_ParseTuple(_args, "ld",
	                      &n,
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->started(n,
	                          t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playable_notificationObj_stopped(playable_notificationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable::cookie_type n;
	double t;
	if (!PyArg_ParseTuple(_args, "ld",
	                      &n,
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->stopped(n,
	                          t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playable_notificationObj_clicked(playable_notificationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable::cookie_type n;
	double t;
	if (!PyArg_ParseTuple(_args, "ld",
	                      &n,
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->clicked(n,
	                          t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playable_notificationObj_pointed(playable_notificationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable::cookie_type n;
	double t;
	if (!PyArg_ParseTuple(_args, "ld",
	                      &n,
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->pointed(n,
	                          t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playable_notificationObj_transitioned(playable_notificationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable::cookie_type n;
	double t;
	if (!PyArg_ParseTuple(_args, "ld",
	                      &n,
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->transitioned(n,
	                               t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playable_notificationObj_marker_seen(playable_notificationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable::cookie_type n;
	char* name;
	double t;
	if (!PyArg_ParseTuple(_args, "lsd",
	                      &n,
	                      &name,
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->marker_seen(n,
	                              name,
	                              t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playable_notificationObj_playable_stalled(playable_notificationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable* p;
	char* reason;
	if (!PyArg_ParseTuple(_args, "O&s",
	                      playableObj_Convert, &p,
	                      &reason))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->playable_stalled(p,
	                                   reason);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playable_notificationObj_playable_unstalled(playable_notificationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable* p;
	if (!PyArg_ParseTuple(_args, "O&",
	                      playableObj_Convert, &p))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->playable_unstalled(p);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playable_notificationObj_playable_started(playable_notificationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable* p;
	ambulant::lib::node* n;
	char* comment;
	if (!PyArg_ParseTuple(_args, "O&O&s",
	                      playableObj_Convert, &p,
	                      nodeObj_Convert, &n,
	                      &comment))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->playable_started(p,
	                                   n,
	                                   comment);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playable_notificationObj_playable_resource(playable_notificationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable* p;
	char* resource;
	long amount;
	if (!PyArg_ParseTuple(_args, "O&sl",
	                      playableObj_Convert, &p,
	                      &resource,
	                      &amount))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->playable_resource(p,
	                                    resource,
	                                    amount);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef playable_notificationObj_methods[] = {
	{"started", (PyCFunction)playable_notificationObj_started, 1,
	 PyDoc_STR("(ambulant::common::playable::cookie_type n, double t) -> None")},
	{"stopped", (PyCFunction)playable_notificationObj_stopped, 1,
	 PyDoc_STR("(ambulant::common::playable::cookie_type n, double t) -> None")},
	{"clicked", (PyCFunction)playable_notificationObj_clicked, 1,
	 PyDoc_STR("(ambulant::common::playable::cookie_type n, double t) -> None")},
	{"pointed", (PyCFunction)playable_notificationObj_pointed, 1,
	 PyDoc_STR("(ambulant::common::playable::cookie_type n, double t) -> None")},
	{"transitioned", (PyCFunction)playable_notificationObj_transitioned, 1,
	 PyDoc_STR("(ambulant::common::playable::cookie_type n, double t) -> None")},
	{"marker_seen", (PyCFunction)playable_notificationObj_marker_seen, 1,
	 PyDoc_STR("(ambulant::common::playable::cookie_type n, char* name, double t) -> None")},
	{"playable_stalled", (PyCFunction)playable_notificationObj_playable_stalled, 1,
	 PyDoc_STR("(ambulant::common::playable* p, char* reason) -> None")},
	{"playable_unstalled", (PyCFunction)playable_notificationObj_playable_unstalled, 1,
	 PyDoc_STR("(ambulant::common::playable* p) -> None")},
	{"playable_started", (PyCFunction)playable_notificationObj_playable_started, 1,
	 PyDoc_STR("(ambulant::common::playable* p, ambulant::lib::node* n, char* comment) -> None")},
	{"playable_resource", (PyCFunction)playable_notificationObj_playable_resource, 1,
	 PyDoc_STR("(ambulant::common::playable* p, char* resource, long amount) -> None")},
	{NULL, NULL, 0}
};

#define playable_notificationObj_getsetlist NULL


static int playable_notificationObj_compare(playable_notificationObject *self, playable_notificationObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define playable_notificationObj_repr NULL

static long playable_notificationObj_hash(playable_notificationObject *self)
{
	return (long)self->ob_itself;
}
static int playable_notificationObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::playable_notification* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, playable_notificationObj_Convert, &itself))
	{
		((playable_notificationObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define playable_notificationObj_tp_alloc PyType_GenericAlloc

static PyObject *playable_notificationObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((playable_notificationObject *)_self)->ob_itself = NULL;
	return _self;
}

#define playable_notificationObj_tp_free PyObject_Del


PyTypeObject playable_notification_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.playable_notification", /*tp_name*/
	sizeof(playable_notificationObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) playable_notificationObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) playable_notificationObj_compare, /*tp_compare*/
	(reprfunc) playable_notificationObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) playable_notificationObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	playable_notificationObj_methods, /* tp_methods */
	0, /*tp_members*/
	playable_notificationObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	playable_notificationObj_tp_init, /* tp_init */
	playable_notificationObj_tp_alloc, /* tp_alloc */
	playable_notificationObj_tp_new, /* tp_new */
	playable_notificationObj_tp_free, /* tp_free */
};

/* ------------- End object type playable_notification -------------- */


/* ------------------ Object type playable_factory ------------------ */

extern PyTypeObject playable_factory_Type;

inline bool playable_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &playable_factory_Type);
}

typedef struct playable_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::playable_factory* ob_itself;
} playable_factoryObject;

PyObject *playable_factoryObj_New(ambulant::common::playable_factory* itself)
{
	playable_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_playable_factory
	playable_factory *encaps_itself = dynamic_cast<playable_factory *>(itself);
	if (encaps_itself && encaps_itself->py_playable_factory)
	{
		Py_INCREF(encaps_itself->py_playable_factory);
		return encaps_itself->py_playable_factory;
	}
#endif
	it = PyObject_NEW(playable_factoryObject, &playable_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int playable_factoryObj_Convert(PyObject *v, ambulant::common::playable_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_playable_factory
	if (!playable_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_playable_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!playable_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "playable_factory required");
		return 0;
	}
	*p_itself = ((playable_factoryObject *)v)->ob_itself;
	return 1;
}

static void playable_factoryObj_dealloc(playable_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *playable_factoryObj_supports(playable_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::renderer_select* rs;
	if (!PyArg_ParseTuple(_args, "O&",
	                      renderer_selectObj_Convert, &rs))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->supports(rs);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *playable_factoryObj_new_playable(playable_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_notification* context;
	ambulant::common::playable::cookie_type cookie;
	ambulant::lib::node* node;
	ambulant::lib::event_processor* evp;
	if (!PyArg_ParseTuple(_args, "O&lO&O&",
	                      playable_notificationObj_Convert, &context,
	                      &cookie,
	                      nodeObj_Convert, &node,
	                      event_processorObj_Convert, &evp))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::playable* _rv = _self->ob_itself->new_playable(context,
	                                                                 cookie,
	                                                                 node,
	                                                                 evp);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playableObj_New, _rv);
	return _res;
}

static PyObject *playable_factoryObj_new_aux_audio_playable(playable_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_notification* context;
	ambulant::common::playable::cookie_type cookie;
	ambulant::lib::node* node;
	ambulant::lib::event_processor* evp;
	ambulant::net::audio_datasource* src;
	if (!PyArg_ParseTuple(_args, "O&lO&O&O&",
	                      playable_notificationObj_Convert, &context,
	                      &cookie,
	                      nodeObj_Convert, &node,
	                      event_processorObj_Convert, &evp,
	                      audio_datasourceObj_Convert, &src))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::playable* _rv = _self->ob_itself->new_aux_audio_playable(context,
	                                                                           cookie,
	                                                                           node,
	                                                                           evp,
	                                                                           src);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playableObj_New, _rv);
	return _res;
}

static PyMethodDef playable_factoryObj_methods[] = {
	{"supports", (PyCFunction)playable_factoryObj_supports, 1,
	 PyDoc_STR("(ambulant::common::renderer_select* rs) -> (bool _rv)")},
	{"new_playable", (PyCFunction)playable_factoryObj_new_playable, 1,
	 PyDoc_STR("(ambulant::common::playable_notification* context, ambulant::common::playable::cookie_type cookie, ambulant::lib::node* node, ambulant::lib::event_processor* evp) -> (ambulant::common::playable* _rv)")},
	{"new_aux_audio_playable", (PyCFunction)playable_factoryObj_new_aux_audio_playable, 1,
	 PyDoc_STR("(ambulant::common::playable_notification* context, ambulant::common::playable::cookie_type cookie, ambulant::lib::node* node, ambulant::lib::event_processor* evp, ambulant::net::audio_datasource* src) -> (ambulant::common::playable* _rv)")},
	{NULL, NULL, 0}
};

#define playable_factoryObj_getsetlist NULL


static int playable_factoryObj_compare(playable_factoryObject *self, playable_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define playable_factoryObj_repr NULL

static long playable_factoryObj_hash(playable_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int playable_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::playable_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, playable_factoryObj_Convert, &itself))
	{
		((playable_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define playable_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *playable_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((playable_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define playable_factoryObj_tp_free PyObject_Del


PyTypeObject playable_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.playable_factory", /*tp_name*/
	sizeof(playable_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) playable_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) playable_factoryObj_compare, /*tp_compare*/
	(reprfunc) playable_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) playable_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	playable_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	playable_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	playable_factoryObj_tp_init, /* tp_init */
	playable_factoryObj_tp_alloc, /* tp_alloc */
	playable_factoryObj_tp_new, /* tp_new */
	playable_factoryObj_tp_free, /* tp_free */
};

/* ---------------- End object type playable_factory ---------------- */


/* -------------- Object type global_playable_factory --------------- */

extern PyTypeObject global_playable_factory_Type;

inline bool global_playable_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &global_playable_factory_Type);
}

typedef struct global_playable_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::global_playable_factory* ob_itself;
} global_playable_factoryObject;

PyObject *global_playable_factoryObj_New(ambulant::common::global_playable_factory* itself)
{
	global_playable_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_global_playable_factory
	global_playable_factory *encaps_itself = dynamic_cast<global_playable_factory *>(itself);
	if (encaps_itself && encaps_itself->py_global_playable_factory)
	{
		Py_INCREF(encaps_itself->py_global_playable_factory);
		return encaps_itself->py_global_playable_factory;
	}
#endif
	it = PyObject_NEW(global_playable_factoryObject, &global_playable_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int global_playable_factoryObj_Convert(PyObject *v, ambulant::common::global_playable_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_global_playable_factory
	if (!global_playable_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_global_playable_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!global_playable_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "global_playable_factory required");
		return 0;
	}
	*p_itself = ((global_playable_factoryObject *)v)->ob_itself;
	return 1;
}

static void global_playable_factoryObj_dealloc(global_playable_factoryObject *self)
{
	playable_factory_Type.tp_dealloc((PyObject *)self);
}

static PyObject *global_playable_factoryObj_add_factory(global_playable_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* rf;
	if (!PyArg_ParseTuple(_args, "O&",
	                      playable_factoryObj_Convert, &rf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->add_factory(rf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *global_playable_factoryObj_preferred_renderer(global_playable_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* name;
	if (!PyArg_ParseTuple(_args, "s",
	                      &name))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->preferred_renderer(name);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef global_playable_factoryObj_methods[] = {
	{"add_factory", (PyCFunction)global_playable_factoryObj_add_factory, 1,
	 PyDoc_STR("(ambulant::common::playable_factory* rf) -> None")},
	{"preferred_renderer", (PyCFunction)global_playable_factoryObj_preferred_renderer, 1,
	 PyDoc_STR("(char* name) -> None")},
	{NULL, NULL, 0}
};

#define global_playable_factoryObj_getsetlist NULL


static int global_playable_factoryObj_compare(global_playable_factoryObject *self, global_playable_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define global_playable_factoryObj_repr NULL

static long global_playable_factoryObj_hash(global_playable_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int global_playable_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::global_playable_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, global_playable_factoryObj_Convert, &itself))
	{
		((global_playable_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define global_playable_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *global_playable_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((global_playable_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define global_playable_factoryObj_tp_free PyObject_Del


PyTypeObject global_playable_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.global_playable_factory", /*tp_name*/
	sizeof(global_playable_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) global_playable_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) global_playable_factoryObj_compare, /*tp_compare*/
	(reprfunc) global_playable_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) global_playable_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	global_playable_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	global_playable_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	global_playable_factoryObj_tp_init, /* tp_init */
	global_playable_factoryObj_tp_alloc, /* tp_alloc */
	global_playable_factoryObj_tp_new, /* tp_new */
	global_playable_factoryObj_tp_free, /* tp_free */
};

/* ------------ End object type global_playable_factory ------------- */


/* ---------------------- Object type recorder ---------------------- */

extern PyTypeObject recorder_Type;

inline bool recorderObj_Check(PyObject *x)
{
	return ((x)->ob_type == &recorder_Type);
}

typedef struct recorderObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::recorder* ob_itself;
} recorderObject;

PyObject *recorderObj_New(ambulant::common::recorder* itself)
{
	recorderObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_recorder
	recorder *encaps_itself = dynamic_cast<recorder *>(itself);
	if (encaps_itself && encaps_itself->py_recorder)
	{
		Py_INCREF(encaps_itself->py_recorder);
		return encaps_itself->py_recorder;
	}
#endif
	it = PyObject_NEW(recorderObject, &recorder_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int recorderObj_Convert(PyObject *v, ambulant::common::recorder* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_recorder
	if (!recorderObj_Check(v))
	{
		*p_itself = Py_WrapAs_recorder(v);
		if (*p_itself) return 1;
	}
#endif
	if (!recorderObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "recorder required");
		return 0;
	}
	*p_itself = ((recorderObject *)v)->ob_itself;
	return 1;
}

static void recorderObj_dealloc(recorderObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *recorderObj_new_video_data(recorderObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char *data__in__;
	size_t data__len__;
	int data__in_len__;
	ambulant::lib::timer::time_type documenttimestamp;
	if (!PyArg_ParseTuple(_args, "s#l",
	                      &data__in__, &data__in_len__,
	                      &documenttimestamp))
		return NULL;
	data__len__ = data__in_len__;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->new_video_data(data__in__, data__len__,
	                                 documenttimestamp);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *recorderObj_new_audio_data(recorderObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char *data__in__;
	size_t data__len__;
	int data__in_len__;
	ambulant::lib::timer::time_type documenttimestamp;
	if (!PyArg_ParseTuple(_args, "s#l",
	                      &data__in__, &data__in_len__,
	                      &documenttimestamp))
		return NULL;
	data__len__ = data__in_len__;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->new_audio_data(data__in__, data__len__,
	                                 documenttimestamp);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef recorderObj_methods[] = {
	{"new_video_data", (PyCFunction)recorderObj_new_video_data, 1,
	 PyDoc_STR("(Buffer data, ambulant::lib::timer::time_type documenttimestamp) -> None")},
	{"new_audio_data", (PyCFunction)recorderObj_new_audio_data, 1,
	 PyDoc_STR("(Buffer data, ambulant::lib::timer::time_type documenttimestamp) -> None")},
	{NULL, NULL, 0}
};

#define recorderObj_getsetlist NULL


static int recorderObj_compare(recorderObject *self, recorderObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define recorderObj_repr NULL

static long recorderObj_hash(recorderObject *self)
{
	return (long)self->ob_itself;
}
static int recorderObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::recorder* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, recorderObj_Convert, &itself))
	{
		((recorderObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define recorderObj_tp_alloc PyType_GenericAlloc

static PyObject *recorderObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((recorderObject *)_self)->ob_itself = NULL;
	return _self;
}

#define recorderObj_tp_free PyObject_Del


PyTypeObject recorder_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.recorder", /*tp_name*/
	sizeof(recorderObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) recorderObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) recorderObj_compare, /*tp_compare*/
	(reprfunc) recorderObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) recorderObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	recorderObj_methods, /* tp_methods */
	0, /*tp_members*/
	recorderObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	recorderObj_tp_init, /* tp_init */
	recorderObj_tp_alloc, /* tp_alloc */
	recorderObj_tp_new, /* tp_new */
	recorderObj_tp_free, /* tp_free */
};

/* -------------------- End object type recorder -------------------- */


/* ------------------ Object type recorder_factory ------------------ */

extern PyTypeObject recorder_factory_Type;

inline bool recorder_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &recorder_factory_Type);
}

typedef struct recorder_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::recorder_factory* ob_itself;
} recorder_factoryObject;

PyObject *recorder_factoryObj_New(ambulant::common::recorder_factory* itself)
{
	recorder_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_recorder_factory
	recorder_factory *encaps_itself = dynamic_cast<recorder_factory *>(itself);
	if (encaps_itself && encaps_itself->py_recorder_factory)
	{
		Py_INCREF(encaps_itself->py_recorder_factory);
		return encaps_itself->py_recorder_factory;
	}
#endif
	it = PyObject_NEW(recorder_factoryObject, &recorder_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int recorder_factoryObj_Convert(PyObject *v, ambulant::common::recorder_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_recorder_factory
	if (!recorder_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_recorder_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!recorder_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "recorder_factory required");
		return 0;
	}
	*p_itself = ((recorder_factoryObject *)v)->ob_itself;
	return 1;
}

static void recorder_factoryObj_dealloc(recorder_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *recorder_factoryObj_new_recorder(recorder_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::pixel_order pixel_order;
	ambulant::lib::size window_size;
	if (!PyArg_ParseTuple(_args, "lO&",
	                      &pixel_order,
	                      ambulant_size_Convert, &window_size))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::recorder* _rv = _self->ob_itself->new_recorder(pixel_order,
	                                                                 window_size);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     recorderObj_New, _rv);
	return _res;
}

static PyMethodDef recorder_factoryObj_methods[] = {
	{"new_recorder", (PyCFunction)recorder_factoryObj_new_recorder, 1,
	 PyDoc_STR("(ambulant::net::pixel_order pixel_order, ambulant::lib::size window_size) -> (ambulant::common::recorder* _rv)")},
	{NULL, NULL, 0}
};

#define recorder_factoryObj_getsetlist NULL


static int recorder_factoryObj_compare(recorder_factoryObject *self, recorder_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define recorder_factoryObj_repr NULL

static long recorder_factoryObj_hash(recorder_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int recorder_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::recorder_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, recorder_factoryObj_Convert, &itself))
	{
		((recorder_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define recorder_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *recorder_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((recorder_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define recorder_factoryObj_tp_free PyObject_Del


PyTypeObject recorder_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.recorder_factory", /*tp_name*/
	sizeof(recorder_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) recorder_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) recorder_factoryObj_compare, /*tp_compare*/
	(reprfunc) recorder_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) recorder_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	recorder_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	recorder_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	recorder_factoryObj_tp_init, /* tp_init */
	recorder_factoryObj_tp_alloc, /* tp_alloc */
	recorder_factoryObj_tp_new, /* tp_new */
	recorder_factoryObj_tp_free, /* tp_free */
};

/* ---------------- End object type recorder_factory ---------------- */


/* ------------------ Object type renderer_select ------------------- */

extern PyTypeObject renderer_select_Type;

inline bool renderer_selectObj_Check(PyObject *x)
{
	return ((x)->ob_type == &renderer_select_Type);
}

typedef struct renderer_selectObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::renderer_select* ob_itself;
} renderer_selectObject;

PyObject *renderer_selectObj_New(ambulant::common::renderer_select* itself)
{
	renderer_selectObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_renderer_select
	renderer_select *encaps_itself = dynamic_cast<renderer_select *>(itself);
	if (encaps_itself && encaps_itself->py_renderer_select)
	{
		Py_INCREF(encaps_itself->py_renderer_select);
		return encaps_itself->py_renderer_select;
	}
#endif
	it = PyObject_NEW(renderer_selectObject, &renderer_select_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int renderer_selectObj_Convert(PyObject *v, ambulant::common::renderer_select* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_renderer_select
	if (!renderer_selectObj_Check(v))
	{
		*p_itself = Py_WrapAs_renderer_select(v);
		if (*p_itself) return 1;
	}
#endif
	if (!renderer_selectObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "renderer_select required");
		return 0;
	}
	*p_itself = ((renderer_selectObject *)v)->ob_itself;
	return 1;
}

static void renderer_selectObj_dealloc(renderer_selectObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *renderer_selectObj_get_tag(renderer_selectObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::lib::xml_string& _rv = _self->ob_itself->get_tag();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *renderer_selectObj_get_url(renderer_selectObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::net::url& _rv = _self->ob_itself->get_url();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_url_New(_rv));
	return _res;
}

static PyObject *renderer_selectObj_get_mimetype(renderer_selectObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const std::string& _rv = _self->ob_itself->get_mimetype();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *renderer_selectObj_get_renderer_uri(renderer_selectObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const char * _rv = _self->ob_itself->get_renderer_uri();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("z",
	                     _rv);
	return _res;
}

static PyObject *renderer_selectObj_get_playable_factory(renderer_selectObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::playable_factory* _rv = _self->ob_itself->get_playable_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}

static PyObject *renderer_selectObj_set_playable_factory(renderer_selectObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* pf;
	if (!PyArg_ParseTuple(_args, "O&",
	                      playable_factoryObj_Convert, &pf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_playable_factory(pf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef renderer_selectObj_methods[] = {
	{"get_tag", (PyCFunction)renderer_selectObj_get_tag, 1,
	 PyDoc_STR("() -> (const ambulant::lib::xml_string& _rv)")},
	{"get_url", (PyCFunction)renderer_selectObj_get_url, 1,
	 PyDoc_STR("() -> (const ambulant::net::url& _rv)")},
	{"get_mimetype", (PyCFunction)renderer_selectObj_get_mimetype, 1,
	 PyDoc_STR("() -> (const std::string& _rv)")},
	{"get_renderer_uri", (PyCFunction)renderer_selectObj_get_renderer_uri, 1,
	 PyDoc_STR("() -> (const char * _rv)")},
	{"get_playable_factory", (PyCFunction)renderer_selectObj_get_playable_factory, 1,
	 PyDoc_STR("() -> (ambulant::common::playable_factory* _rv)")},
	{"set_playable_factory", (PyCFunction)renderer_selectObj_set_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::playable_factory* pf) -> None")},
	{NULL, NULL, 0}
};

#define renderer_selectObj_getsetlist NULL


static int renderer_selectObj_compare(renderer_selectObject *self, renderer_selectObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define renderer_selectObj_repr NULL

static long renderer_selectObj_hash(renderer_selectObject *self)
{
	return (long)self->ob_itself;
}
static int renderer_selectObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::renderer_select* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	{
		ambulant::lib::node* n;
		if (PyArg_ParseTuple(_args, "O&",
		                     nodeObj_Convert, &n))
		{
			((renderer_selectObject *)_self)->ob_itself = new ambulant::common::renderer_select(n);
			return 0;
		}
	}

	{
		char* uri;
		if (PyArg_ParseTuple(_args, "s",
		                     &uri))
		{
			((renderer_selectObject *)_self)->ob_itself = new ambulant::common::renderer_select(uri);
			return 0;
		}
	}

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, renderer_selectObj_Convert, &itself))
	{
		((renderer_selectObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define renderer_selectObj_tp_alloc PyType_GenericAlloc

static PyObject *renderer_selectObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((renderer_selectObject *)_self)->ob_itself = NULL;
	return _self;
}

#define renderer_selectObj_tp_free PyObject_Del


PyTypeObject renderer_select_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.renderer_select", /*tp_name*/
	sizeof(renderer_selectObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) renderer_selectObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) renderer_selectObj_compare, /*tp_compare*/
	(reprfunc) renderer_selectObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) renderer_selectObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	renderer_selectObj_methods, /* tp_methods */
	0, /*tp_members*/
	renderer_selectObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	renderer_selectObj_tp_init, /* tp_init */
	renderer_selectObj_tp_alloc, /* tp_alloc */
	renderer_selectObj_tp_new, /* tp_new */
	renderer_selectObj_tp_free, /* tp_free */
};

/* ---------------- End object type renderer_select ----------------- */


/* ------------------- Object type focus_feedback ------------------- */

extern PyTypeObject focus_feedback_Type;

inline bool focus_feedbackObj_Check(PyObject *x)
{
	return ((x)->ob_type == &focus_feedback_Type);
}

typedef struct focus_feedbackObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::focus_feedback* ob_itself;
} focus_feedbackObject;

PyObject *focus_feedbackObj_New(ambulant::common::focus_feedback* itself)
{
	focus_feedbackObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_focus_feedback
	focus_feedback *encaps_itself = dynamic_cast<focus_feedback *>(itself);
	if (encaps_itself && encaps_itself->py_focus_feedback)
	{
		Py_INCREF(encaps_itself->py_focus_feedback);
		return encaps_itself->py_focus_feedback;
	}
#endif
	it = PyObject_NEW(focus_feedbackObject, &focus_feedback_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int focus_feedbackObj_Convert(PyObject *v, ambulant::common::focus_feedback* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_focus_feedback
	if (!focus_feedbackObj_Check(v))
	{
		*p_itself = Py_WrapAs_focus_feedback(v);
		if (*p_itself) return 1;
	}
#endif
	if (!focus_feedbackObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "focus_feedback required");
		return 0;
	}
	*p_itself = ((focus_feedbackObject *)v)->ob_itself;
	return 1;
}

static void focus_feedbackObj_dealloc(focus_feedbackObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *focus_feedbackObj_node_focussed(focus_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &n))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->node_focussed(n);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef focus_feedbackObj_methods[] = {
	{"node_focussed", (PyCFunction)focus_feedbackObj_node_focussed, 1,
	 PyDoc_STR("(ambulant::lib::node* n) -> None")},
	{NULL, NULL, 0}
};

#define focus_feedbackObj_getsetlist NULL


static int focus_feedbackObj_compare(focus_feedbackObject *self, focus_feedbackObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define focus_feedbackObj_repr NULL

static long focus_feedbackObj_hash(focus_feedbackObject *self)
{
	return (long)self->ob_itself;
}
static int focus_feedbackObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::focus_feedback* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, focus_feedbackObj_Convert, &itself))
	{
		((focus_feedbackObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define focus_feedbackObj_tp_alloc PyType_GenericAlloc

static PyObject *focus_feedbackObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((focus_feedbackObject *)_self)->ob_itself = NULL;
	return _self;
}

#define focus_feedbackObj_tp_free PyObject_Del


PyTypeObject focus_feedback_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.focus_feedback", /*tp_name*/
	sizeof(focus_feedbackObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) focus_feedbackObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) focus_feedbackObj_compare, /*tp_compare*/
	(reprfunc) focus_feedbackObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) focus_feedbackObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	focus_feedbackObj_methods, /* tp_methods */
	0, /*tp_members*/
	focus_feedbackObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	focus_feedbackObj_tp_init, /* tp_init */
	focus_feedbackObj_tp_alloc, /* tp_alloc */
	focus_feedbackObj_tp_new, /* tp_new */
	focus_feedbackObj_tp_free, /* tp_free */
};

/* ----------------- End object type focus_feedback ----------------- */


/* ------------------ Object type player_feedback ------------------- */

extern PyTypeObject player_feedback_Type;

inline bool player_feedbackObj_Check(PyObject *x)
{
	return ((x)->ob_type == &player_feedback_Type);
}

typedef struct player_feedbackObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::player_feedback* ob_itself;
} player_feedbackObject;

PyObject *player_feedbackObj_New(ambulant::common::player_feedback* itself)
{
	player_feedbackObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_player_feedback
	player_feedback *encaps_itself = dynamic_cast<player_feedback *>(itself);
	if (encaps_itself && encaps_itself->py_player_feedback)
	{
		Py_INCREF(encaps_itself->py_player_feedback);
		return encaps_itself->py_player_feedback;
	}
#endif
	it = PyObject_NEW(player_feedbackObject, &player_feedback_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int player_feedbackObj_Convert(PyObject *v, ambulant::common::player_feedback* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_player_feedback
	if (!player_feedbackObj_Check(v))
	{
		*p_itself = Py_WrapAs_player_feedback(v);
		if (*p_itself) return 1;
	}
#endif
	if (!player_feedbackObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "player_feedback required");
		return 0;
	}
	*p_itself = ((player_feedbackObject *)v)->ob_itself;
	return 1;
}

static void player_feedbackObj_dealloc(player_feedbackObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *player_feedbackObj_document_loaded(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::document* doc;
	if (!PyArg_ParseTuple(_args, "O&",
	                      documentObj_Convert, &doc))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->document_loaded(doc);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *player_feedbackObj_document_started(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->document_started();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *player_feedbackObj_document_stopped(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->document_stopped();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *player_feedbackObj_node_started(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &n))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->node_started(n);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *player_feedbackObj_node_filled(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &n))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->node_filled(n);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *player_feedbackObj_node_stopped(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &n))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->node_stopped(n);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *player_feedbackObj_playable_started(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable* p;
	ambulant::lib::node* n;
	char* comment;
	if (!PyArg_ParseTuple(_args, "O&O&s",
	                      playableObj_Convert, &p,
	                      nodeObj_Convert, &n,
	                      &comment))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->playable_started(p,
	                                   n,
	                                   comment);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *player_feedbackObj_playable_stalled(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable* p;
	char* reason;
	if (!PyArg_ParseTuple(_args, "O&s",
	                      playableObj_Convert, &p,
	                      &reason))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->playable_stalled(p,
	                                   reason);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *player_feedbackObj_playable_unstalled(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable* p;
	if (!PyArg_ParseTuple(_args, "O&",
	                      playableObj_Convert, &p))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->playable_unstalled(p);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *player_feedbackObj_playable_cached(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable* p;
	if (!PyArg_ParseTuple(_args, "O&",
	                      playableObj_Convert, &p))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->playable_cached(p);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *player_feedbackObj_playable_deleted(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable* p;
	if (!PyArg_ParseTuple(_args, "O&",
	                      playableObj_Convert, &p))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->playable_deleted(p);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *player_feedbackObj_playable_resource(player_feedbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable* p;
	char* resource;
	long amount;
	if (!PyArg_ParseTuple(_args, "O&sl",
	                      playableObj_Convert, &p,
	                      &resource,
	                      &amount))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->playable_resource(p,
	                                    resource,
	                                    amount);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef player_feedbackObj_methods[] = {
	{"document_loaded", (PyCFunction)player_feedbackObj_document_loaded, 1,
	 PyDoc_STR("(ambulant::lib::document* doc) -> None")},
	{"document_started", (PyCFunction)player_feedbackObj_document_started, 1,
	 PyDoc_STR("() -> None")},
	{"document_stopped", (PyCFunction)player_feedbackObj_document_stopped, 1,
	 PyDoc_STR("() -> None")},
	{"node_started", (PyCFunction)player_feedbackObj_node_started, 1,
	 PyDoc_STR("(ambulant::lib::node* n) -> None")},
	{"node_filled", (PyCFunction)player_feedbackObj_node_filled, 1,
	 PyDoc_STR("(ambulant::lib::node* n) -> None")},
	{"node_stopped", (PyCFunction)player_feedbackObj_node_stopped, 1,
	 PyDoc_STR("(ambulant::lib::node* n) -> None")},
	{"playable_started", (PyCFunction)player_feedbackObj_playable_started, 1,
	 PyDoc_STR("(ambulant::common::playable* p, ambulant::lib::node* n, char* comment) -> None")},
	{"playable_stalled", (PyCFunction)player_feedbackObj_playable_stalled, 1,
	 PyDoc_STR("(ambulant::common::playable* p, char* reason) -> None")},
	{"playable_unstalled", (PyCFunction)player_feedbackObj_playable_unstalled, 1,
	 PyDoc_STR("(ambulant::common::playable* p) -> None")},
	{"playable_cached", (PyCFunction)player_feedbackObj_playable_cached, 1,
	 PyDoc_STR("(ambulant::common::playable* p) -> None")},
	{"playable_deleted", (PyCFunction)player_feedbackObj_playable_deleted, 1,
	 PyDoc_STR("(ambulant::common::playable* p) -> None")},
	{"playable_resource", (PyCFunction)player_feedbackObj_playable_resource, 1,
	 PyDoc_STR("(ambulant::common::playable* p, char* resource, long amount) -> None")},
	{NULL, NULL, 0}
};

#define player_feedbackObj_getsetlist NULL


static int player_feedbackObj_compare(player_feedbackObject *self, player_feedbackObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define player_feedbackObj_repr NULL

static long player_feedbackObj_hash(player_feedbackObject *self)
{
	return (long)self->ob_itself;
}
static int player_feedbackObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::player_feedback* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, player_feedbackObj_Convert, &itself))
	{
		((player_feedbackObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define player_feedbackObj_tp_alloc PyType_GenericAlloc

static PyObject *player_feedbackObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((player_feedbackObject *)_self)->ob_itself = NULL;
	return _self;
}

#define player_feedbackObj_tp_free PyObject_Del


PyTypeObject player_feedback_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.player_feedback", /*tp_name*/
	sizeof(player_feedbackObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) player_feedbackObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) player_feedbackObj_compare, /*tp_compare*/
	(reprfunc) player_feedbackObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) player_feedbackObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	player_feedbackObj_methods, /* tp_methods */
	0, /*tp_members*/
	player_feedbackObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	player_feedbackObj_tp_init, /* tp_init */
	player_feedbackObj_tp_alloc, /* tp_alloc */
	player_feedbackObj_tp_new, /* tp_new */
	player_feedbackObj_tp_free, /* tp_free */
};

/* ---------------- End object type player_feedback ----------------- */


/* ----------------------- Object type player ----------------------- */

extern PyTypeObject player_Type;

inline bool playerObj_Check(PyObject *x)
{
	return ((x)->ob_type == &player_Type);
}

typedef struct playerObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::player* ob_itself;
} playerObject;

PyObject *playerObj_New(ambulant::common::player* itself)
{
	playerObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_player
	player *encaps_itself = dynamic_cast<player *>(itself);
	if (encaps_itself && encaps_itself->py_player)
	{
		Py_INCREF(encaps_itself->py_player);
		return encaps_itself->py_player;
	}
#endif
	it = PyObject_NEW(playerObject, &player_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int playerObj_Convert(PyObject *v, ambulant::common::player* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_player
	if (!playerObj_Check(v))
	{
		*p_itself = Py_WrapAs_player(v);
		if (*p_itself) return 1;
	}
#endif
	if (!playerObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "player required");
		return 0;
	}
	*p_itself = ((playerObject *)v)->ob_itself;
	return 1;
}

static void playerObj_dealloc(playerObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *playerObj_initialize(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->initialize();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_terminate(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->terminate();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_get_timer(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::timer* _rv = _self->ob_itself->get_timer();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     timerObj_New, _rv);
	return _res;
}

static PyObject *playerObj_get_evp(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::event_processor* _rv = _self->ob_itself->get_evp();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     event_processorObj_New, _rv);
	return _res;
}

static PyObject *playerObj_start(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->start();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_stop(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->stop();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_pause(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->pause();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_resume(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->resume();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_is_playing(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_playing();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *playerObj_is_pausing(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_pausing();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *playerObj_is_done(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_done();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *playerObj_after_mousemove(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	int _rv = _self->ob_itself->after_mousemove();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("i",
	                     _rv);
	return _res;
}

static PyObject *playerObj_before_mousemove(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int cursor;
	if (!PyArg_ParseTuple(_args, "i",
	                      &cursor))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->before_mousemove(cursor);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_on_char(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int ch;
	if (!PyArg_ParseTuple(_args, "i",
	                      &ch))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->on_char(ch);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_on_state_change(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* ref;
	if (!PyArg_ParseTuple(_args, "s",
	                      &ref))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->on_state_change(ref);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_get_state_engine(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::state_component* _rv = _self->ob_itself->get_state_engine();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     state_componentObj_New, _rv);
	return _res;
}

static PyObject *playerObj_on_focus_advance(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->on_focus_advance();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_on_focus_activate(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->on_focus_activate();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_set_focus_feedback(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::focus_feedback* fb;
	if (!PyArg_ParseTuple(_args, "O&",
	                      focus_feedbackObj_Convert, &fb))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_focus_feedback(fb);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_set_feedback(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::player_feedback* fb;
	if (!PyArg_ParseTuple(_args, "O&",
	                      player_feedbackObj_Convert, &fb))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_feedback(fb);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *playerObj_get_feedback(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::player_feedback* _rv = _self->ob_itself->get_feedback();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     player_feedbackObj_New, _rv);
	return _res;
}

static PyObject *playerObj_goto_node(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &n))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->goto_node(n);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *playerObj_highlight(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	bool on;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      nodeObj_Convert, &n,
	                      bool_Convert, &on))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->highlight(n,
	                                       on);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

#ifdef WITH_REMOTE_SYNC

static PyObject *playerObj_clicked_external(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* n;
	ambulant::lib::timer::time_type t;
	if (!PyArg_ParseTuple(_args, "O&l",
	                      nodeObj_Convert, &n,
	                      &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->clicked_external(n,
	                                   t);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}
#endif

#ifdef WITH_REMOTE_SYNC

static PyObject *playerObj_uses_external_sync(playerObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->uses_external_sync();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}
#endif

static PyMethodDef playerObj_methods[] = {
	{"initialize", (PyCFunction)playerObj_initialize, 1,
	 PyDoc_STR("() -> None")},
	{"terminate", (PyCFunction)playerObj_terminate, 1,
	 PyDoc_STR("() -> None")},
	{"get_timer", (PyCFunction)playerObj_get_timer, 1,
	 PyDoc_STR("() -> (ambulant::lib::timer* _rv)")},
	{"get_evp", (PyCFunction)playerObj_get_evp, 1,
	 PyDoc_STR("() -> (ambulant::lib::event_processor* _rv)")},
	{"start", (PyCFunction)playerObj_start, 1,
	 PyDoc_STR("() -> None")},
	{"stop", (PyCFunction)playerObj_stop, 1,
	 PyDoc_STR("() -> None")},
	{"pause", (PyCFunction)playerObj_pause, 1,
	 PyDoc_STR("() -> None")},
	{"resume", (PyCFunction)playerObj_resume, 1,
	 PyDoc_STR("() -> None")},
	{"is_playing", (PyCFunction)playerObj_is_playing, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"is_pausing", (PyCFunction)playerObj_is_pausing, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"is_done", (PyCFunction)playerObj_is_done, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"after_mousemove", (PyCFunction)playerObj_after_mousemove, 1,
	 PyDoc_STR("() -> (int _rv)")},
	{"before_mousemove", (PyCFunction)playerObj_before_mousemove, 1,
	 PyDoc_STR("(int cursor) -> None")},
	{"on_char", (PyCFunction)playerObj_on_char, 1,
	 PyDoc_STR("(int ch) -> None")},
	{"on_state_change", (PyCFunction)playerObj_on_state_change, 1,
	 PyDoc_STR("(char* ref) -> None")},
	{"get_state_engine", (PyCFunction)playerObj_get_state_engine, 1,
	 PyDoc_STR("() -> (ambulant::common::state_component* _rv)")},
	{"on_focus_advance", (PyCFunction)playerObj_on_focus_advance, 1,
	 PyDoc_STR("() -> None")},
	{"on_focus_activate", (PyCFunction)playerObj_on_focus_activate, 1,
	 PyDoc_STR("() -> None")},
	{"set_focus_feedback", (PyCFunction)playerObj_set_focus_feedback, 1,
	 PyDoc_STR("(ambulant::common::focus_feedback* fb) -> None")},
	{"set_feedback", (PyCFunction)playerObj_set_feedback, 1,
	 PyDoc_STR("(ambulant::common::player_feedback* fb) -> None")},
	{"get_feedback", (PyCFunction)playerObj_get_feedback, 1,
	 PyDoc_STR("() -> (ambulant::common::player_feedback* _rv)")},
	{"goto_node", (PyCFunction)playerObj_goto_node, 1,
	 PyDoc_STR("(ambulant::lib::node* n) -> (bool _rv)")},
	{"highlight", (PyCFunction)playerObj_highlight, 1,
	 PyDoc_STR("(ambulant::lib::node* n, bool on) -> (bool _rv)")},

#ifdef WITH_REMOTE_SYNC
	{"clicked_external", (PyCFunction)playerObj_clicked_external, 1,
	 PyDoc_STR("(ambulant::lib::node* n, ambulant::lib::timer::time_type t) -> None")},
#endif

#ifdef WITH_REMOTE_SYNC
	{"uses_external_sync", (PyCFunction)playerObj_uses_external_sync, 1,
	 PyDoc_STR("() -> (bool _rv)")},
#endif
	{NULL, NULL, 0}
};

#define playerObj_getsetlist NULL


static int playerObj_compare(playerObject *self, playerObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define playerObj_repr NULL

static long playerObj_hash(playerObject *self)
{
	return (long)self->ob_itself;
}
static int playerObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::player* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, playerObj_Convert, &itself))
	{
		((playerObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define playerObj_tp_alloc PyType_GenericAlloc

static PyObject *playerObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((playerObject *)_self)->ob_itself = NULL;
	return _self;
}

#define playerObj_tp_free PyObject_Del


PyTypeObject player_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.player", /*tp_name*/
	sizeof(playerObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) playerObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) playerObj_compare, /*tp_compare*/
	(reprfunc) playerObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) playerObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	playerObj_methods, /* tp_methods */
	0, /*tp_members*/
	playerObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	playerObj_tp_init, /* tp_init */
	playerObj_tp_alloc, /* tp_alloc */
	playerObj_tp_new, /* tp_new */
	playerObj_tp_free, /* tp_free */
};

/* --------------------- End object type player --------------------- */


/* -------------------- Object type region_info --------------------- */

extern PyTypeObject region_info_Type;

inline bool region_infoObj_Check(PyObject *x)
{
	return ((x)->ob_type == &region_info_Type);
}

typedef struct region_infoObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::region_info* ob_itself;
} region_infoObject;

PyObject *region_infoObj_New(ambulant::common::region_info* itself)
{
	region_infoObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_region_info
	region_info *encaps_itself = dynamic_cast<region_info *>(itself);
	if (encaps_itself && encaps_itself->py_region_info)
	{
		Py_INCREF(encaps_itself->py_region_info);
		return encaps_itself->py_region_info;
	}
#endif
	it = PyObject_NEW(region_infoObject, &region_info_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int region_infoObj_Convert(PyObject *v, ambulant::common::region_info* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_region_info
	if (!region_infoObj_Check(v))
	{
		*p_itself = Py_WrapAs_region_info(v);
		if (*p_itself) return 1;
	}
#endif
	if (!region_infoObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "region_info required");
		return 0;
	}
	*p_itself = ((region_infoObject *)v)->ob_itself;
	return 1;
}

static void region_infoObj_dealloc(region_infoObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *region_infoObj_get_name(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	std::string _rv = _self->ob_itself->get_name();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *region_infoObj_get_rect(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::rect * default_rect;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_rect_Convert, &default_rect))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::rect _rv = _self->ob_itself->get_rect(default_rect);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_rect_New(_rv));
	return _res;
}

static PyObject *region_infoObj_get_fit(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::fit_t _rv = _self->ob_itself->get_fit();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_get_bgcolor(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::color_t _rv = _self->ob_itself->get_bgcolor();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_get_bgopacity(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_bgopacity();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_get_transparent(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->get_transparent();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *region_infoObj_get_zindex(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::zindex_t _rv = _self->ob_itself->get_zindex();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_get_showbackground(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->get_showbackground();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *region_infoObj_is_subregion(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_subregion();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *region_infoObj_get_soundlevel(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_soundlevel();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_get_soundalign(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::sound_alignment _rv = _self->ob_itself->get_soundalign();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_get_tiling(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::tiling _rv = _self->ob_itself->get_tiling();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_get_bgimage(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const char * _rv = _self->ob_itself->get_bgimage();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("z",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_get_crop_rect(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::size srcsize;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_size_Convert, &srcsize))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::rect _rv = _self->ob_itself->get_crop_rect(srcsize);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_rect_New(_rv));
	return _res;
}

static PyObject *region_infoObj_get_mediaopacity(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_mediaopacity();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_get_mediabgopacity(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_mediabgopacity();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_is_chromakey_specified(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->is_chromakey_specified();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *region_infoObj_get_chromakey(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::color_t _rv = _self->ob_itself->get_chromakey();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_get_chromakeytolerance(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::color_t _rv = _self->ob_itself->get_chromakeytolerance();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *region_infoObj_get_chromakeyopacity(region_infoObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_chromakeyopacity();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyMethodDef region_infoObj_methods[] = {
	{"get_name", (PyCFunction)region_infoObj_get_name, 1,
	 PyDoc_STR("() -> (std::string _rv)")},
	{"get_rect", (PyCFunction)region_infoObj_get_rect, 1,
	 PyDoc_STR("(ambulant::lib::rect * default_rect) -> (ambulant::lib::rect _rv)")},
	{"get_fit", (PyCFunction)region_infoObj_get_fit, 1,
	 PyDoc_STR("() -> (ambulant::common::fit_t _rv)")},
	{"get_bgcolor", (PyCFunction)region_infoObj_get_bgcolor, 1,
	 PyDoc_STR("() -> (ambulant::lib::color_t _rv)")},
	{"get_bgopacity", (PyCFunction)region_infoObj_get_bgopacity, 1,
	 PyDoc_STR("() -> (double _rv)")},
	{"get_transparent", (PyCFunction)region_infoObj_get_transparent, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"get_zindex", (PyCFunction)region_infoObj_get_zindex, 1,
	 PyDoc_STR("() -> (ambulant::common::zindex_t _rv)")},
	{"get_showbackground", (PyCFunction)region_infoObj_get_showbackground, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"is_subregion", (PyCFunction)region_infoObj_is_subregion, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"get_soundlevel", (PyCFunction)region_infoObj_get_soundlevel, 1,
	 PyDoc_STR("() -> (double _rv)")},
	{"get_soundalign", (PyCFunction)region_infoObj_get_soundalign, 1,
	 PyDoc_STR("() -> (ambulant::common::sound_alignment _rv)")},
	{"get_tiling", (PyCFunction)region_infoObj_get_tiling, 1,
	 PyDoc_STR("() -> (ambulant::common::tiling _rv)")},
	{"get_bgimage", (PyCFunction)region_infoObj_get_bgimage, 1,
	 PyDoc_STR("() -> (const char * _rv)")},
	{"get_crop_rect", (PyCFunction)region_infoObj_get_crop_rect, 1,
	 PyDoc_STR("(ambulant::lib::size srcsize) -> (ambulant::lib::rect _rv)")},
	{"get_mediaopacity", (PyCFunction)region_infoObj_get_mediaopacity, 1,
	 PyDoc_STR("() -> (double _rv)")},
	{"get_mediabgopacity", (PyCFunction)region_infoObj_get_mediabgopacity, 1,
	 PyDoc_STR("() -> (double _rv)")},
	{"is_chromakey_specified", (PyCFunction)region_infoObj_is_chromakey_specified, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"get_chromakey", (PyCFunction)region_infoObj_get_chromakey, 1,
	 PyDoc_STR("() -> (ambulant::lib::color_t _rv)")},
	{"get_chromakeytolerance", (PyCFunction)region_infoObj_get_chromakeytolerance, 1,
	 PyDoc_STR("() -> (ambulant::lib::color_t _rv)")},
	{"get_chromakeyopacity", (PyCFunction)region_infoObj_get_chromakeyopacity, 1,
	 PyDoc_STR("() -> (double _rv)")},
	{NULL, NULL, 0}
};

#define region_infoObj_getsetlist NULL


static int region_infoObj_compare(region_infoObject *self, region_infoObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define region_infoObj_repr NULL

static long region_infoObj_hash(region_infoObject *self)
{
	return (long)self->ob_itself;
}
static int region_infoObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::region_info* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, region_infoObj_Convert, &itself))
	{
		((region_infoObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define region_infoObj_tp_alloc PyType_GenericAlloc

static PyObject *region_infoObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((region_infoObject *)_self)->ob_itself = NULL;
	return _self;
}

#define region_infoObj_tp_free PyObject_Del


PyTypeObject region_info_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.region_info", /*tp_name*/
	sizeof(region_infoObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) region_infoObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) region_infoObj_compare, /*tp_compare*/
	(reprfunc) region_infoObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) region_infoObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	region_infoObj_methods, /* tp_methods */
	0, /*tp_members*/
	region_infoObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	region_infoObj_tp_init, /* tp_init */
	region_infoObj_tp_alloc, /* tp_alloc */
	region_infoObj_tp_new, /* tp_new */
	region_infoObj_tp_free, /* tp_free */
};

/* ------------------ End object type region_info ------------------- */


/* --------------- Object type animation_destination ---------------- */

extern PyTypeObject animation_destination_Type;

inline bool animation_destinationObj_Check(PyObject *x)
{
	return ((x)->ob_type == &animation_destination_Type);
}

typedef struct animation_destinationObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::animation_destination* ob_itself;
} animation_destinationObject;

PyObject *animation_destinationObj_New(ambulant::common::animation_destination* itself)
{
	animation_destinationObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_animation_destination
	animation_destination *encaps_itself = dynamic_cast<animation_destination *>(itself);
	if (encaps_itself && encaps_itself->py_animation_destination)
	{
		Py_INCREF(encaps_itself->py_animation_destination);
		return encaps_itself->py_animation_destination;
	}
#endif
	it = PyObject_NEW(animation_destinationObject, &animation_destination_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int animation_destinationObj_Convert(PyObject *v, ambulant::common::animation_destination* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_animation_destination
	if (!animation_destinationObj_Check(v))
	{
		*p_itself = Py_WrapAs_animation_destination(v);
		if (*p_itself) return 1;
	}
#endif
	if (!animation_destinationObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "animation_destination required");
		return 0;
	}
	*p_itself = ((animation_destinationObject *)v)->ob_itself;
	return 1;
}

static void animation_destinationObj_dealloc(animation_destinationObject *self)
{
	region_info_Type.tp_dealloc((PyObject *)self);
}

static PyObject *animation_destinationObj_get_region_dim(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string which;
	bool fromdom;
	char *which_cstr="";
	if (!PyArg_ParseTuple(_args, "sO&",
	                      &which_cstr,
	                      bool_Convert, &fromdom))
		return NULL;
	which = which_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::region_dim _rv = _self->ob_itself->get_region_dim(which,
	                                                                    fromdom);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O",
	                     ambulant_region_dim_New(_rv));
	return _res;
}

static PyObject *animation_destinationObj_get_region_color(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string which;
	bool fromdom;
	char *which_cstr="";
	if (!PyArg_ParseTuple(_args, "sO&",
	                      &which_cstr,
	                      bool_Convert, &fromdom))
		return NULL;
	which = which_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::color_t _rv = _self->ob_itself->get_region_color(which,
	                                                                fromdom);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *animation_destinationObj_get_region_zindex(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool fromdom;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &fromdom))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::zindex_t _rv = _self->ob_itself->get_region_zindex(fromdom);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *animation_destinationObj_get_region_soundlevel(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool fromdom;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &fromdom))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_region_soundlevel(fromdom);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyObject *animation_destinationObj_get_region_soundalign(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool fromdom;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &fromdom))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::sound_alignment _rv = _self->ob_itself->get_region_soundalign(fromdom);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *animation_destinationObj_get_region_opacity(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string which;
	bool fromdom;
	char *which_cstr="";
	if (!PyArg_ParseTuple(_args, "sO&",
	                      &which_cstr,
	                      bool_Convert, &fromdom))
		return NULL;
	which = which_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	double _rv = _self->ob_itself->get_region_opacity(which,
	                                                  fromdom);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("d",
	                     _rv);
	return _res;
}

static PyObject *animation_destinationObj_set_region_dim(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string which;
	ambulant::common::region_dim rd;
	char *which_cstr="";
	if (!PyArg_ParseTuple(_args, "sO&",
	                      &which_cstr,
	                      ambulant_region_dim_Convert, &rd))
		return NULL;
	which = which_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_region_dim(which,
	                                 rd);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *animation_destinationObj_set_region_color(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string which;
	ambulant::lib::color_t clr;
	char *which_cstr="";
	if (!PyArg_ParseTuple(_args, "sl",
	                      &which_cstr,
	                      &clr))
		return NULL;
	which = which_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_region_color(which,
	                                   clr);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *animation_destinationObj_set_region_zindex(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::zindex_t z;
	if (!PyArg_ParseTuple(_args, "l",
	                      &z))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_region_zindex(z);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *animation_destinationObj_set_region_soundlevel(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	double level;
	if (!PyArg_ParseTuple(_args, "d",
	                      &level))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_region_soundlevel(level);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *animation_destinationObj_set_region_soundalign(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::sound_alignment sa;
	if (!PyArg_ParseTuple(_args, "l",
	                      &sa))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_region_soundalign(sa);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *animation_destinationObj_set_region_opacity(animation_destinationObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string which;
	double level;
	char *which_cstr="";
	if (!PyArg_ParseTuple(_args, "sd",
	                      &which_cstr,
	                      &level))
		return NULL;
	which = which_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_region_opacity(which,
	                                     level);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef animation_destinationObj_methods[] = {
	{"get_region_dim", (PyCFunction)animation_destinationObj_get_region_dim, 1,
	 PyDoc_STR("(std::string which, bool fromdom) -> (ambulant::common::region_dim _rv)")},
	{"get_region_color", (PyCFunction)animation_destinationObj_get_region_color, 1,
	 PyDoc_STR("(std::string which, bool fromdom) -> (ambulant::lib::color_t _rv)")},
	{"get_region_zindex", (PyCFunction)animation_destinationObj_get_region_zindex, 1,
	 PyDoc_STR("(bool fromdom) -> (ambulant::common::zindex_t _rv)")},
	{"get_region_soundlevel", (PyCFunction)animation_destinationObj_get_region_soundlevel, 1,
	 PyDoc_STR("(bool fromdom) -> (double _rv)")},
	{"get_region_soundalign", (PyCFunction)animation_destinationObj_get_region_soundalign, 1,
	 PyDoc_STR("(bool fromdom) -> (ambulant::common::sound_alignment _rv)")},
	{"get_region_opacity", (PyCFunction)animation_destinationObj_get_region_opacity, 1,
	 PyDoc_STR("(std::string which, bool fromdom) -> (double _rv)")},
	{"set_region_dim", (PyCFunction)animation_destinationObj_set_region_dim, 1,
	 PyDoc_STR("(std::string which, ambulant::common::region_dim rd) -> None")},
	{"set_region_color", (PyCFunction)animation_destinationObj_set_region_color, 1,
	 PyDoc_STR("(std::string which, ambulant::lib::color_t clr) -> None")},
	{"set_region_zindex", (PyCFunction)animation_destinationObj_set_region_zindex, 1,
	 PyDoc_STR("(ambulant::common::zindex_t z) -> None")},
	{"set_region_soundlevel", (PyCFunction)animation_destinationObj_set_region_soundlevel, 1,
	 PyDoc_STR("(double level) -> None")},
	{"set_region_soundalign", (PyCFunction)animation_destinationObj_set_region_soundalign, 1,
	 PyDoc_STR("(ambulant::common::sound_alignment sa) -> None")},
	{"set_region_opacity", (PyCFunction)animation_destinationObj_set_region_opacity, 1,
	 PyDoc_STR("(std::string which, double level) -> None")},
	{NULL, NULL, 0}
};

#define animation_destinationObj_getsetlist NULL


static int animation_destinationObj_compare(animation_destinationObject *self, animation_destinationObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define animation_destinationObj_repr NULL

static long animation_destinationObj_hash(animation_destinationObject *self)
{
	return (long)self->ob_itself;
}
static int animation_destinationObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::animation_destination* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, animation_destinationObj_Convert, &itself))
	{
		((animation_destinationObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define animation_destinationObj_tp_alloc PyType_GenericAlloc

static PyObject *animation_destinationObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((animation_destinationObject *)_self)->ob_itself = NULL;
	return _self;
}

#define animation_destinationObj_tp_free PyObject_Del


PyTypeObject animation_destination_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.animation_destination", /*tp_name*/
	sizeof(animation_destinationObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) animation_destinationObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) animation_destinationObj_compare, /*tp_compare*/
	(reprfunc) animation_destinationObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) animation_destinationObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	animation_destinationObj_methods, /* tp_methods */
	0, /*tp_members*/
	animation_destinationObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	animation_destinationObj_tp_init, /* tp_init */
	animation_destinationObj_tp_alloc, /* tp_alloc */
	animation_destinationObj_tp_new, /* tp_new */
	animation_destinationObj_tp_free, /* tp_free */
};

/* ------------- End object type animation_destination -------------- */


/* ----------------- Object type state_test_methods ----------------- */

extern PyTypeObject state_test_methods_Type;

inline bool state_test_methodsObj_Check(PyObject *x)
{
	return ((x)->ob_type == &state_test_methods_Type);
}

typedef struct state_test_methodsObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::state_test_methods* ob_itself;
} state_test_methodsObject;

PyObject *state_test_methodsObj_New(ambulant::common::state_test_methods* itself)
{
	state_test_methodsObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_state_test_methods
	state_test_methods *encaps_itself = dynamic_cast<state_test_methods *>(itself);
	if (encaps_itself && encaps_itself->py_state_test_methods)
	{
		Py_INCREF(encaps_itself->py_state_test_methods);
		return encaps_itself->py_state_test_methods;
	}
#endif
	it = PyObject_NEW(state_test_methodsObject, &state_test_methods_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int state_test_methodsObj_Convert(PyObject *v, ambulant::common::state_test_methods* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_state_test_methods
	if (!state_test_methodsObj_Check(v))
	{
		*p_itself = Py_WrapAs_state_test_methods(v);
		if (*p_itself) return 1;
	}
#endif
	if (!state_test_methodsObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "state_test_methods required");
		return 0;
	}
	*p_itself = ((state_test_methodsObject *)v)->ob_itself;
	return 1;
}

static void state_test_methodsObj_dealloc(state_test_methodsObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *state_test_methodsObj_smil_audio_desc(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->smil_audio_desc();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *state_test_methodsObj_smil_bitrate(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	int _rv = _self->ob_itself->smil_bitrate();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("i",
	                     _rv);
	return _res;
}

static PyObject *state_test_methodsObj_smil_captions(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->smil_captions();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *state_test_methodsObj_smil_component(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string uri;
	char *uri_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &uri_cstr))
		return NULL;
	uri = uri_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->smil_component(uri);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *state_test_methodsObj_smil_custom_test(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string name;
	char *name_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &name_cstr))
		return NULL;
	name = name_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->smil_custom_test(name);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *state_test_methodsObj_smil_cpu(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	std::string _rv = _self->ob_itself->smil_cpu();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *state_test_methodsObj_smil_language(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string lang;
	char *lang_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &lang_cstr))
		return NULL;
	lang = lang_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	float _rv = _self->ob_itself->smil_language(lang);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("f",
	                     _rv);
	return _res;
}

static PyObject *state_test_methodsObj_smil_operating_system(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	std::string _rv = _self->ob_itself->smil_operating_system();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *state_test_methodsObj_smil_overdub_or_subtitle(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	std::string _rv = _self->ob_itself->smil_overdub_or_subtitle();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *state_test_methodsObj_smil_required(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string uri;
	char *uri_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &uri_cstr))
		return NULL;
	uri = uri_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->smil_required(uri);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *state_test_methodsObj_smil_screen_depth(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	int _rv = _self->ob_itself->smil_screen_depth();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("i",
	                     _rv);
	return _res;
}

static PyObject *state_test_methodsObj_smil_screen_height(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	int _rv = _self->ob_itself->smil_screen_height();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("i",
	                     _rv);
	return _res;
}

static PyObject *state_test_methodsObj_smil_screen_width(state_test_methodsObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	int _rv = _self->ob_itself->smil_screen_width();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("i",
	                     _rv);
	return _res;
}

static PyMethodDef state_test_methodsObj_methods[] = {
	{"smil_audio_desc", (PyCFunction)state_test_methodsObj_smil_audio_desc, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"smil_bitrate", (PyCFunction)state_test_methodsObj_smil_bitrate, 1,
	 PyDoc_STR("() -> (int _rv)")},
	{"smil_captions", (PyCFunction)state_test_methodsObj_smil_captions, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"smil_component", (PyCFunction)state_test_methodsObj_smil_component, 1,
	 PyDoc_STR("(std::string uri) -> (bool _rv)")},
	{"smil_custom_test", (PyCFunction)state_test_methodsObj_smil_custom_test, 1,
	 PyDoc_STR("(std::string name) -> (bool _rv)")},
	{"smil_cpu", (PyCFunction)state_test_methodsObj_smil_cpu, 1,
	 PyDoc_STR("() -> (std::string _rv)")},
	{"smil_language", (PyCFunction)state_test_methodsObj_smil_language, 1,
	 PyDoc_STR("(std::string lang) -> (float _rv)")},
	{"smil_operating_system", (PyCFunction)state_test_methodsObj_smil_operating_system, 1,
	 PyDoc_STR("() -> (std::string _rv)")},
	{"smil_overdub_or_subtitle", (PyCFunction)state_test_methodsObj_smil_overdub_or_subtitle, 1,
	 PyDoc_STR("() -> (std::string _rv)")},
	{"smil_required", (PyCFunction)state_test_methodsObj_smil_required, 1,
	 PyDoc_STR("(std::string uri) -> (bool _rv)")},
	{"smil_screen_depth", (PyCFunction)state_test_methodsObj_smil_screen_depth, 1,
	 PyDoc_STR("() -> (int _rv)")},
	{"smil_screen_height", (PyCFunction)state_test_methodsObj_smil_screen_height, 1,
	 PyDoc_STR("() -> (int _rv)")},
	{"smil_screen_width", (PyCFunction)state_test_methodsObj_smil_screen_width, 1,
	 PyDoc_STR("() -> (int _rv)")},
	{NULL, NULL, 0}
};

#define state_test_methodsObj_getsetlist NULL


static int state_test_methodsObj_compare(state_test_methodsObject *self, state_test_methodsObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define state_test_methodsObj_repr NULL

static long state_test_methodsObj_hash(state_test_methodsObject *self)
{
	return (long)self->ob_itself;
}
static int state_test_methodsObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::state_test_methods* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, state_test_methodsObj_Convert, &itself))
	{
		((state_test_methodsObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define state_test_methodsObj_tp_alloc PyType_GenericAlloc

static PyObject *state_test_methodsObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((state_test_methodsObject *)_self)->ob_itself = NULL;
	return _self;
}

#define state_test_methodsObj_tp_free PyObject_Del


PyTypeObject state_test_methods_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.state_test_methods", /*tp_name*/
	sizeof(state_test_methodsObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) state_test_methodsObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) state_test_methodsObj_compare, /*tp_compare*/
	(reprfunc) state_test_methodsObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) state_test_methodsObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	state_test_methodsObj_methods, /* tp_methods */
	0, /*tp_members*/
	state_test_methodsObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	state_test_methodsObj_tp_init, /* tp_init */
	state_test_methodsObj_tp_alloc, /* tp_alloc */
	state_test_methodsObj_tp_new, /* tp_new */
	state_test_methodsObj_tp_free, /* tp_free */
};

/* --------------- End object type state_test_methods --------------- */


/* --------------- Object type state_change_callback ---------------- */

extern PyTypeObject state_change_callback_Type;

inline bool state_change_callbackObj_Check(PyObject *x)
{
	return ((x)->ob_type == &state_change_callback_Type);
}

typedef struct state_change_callbackObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::state_change_callback* ob_itself;
} state_change_callbackObject;

PyObject *state_change_callbackObj_New(ambulant::common::state_change_callback* itself)
{
	state_change_callbackObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_state_change_callback
	state_change_callback *encaps_itself = dynamic_cast<state_change_callback *>(itself);
	if (encaps_itself && encaps_itself->py_state_change_callback)
	{
		Py_INCREF(encaps_itself->py_state_change_callback);
		return encaps_itself->py_state_change_callback;
	}
#endif
	it = PyObject_NEW(state_change_callbackObject, &state_change_callback_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int state_change_callbackObj_Convert(PyObject *v, ambulant::common::state_change_callback* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_state_change_callback
	if (!state_change_callbackObj_Check(v))
	{
		*p_itself = Py_WrapAs_state_change_callback(v);
		if (*p_itself) return 1;
	}
#endif
	if (!state_change_callbackObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "state_change_callback required");
		return 0;
	}
	*p_itself = ((state_change_callbackObject *)v)->ob_itself;
	return 1;
}

static void state_change_callbackObj_dealloc(state_change_callbackObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *state_change_callbackObj_on_state_change(state_change_callbackObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* ref;
	if (!PyArg_ParseTuple(_args, "s",
	                      &ref))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->on_state_change(ref);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef state_change_callbackObj_methods[] = {
	{"on_state_change", (PyCFunction)state_change_callbackObj_on_state_change, 1,
	 PyDoc_STR("(char* ref) -> None")},
	{NULL, NULL, 0}
};

#define state_change_callbackObj_getsetlist NULL


static int state_change_callbackObj_compare(state_change_callbackObject *self, state_change_callbackObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define state_change_callbackObj_repr NULL

static long state_change_callbackObj_hash(state_change_callbackObject *self)
{
	return (long)self->ob_itself;
}
static int state_change_callbackObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::state_change_callback* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, state_change_callbackObj_Convert, &itself))
	{
		((state_change_callbackObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define state_change_callbackObj_tp_alloc PyType_GenericAlloc

static PyObject *state_change_callbackObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((state_change_callbackObject *)_self)->ob_itself = NULL;
	return _self;
}

#define state_change_callbackObj_tp_free PyObject_Del


PyTypeObject state_change_callback_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.state_change_callback", /*tp_name*/
	sizeof(state_change_callbackObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) state_change_callbackObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) state_change_callbackObj_compare, /*tp_compare*/
	(reprfunc) state_change_callbackObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) state_change_callbackObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	state_change_callbackObj_methods, /* tp_methods */
	0, /*tp_members*/
	state_change_callbackObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	state_change_callbackObj_tp_init, /* tp_init */
	state_change_callbackObj_tp_alloc, /* tp_alloc */
	state_change_callbackObj_tp_new, /* tp_new */
	state_change_callbackObj_tp_free, /* tp_free */
};

/* ------------- End object type state_change_callback -------------- */


/* ------------------ Object type state_component ------------------- */

extern PyTypeObject state_component_Type;

inline bool state_componentObj_Check(PyObject *x)
{
	return ((x)->ob_type == &state_component_Type);
}

typedef struct state_componentObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::state_component* ob_itself;
} state_componentObject;

PyObject *state_componentObj_New(ambulant::common::state_component* itself)
{
	state_componentObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_state_component
	state_component *encaps_itself = dynamic_cast<state_component *>(itself);
	if (encaps_itself && encaps_itself->py_state_component)
	{
		Py_INCREF(encaps_itself->py_state_component);
		return encaps_itself->py_state_component;
	}
#endif
	it = PyObject_NEW(state_componentObject, &state_component_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int state_componentObj_Convert(PyObject *v, ambulant::common::state_component* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_state_component
	if (!state_componentObj_Check(v))
	{
		*p_itself = Py_WrapAs_state_component(v);
		if (*p_itself) return 1;
	}
#endif
	if (!state_componentObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "state_component required");
		return 0;
	}
	*p_itself = ((state_componentObject *)v)->ob_itself;
	return 1;
}

static void state_componentObj_dealloc(state_componentObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *state_componentObj_register_state_test_methods(state_componentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::state_test_methods* stm;
	if (!PyArg_ParseTuple(_args, "O&",
	                      state_test_methodsObj_Convert, &stm))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->register_state_test_methods(stm);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *state_componentObj_declare_state(state_componentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* state;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &state))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->declare_state(state);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *state_componentObj_bool_expression(state_componentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* expr;
	if (!PyArg_ParseTuple(_args, "s",
	                      &expr))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->bool_expression(expr);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *state_componentObj_set_value(state_componentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* var;
	char* expr;
	if (!PyArg_ParseTuple(_args, "ss",
	                      &var,
	                      &expr))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->set_value(var,
	                            expr);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *state_componentObj_new_value(state_componentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* ref;
	char* where;
	char* name;
	char* expr;
	if (!PyArg_ParseTuple(_args, "ssss",
	                      &ref,
	                      &where,
	                      &name,
	                      &expr))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->new_value(ref,
	                            where,
	                            name,
	                            expr);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *state_componentObj_del_value(state_componentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* ref;
	if (!PyArg_ParseTuple(_args, "s",
	                      &ref))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->del_value(ref);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *state_componentObj_send(state_componentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node* submission;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &submission))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->send(submission);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *state_componentObj_string_expression(state_componentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* expr;
	if (!PyArg_ParseTuple(_args, "s",
	                      &expr))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	std::string _rv = _self->ob_itself->string_expression(expr);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyObject *state_componentObj_want_state_change(state_componentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* ref;
	ambulant::common::state_change_callback* cb;
	if (!PyArg_ParseTuple(_args, "sO&",
	                      &ref,
	                      state_change_callbackObj_Convert, &cb))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->want_state_change(ref,
	                                    cb);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *state_componentObj_getsubtree(state_componentObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* ref;
	bool as_query;
	if (!PyArg_ParseTuple(_args, "sO&",
	                      &ref,
	                      bool_Convert, &as_query))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	std::string _rv = _self->ob_itself->getsubtree(ref,
	                                               as_query);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("s",
	                     _rv.c_str());
	return _res;
}

static PyMethodDef state_componentObj_methods[] = {
	{"register_state_test_methods", (PyCFunction)state_componentObj_register_state_test_methods, 1,
	 PyDoc_STR("(ambulant::common::state_test_methods* stm) -> None")},
	{"declare_state", (PyCFunction)state_componentObj_declare_state, 1,
	 PyDoc_STR("(ambulant::lib::node* state) -> None")},
	{"bool_expression", (PyCFunction)state_componentObj_bool_expression, 1,
	 PyDoc_STR("(char* expr) -> (bool _rv)")},
	{"set_value", (PyCFunction)state_componentObj_set_value, 1,
	 PyDoc_STR("(char* var, char* expr) -> None")},
	{"new_value", (PyCFunction)state_componentObj_new_value, 1,
	 PyDoc_STR("(char* ref, char* where, char* name, char* expr) -> None")},
	{"del_value", (PyCFunction)state_componentObj_del_value, 1,
	 PyDoc_STR("(char* ref) -> None")},
	{"send", (PyCFunction)state_componentObj_send, 1,
	 PyDoc_STR("(ambulant::lib::node* submission) -> None")},
	{"string_expression", (PyCFunction)state_componentObj_string_expression, 1,
	 PyDoc_STR("(char* expr) -> (std::string _rv)")},
	{"want_state_change", (PyCFunction)state_componentObj_want_state_change, 1,
	 PyDoc_STR("(char* ref, ambulant::common::state_change_callback* cb) -> None")},
	{"getsubtree", (PyCFunction)state_componentObj_getsubtree, 1,
	 PyDoc_STR("(char* ref, bool as_query) -> (std::string _rv)")},
	{NULL, NULL, 0}
};

#define state_componentObj_getsetlist NULL


static int state_componentObj_compare(state_componentObject *self, state_componentObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define state_componentObj_repr NULL

static long state_componentObj_hash(state_componentObject *self)
{
	return (long)self->ob_itself;
}
static int state_componentObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::state_component* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, state_componentObj_Convert, &itself))
	{
		((state_componentObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define state_componentObj_tp_alloc PyType_GenericAlloc

static PyObject *state_componentObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((state_componentObject *)_self)->ob_itself = NULL;
	return _self;
}

#define state_componentObj_tp_free PyObject_Del


PyTypeObject state_component_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.state_component", /*tp_name*/
	sizeof(state_componentObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) state_componentObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) state_componentObj_compare, /*tp_compare*/
	(reprfunc) state_componentObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) state_componentObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	state_componentObj_methods, /* tp_methods */
	0, /*tp_members*/
	state_componentObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	state_componentObj_tp_init, /* tp_init */
	state_componentObj_tp_alloc, /* tp_alloc */
	state_componentObj_tp_new, /* tp_new */
	state_componentObj_tp_free, /* tp_free */
};

/* ---------------- End object type state_component ----------------- */


/* -------------- Object type state_component_factory --------------- */

extern PyTypeObject state_component_factory_Type;

inline bool state_component_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &state_component_factory_Type);
}

typedef struct state_component_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::state_component_factory* ob_itself;
} state_component_factoryObject;

PyObject *state_component_factoryObj_New(ambulant::common::state_component_factory* itself)
{
	state_component_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_state_component_factory
	state_component_factory *encaps_itself = dynamic_cast<state_component_factory *>(itself);
	if (encaps_itself && encaps_itself->py_state_component_factory)
	{
		Py_INCREF(encaps_itself->py_state_component_factory);
		return encaps_itself->py_state_component_factory;
	}
#endif
	it = PyObject_NEW(state_component_factoryObject, &state_component_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int state_component_factoryObj_Convert(PyObject *v, ambulant::common::state_component_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_state_component_factory
	if (!state_component_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_state_component_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!state_component_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "state_component_factory required");
		return 0;
	}
	*p_itself = ((state_component_factoryObject *)v)->ob_itself;
	return 1;
}

static void state_component_factoryObj_dealloc(state_component_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *state_component_factoryObj_new_state_component(state_component_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	char* uri;
	if (!PyArg_ParseTuple(_args, "s",
	                      &uri))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::state_component* _rv = _self->ob_itself->new_state_component(uri);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     state_componentObj_New, _rv);
	return _res;
}

static PyMethodDef state_component_factoryObj_methods[] = {
	{"new_state_component", (PyCFunction)state_component_factoryObj_new_state_component, 1,
	 PyDoc_STR("(char* uri) -> (ambulant::common::state_component* _rv)")},
	{NULL, NULL, 0}
};

#define state_component_factoryObj_getsetlist NULL


static int state_component_factoryObj_compare(state_component_factoryObject *self, state_component_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define state_component_factoryObj_repr NULL

static long state_component_factoryObj_hash(state_component_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int state_component_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::state_component_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, state_component_factoryObj_Convert, &itself))
	{
		((state_component_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define state_component_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *state_component_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((state_component_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define state_component_factoryObj_tp_free PyObject_Del


PyTypeObject state_component_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.state_component_factory", /*tp_name*/
	sizeof(state_component_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) state_component_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) state_component_factoryObj_compare, /*tp_compare*/
	(reprfunc) state_component_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) state_component_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	state_component_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	state_component_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	state_component_factoryObj_tp_init, /* tp_init */
	state_component_factoryObj_tp_alloc, /* tp_alloc */
	state_component_factoryObj_tp_new, /* tp_new */
	state_component_factoryObj_tp_free, /* tp_free */
};

/* ------------ End object type state_component_factory ------------- */


/* ----------- Object type global_state_component_factory ----------- */

extern PyTypeObject global_state_component_factory_Type;

inline bool global_state_component_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &global_state_component_factory_Type);
}

typedef struct global_state_component_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::common::global_state_component_factory* ob_itself;
} global_state_component_factoryObject;

PyObject *global_state_component_factoryObj_New(ambulant::common::global_state_component_factory* itself)
{
	global_state_component_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_global_state_component_factory
	global_state_component_factory *encaps_itself = dynamic_cast<global_state_component_factory *>(itself);
	if (encaps_itself && encaps_itself->py_global_state_component_factory)
	{
		Py_INCREF(encaps_itself->py_global_state_component_factory);
		return encaps_itself->py_global_state_component_factory;
	}
#endif
	it = PyObject_NEW(global_state_component_factoryObject, &global_state_component_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int global_state_component_factoryObj_Convert(PyObject *v, ambulant::common::global_state_component_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_global_state_component_factory
	if (!global_state_component_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_global_state_component_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!global_state_component_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "global_state_component_factory required");
		return 0;
	}
	*p_itself = ((global_state_component_factoryObject *)v)->ob_itself;
	return 1;
}

static void global_state_component_factoryObj_dealloc(global_state_component_factoryObject *self)
{
	state_component_factory_Type.tp_dealloc((PyObject *)self);
}

static PyObject *global_state_component_factoryObj_add_factory(global_state_component_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::state_component_factory* sf;
	if (!PyArg_ParseTuple(_args, "O&",
	                      state_component_factoryObj_Convert, &sf))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->add_factory(sf);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef global_state_component_factoryObj_methods[] = {
	{"add_factory", (PyCFunction)global_state_component_factoryObj_add_factory, 1,
	 PyDoc_STR("(ambulant::common::state_component_factory* sf) -> None")},
	{NULL, NULL, 0}
};

#define global_state_component_factoryObj_getsetlist NULL


static int global_state_component_factoryObj_compare(global_state_component_factoryObject *self, global_state_component_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define global_state_component_factoryObj_repr NULL

static long global_state_component_factoryObj_hash(global_state_component_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int global_state_component_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::common::global_state_component_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, global_state_component_factoryObj_Convert, &itself))
	{
		((global_state_component_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define global_state_component_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *global_state_component_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((global_state_component_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define global_state_component_factoryObj_tp_free PyObject_Del


PyTypeObject global_state_component_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.global_state_component_factory", /*tp_name*/
	sizeof(global_state_component_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) global_state_component_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) global_state_component_factoryObj_compare, /*tp_compare*/
	(reprfunc) global_state_component_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) global_state_component_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	global_state_component_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	global_state_component_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	global_state_component_factoryObj_tp_init, /* tp_init */
	global_state_component_factoryObj_tp_alloc, /* tp_alloc */
	global_state_component_factoryObj_tp_new, /* tp_new */
	global_state_component_factoryObj_tp_free, /* tp_free */
};

/* --------- End object type global_state_component_factory --------- */


/* -------------------- Object type none_window --------------------- */

extern PyTypeObject none_window_Type;

inline bool none_windowObj_Check(PyObject *x)
{
	return ((x)->ob_type == &none_window_Type);
}

typedef struct none_windowObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::gui::none::none_window* ob_itself;
} none_windowObject;

PyObject *none_windowObj_New(ambulant::gui::none::none_window* itself)
{
	none_windowObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_none_window
	none_window *encaps_itself = dynamic_cast<none_window *>(itself);
	if (encaps_itself && encaps_itself->py_none_window)
	{
		Py_INCREF(encaps_itself->py_none_window);
		return encaps_itself->py_none_window;
	}
#endif
	it = PyObject_NEW(none_windowObject, &none_window_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int none_windowObj_Convert(PyObject *v, ambulant::gui::none::none_window* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_none_window
	if (!none_windowObj_Check(v))
	{
		*p_itself = Py_WrapAs_none_window(v);
		if (*p_itself) return 1;
	}
#endif
	if (!none_windowObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "none_window required");
		return 0;
	}
	*p_itself = ((none_windowObject *)v)->ob_itself;
	return 1;
}

static void none_windowObj_dealloc(none_windowObject *self)
{
	gui_window_Type.tp_dealloc((PyObject *)self);
}

static PyObject *none_windowObj_need_redraw(none_windowObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::rect r;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_rect_Convert, &r))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->need_redraw(r);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *none_windowObj_need_events(none_windowObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool want;
	if (!PyArg_ParseTuple(_args, "O&",
	                      bool_Convert, &want))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->need_events(want);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *none_windowObj_redraw_now(none_windowObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->redraw_now();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef none_windowObj_methods[] = {
	{"need_redraw", (PyCFunction)none_windowObj_need_redraw, 1,
	 PyDoc_STR("(ambulant::lib::rect r) -> None")},
	{"need_events", (PyCFunction)none_windowObj_need_events, 1,
	 PyDoc_STR("(bool want) -> None")},
	{"redraw_now", (PyCFunction)none_windowObj_redraw_now, 1,
	 PyDoc_STR("() -> None")},
	{NULL, NULL, 0}
};

#define none_windowObj_getsetlist NULL


static int none_windowObj_compare(none_windowObject *self, none_windowObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define none_windowObj_repr NULL

static long none_windowObj_hash(none_windowObject *self)
{
	return (long)self->ob_itself;
}
static int none_windowObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::gui::none::none_window* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	{
		std::string name;
		ambulant::lib::size bounds;
		ambulant::common::gui_events* handler;
		char *name_cstr="";
		if (PyArg_ParseTuple(_args, "sO&O&",
		                     &name_cstr,
		                     ambulant_size_Convert, &bounds,
		                     gui_eventsObj_Convert, &handler))
		{
			name = name_cstr;
			((none_windowObject *)_self)->ob_itself = new ambulant::gui::none::none_window(name,
			                                                                               bounds,
			                                                                               handler);
			return 0;
		}
	}

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, none_windowObj_Convert, &itself))
	{
		((none_windowObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define none_windowObj_tp_alloc PyType_GenericAlloc

static PyObject *none_windowObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((none_windowObject *)_self)->ob_itself = NULL;
	return _self;
}

#define none_windowObj_tp_free PyObject_Del


PyTypeObject none_window_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.none_window", /*tp_name*/
	sizeof(none_windowObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) none_windowObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) none_windowObj_compare, /*tp_compare*/
	(reprfunc) none_windowObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) none_windowObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	none_windowObj_methods, /* tp_methods */
	0, /*tp_members*/
	none_windowObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	none_windowObj_tp_init, /* tp_init */
	none_windowObj_tp_alloc, /* tp_alloc */
	none_windowObj_tp_new, /* tp_new */
	none_windowObj_tp_free, /* tp_free */
};

/* ------------------ End object type none_window ------------------- */


/* ---------------- Object type none_window_factory ----------------- */

extern PyTypeObject none_window_factory_Type;

inline bool none_window_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &none_window_factory_Type);
}

typedef struct none_window_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::gui::none::none_window_factory* ob_itself;
} none_window_factoryObject;

PyObject *none_window_factoryObj_New(ambulant::gui::none::none_window_factory* itself)
{
	none_window_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_none_window_factory
	none_window_factory *encaps_itself = dynamic_cast<none_window_factory *>(itself);
	if (encaps_itself && encaps_itself->py_none_window_factory)
	{
		Py_INCREF(encaps_itself->py_none_window_factory);
		return encaps_itself->py_none_window_factory;
	}
#endif
	it = PyObject_NEW(none_window_factoryObject, &none_window_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int none_window_factoryObj_Convert(PyObject *v, ambulant::gui::none::none_window_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_none_window_factory
	if (!none_window_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_none_window_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!none_window_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "none_window_factory required");
		return 0;
	}
	*p_itself = ((none_window_factoryObject *)v)->ob_itself;
	return 1;
}

static void none_window_factoryObj_dealloc(none_window_factoryObject *self)
{
	window_factory_Type.tp_dealloc((PyObject *)self);
}

static PyObject *none_window_factoryObj_new_window(none_window_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string name;
	ambulant::lib::size bounds;
	ambulant::common::gui_events* handler;
	char *name_cstr="";
	if (!PyArg_ParseTuple(_args, "sO&O&",
	                      &name_cstr,
	                      ambulant_size_Convert, &bounds,
	                      gui_eventsObj_Convert, &handler))
		return NULL;
	name = name_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::gui_window* _rv = _self->ob_itself->new_window(name,
	                                                                 bounds,
	                                                                 handler);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     gui_windowObj_New, _rv);
	return _res;
}

static PyObject *none_window_factoryObj_new_background_renderer(none_window_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::region_info* src;
	if (!PyArg_ParseTuple(_args, "O&",
	                      region_infoObj_Convert, &src))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::common::bgrenderer* _rv = _self->ob_itself->new_background_renderer(src);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bgrendererObj_New, _rv);
	return _res;
}

static PyMethodDef none_window_factoryObj_methods[] = {
	{"new_window", (PyCFunction)none_window_factoryObj_new_window, 1,
	 PyDoc_STR("(std::string name, ambulant::lib::size bounds, ambulant::common::gui_events* handler) -> (ambulant::common::gui_window* _rv)")},
	{"new_background_renderer", (PyCFunction)none_window_factoryObj_new_background_renderer, 1,
	 PyDoc_STR("(ambulant::common::region_info* src) -> (ambulant::common::bgrenderer* _rv)")},
	{NULL, NULL, 0}
};

#define none_window_factoryObj_getsetlist NULL


static int none_window_factoryObj_compare(none_window_factoryObject *self, none_window_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define none_window_factoryObj_repr NULL

static long none_window_factoryObj_hash(none_window_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int none_window_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::gui::none::none_window_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	{
		if (PyArg_ParseTuple(_args, ""))
		{
			((none_window_factoryObject *)_self)->ob_itself = new ambulant::gui::none::none_window_factory();
			return 0;
		}
	}

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, none_window_factoryObj_Convert, &itself))
	{
		((none_window_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define none_window_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *none_window_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((none_window_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define none_window_factoryObj_tp_free PyObject_Del


PyTypeObject none_window_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.none_window_factory", /*tp_name*/
	sizeof(none_window_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) none_window_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) none_window_factoryObj_compare, /*tp_compare*/
	(reprfunc) none_window_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) none_window_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	none_window_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	none_window_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	none_window_factoryObj_tp_init, /* tp_init */
	none_window_factoryObj_tp_alloc, /* tp_alloc */
	none_window_factoryObj_tp_new, /* tp_new */
	none_window_factoryObj_tp_free, /* tp_free */
};

/* -------------- End object type none_window_factory --------------- */


/* --------------------- Object type datasource --------------------- */

extern PyTypeObject datasource_Type;

inline bool datasourceObj_Check(PyObject *x)
{
	return ((x)->ob_type == &datasource_Type);
}

typedef struct datasourceObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::datasource* ob_itself;
} datasourceObject;

PyObject *datasourceObj_New(ambulant::net::datasource* itself)
{
	datasourceObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_datasource
	datasource *encaps_itself = dynamic_cast<datasource *>(itself);
	if (encaps_itself && encaps_itself->py_datasource)
	{
		Py_INCREF(encaps_itself->py_datasource);
		return encaps_itself->py_datasource;
	}
#endif
	it = PyObject_NEW(datasourceObject, &datasource_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int datasourceObj_Convert(PyObject *v, ambulant::net::datasource* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_datasource
	if (!datasourceObj_Check(v))
	{
		*p_itself = Py_WrapAs_datasource(v);
		if (*p_itself) return 1;
	}
#endif
	if (!datasourceObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "datasource required");
		return 0;
	}
	*p_itself = ((datasourceObject *)v)->ob_itself;
	return 1;
}

static void datasourceObj_dealloc(datasourceObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *datasourceObj_start(datasourceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::event_processor* evp;
	ambulant::lib::event* callback;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      event_processorObj_Convert, &evp,
	                      eventObj_Convert, &callback))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->start(evp,
	                        callback);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *datasourceObj_start_prefetch(datasourceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::event_processor* evp;
	if (!PyArg_ParseTuple(_args, "O&",
	                      event_processorObj_Convert, &evp))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->start_prefetch(evp);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *datasourceObj_stop(datasourceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->stop();
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *datasourceObj_end_of_file(datasourceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself->end_of_file();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyObject *datasourceObj_size(datasourceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	size_t _rv = _self->ob_itself->size();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("l",
	                     _rv);
	return _res;
}

static PyObject *datasourceObj_readdone(datasourceObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	size_t len;
	if (!PyArg_ParseTuple(_args, "l",
	                      &len))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->readdone(len);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef datasourceObj_methods[] = {
	{"start", (PyCFunction)datasourceObj_start, 1,
	 PyDoc_STR("(ambulant::lib::event_processor* evp, ambulant::lib::event* callback) -> None")},
	{"start_prefetch", (PyCFunction)datasourceObj_start_prefetch, 1,
	 PyDoc_STR("(ambulant::lib::event_processor* evp) -> None")},
	{"stop", (PyCFunction)datasourceObj_stop, 1,
	 PyDoc_STR("() -> None")},
	{"end_of_file", (PyCFunction)datasourceObj_end_of_file, 1,
	 PyDoc_STR("() -> (bool _rv)")},
	{"size", (PyCFunction)datasourceObj_size, 1,
	 PyDoc_STR("() -> (size_t _rv)")},
	{"readdone", (PyCFunction)datasourceObj_readdone, 1,
	 PyDoc_STR("(size_t len) -> None")},
	{NULL, NULL, 0}
};

#define datasourceObj_getsetlist NULL


static int datasourceObj_compare(datasourceObject *self, datasourceObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define datasourceObj_repr NULL

static long datasourceObj_hash(datasourceObject *self)
{
	return (long)self->ob_itself;
}
static int datasourceObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::datasource* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, datasourceObj_Convert, &itself))
	{
		((datasourceObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define datasourceObj_tp_alloc PyType_GenericAlloc

static PyObject *datasourceObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((datasourceObject *)_self)->ob_itself = NULL;
	return _self;
}

#define datasourceObj_tp_free PyObject_Del


PyTypeObject datasource_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.datasource", /*tp_name*/
	sizeof(datasourceObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) datasourceObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) datasourceObj_compare, /*tp_compare*/
	(reprfunc) datasourceObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) datasourceObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	datasourceObj_methods, /* tp_methods */
	0, /*tp_members*/
	datasourceObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	datasourceObj_tp_init, /* tp_init */
	datasourceObj_tp_alloc, /* tp_alloc */
	datasourceObj_tp_new, /* tp_new */
	datasourceObj_tp_free, /* tp_free */
};

/* ------------------- End object type datasource ------------------- */


/* ------------------ Object type audio_datasource ------------------ */

extern PyTypeObject audio_datasource_Type;

inline bool audio_datasourceObj_Check(PyObject *x)
{
	return ((x)->ob_type == &audio_datasource_Type);
}

typedef struct audio_datasourceObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::audio_datasource* ob_itself;
} audio_datasourceObject;

PyObject *audio_datasourceObj_New(ambulant::net::audio_datasource* itself)
{
	audio_datasourceObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_audio_datasource
	audio_datasource *encaps_itself = dynamic_cast<audio_datasource *>(itself);
	if (encaps_itself && encaps_itself->py_audio_datasource)
	{
		Py_INCREF(encaps_itself->py_audio_datasource);
		return encaps_itself->py_audio_datasource;
	}
#endif
	it = PyObject_NEW(audio_datasourceObject, &audio_datasource_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int audio_datasourceObj_Convert(PyObject *v, ambulant::net::audio_datasource* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_audio_datasource
	if (!audio_datasourceObj_Check(v))
	{
		*p_itself = Py_WrapAs_audio_datasource(v);
		if (*p_itself) return 1;
	}
#endif
	if (!audio_datasourceObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "audio_datasource required");
		return 0;
	}
	*p_itself = ((audio_datasourceObject *)v)->ob_itself;
	return 1;
}

static void audio_datasourceObj_dealloc(audio_datasourceObject *self)
{
	datasource_Type.tp_dealloc((PyObject *)self);
}

static PyMethodDef audio_datasourceObj_methods[] = {
	{NULL, NULL, 0}
};

#define audio_datasourceObj_getsetlist NULL


static int audio_datasourceObj_compare(audio_datasourceObject *self, audio_datasourceObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define audio_datasourceObj_repr NULL

static long audio_datasourceObj_hash(audio_datasourceObject *self)
{
	return (long)self->ob_itself;
}
static int audio_datasourceObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::audio_datasource* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, audio_datasourceObj_Convert, &itself))
	{
		((audio_datasourceObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define audio_datasourceObj_tp_alloc PyType_GenericAlloc

static PyObject *audio_datasourceObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((audio_datasourceObject *)_self)->ob_itself = NULL;
	return _self;
}

#define audio_datasourceObj_tp_free PyObject_Del


PyTypeObject audio_datasource_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.audio_datasource", /*tp_name*/
	sizeof(audio_datasourceObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) audio_datasourceObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) audio_datasourceObj_compare, /*tp_compare*/
	(reprfunc) audio_datasourceObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) audio_datasourceObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	audio_datasourceObj_methods, /* tp_methods */
	0, /*tp_members*/
	audio_datasourceObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	audio_datasourceObj_tp_init, /* tp_init */
	audio_datasourceObj_tp_alloc, /* tp_alloc */
	audio_datasourceObj_tp_new, /* tp_new */
	audio_datasourceObj_tp_free, /* tp_free */
};

/* ---------------- End object type audio_datasource ---------------- */


/* ------------------ Object type video_datasource ------------------ */

extern PyTypeObject video_datasource_Type;

inline bool video_datasourceObj_Check(PyObject *x)
{
	return ((x)->ob_type == &video_datasource_Type);
}

typedef struct video_datasourceObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::video_datasource* ob_itself;
} video_datasourceObject;

PyObject *video_datasourceObj_New(ambulant::net::video_datasource* itself)
{
	video_datasourceObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_video_datasource
	video_datasource *encaps_itself = dynamic_cast<video_datasource *>(itself);
	if (encaps_itself && encaps_itself->py_video_datasource)
	{
		Py_INCREF(encaps_itself->py_video_datasource);
		return encaps_itself->py_video_datasource;
	}
#endif
	it = PyObject_NEW(video_datasourceObject, &video_datasource_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int video_datasourceObj_Convert(PyObject *v, ambulant::net::video_datasource* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_video_datasource
	if (!video_datasourceObj_Check(v))
	{
		*p_itself = Py_WrapAs_video_datasource(v);
		if (*p_itself) return 1;
	}
#endif
	if (!video_datasourceObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "video_datasource required");
		return 0;
	}
	*p_itself = ((video_datasourceObject *)v)->ob_itself;
	return 1;
}

static void video_datasourceObj_dealloc(video_datasourceObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyMethodDef video_datasourceObj_methods[] = {
	{NULL, NULL, 0}
};

#define video_datasourceObj_getsetlist NULL


static int video_datasourceObj_compare(video_datasourceObject *self, video_datasourceObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define video_datasourceObj_repr NULL

static long video_datasourceObj_hash(video_datasourceObject *self)
{
	return (long)self->ob_itself;
}
static int video_datasourceObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::video_datasource* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, video_datasourceObj_Convert, &itself))
	{
		((video_datasourceObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define video_datasourceObj_tp_alloc PyType_GenericAlloc

static PyObject *video_datasourceObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((video_datasourceObject *)_self)->ob_itself = NULL;
	return _self;
}

#define video_datasourceObj_tp_free PyObject_Del


PyTypeObject video_datasource_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.video_datasource", /*tp_name*/
	sizeof(video_datasourceObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) video_datasourceObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) video_datasourceObj_compare, /*tp_compare*/
	(reprfunc) video_datasourceObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) video_datasourceObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	video_datasourceObj_methods, /* tp_methods */
	0, /*tp_members*/
	video_datasourceObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	video_datasourceObj_tp_init, /* tp_init */
	video_datasourceObj_tp_alloc, /* tp_alloc */
	video_datasourceObj_tp_new, /* tp_new */
	video_datasourceObj_tp_free, /* tp_free */
};

/* ---------------- End object type video_datasource ---------------- */


/* ----------------- Object type datasource_factory ----------------- */

extern PyTypeObject datasource_factory_Type;

inline bool datasource_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &datasource_factory_Type);
}

typedef struct datasource_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::datasource_factory* ob_itself;
} datasource_factoryObject;

PyObject *datasource_factoryObj_New(ambulant::net::datasource_factory* itself)
{
	datasource_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_datasource_factory
	datasource_factory *encaps_itself = dynamic_cast<datasource_factory *>(itself);
	if (encaps_itself && encaps_itself->py_datasource_factory)
	{
		Py_INCREF(encaps_itself->py_datasource_factory);
		return encaps_itself->py_datasource_factory;
	}
#endif
	it = PyObject_NEW(datasource_factoryObject, &datasource_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int datasource_factoryObj_Convert(PyObject *v, ambulant::net::datasource_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_datasource_factory
	if (!datasource_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_datasource_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!datasource_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "datasource_factory required");
		return 0;
	}
	*p_itself = ((datasource_factoryObject *)v)->ob_itself;
	return 1;
}

static void datasource_factoryObj_dealloc(datasource_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *datasource_factoryObj_new_raw_datasource(datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url url;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_url_Convert, &url))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::datasource* _rv = _self->ob_itself->new_raw_datasource(url);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     datasourceObj_New, _rv);
	return _res;
}

static PyObject *datasource_factoryObj_new_audio_datasource(datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url url;
	ambulant::net::audio_format_choices fmt;
	ambulant::net::timestamp_t clip_begin;
	ambulant::net::timestamp_t clip_end;
	if (!PyArg_ParseTuple(_args, "O&O&LL",
	                      ambulant_url_Convert, &url,
	                      audio_format_choicesObj_Convert, &fmt,
	                      &clip_begin,
	                      &clip_end))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::audio_datasource* _rv = _self->ob_itself->new_audio_datasource(url,
	                                                                              fmt,
	                                                                              clip_begin,
	                                                                              clip_end);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     audio_datasourceObj_New, _rv);
	return _res;
}

static PyObject *datasource_factoryObj_new_video_datasource(datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url url;
	ambulant::net::timestamp_t clip_begin;
	ambulant::net::timestamp_t clip_end;
	if (!PyArg_ParseTuple(_args, "O&LL",
	                      ambulant_url_Convert, &url,
	                      &clip_begin,
	                      &clip_end))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::video_datasource* _rv = _self->ob_itself->new_video_datasource(url,
	                                                                              clip_begin,
	                                                                              clip_end);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     video_datasourceObj_New, _rv);
	return _res;
}

static PyObject *datasource_factoryObj_new_audio_filter(datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url url;
	ambulant::net::audio_format_choices fmt;
	ambulant::net::audio_datasource* ds;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ambulant_url_Convert, &url,
	                      audio_format_choicesObj_Convert, &fmt,
	                      audio_datasourceObj_Convert, &ds))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::audio_datasource* _rv = _self->ob_itself->new_audio_filter(url,
	                                                                          fmt,
	                                                                          ds);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     audio_datasourceObj_New, _rv);
	return _res;
}

static PyObject *datasource_factoryObj_add_raw_factory(datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::raw_datasource_factory* df;
	if (!PyArg_ParseTuple(_args, "O&",
	                      raw_datasource_factoryObj_Convert, &df))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->add_raw_factory(df);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *datasource_factoryObj_add_audio_factory(datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::audio_datasource_factory* df;
	if (!PyArg_ParseTuple(_args, "O&",
	                      audio_datasource_factoryObj_Convert, &df))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->add_audio_factory(df);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *datasource_factoryObj_add_audio_parser_finder(datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::audio_parser_finder* df;
	if (!PyArg_ParseTuple(_args, "O&",
	                      audio_parser_finderObj_Convert, &df))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->add_audio_parser_finder(df);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *datasource_factoryObj_add_audio_filter_finder(datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::audio_filter_finder* df;
	if (!PyArg_ParseTuple(_args, "O&",
	                      audio_filter_finderObj_Convert, &df))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->add_audio_filter_finder(df);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *datasource_factoryObj_add_audio_decoder_finder(datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::audio_decoder_finder* df;
	if (!PyArg_ParseTuple(_args, "O&",
	                      audio_decoder_finderObj_Convert, &df))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->add_audio_decoder_finder(df);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *datasource_factoryObj_add_video_factory(datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::video_datasource_factory* df;
	if (!PyArg_ParseTuple(_args, "O&",
	                      video_datasource_factoryObj_Convert, &df))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself->add_video_factory(df);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyMethodDef datasource_factoryObj_methods[] = {
	{"new_raw_datasource", (PyCFunction)datasource_factoryObj_new_raw_datasource, 1,
	 PyDoc_STR("(ambulant::net::url url) -> (ambulant::net::datasource* _rv)")},
	{"new_audio_datasource", (PyCFunction)datasource_factoryObj_new_audio_datasource, 1,
	 PyDoc_STR("(ambulant::net::url url, ambulant::net::audio_format_choices fmt, ambulant::net::timestamp_t clip_begin, ambulant::net::timestamp_t clip_end) -> (ambulant::net::audio_datasource* _rv)")},
	{"new_video_datasource", (PyCFunction)datasource_factoryObj_new_video_datasource, 1,
	 PyDoc_STR("(ambulant::net::url url, ambulant::net::timestamp_t clip_begin, ambulant::net::timestamp_t clip_end) -> (ambulant::net::video_datasource* _rv)")},
	{"new_audio_filter", (PyCFunction)datasource_factoryObj_new_audio_filter, 1,
	 PyDoc_STR("(ambulant::net::url url, ambulant::net::audio_format_choices fmt, ambulant::net::audio_datasource* ds) -> (ambulant::net::audio_datasource* _rv)")},
	{"add_raw_factory", (PyCFunction)datasource_factoryObj_add_raw_factory, 1,
	 PyDoc_STR("(ambulant::net::raw_datasource_factory* df) -> None")},
	{"add_audio_factory", (PyCFunction)datasource_factoryObj_add_audio_factory, 1,
	 PyDoc_STR("(ambulant::net::audio_datasource_factory* df) -> None")},
	{"add_audio_parser_finder", (PyCFunction)datasource_factoryObj_add_audio_parser_finder, 1,
	 PyDoc_STR("(ambulant::net::audio_parser_finder* df) -> None")},
	{"add_audio_filter_finder", (PyCFunction)datasource_factoryObj_add_audio_filter_finder, 1,
	 PyDoc_STR("(ambulant::net::audio_filter_finder* df) -> None")},
	{"add_audio_decoder_finder", (PyCFunction)datasource_factoryObj_add_audio_decoder_finder, 1,
	 PyDoc_STR("(ambulant::net::audio_decoder_finder* df) -> None")},
	{"add_video_factory", (PyCFunction)datasource_factoryObj_add_video_factory, 1,
	 PyDoc_STR("(ambulant::net::video_datasource_factory* df) -> None")},
	{NULL, NULL, 0}
};

#define datasource_factoryObj_getsetlist NULL


static int datasource_factoryObj_compare(datasource_factoryObject *self, datasource_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define datasource_factoryObj_repr NULL

static long datasource_factoryObj_hash(datasource_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int datasource_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::datasource_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	{
		if (PyArg_ParseTuple(_args, ""))
		{
			((datasource_factoryObject *)_self)->ob_itself = new ambulant::net::datasource_factory();
			return 0;
		}
	}

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, datasource_factoryObj_Convert, &itself))
	{
		((datasource_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define datasource_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *datasource_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((datasource_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define datasource_factoryObj_tp_free PyObject_Del


PyTypeObject datasource_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.datasource_factory", /*tp_name*/
	sizeof(datasource_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) datasource_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) datasource_factoryObj_compare, /*tp_compare*/
	(reprfunc) datasource_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) datasource_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	datasource_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	datasource_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	datasource_factoryObj_tp_init, /* tp_init */
	datasource_factoryObj_tp_alloc, /* tp_alloc */
	datasource_factoryObj_tp_new, /* tp_new */
	datasource_factoryObj_tp_free, /* tp_free */
};

/* --------------- End object type datasource_factory --------------- */


/* --------------- Object type raw_datasource_factory --------------- */

extern PyTypeObject raw_datasource_factory_Type;

inline bool raw_datasource_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &raw_datasource_factory_Type);
}

typedef struct raw_datasource_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::raw_datasource_factory* ob_itself;
} raw_datasource_factoryObject;

PyObject *raw_datasource_factoryObj_New(ambulant::net::raw_datasource_factory* itself)
{
	raw_datasource_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_raw_datasource_factory
	raw_datasource_factory *encaps_itself = dynamic_cast<raw_datasource_factory *>(itself);
	if (encaps_itself && encaps_itself->py_raw_datasource_factory)
	{
		Py_INCREF(encaps_itself->py_raw_datasource_factory);
		return encaps_itself->py_raw_datasource_factory;
	}
#endif
	it = PyObject_NEW(raw_datasource_factoryObject, &raw_datasource_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int raw_datasource_factoryObj_Convert(PyObject *v, ambulant::net::raw_datasource_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_raw_datasource_factory
	if (!raw_datasource_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_raw_datasource_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!raw_datasource_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "raw_datasource_factory required");
		return 0;
	}
	*p_itself = ((raw_datasource_factoryObject *)v)->ob_itself;
	return 1;
}

static void raw_datasource_factoryObj_dealloc(raw_datasource_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *raw_datasource_factoryObj_new_raw_datasource(raw_datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url url;
	if (!PyArg_ParseTuple(_args, "O&",
	                      ambulant_url_Convert, &url))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::datasource* _rv = _self->ob_itself->new_raw_datasource(url);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     datasourceObj_New, _rv);
	return _res;
}

static PyMethodDef raw_datasource_factoryObj_methods[] = {
	{"new_raw_datasource", (PyCFunction)raw_datasource_factoryObj_new_raw_datasource, 1,
	 PyDoc_STR("(ambulant::net::url url) -> (ambulant::net::datasource* _rv)")},
	{NULL, NULL, 0}
};

#define raw_datasource_factoryObj_getsetlist NULL


static int raw_datasource_factoryObj_compare(raw_datasource_factoryObject *self, raw_datasource_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define raw_datasource_factoryObj_repr NULL

static long raw_datasource_factoryObj_hash(raw_datasource_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int raw_datasource_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::raw_datasource_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, raw_datasource_factoryObj_Convert, &itself))
	{
		((raw_datasource_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define raw_datasource_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *raw_datasource_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((raw_datasource_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define raw_datasource_factoryObj_tp_free PyObject_Del


PyTypeObject raw_datasource_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.raw_datasource_factory", /*tp_name*/
	sizeof(raw_datasource_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) raw_datasource_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) raw_datasource_factoryObj_compare, /*tp_compare*/
	(reprfunc) raw_datasource_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) raw_datasource_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	raw_datasource_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	raw_datasource_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	raw_datasource_factoryObj_tp_init, /* tp_init */
	raw_datasource_factoryObj_tp_alloc, /* tp_alloc */
	raw_datasource_factoryObj_tp_new, /* tp_new */
	raw_datasource_factoryObj_tp_free, /* tp_free */
};

/* ------------- End object type raw_datasource_factory ------------- */


/* -------------- Object type audio_datasource_factory -------------- */

extern PyTypeObject audio_datasource_factory_Type;

inline bool audio_datasource_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &audio_datasource_factory_Type);
}

typedef struct audio_datasource_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::audio_datasource_factory* ob_itself;
} audio_datasource_factoryObject;

PyObject *audio_datasource_factoryObj_New(ambulant::net::audio_datasource_factory* itself)
{
	audio_datasource_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_audio_datasource_factory
	audio_datasource_factory *encaps_itself = dynamic_cast<audio_datasource_factory *>(itself);
	if (encaps_itself && encaps_itself->py_audio_datasource_factory)
	{
		Py_INCREF(encaps_itself->py_audio_datasource_factory);
		return encaps_itself->py_audio_datasource_factory;
	}
#endif
	it = PyObject_NEW(audio_datasource_factoryObject, &audio_datasource_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int audio_datasource_factoryObj_Convert(PyObject *v, ambulant::net::audio_datasource_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_audio_datasource_factory
	if (!audio_datasource_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_audio_datasource_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!audio_datasource_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "audio_datasource_factory required");
		return 0;
	}
	*p_itself = ((audio_datasource_factoryObject *)v)->ob_itself;
	return 1;
}

static void audio_datasource_factoryObj_dealloc(audio_datasource_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *audio_datasource_factoryObj_new_audio_datasource(audio_datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url url;
	ambulant::net::audio_format_choices fmt;
	ambulant::net::timestamp_t clip_begin;
	ambulant::net::timestamp_t clip_end;
	if (!PyArg_ParseTuple(_args, "O&O&LL",
	                      ambulant_url_Convert, &url,
	                      audio_format_choicesObj_Convert, &fmt,
	                      &clip_begin,
	                      &clip_end))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::audio_datasource* _rv = _self->ob_itself->new_audio_datasource(url,
	                                                                              fmt,
	                                                                              clip_begin,
	                                                                              clip_end);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     audio_datasourceObj_New, _rv);
	return _res;
}

static PyMethodDef audio_datasource_factoryObj_methods[] = {
	{"new_audio_datasource", (PyCFunction)audio_datasource_factoryObj_new_audio_datasource, 1,
	 PyDoc_STR("(ambulant::net::url url, ambulant::net::audio_format_choices fmt, ambulant::net::timestamp_t clip_begin, ambulant::net::timestamp_t clip_end) -> (ambulant::net::audio_datasource* _rv)")},
	{NULL, NULL, 0}
};

#define audio_datasource_factoryObj_getsetlist NULL


static int audio_datasource_factoryObj_compare(audio_datasource_factoryObject *self, audio_datasource_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define audio_datasource_factoryObj_repr NULL

static long audio_datasource_factoryObj_hash(audio_datasource_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int audio_datasource_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::audio_datasource_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, audio_datasource_factoryObj_Convert, &itself))
	{
		((audio_datasource_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define audio_datasource_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *audio_datasource_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((audio_datasource_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define audio_datasource_factoryObj_tp_free PyObject_Del


PyTypeObject audio_datasource_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.audio_datasource_factory", /*tp_name*/
	sizeof(audio_datasource_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) audio_datasource_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) audio_datasource_factoryObj_compare, /*tp_compare*/
	(reprfunc) audio_datasource_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) audio_datasource_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	audio_datasource_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	audio_datasource_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	audio_datasource_factoryObj_tp_init, /* tp_init */
	audio_datasource_factoryObj_tp_alloc, /* tp_alloc */
	audio_datasource_factoryObj_tp_new, /* tp_new */
	audio_datasource_factoryObj_tp_free, /* tp_free */
};

/* ------------ End object type audio_datasource_factory ------------ */


/* --------------- Object type pkt_datasource_factory --------------- */

extern PyTypeObject pkt_datasource_factory_Type;

inline bool pkt_datasource_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &pkt_datasource_factory_Type);
}

typedef struct pkt_datasource_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::pkt_datasource_factory* ob_itself;
} pkt_datasource_factoryObject;

PyObject *pkt_datasource_factoryObj_New(ambulant::net::pkt_datasource_factory* itself)
{
	pkt_datasource_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_pkt_datasource_factory
	pkt_datasource_factory *encaps_itself = dynamic_cast<pkt_datasource_factory *>(itself);
	if (encaps_itself && encaps_itself->py_pkt_datasource_factory)
	{
		Py_INCREF(encaps_itself->py_pkt_datasource_factory);
		return encaps_itself->py_pkt_datasource_factory;
	}
#endif
	it = PyObject_NEW(pkt_datasource_factoryObject, &pkt_datasource_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int pkt_datasource_factoryObj_Convert(PyObject *v, ambulant::net::pkt_datasource_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_pkt_datasource_factory
	if (!pkt_datasource_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_pkt_datasource_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!pkt_datasource_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "pkt_datasource_factory required");
		return 0;
	}
	*p_itself = ((pkt_datasource_factoryObject *)v)->ob_itself;
	return 1;
}

static void pkt_datasource_factoryObj_dealloc(pkt_datasource_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyMethodDef pkt_datasource_factoryObj_methods[] = {
	{NULL, NULL, 0}
};

#define pkt_datasource_factoryObj_getsetlist NULL


static int pkt_datasource_factoryObj_compare(pkt_datasource_factoryObject *self, pkt_datasource_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define pkt_datasource_factoryObj_repr NULL

static long pkt_datasource_factoryObj_hash(pkt_datasource_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int pkt_datasource_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::pkt_datasource_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, pkt_datasource_factoryObj_Convert, &itself))
	{
		((pkt_datasource_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define pkt_datasource_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *pkt_datasource_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((pkt_datasource_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define pkt_datasource_factoryObj_tp_free PyObject_Del


PyTypeObject pkt_datasource_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.pkt_datasource_factory", /*tp_name*/
	sizeof(pkt_datasource_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) pkt_datasource_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) pkt_datasource_factoryObj_compare, /*tp_compare*/
	(reprfunc) pkt_datasource_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) pkt_datasource_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	pkt_datasource_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	pkt_datasource_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	pkt_datasource_factoryObj_tp_init, /* tp_init */
	pkt_datasource_factoryObj_tp_alloc, /* tp_alloc */
	pkt_datasource_factoryObj_tp_new, /* tp_new */
	pkt_datasource_factoryObj_tp_free, /* tp_free */
};

/* ------------- End object type pkt_datasource_factory ------------- */


/* -------------- Object type video_datasource_factory -------------- */

extern PyTypeObject video_datasource_factory_Type;

inline bool video_datasource_factoryObj_Check(PyObject *x)
{
	return ((x)->ob_type == &video_datasource_factory_Type);
}

typedef struct video_datasource_factoryObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::video_datasource_factory* ob_itself;
} video_datasource_factoryObject;

PyObject *video_datasource_factoryObj_New(ambulant::net::video_datasource_factory* itself)
{
	video_datasource_factoryObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_video_datasource_factory
	video_datasource_factory *encaps_itself = dynamic_cast<video_datasource_factory *>(itself);
	if (encaps_itself && encaps_itself->py_video_datasource_factory)
	{
		Py_INCREF(encaps_itself->py_video_datasource_factory);
		return encaps_itself->py_video_datasource_factory;
	}
#endif
	it = PyObject_NEW(video_datasource_factoryObject, &video_datasource_factory_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int video_datasource_factoryObj_Convert(PyObject *v, ambulant::net::video_datasource_factory* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_video_datasource_factory
	if (!video_datasource_factoryObj_Check(v))
	{
		*p_itself = Py_WrapAs_video_datasource_factory(v);
		if (*p_itself) return 1;
	}
#endif
	if (!video_datasource_factoryObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "video_datasource_factory required");
		return 0;
	}
	*p_itself = ((video_datasource_factoryObject *)v)->ob_itself;
	return 1;
}

static void video_datasource_factoryObj_dealloc(video_datasource_factoryObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *video_datasource_factoryObj_new_video_datasource(video_datasource_factoryObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url url;
	ambulant::net::timestamp_t clip_begin;
	ambulant::net::timestamp_t clip_end;
	if (!PyArg_ParseTuple(_args, "O&LL",
	                      ambulant_url_Convert, &url,
	                      &clip_begin,
	                      &clip_end))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::video_datasource* _rv = _self->ob_itself->new_video_datasource(url,
	                                                                              clip_begin,
	                                                                              clip_end);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     video_datasourceObj_New, _rv);
	return _res;
}

static PyMethodDef video_datasource_factoryObj_methods[] = {
	{"new_video_datasource", (PyCFunction)video_datasource_factoryObj_new_video_datasource, 1,
	 PyDoc_STR("(ambulant::net::url url, ambulant::net::timestamp_t clip_begin, ambulant::net::timestamp_t clip_end) -> (ambulant::net::video_datasource* _rv)")},
	{NULL, NULL, 0}
};

#define video_datasource_factoryObj_getsetlist NULL


static int video_datasource_factoryObj_compare(video_datasource_factoryObject *self, video_datasource_factoryObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define video_datasource_factoryObj_repr NULL

static long video_datasource_factoryObj_hash(video_datasource_factoryObject *self)
{
	return (long)self->ob_itself;
}
static int video_datasource_factoryObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::video_datasource_factory* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, video_datasource_factoryObj_Convert, &itself))
	{
		((video_datasource_factoryObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define video_datasource_factoryObj_tp_alloc PyType_GenericAlloc

static PyObject *video_datasource_factoryObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((video_datasource_factoryObject *)_self)->ob_itself = NULL;
	return _self;
}

#define video_datasource_factoryObj_tp_free PyObject_Del


PyTypeObject video_datasource_factory_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.video_datasource_factory", /*tp_name*/
	sizeof(video_datasource_factoryObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) video_datasource_factoryObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) video_datasource_factoryObj_compare, /*tp_compare*/
	(reprfunc) video_datasource_factoryObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) video_datasource_factoryObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	video_datasource_factoryObj_methods, /* tp_methods */
	0, /*tp_members*/
	video_datasource_factoryObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	video_datasource_factoryObj_tp_init, /* tp_init */
	video_datasource_factoryObj_tp_alloc, /* tp_alloc */
	video_datasource_factoryObj_tp_new, /* tp_new */
	video_datasource_factoryObj_tp_free, /* tp_free */
};

/* ------------ End object type video_datasource_factory ------------ */


/* ---------------- Object type audio_parser_finder ----------------- */

extern PyTypeObject audio_parser_finder_Type;

inline bool audio_parser_finderObj_Check(PyObject *x)
{
	return ((x)->ob_type == &audio_parser_finder_Type);
}

typedef struct audio_parser_finderObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::audio_parser_finder* ob_itself;
} audio_parser_finderObject;

PyObject *audio_parser_finderObj_New(ambulant::net::audio_parser_finder* itself)
{
	audio_parser_finderObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_audio_parser_finder
	audio_parser_finder *encaps_itself = dynamic_cast<audio_parser_finder *>(itself);
	if (encaps_itself && encaps_itself->py_audio_parser_finder)
	{
		Py_INCREF(encaps_itself->py_audio_parser_finder);
		return encaps_itself->py_audio_parser_finder;
	}
#endif
	it = PyObject_NEW(audio_parser_finderObject, &audio_parser_finder_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int audio_parser_finderObj_Convert(PyObject *v, ambulant::net::audio_parser_finder* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_audio_parser_finder
	if (!audio_parser_finderObj_Check(v))
	{
		*p_itself = Py_WrapAs_audio_parser_finder(v);
		if (*p_itself) return 1;
	}
#endif
	if (!audio_parser_finderObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "audio_parser_finder required");
		return 0;
	}
	*p_itself = ((audio_parser_finderObject *)v)->ob_itself;
	return 1;
}

static void audio_parser_finderObj_dealloc(audio_parser_finderObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *audio_parser_finderObj_new_audio_parser(audio_parser_finderObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::url url;
	ambulant::net::audio_format_choices hint;
	ambulant::net::audio_datasource* src;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      ambulant_url_Convert, &url,
	                      audio_format_choicesObj_Convert, &hint,
	                      audio_datasourceObj_Convert, &src))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::audio_datasource* _rv = _self->ob_itself->new_audio_parser(url,
	                                                                          hint,
	                                                                          src);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     audio_datasourceObj_New, _rv);
	return _res;
}

static PyMethodDef audio_parser_finderObj_methods[] = {
	{"new_audio_parser", (PyCFunction)audio_parser_finderObj_new_audio_parser, 1,
	 PyDoc_STR("(ambulant::net::url url, ambulant::net::audio_format_choices hint, ambulant::net::audio_datasource* src) -> (ambulant::net::audio_datasource* _rv)")},
	{NULL, NULL, 0}
};

#define audio_parser_finderObj_getsetlist NULL


static int audio_parser_finderObj_compare(audio_parser_finderObject *self, audio_parser_finderObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define audio_parser_finderObj_repr NULL

static long audio_parser_finderObj_hash(audio_parser_finderObject *self)
{
	return (long)self->ob_itself;
}
static int audio_parser_finderObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::audio_parser_finder* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, audio_parser_finderObj_Convert, &itself))
	{
		((audio_parser_finderObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define audio_parser_finderObj_tp_alloc PyType_GenericAlloc

static PyObject *audio_parser_finderObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((audio_parser_finderObject *)_self)->ob_itself = NULL;
	return _self;
}

#define audio_parser_finderObj_tp_free PyObject_Del


PyTypeObject audio_parser_finder_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.audio_parser_finder", /*tp_name*/
	sizeof(audio_parser_finderObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) audio_parser_finderObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) audio_parser_finderObj_compare, /*tp_compare*/
	(reprfunc) audio_parser_finderObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) audio_parser_finderObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	audio_parser_finderObj_methods, /* tp_methods */
	0, /*tp_members*/
	audio_parser_finderObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	audio_parser_finderObj_tp_init, /* tp_init */
	audio_parser_finderObj_tp_alloc, /* tp_alloc */
	audio_parser_finderObj_tp_new, /* tp_new */
	audio_parser_finderObj_tp_free, /* tp_free */
};

/* -------------- End object type audio_parser_finder --------------- */


/* ---------------- Object type audio_filter_finder ----------------- */

extern PyTypeObject audio_filter_finder_Type;

inline bool audio_filter_finderObj_Check(PyObject *x)
{
	return ((x)->ob_type == &audio_filter_finder_Type);
}

typedef struct audio_filter_finderObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::audio_filter_finder* ob_itself;
} audio_filter_finderObject;

PyObject *audio_filter_finderObj_New(ambulant::net::audio_filter_finder* itself)
{
	audio_filter_finderObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_audio_filter_finder
	audio_filter_finder *encaps_itself = dynamic_cast<audio_filter_finder *>(itself);
	if (encaps_itself && encaps_itself->py_audio_filter_finder)
	{
		Py_INCREF(encaps_itself->py_audio_filter_finder);
		return encaps_itself->py_audio_filter_finder;
	}
#endif
	it = PyObject_NEW(audio_filter_finderObject, &audio_filter_finder_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int audio_filter_finderObj_Convert(PyObject *v, ambulant::net::audio_filter_finder* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_audio_filter_finder
	if (!audio_filter_finderObj_Check(v))
	{
		*p_itself = Py_WrapAs_audio_filter_finder(v);
		if (*p_itself) return 1;
	}
#endif
	if (!audio_filter_finderObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "audio_filter_finder required");
		return 0;
	}
	*p_itself = ((audio_filter_finderObject *)v)->ob_itself;
	return 1;
}

static void audio_filter_finderObj_dealloc(audio_filter_finderObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyObject *audio_filter_finderObj_new_audio_filter(audio_filter_finderObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::audio_datasource* src;
	ambulant::net::audio_format_choices fmts;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      audio_datasourceObj_Convert, &src,
	                      audio_format_choicesObj_Convert, &fmts))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::net::audio_datasource* _rv = _self->ob_itself->new_audio_filter(src,
	                                                                          fmts);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     audio_datasourceObj_New, _rv);
	return _res;
}

static PyMethodDef audio_filter_finderObj_methods[] = {
	{"new_audio_filter", (PyCFunction)audio_filter_finderObj_new_audio_filter, 1,
	 PyDoc_STR("(ambulant::net::audio_datasource* src, ambulant::net::audio_format_choices fmts) -> (ambulant::net::audio_datasource* _rv)")},
	{NULL, NULL, 0}
};

#define audio_filter_finderObj_getsetlist NULL


static int audio_filter_finderObj_compare(audio_filter_finderObject *self, audio_filter_finderObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define audio_filter_finderObj_repr NULL

static long audio_filter_finderObj_hash(audio_filter_finderObject *self)
{
	return (long)self->ob_itself;
}
static int audio_filter_finderObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::audio_filter_finder* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, audio_filter_finderObj_Convert, &itself))
	{
		((audio_filter_finderObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define audio_filter_finderObj_tp_alloc PyType_GenericAlloc

static PyObject *audio_filter_finderObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((audio_filter_finderObject *)_self)->ob_itself = NULL;
	return _self;
}

#define audio_filter_finderObj_tp_free PyObject_Del


PyTypeObject audio_filter_finder_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.audio_filter_finder", /*tp_name*/
	sizeof(audio_filter_finderObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) audio_filter_finderObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) audio_filter_finderObj_compare, /*tp_compare*/
	(reprfunc) audio_filter_finderObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) audio_filter_finderObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	audio_filter_finderObj_methods, /* tp_methods */
	0, /*tp_members*/
	audio_filter_finderObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	audio_filter_finderObj_tp_init, /* tp_init */
	audio_filter_finderObj_tp_alloc, /* tp_alloc */
	audio_filter_finderObj_tp_new, /* tp_new */
	audio_filter_finderObj_tp_free, /* tp_free */
};

/* -------------- End object type audio_filter_finder --------------- */


/* ---------------- Object type audio_decoder_finder ---------------- */

extern PyTypeObject audio_decoder_finder_Type;

inline bool audio_decoder_finderObj_Check(PyObject *x)
{
	return ((x)->ob_type == &audio_decoder_finder_Type);
}

typedef struct audio_decoder_finderObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::audio_decoder_finder* ob_itself;
} audio_decoder_finderObject;

PyObject *audio_decoder_finderObj_New(ambulant::net::audio_decoder_finder* itself)
{
	audio_decoder_finderObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_audio_decoder_finder
	audio_decoder_finder *encaps_itself = dynamic_cast<audio_decoder_finder *>(itself);
	if (encaps_itself && encaps_itself->py_audio_decoder_finder)
	{
		Py_INCREF(encaps_itself->py_audio_decoder_finder);
		return encaps_itself->py_audio_decoder_finder;
	}
#endif
	it = PyObject_NEW(audio_decoder_finderObject, &audio_decoder_finder_Type);
	if (it == NULL) return NULL;
	/* XXXX Should we tp_init or tp_new our basetype? */
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = itself;
	return (PyObject *)it;
}

int audio_decoder_finderObj_Convert(PyObject *v, ambulant::net::audio_decoder_finder* *p_itself)
{
	if (v == Py_None)
	{
		*p_itself = NULL;
		return 1;
	}
#ifdef BGEN_BACK_SUPPORT_audio_decoder_finder
	if (!audio_decoder_finderObj_Check(v))
	{
		*p_itself = Py_WrapAs_audio_decoder_finder(v);
		if (*p_itself) return 1;
	}
#endif
	if (!audio_decoder_finderObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "audio_decoder_finder required");
		return 0;
	}
	*p_itself = ((audio_decoder_finderObject *)v)->ob_itself;
	return 1;
}

static void audio_decoder_finderObj_dealloc(audio_decoder_finderObject *self)
{
	pycppbridge_Type.tp_dealloc((PyObject *)self);
}

static PyMethodDef audio_decoder_finderObj_methods[] = {
	{NULL, NULL, 0}
};

#define audio_decoder_finderObj_getsetlist NULL


static int audio_decoder_finderObj_compare(audio_decoder_finderObject *self, audio_decoder_finderObject *other)
{
	if ( self->ob_itself > other->ob_itself ) return 1;
	if ( self->ob_itself < other->ob_itself ) return -1;
	return 0;
}

#define audio_decoder_finderObj_repr NULL

static long audio_decoder_finderObj_hash(audio_decoder_finderObject *self)
{
	return (long)self->ob_itself;
}
static int audio_decoder_finderObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::audio_decoder_finder* itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, audio_decoder_finderObj_Convert, &itself))
	{
		((audio_decoder_finderObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define audio_decoder_finderObj_tp_alloc PyType_GenericAlloc

static PyObject *audio_decoder_finderObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	((audio_decoder_finderObject *)_self)->ob_itself = NULL;
	return _self;
}

#define audio_decoder_finderObj_tp_free PyObject_Del


PyTypeObject audio_decoder_finder_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.audio_decoder_finder", /*tp_name*/
	sizeof(audio_decoder_finderObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) audio_decoder_finderObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) audio_decoder_finderObj_compare, /*tp_compare*/
	(reprfunc) audio_decoder_finderObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) audio_decoder_finderObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	audio_decoder_finderObj_methods, /* tp_methods */
	0, /*tp_members*/
	audio_decoder_finderObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	audio_decoder_finderObj_tp_init, /* tp_init */
	audio_decoder_finderObj_tp_alloc, /* tp_alloc */
	audio_decoder_finderObj_tp_new, /* tp_new */
	audio_decoder_finderObj_tp_free, /* tp_free */
};

/* -------------- End object type audio_decoder_finder -------------- */


/* ---------------- Object type audio_format_choices ---------------- */

extern PyTypeObject audio_format_choices_Type;

inline bool audio_format_choicesObj_Check(PyObject *x)
{
	return ((x)->ob_type == &audio_format_choices_Type);
}

typedef struct audio_format_choicesObject {
	PyObject_HEAD
	void *ob_dummy_wrapper; // Overlays bridge object storage
	ambulant::net::audio_format_choices ob_itself;
} audio_format_choicesObject;

PyObject *audio_format_choicesObj_New(const ambulant::net::audio_format_choices *itself)
{
	audio_format_choicesObject *it;
	if (itself == NULL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
#ifdef BGEN_BACK_SUPPORT_audio_format_choices
	audio_format_choices *encaps_itself = dynamic_cast<audio_format_choices *>(itself);
	if (encaps_itself && encaps_itself->py_audio_format_choices)
	{
		Py_INCREF(encaps_itself->py_audio_format_choices);
		return encaps_itself->py_audio_format_choices;
	}
#endif
	it = PyObject_NEW(audio_format_choicesObject, &audio_format_choices_Type);
	if (it == NULL) return NULL;
	it->ob_dummy_wrapper = NULL; // XXXX Should be done in base class
	it->ob_itself = *itself;
	return (PyObject *)it;
}

int audio_format_choicesObj_Convert(PyObject *v, ambulant::net::audio_format_choices *p_itself)
{
#ifdef BGEN_BACK_SUPPORT_audio_format_choices
	if (!audio_format_choicesObj_Check(v))
	{
		*p_itself = Py_WrapAs_audio_format_choices(v);
		if (*p_itself) return 1;
	}
#endif
	if (!audio_format_choicesObj_Check(v))
	{
		PyErr_SetString(PyExc_TypeError, "audio_format_choices required");
		return 0;
	}
	*p_itself = ((audio_format_choicesObject *)v)->ob_itself;
	return 1;
}

static void audio_format_choicesObj_dealloc(audio_format_choicesObject *self)
{
	self->ob_type->tp_free((PyObject *)self);
}

static PyObject *audio_format_choicesObj_best(audio_format_choicesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	const ambulant::net::audio_format& _rv = _self->ob_itself.best();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("(ssiii)",
	                     _rv.mime_type.c_str(), _rv.name.c_str(), _rv.parameters, _rv.samplerate, _rv.channels, _rv.bits);
	return _res;
}

static PyObject *audio_format_choicesObj_add_samplerate(audio_format_choicesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int samplerate;
	if (!PyArg_ParseTuple(_args, "i",
	                      &samplerate))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself.add_samplerate(samplerate);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *audio_format_choicesObj_add_channels(audio_format_choicesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int channels;
	if (!PyArg_ParseTuple(_args, "i",
	                      &channels))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself.add_channels(channels);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *audio_format_choicesObj_add_bits(audio_format_choicesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int bits;
	if (!PyArg_ParseTuple(_args, "i",
	                      &bits))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself.add_bits(bits);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *audio_format_choicesObj_add_named_format(audio_format_choicesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	std::string name;
	char *name_cstr="";
	if (!PyArg_ParseTuple(_args, "s",
	                      &name_cstr))
		return NULL;
	name = name_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_self->ob_itself.add_named_format(name);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *audio_format_choicesObj_contains(audio_format_choicesObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::audio_format fmt;
	char *fmt_mime_type_cstr="";
	char *fmt_name_cstr="";
	if (!PyArg_ParseTuple(_args, "(ssiii)",
	                      &fmt_mime_type_cstr, &fmt_name_cstr, &fmt.samplerate, &fmt.channels, &fmt.bits))
		return NULL;
	fmt.mime_type = fmt_mime_type_cstr;
	fmt.name = fmt_name_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	bool _rv = _self->ob_itself.contains(fmt);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     bool_New, _rv);
	return _res;
}

static PyMethodDef audio_format_choicesObj_methods[] = {
	{"best", (PyCFunction)audio_format_choicesObj_best, 1,
	 PyDoc_STR("() -> (const ambulant::net::audio_format& _rv)")},
	{"add_samplerate", (PyCFunction)audio_format_choicesObj_add_samplerate, 1,
	 PyDoc_STR("(int samplerate) -> None")},
	{"add_channels", (PyCFunction)audio_format_choicesObj_add_channels, 1,
	 PyDoc_STR("(int channels) -> None")},
	{"add_bits", (PyCFunction)audio_format_choicesObj_add_bits, 1,
	 PyDoc_STR("(int bits) -> None")},
	{"add_named_format", (PyCFunction)audio_format_choicesObj_add_named_format, 1,
	 PyDoc_STR("(std::string name) -> None")},
	{"contains", (PyCFunction)audio_format_choicesObj_contains, 1,
	 PyDoc_STR("(ambulant::net::audio_format fmt) -> (bool _rv)")},
	{NULL, NULL, 0}
};

#define audio_format_choicesObj_getsetlist NULL


#define audio_format_choicesObj_compare NULL

#define audio_format_choicesObj_repr NULL

#define audio_format_choicesObj_hash NULL
static int audio_format_choicesObj_tp_init(PyObject *_self, PyObject *_args, PyObject *_kwds)
{
	ambulant::net::audio_format_choices itself;
	Py_KEYWORDS_STRING_TYPE *kw[] = {"itself", 0};

	{
		if (PyArg_ParseTuple(_args, ""))
		{
			((audio_format_choicesObject *)_self)->ob_itself = ambulant::net::audio_format_choices();
			return 0;
		}
	}

	{
		ambulant::net::audio_format fmt;
		char *fmt_mime_type_cstr="";
		char *fmt_name_cstr="";
		if (PyArg_ParseTuple(_args, "(ssiii)",
		                     &fmt_mime_type_cstr, &fmt_name_cstr, &fmt.samplerate, &fmt.channels, &fmt.bits))
		{
			fmt.mime_type = fmt_mime_type_cstr;
			fmt.name = fmt_name_cstr;
			((audio_format_choicesObject *)_self)->ob_itself = ambulant::net::audio_format_choices(fmt);
			return 0;
		}
	}

	{
		int samplerate;
		int channels;
		int bits;
		if (PyArg_ParseTuple(_args, "iii",
		                     &samplerate,
		                     &channels,
		                     &bits))
		{
			((audio_format_choicesObject *)_self)->ob_itself = ambulant::net::audio_format_choices(samplerate,
			                                                                                       channels,
			                                                                                       bits);
			return 0;
		}
	}

	{
		std::string name;
		char *name_cstr="";
		if (PyArg_ParseTuple(_args, "s",
		                     &name_cstr))
		{
			name = name_cstr;
			((audio_format_choicesObject *)_self)->ob_itself = ambulant::net::audio_format_choices(name);
			return 0;
		}
	}

	if (PyArg_ParseTupleAndKeywords(_args, _kwds, "O&", kw, audio_format_choicesObj_Convert, &itself))
	{
		((audio_format_choicesObject *)_self)->ob_itself = itself;
		return 0;
	}
	return -1;
}

#define audio_format_choicesObj_tp_alloc PyType_GenericAlloc

static PyObject *audio_format_choicesObj_tp_new(PyTypeObject *type, PyObject *_args, PyObject *_kwds)
{
	PyObject *_self;

	if ((_self = type->tp_alloc(type, 0)) == NULL) return NULL;
	return _self;
}

#define audio_format_choicesObj_tp_free PyObject_Del


PyTypeObject audio_format_choices_Type = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"ambulant.audio_format_choices", /*tp_name*/
	sizeof(audio_format_choicesObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	/* methods */
	(destructor) audio_format_choicesObj_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	(getattrfunc)0, /*tp_getattr*/
	(setattrfunc)0, /*tp_setattr*/
	(cmpfunc) audio_format_choicesObj_compare, /*tp_compare*/
	(reprfunc) audio_format_choicesObj_repr, /*tp_repr*/
	(PyNumberMethods *)0, /* tp_as_number */
	(PySequenceMethods *)0, /* tp_as_sequence */
	(PyMappingMethods *)0, /* tp_as_mapping */
	(hashfunc) audio_format_choicesObj_hash, /*tp_hash*/
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro */
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
	0, /*tp_doc*/
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	audio_format_choicesObj_methods, /* tp_methods */
	0, /*tp_members*/
	audio_format_choicesObj_getsetlist, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	audio_format_choicesObj_tp_init, /* tp_init */
	audio_format_choicesObj_tp_alloc, /* tp_alloc */
	audio_format_choicesObj_tp_new, /* tp_new */
	audio_format_choicesObj_tp_free, /* tp_free */
};

/* -------------- End object type audio_format_choices -------------- */


static PyObject *PyAm_get_version(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	const char * _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::get_version();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("z",
	                     _rv);
	return _res;
}

static PyObject *PyAm_get_logger_1(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::logger* _rv;
	char* name;
	if (!PyArg_ParseTuple(_args, "s",
	                      &name))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::lib::logger::get_logger(name);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     loggerObj_New, _rv);
	return _res;
}

static PyObject *PyAm_get_logger_2(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::logger* _rv;
	char* name;
	int pos;
	if (!PyArg_ParseTuple(_args, "si",
	                      &name,
	                      &pos))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::lib::logger::get_logger(name,
	                                        pos);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     loggerObj_New, _rv);
	return _res;
}

static PyObject *PyAm_set_loggers_level(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	int level;
	if (!PyArg_ParseTuple(_args, "i",
	                      &level))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	ambulant::lib::logger::set_loggers_level(level);
	PyEval_RestoreThread(_save);
	Py_INCREF(Py_None);
	_res = Py_None;
	return _res;
}

static PyObject *PyAm_get_level_name(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	const char * _rv;
	int level;
	if (!PyArg_ParseTuple(_args, "i",
	                      &level))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::lib::logger::get_level_name(level);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("z",
	                     _rv);
	return _res;
}

static PyObject *PyAm_get_builtin_node_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::node_factory* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::lib::get_builtin_node_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     node_factoryObj_New, _rv);
	return _res;
}

static PyObject *PyAm_create_from_url(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::document* _rv;
	ambulant::common::factories* factory;
	ambulant::net::url u;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      factoriesObj_Convert, &factory,
	                      ambulant_url_Convert, &u))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::lib::document::create_from_url(factory,
	                                               u);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     documentObj_New, _rv);
	return _res;
}

static PyObject *PyAm_create_from_file(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::document* _rv;
	ambulant::common::factories* factory;
	std::string filename;
	char *filename_cstr="";
	if (!PyArg_ParseTuple(_args, "O&s",
	                      factoriesObj_Convert, &factory,
	                      &filename_cstr))
		return NULL;
	filename = filename_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::lib::document::create_from_file(factory,
	                                                filename);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     documentObj_New, _rv);
	return _res;
}

static PyObject *PyAm_create_from_string(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::document* _rv;
	ambulant::common::factories* factory;
	std::string smil_src;
	std::string src_id;
	char *smil_src_cstr="";
	char *src_id_cstr="";
	if (!PyArg_ParseTuple(_args, "O&ss",
	                      factoriesObj_Convert, &factory,
	                      &smil_src_cstr,
	                      &src_id_cstr))
		return NULL;
	smil_src = smil_src_cstr;
	src_id = src_id_cstr;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::lib::document::create_from_string(factory,
	                                                  smil_src,
	                                                  src_id);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     documentObj_New, _rv);
	return _res;
}

static PyObject *PyAm_event_processor_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::event_processor* _rv;
	ambulant::lib::timer* t;
	if (!PyArg_ParseTuple(_args, "O&",
	                      timerObj_Convert, &t))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::lib::event_processor_factory(t);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     event_processorObj_New, _rv);
	return _res;
}

static PyObject *PyAm_get_parser_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::global_parser_factory* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::lib::global_parser_factory::get_parser_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     global_parser_factoryObj_New, _rv);
	return _res;
}

static PyObject *PyAm_realtime_timer_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::timer* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::lib::realtime_timer_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     timerObj_New, _rv);
	return _res;
}

static PyObject *PyAm_from_node(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::lib::transition_info* _rv;
	ambulant::lib::node* n;
	if (!PyArg_ParseTuple(_args, "O&",
	                      nodeObj_Convert, &n))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::lib::transition_info::from_node(n);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     transition_infoObj_New, _rv);
	return _res;
}

static PyObject *PyAm_create_smil2_layout_manager(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::layout_manager* _rv;
	ambulant::common::factories* factory;
	ambulant::lib::document* doc;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      factoriesObj_Convert, &factory,
	                      documentObj_Convert, &doc))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::common::create_smil2_layout_manager(factory,
	                                                    doc);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     layout_managerObj_New, _rv);
	return _res;
}

static PyObject *PyAm_create_smil_surface_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::surface_factory* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::common::create_smil_surface_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     surface_factoryObj_New, _rv);
	return _res;
}

static PyObject *PyAm_get_global_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::global_playable_factory* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::common::get_global_playable_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     global_playable_factoryObj_New, _rv);
	return _res;
}

static PyObject *PyAm_create_smil2_player(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::player* _rv;
	ambulant::lib::document* doc;
	ambulant::common::factories* factory;
	ambulant::common::embedder* sys;
	if (!PyArg_ParseTuple(_args, "O&O&O&",
	                      documentObj_Convert, &doc,
	                      factoriesObj_Convert, &factory,
	                      embedderObj_Convert, &sys))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::common::create_smil2_player(doc,
	                                            factory,
	                                            sys);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playerObj_New, _rv);
	return _res;
}

static PyObject *PyAm_get_global_state_component_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::global_state_component_factory* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::common::get_global_state_component_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     global_state_component_factoryObj_New, _rv);
	return _res;
}

static PyObject *PyAm_create_none_window_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::window_factory* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::none::create_none_window_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     window_factoryObj_New, _rv);
	return _res;
}

#ifdef WITH_GTK

static PyObject *PyAm_create_gtk_window_factory_unsafe(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::window_factory* _rv;
	void * gtk_parent_widget;
	ambulant::common::gui_player* gpl;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      PyGObjectAsVoidPtr_Convert, &gtk_parent_widget,
	                      gui_playerObj_Convert, &gpl))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::gtk::create_gtk_window_factory_unsafe(gtk_parent_widget,
	                                                           gpl);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     window_factoryObj_New, _rv);
	return _res;
}
#endif

#ifdef WITH_GTK

static PyObject *PyAm_create_gtk_video_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* _rv;
	ambulant::common::factories* factory;
	if (!PyArg_ParseTuple(_args, "O&",
	                      factoriesObj_Convert, &factory))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::gtk::create_gtk_video_playable_factory(factory,
	                                                            NULL);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}
#endif

#ifdef WITH_GTK

static PyObject *PyAm_create_gtk_text_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* _rv;
	ambulant::common::factories* factory;
	if (!PyArg_ParseTuple(_args, "O&",
	                      factoriesObj_Convert, &factory))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::gtk::create_gtk_text_playable_factory(factory,
	                                                           NULL);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}
#endif

#ifdef WITH_GTK

static PyObject *PyAm_create_gtk_smiltext_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* _rv;
	ambulant::common::factories* factory;
	if (!PyArg_ParseTuple(_args, "O&",
	                      factoriesObj_Convert, &factory))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::gtk::create_gtk_smiltext_playable_factory(factory,
	                                                               NULL);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}
#endif

#ifdef WITH_GTK

static PyObject *PyAm_create_gtk_image_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* _rv;
	ambulant::common::factories* factory;
	if (!PyArg_ParseTuple(_args, "O&",
	                      factoriesObj_Convert, &factory))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::gtk::create_gtk_image_playable_factory(factory,
	                                                            NULL);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}
#endif

#ifdef WITH_GTK

static PyObject *PyAm_create_gtk_fill_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* _rv;
	ambulant::common::factories* factory;
	if (!PyArg_ParseTuple(_args, "O&",
	                      factoriesObj_Convert, &factory))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::gtk::create_gtk_fill_playable_factory(factory,
	                                                           NULL);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}
#endif

#ifdef WITH_SDL

static PyObject *PyAm_create_sdl_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* _rv;
	ambulant::common::factories* factory;
	if (!PyArg_ParseTuple(_args, "O&",
	                      factoriesObj_Convert, &factory))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::sdl::create_sdl_playable_factory(factory);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}
#endif

#if defined(WITH_SDL2) && defined(WITH_SDL_IMAGE)

static PyObject *PyAm_create_sdl_video_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* _rv;
	ambulant::common::factories* factory;
	if (!PyArg_ParseTuple(_args, "O&",
	                      factoriesObj_Convert, &factory))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::sdl::create_sdl_video_playable_factory(factory,
	                                                            NULL);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}
#endif

#if defined(WITH_SDL2) && defined(WITH_SDL_IMAGE)

static PyObject *PyAm_create_sdl_text_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* _rv;
	ambulant::common::factories* factory;
	if (!PyArg_ParseTuple(_args, "O&",
	                      factoriesObj_Convert, &factory))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::sdl::create_sdl_text_playable_factory(factory,
	                                                           NULL);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}
#endif

#if defined(WITH_SDL2) && (defined(WITH_SDL_TTF) || defined(WITH_SDL_PANGO))

static PyObject *PyAm_create_sdl_smiltext_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* _rv;
	ambulant::common::factories* factory;
	if (!PyArg_ParseTuple(_args, "O&",
	                      factoriesObj_Convert, &factory))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::sdl::create_sdl_smiltext_playable_factory(factory,
	                                                               NULL);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}
#endif

#if defined(WITH_SDL2) && defined(WITH_SDL_IMAGE)

static PyObject *PyAm_create_sdl_image_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* _rv;
	ambulant::common::factories* factory;
	if (!PyArg_ParseTuple(_args, "O&",
	                      factoriesObj_Convert, &factory))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::sdl::create_sdl_image_playable_factory(factory,
	                                                            NULL);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}
#endif

#if defined(WITH_SDL2) && defined(WITH_SDL_IMAGE)

static PyObject *PyAm_create_sdl_fill_playable_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::common::playable_factory* _rv;
	ambulant::common::factories* factory;
	if (!PyArg_ParseTuple(_args, "O&",
	                      factoriesObj_Convert, &factory))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::gui::sdl::create_sdl_fill_playable_factory(factory,
	                                                           NULL);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     playable_factoryObj_New, _rv);
	return _res;
}
#endif

static PyObject *PyAm_read_data_from_url(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool _rv;
	ambulant::net::url url;
	ambulant::net::datasource_factory* df;
	char *result__out__;
	size_t result__len__;
	if (!PyArg_ParseTuple(_args, "O&O&",
	                      ambulant_url_Convert, &url,
	                      datasource_factoryObj_Convert, &df))
		return NULL;
	result__out__ = NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::net::read_data_from_url(url,
	                                        df,
	                                        &result__out__, &result__len__);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&z#",
	                     bool_New, _rv,
	                     result__out__, (int)result__len__);
	if( result__out__ ) free(result__out__);
	return _res;
}

static PyObject *PyAm_read_data_from_datasource(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	bool _rv;
	ambulant::net::datasource* src;
	char *result__out__;
	size_t result__len__;
	if (!PyArg_ParseTuple(_args, "O&",
	                      datasourceObj_Convert, &src))
		return NULL;
	result__out__ = NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::net::read_data_from_datasource(src,
	                                               &result__out__, &result__len__);
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&z#",
	                     bool_New, _rv,
	                     result__out__, (int)result__len__);
	if( result__out__ ) free(result__out__);
	return _res;
}

#ifndef AMBULANT_PLATFORM_WIN32

static PyObject *PyAm_create_posix_datasource_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::raw_datasource_factory* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::net::create_posix_datasource_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     raw_datasource_factoryObj_New, _rv);
	return _res;
}
#endif

static PyObject *PyAm_create_stdio_datasource_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::raw_datasource_factory* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::net::create_stdio_datasource_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     raw_datasource_factoryObj_New, _rv);
	return _res;
}

#ifdef WITH_FFMPEG

static PyObject *PyAm_get_ffmpeg_raw_datasource_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::raw_datasource_factory* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::net::get_ffmpeg_raw_datasource_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     raw_datasource_factoryObj_New, _rv);
	return _res;
}
#endif

#ifdef WITH_FFMPEG

static PyObject *PyAm_get_ffmpeg_video_datasource_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::video_datasource_factory* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::net::get_ffmpeg_video_datasource_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     video_datasource_factoryObj_New, _rv);
	return _res;
}
#endif

#ifdef WITH_FFMPEG

static PyObject *PyAm_get_ffmpeg_audio_datasource_factory(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::audio_datasource_factory* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::net::get_ffmpeg_audio_datasource_factory();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     audio_datasource_factoryObj_New, _rv);
	return _res;
}
#endif

#ifdef WITH_FFMPEG

static PyObject *PyAm_get_ffmpeg_audio_decoder_finder(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::audio_decoder_finder* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::net::get_ffmpeg_audio_decoder_finder();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     audio_decoder_finderObj_New, _rv);
	return _res;
}
#endif

#if defined(WITH_FFMPEG) && defined(WITH_RESAMPLE_DATASOURCE)

static PyObject *PyAm_get_ffmpeg_audio_filter_finder(PyObject *_self, PyObject *_args)
{
	PyObject *_res = NULL;
	ambulant::net::audio_filter_finder* _rv;
	if (!PyArg_ParseTuple(_args, ""))
		return NULL;
	PyThreadState *_save = PyEval_SaveThread();
	_rv = ambulant::net::get_ffmpeg_audio_filter_finder();
	PyEval_RestoreThread(_save);
	_res = Py_BuildValue("O&",
	                     audio_filter_finderObj_New, _rv);
	return _res;
}
#endif

static PyMethodDef PyAm_methods[] = {
	{"get_version", (PyCFunction)PyAm_get_version, 1,
	 PyDoc_STR("() -> (const char * _rv)")},
	{"get_logger_1", (PyCFunction)PyAm_get_logger_1, 1,
	 PyDoc_STR("(char* name) -> (ambulant::lib::logger* _rv)")},
	{"get_logger_2", (PyCFunction)PyAm_get_logger_2, 1,
	 PyDoc_STR("(char* name, int pos) -> (ambulant::lib::logger* _rv)")},
	{"set_loggers_level", (PyCFunction)PyAm_set_loggers_level, 1,
	 PyDoc_STR("(int level) -> None")},
	{"get_level_name", (PyCFunction)PyAm_get_level_name, 1,
	 PyDoc_STR("(int level) -> (const char * _rv)")},
	{"get_builtin_node_factory", (PyCFunction)PyAm_get_builtin_node_factory, 1,
	 PyDoc_STR("() -> (ambulant::lib::node_factory* _rv)")},
	{"create_from_url", (PyCFunction)PyAm_create_from_url, 1,
	 PyDoc_STR("(ambulant::common::factories* factory, ambulant::net::url u) -> (ambulant::lib::document* _rv)")},
	{"create_from_file", (PyCFunction)PyAm_create_from_file, 1,
	 PyDoc_STR("(ambulant::common::factories* factory, std::string filename) -> (ambulant::lib::document* _rv)")},
	{"create_from_string", (PyCFunction)PyAm_create_from_string, 1,
	 PyDoc_STR("(ambulant::common::factories* factory, std::string smil_src, std::string src_id) -> (ambulant::lib::document* _rv)")},
	{"event_processor_factory", (PyCFunction)PyAm_event_processor_factory, 1,
	 PyDoc_STR("(ambulant::lib::timer* t) -> (ambulant::lib::event_processor* _rv)")},
	{"get_parser_factory", (PyCFunction)PyAm_get_parser_factory, 1,
	 PyDoc_STR("() -> (ambulant::lib::global_parser_factory* _rv)")},
	{"realtime_timer_factory", (PyCFunction)PyAm_realtime_timer_factory, 1,
	 PyDoc_STR("() -> (ambulant::lib::timer* _rv)")},
	{"from_node", (PyCFunction)PyAm_from_node, 1,
	 PyDoc_STR("(ambulant::lib::node* n) -> (ambulant::lib::transition_info* _rv)")},
	{"create_smil2_layout_manager", (PyCFunction)PyAm_create_smil2_layout_manager, 1,
	 PyDoc_STR("(ambulant::common::factories* factory, ambulant::lib::document* doc) -> (ambulant::common::layout_manager* _rv)")},
	{"create_smil_surface_factory", (PyCFunction)PyAm_create_smil_surface_factory, 1,
	 PyDoc_STR("() -> (ambulant::common::surface_factory* _rv)")},
	{"get_global_playable_factory", (PyCFunction)PyAm_get_global_playable_factory, 1,
	 PyDoc_STR("() -> (ambulant::common::global_playable_factory* _rv)")},
	{"create_smil2_player", (PyCFunction)PyAm_create_smil2_player, 1,
	 PyDoc_STR("(ambulant::lib::document* doc, ambulant::common::factories* factory, ambulant::common::embedder* sys) -> (ambulant::common::player* _rv)")},
	{"get_global_state_component_factory", (PyCFunction)PyAm_get_global_state_component_factory, 1,
	 PyDoc_STR("() -> (ambulant::common::global_state_component_factory* _rv)")},
	{"create_none_window_factory", (PyCFunction)PyAm_create_none_window_factory, 1,
	 PyDoc_STR("() -> (ambulant::common::window_factory* _rv)")},

#ifdef WITH_GTK
	{"create_gtk_window_factory_unsafe", (PyCFunction)PyAm_create_gtk_window_factory_unsafe, 1,
	 PyDoc_STR("(void * gtk_parent_widget, ambulant::common::gui_player* gpl) -> (ambulant::common::window_factory* _rv)")},
#endif

#ifdef WITH_GTK
	{"create_gtk_video_playable_factory", (PyCFunction)PyAm_create_gtk_video_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::factories* factory) -> (ambulant::common::playable_factory* _rv)")},
#endif

#ifdef WITH_GTK
	{"create_gtk_text_playable_factory", (PyCFunction)PyAm_create_gtk_text_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::factories* factory) -> (ambulant::common::playable_factory* _rv)")},
#endif

#ifdef WITH_GTK
	{"create_gtk_smiltext_playable_factory", (PyCFunction)PyAm_create_gtk_smiltext_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::factories* factory) -> (ambulant::common::playable_factory* _rv)")},
#endif

#ifdef WITH_GTK
	{"create_gtk_image_playable_factory", (PyCFunction)PyAm_create_gtk_image_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::factories* factory) -> (ambulant::common::playable_factory* _rv)")},
#endif

#ifdef WITH_GTK
	{"create_gtk_fill_playable_factory", (PyCFunction)PyAm_create_gtk_fill_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::factories* factory) -> (ambulant::common::playable_factory* _rv)")},
#endif

#ifdef WITH_SDL
	{"create_sdl_playable_factory", (PyCFunction)PyAm_create_sdl_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::factories* factory) -> (ambulant::common::playable_factory* _rv)")},
#endif

#if defined(WITH_SDL2) && defined(WITH_SDL_IMAGE)
	{"create_sdl_video_playable_factory", (PyCFunction)PyAm_create_sdl_video_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::factories* factory) -> (ambulant::common::playable_factory* _rv)")},
#endif

#if defined(WITH_SDL2) && defined(WITH_SDL_IMAGE)
	{"create_sdl_text_playable_factory", (PyCFunction)PyAm_create_sdl_text_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::factories* factory) -> (ambulant::common::playable_factory* _rv)")},
#endif

#if defined(WITH_SDL2) && (defined(WITH_SDL_TTF) || defined(WITH_SDL_PANGO))
	{"create_sdl_smiltext_playable_factory", (PyCFunction)PyAm_create_sdl_smiltext_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::factories* factory) -> (ambulant::common::playable_factory* _rv)")},
#endif

#if defined(WITH_SDL2) && defined(WITH_SDL_IMAGE)
	{"create_sdl_image_playable_factory", (PyCFunction)PyAm_create_sdl_image_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::factories* factory) -> (ambulant::common::playable_factory* _rv)")},
#endif

#if defined(WITH_SDL2) && defined(WITH_SDL_IMAGE)
	{"create_sdl_fill_playable_factory", (PyCFunction)PyAm_create_sdl_fill_playable_factory, 1,
	 PyDoc_STR("(ambulant::common::factories* factory) -> (ambulant::common::playable_factory* _rv)")},
#endif
	{"read_data_from_url", (PyCFunction)PyAm_read_data_from_url, 1,
	 PyDoc_STR("(ambulant::net::url url, ambulant::net::datasource_factory* df, Buffer result) -> (bool _rv, Buffer result)")},
	{"read_data_from_datasource", (PyCFunction)PyAm_read_data_from_datasource, 1,
	 PyDoc_STR("(ambulant::net::datasource* src, Buffer result) -> (bool _rv, Buffer result)")},

#ifndef AMBULANT_PLATFORM_WIN32
	{"create_posix_datasource_factory", (PyCFunction)PyAm_create_posix_datasource_factory, 1,
	 PyDoc_STR("() -> (ambulant::net::raw_datasource_factory* _rv)")},
#endif
	{"create_stdio_datasource_factory", (PyCFunction)PyAm_create_stdio_datasource_factory, 1,
	 PyDoc_STR("() -> (ambulant::net::raw_datasource_factory* _rv)")},

#ifdef WITH_FFMPEG
	{"get_ffmpeg_raw_datasource_factory", (PyCFunction)PyAm_get_ffmpeg_raw_datasource_factory, 1,
	 PyDoc_STR("() -> (ambulant::net::raw_datasource_factory* _rv)")},
#endif

#ifdef WITH_FFMPEG
	{"get_ffmpeg_video_datasource_factory", (PyCFunction)PyAm_get_ffmpeg_video_datasource_factory, 1,
	 PyDoc_STR("() -> (ambulant::net::video_datasource_factory* _rv)")},
#endif

#ifdef WITH_FFMPEG
	{"get_ffmpeg_audio_datasource_factory", (PyCFunction)PyAm_get_ffmpeg_audio_datasource_factory, 1,
	 PyDoc_STR("() -> (ambulant::net::audio_datasource_factory* _rv)")},
#endif

#ifdef WITH_FFMPEG
	{"get_ffmpeg_audio_decoder_finder", (PyCFunction)PyAm_get_ffmpeg_audio_decoder_finder, 1,
	 PyDoc_STR("() -> (ambulant::net::audio_decoder_finder* _rv)")},
#endif

#if defined(WITH_FFMPEG) && defined(WITH_RESAMPLE_DATASOURCE)
	{"get_ffmpeg_audio_filter_finder", (PyCFunction)PyAm_get_ffmpeg_audio_filter_finder, 1,
	 PyDoc_STR("() -> (ambulant::net::audio_filter_finder* _rv)")},
#endif
	{NULL, NULL, 0}
};


// Helper routines to enable object identity to be maintained
// across the bridge:

cpppybridge *
pycppbridge_getwrapper(PyObject *o)
{
    if (!pycppbridge_Check(o)) {
    	PyErr_Warn(PyExc_Warning, "ambulant: Passing non-pycppbridge object to C++");
    	return NULL;
    }
    pycppbridgeObject *bo = (pycppbridgeObject *)o;
    return bo->ob_wrapper;
}

void
pycppbridge_setwrapper(PyObject *o, cpppybridge *w)
{
    if (!pycppbridge_Check(o)) {
        PyErr_SetString(PyExc_SystemError, "ambulant: attempt to set wrapper for non-bridged object");
    } else {
        pycppbridgeObject *bo = (pycppbridgeObject *)o;
        if (bo->ob_wrapper)
            PyErr_SetString(PyExc_SystemError, "ambulant: attempt to set wrapper second time");
        bo->ob_wrapper = w;
    }
}

// Declare initambulant as a C external:

extern "C" void initambulant(); 


void initambulant(void)
{
	PyObject *m;
	PyObject *d;


	PyEval_InitThreads();
#ifdef WITH_GTK2
	init_pygobject();
	init_pygtk();
	PyObject *module = PyImport_ImportModule("gobject");
	if (module)
	    PyGObject_Type = (PyTypeObject*)PyObject_GetAttrString(module, "GObject");
	Py_XDECREF(module);
#endif // WITH_GTK2


	m = Py_InitModule("ambulant", PyAm_methods);
	d = PyModule_GetDict(m);
	PyAm_Error = PyErr_NewException("ambulant.Error", NULL, NULL);
	if (PyAm_Error == NULL ||
	    PyDict_SetItemString(d, "Error", PyAm_Error) != 0)
		return;
	pycppbridge_Type.ob_type = &PyType_Type;
	if (PyType_Ready(&pycppbridge_Type) < 0) return;
	Py_INCREF(&pycppbridge_Type);
	PyModule_AddObject(m, "pycppbridge", (PyObject *)&pycppbridge_Type);
	ostream_Type.ob_type = &PyType_Type;
	ostream_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&ostream_Type) < 0) return;
	Py_INCREF(&ostream_Type);
	PyModule_AddObject(m, "ostream", (PyObject *)&ostream_Type);
	logger_Type.ob_type = &PyType_Type;
	logger_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&logger_Type) < 0) return;
	Py_INCREF(&logger_Type);
	PyModule_AddObject(m, "logger", (PyObject *)&logger_Type);
	node_context_Type.ob_type = &PyType_Type;
	node_context_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&node_context_Type) < 0) return;
	Py_INCREF(&node_context_Type);
	PyModule_AddObject(m, "node_context", (PyObject *)&node_context_Type);
	node_Type.ob_type = &PyType_Type;
	node_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&node_Type) < 0) return;
	Py_INCREF(&node_Type);
	PyModule_AddObject(m, "node", (PyObject *)&node_Type);
	node_factory_Type.ob_type = &PyType_Type;
	node_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&node_factory_Type) < 0) return;
	Py_INCREF(&node_factory_Type);
	PyModule_AddObject(m, "node_factory", (PyObject *)&node_factory_Type);
	document_Type.ob_type = &PyType_Type;
	document_Type.tp_base = &node_context_Type;
	if (PyType_Ready(&document_Type) < 0) return;
	Py_INCREF(&document_Type);
	PyModule_AddObject(m, "document", (PyObject *)&document_Type);
	event_Type.ob_type = &PyType_Type;
	event_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&event_Type) < 0) return;
	Py_INCREF(&event_Type);
	PyModule_AddObject(m, "event", (PyObject *)&event_Type);
	event_processor_Type.ob_type = &PyType_Type;
	event_processor_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&event_processor_Type) < 0) return;
	Py_INCREF(&event_processor_Type);
	PyModule_AddObject(m, "event_processor", (PyObject *)&event_processor_Type);
	parser_factory_Type.ob_type = &PyType_Type;
	parser_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&parser_factory_Type) < 0) return;
	Py_INCREF(&parser_factory_Type);
	PyModule_AddObject(m, "parser_factory", (PyObject *)&parser_factory_Type);
	global_parser_factory_Type.ob_type = &PyType_Type;
	global_parser_factory_Type.tp_base = &parser_factory_Type;
	if (PyType_Ready(&global_parser_factory_Type) < 0) return;
	Py_INCREF(&global_parser_factory_Type);
	PyModule_AddObject(m, "global_parser_factory", (PyObject *)&global_parser_factory_Type);
	xml_parser_Type.ob_type = &PyType_Type;
	xml_parser_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&xml_parser_Type) < 0) return;
	Py_INCREF(&xml_parser_Type);
	PyModule_AddObject(m, "xml_parser", (PyObject *)&xml_parser_Type);
	system_embedder_Type.ob_type = &PyType_Type;
	system_embedder_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&system_embedder_Type) < 0) return;
	Py_INCREF(&system_embedder_Type);
	PyModule_AddObject(m, "system_embedder", (PyObject *)&system_embedder_Type);
	timer_Type.ob_type = &PyType_Type;
	timer_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&timer_Type) < 0) return;
	Py_INCREF(&timer_Type);
	PyModule_AddObject(m, "timer", (PyObject *)&timer_Type);
	timer_control_Type.ob_type = &PyType_Type;
	timer_control_Type.tp_base = &timer_Type;
	if (PyType_Ready(&timer_control_Type) < 0) return;
	Py_INCREF(&timer_control_Type);
	PyModule_AddObject(m, "timer_control", (PyObject *)&timer_control_Type);
	timer_control_impl_Type.ob_type = &PyType_Type;
	timer_control_impl_Type.tp_base = &timer_control_Type;
	if (PyType_Ready(&timer_control_impl_Type) < 0) return;
	Py_INCREF(&timer_control_impl_Type);
	PyModule_AddObject(m, "timer_control_impl", (PyObject *)&timer_control_impl_Type);
	timer_observer_Type.ob_type = &PyType_Type;
	timer_observer_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&timer_observer_Type) < 0) return;
	Py_INCREF(&timer_observer_Type);
	PyModule_AddObject(m, "timer_observer", (PyObject *)&timer_observer_Type);
	timer_sync_Type.ob_type = &PyType_Type;
	timer_sync_Type.tp_base = &timer_observer_Type;
	if (PyType_Ready(&timer_sync_Type) < 0) return;
	Py_INCREF(&timer_sync_Type);
	PyModule_AddObject(m, "timer_sync", (PyObject *)&timer_sync_Type);
	timer_sync_factory_Type.ob_type = &PyType_Type;
	timer_sync_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&timer_sync_factory_Type) < 0) return;
	Py_INCREF(&timer_sync_factory_Type);
	PyModule_AddObject(m, "timer_sync_factory", (PyObject *)&timer_sync_factory_Type);
	transition_info_Type.ob_type = &PyType_Type;
	transition_info_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&transition_info_Type) < 0) return;
	Py_INCREF(&transition_info_Type);
	PyModule_AddObject(m, "transition_info", (PyObject *)&transition_info_Type);
	embedder_Type.ob_type = &PyType_Type;
	embedder_Type.tp_base = &system_embedder_Type;
	if (PyType_Ready(&embedder_Type) < 0) return;
	Py_INCREF(&embedder_Type);
	PyModule_AddObject(m, "embedder", (PyObject *)&embedder_Type);
	factories_Type.ob_type = &PyType_Type;
	factories_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&factories_Type) < 0) return;
	Py_INCREF(&factories_Type);
	PyModule_AddObject(m, "factories", (PyObject *)&factories_Type);
	gui_screen_Type.ob_type = &PyType_Type;
	gui_screen_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&gui_screen_Type) < 0) return;
	Py_INCREF(&gui_screen_Type);
	PyModule_AddObject(m, "gui_screen", (PyObject *)&gui_screen_Type);
	gui_player_Type.ob_type = &PyType_Type;
	gui_player_Type.tp_base = &factories_Type;
	if (PyType_Ready(&gui_player_Type) < 0) return;
	Py_INCREF(&gui_player_Type);
	PyModule_AddObject(m, "gui_player", (PyObject *)&gui_player_Type);
	alignment_Type.ob_type = &PyType_Type;
	alignment_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&alignment_Type) < 0) return;
	Py_INCREF(&alignment_Type);
	PyModule_AddObject(m, "alignment", (PyObject *)&alignment_Type);
	animation_notification_Type.ob_type = &PyType_Type;
	animation_notification_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&animation_notification_Type) < 0) return;
	Py_INCREF(&animation_notification_Type);
	PyModule_AddObject(m, "animation_notification", (PyObject *)&animation_notification_Type);
	gui_window_Type.ob_type = &PyType_Type;
	gui_window_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&gui_window_Type) < 0) return;
	Py_INCREF(&gui_window_Type);
	PyModule_AddObject(m, "gui_window", (PyObject *)&gui_window_Type);
	gui_events_Type.ob_type = &PyType_Type;
	gui_events_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&gui_events_Type) < 0) return;
	Py_INCREF(&gui_events_Type);
	PyModule_AddObject(m, "gui_events", (PyObject *)&gui_events_Type);
	renderer_Type.ob_type = &PyType_Type;
	renderer_Type.tp_base = &gui_events_Type;
	if (PyType_Ready(&renderer_Type) < 0) return;
	Py_INCREF(&renderer_Type);
	PyModule_AddObject(m, "renderer", (PyObject *)&renderer_Type);
	bgrenderer_Type.ob_type = &PyType_Type;
	bgrenderer_Type.tp_base = &gui_events_Type;
	if (PyType_Ready(&bgrenderer_Type) < 0) return;
	Py_INCREF(&bgrenderer_Type);
	PyModule_AddObject(m, "bgrenderer", (PyObject *)&bgrenderer_Type);
	surface_Type.ob_type = &PyType_Type;
	surface_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&surface_Type) < 0) return;
	Py_INCREF(&surface_Type);
	PyModule_AddObject(m, "surface", (PyObject *)&surface_Type);
	window_factory_Type.ob_type = &PyType_Type;
	window_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&window_factory_Type) < 0) return;
	Py_INCREF(&window_factory_Type);
	PyModule_AddObject(m, "window_factory", (PyObject *)&window_factory_Type);
	surface_template_Type.ob_type = &PyType_Type;
	surface_template_Type.tp_base = &animation_notification_Type;
	if (PyType_Ready(&surface_template_Type) < 0) return;
	Py_INCREF(&surface_template_Type);
	PyModule_AddObject(m, "surface_template", (PyObject *)&surface_template_Type);
	surface_factory_Type.ob_type = &PyType_Type;
	surface_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&surface_factory_Type) < 0) return;
	Py_INCREF(&surface_factory_Type);
	PyModule_AddObject(m, "surface_factory", (PyObject *)&surface_factory_Type);
	layout_manager_Type.ob_type = &PyType_Type;
	layout_manager_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&layout_manager_Type) < 0) return;
	Py_INCREF(&layout_manager_Type);
	PyModule_AddObject(m, "layout_manager", (PyObject *)&layout_manager_Type);
	playable_Type.ob_type = &PyType_Type;
	playable_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&playable_Type) < 0) return;
	Py_INCREF(&playable_Type);
	PyModule_AddObject(m, "playable", (PyObject *)&playable_Type);
	playable_notification_Type.ob_type = &PyType_Type;
	playable_notification_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&playable_notification_Type) < 0) return;
	Py_INCREF(&playable_notification_Type);
	PyModule_AddObject(m, "playable_notification", (PyObject *)&playable_notification_Type);
	playable_factory_Type.ob_type = &PyType_Type;
	playable_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&playable_factory_Type) < 0) return;
	Py_INCREF(&playable_factory_Type);
	PyModule_AddObject(m, "playable_factory", (PyObject *)&playable_factory_Type);
	global_playable_factory_Type.ob_type = &PyType_Type;
	global_playable_factory_Type.tp_base = &playable_factory_Type;
	if (PyType_Ready(&global_playable_factory_Type) < 0) return;
	Py_INCREF(&global_playable_factory_Type);
	PyModule_AddObject(m, "global_playable_factory", (PyObject *)&global_playable_factory_Type);
	recorder_Type.ob_type = &PyType_Type;
	recorder_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&recorder_Type) < 0) return;
	Py_INCREF(&recorder_Type);
	PyModule_AddObject(m, "recorder", (PyObject *)&recorder_Type);
	recorder_factory_Type.ob_type = &PyType_Type;
	recorder_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&recorder_factory_Type) < 0) return;
	Py_INCREF(&recorder_factory_Type);
	PyModule_AddObject(m, "recorder_factory", (PyObject *)&recorder_factory_Type);
	renderer_select_Type.ob_type = &PyType_Type;
	renderer_select_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&renderer_select_Type) < 0) return;
	Py_INCREF(&renderer_select_Type);
	PyModule_AddObject(m, "renderer_select", (PyObject *)&renderer_select_Type);
	focus_feedback_Type.ob_type = &PyType_Type;
	focus_feedback_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&focus_feedback_Type) < 0) return;
	Py_INCREF(&focus_feedback_Type);
	PyModule_AddObject(m, "focus_feedback", (PyObject *)&focus_feedback_Type);
	player_feedback_Type.ob_type = &PyType_Type;
	player_feedback_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&player_feedback_Type) < 0) return;
	Py_INCREF(&player_feedback_Type);
	PyModule_AddObject(m, "player_feedback", (PyObject *)&player_feedback_Type);
	player_Type.ob_type = &PyType_Type;
	player_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&player_Type) < 0) return;
	Py_INCREF(&player_Type);
	PyModule_AddObject(m, "player", (PyObject *)&player_Type);
	region_info_Type.ob_type = &PyType_Type;
	region_info_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&region_info_Type) < 0) return;
	Py_INCREF(&region_info_Type);
	PyModule_AddObject(m, "region_info", (PyObject *)&region_info_Type);
	animation_destination_Type.ob_type = &PyType_Type;
	animation_destination_Type.tp_base = &region_info_Type;
	if (PyType_Ready(&animation_destination_Type) < 0) return;
	Py_INCREF(&animation_destination_Type);
	PyModule_AddObject(m, "animation_destination", (PyObject *)&animation_destination_Type);
	state_test_methods_Type.ob_type = &PyType_Type;
	state_test_methods_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&state_test_methods_Type) < 0) return;
	Py_INCREF(&state_test_methods_Type);
	PyModule_AddObject(m, "state_test_methods", (PyObject *)&state_test_methods_Type);
	state_change_callback_Type.ob_type = &PyType_Type;
	state_change_callback_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&state_change_callback_Type) < 0) return;
	Py_INCREF(&state_change_callback_Type);
	PyModule_AddObject(m, "state_change_callback", (PyObject *)&state_change_callback_Type);
	state_component_Type.ob_type = &PyType_Type;
	state_component_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&state_component_Type) < 0) return;
	Py_INCREF(&state_component_Type);
	PyModule_AddObject(m, "state_component", (PyObject *)&state_component_Type);
	state_component_factory_Type.ob_type = &PyType_Type;
	state_component_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&state_component_factory_Type) < 0) return;
	Py_INCREF(&state_component_factory_Type);
	PyModule_AddObject(m, "state_component_factory", (PyObject *)&state_component_factory_Type);
	global_state_component_factory_Type.ob_type = &PyType_Type;
	global_state_component_factory_Type.tp_base = &state_component_factory_Type;
	if (PyType_Ready(&global_state_component_factory_Type) < 0) return;
	Py_INCREF(&global_state_component_factory_Type);
	PyModule_AddObject(m, "global_state_component_factory", (PyObject *)&global_state_component_factory_Type);
	none_window_Type.ob_type = &PyType_Type;
	none_window_Type.tp_base = &gui_window_Type;
	if (PyType_Ready(&none_window_Type) < 0) return;
	Py_INCREF(&none_window_Type);
	PyModule_AddObject(m, "none_window", (PyObject *)&none_window_Type);
	none_window_factory_Type.ob_type = &PyType_Type;
	none_window_factory_Type.tp_base = &window_factory_Type;
	if (PyType_Ready(&none_window_factory_Type) < 0) return;
	Py_INCREF(&none_window_factory_Type);
	PyModule_AddObject(m, "none_window_factory", (PyObject *)&none_window_factory_Type);
	datasource_Type.ob_type = &PyType_Type;
	datasource_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&datasource_Type) < 0) return;
	Py_INCREF(&datasource_Type);
	PyModule_AddObject(m, "datasource", (PyObject *)&datasource_Type);
	audio_datasource_Type.ob_type = &PyType_Type;
	audio_datasource_Type.tp_base = &datasource_Type;
	if (PyType_Ready(&audio_datasource_Type) < 0) return;
	Py_INCREF(&audio_datasource_Type);
	PyModule_AddObject(m, "audio_datasource", (PyObject *)&audio_datasource_Type);
	video_datasource_Type.ob_type = &PyType_Type;
	video_datasource_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&video_datasource_Type) < 0) return;
	Py_INCREF(&video_datasource_Type);
	PyModule_AddObject(m, "video_datasource", (PyObject *)&video_datasource_Type);
	datasource_factory_Type.ob_type = &PyType_Type;
	datasource_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&datasource_factory_Type) < 0) return;
	Py_INCREF(&datasource_factory_Type);
	PyModule_AddObject(m, "datasource_factory", (PyObject *)&datasource_factory_Type);
	raw_datasource_factory_Type.ob_type = &PyType_Type;
	raw_datasource_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&raw_datasource_factory_Type) < 0) return;
	Py_INCREF(&raw_datasource_factory_Type);
	PyModule_AddObject(m, "raw_datasource_factory", (PyObject *)&raw_datasource_factory_Type);
	audio_datasource_factory_Type.ob_type = &PyType_Type;
	audio_datasource_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&audio_datasource_factory_Type) < 0) return;
	Py_INCREF(&audio_datasource_factory_Type);
	PyModule_AddObject(m, "audio_datasource_factory", (PyObject *)&audio_datasource_factory_Type);
	pkt_datasource_factory_Type.ob_type = &PyType_Type;
	pkt_datasource_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&pkt_datasource_factory_Type) < 0) return;
	Py_INCREF(&pkt_datasource_factory_Type);
	PyModule_AddObject(m, "pkt_datasource_factory", (PyObject *)&pkt_datasource_factory_Type);
	video_datasource_factory_Type.ob_type = &PyType_Type;
	video_datasource_factory_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&video_datasource_factory_Type) < 0) return;
	Py_INCREF(&video_datasource_factory_Type);
	PyModule_AddObject(m, "video_datasource_factory", (PyObject *)&video_datasource_factory_Type);
	audio_parser_finder_Type.ob_type = &PyType_Type;
	audio_parser_finder_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&audio_parser_finder_Type) < 0) return;
	Py_INCREF(&audio_parser_finder_Type);
	PyModule_AddObject(m, "audio_parser_finder", (PyObject *)&audio_parser_finder_Type);
	audio_filter_finder_Type.ob_type = &PyType_Type;
	audio_filter_finder_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&audio_filter_finder_Type) < 0) return;
	Py_INCREF(&audio_filter_finder_Type);
	PyModule_AddObject(m, "audio_filter_finder", (PyObject *)&audio_filter_finder_Type);
	audio_decoder_finder_Type.ob_type = &PyType_Type;
	audio_decoder_finder_Type.tp_base = &pycppbridge_Type;
	if (PyType_Ready(&audio_decoder_finder_Type) < 0) return;
	Py_INCREF(&audio_decoder_finder_Type);
	PyModule_AddObject(m, "audio_decoder_finder", (PyObject *)&audio_decoder_finder_Type);
	audio_format_choices_Type.ob_type = &PyType_Type;
	if (PyType_Ready(&audio_format_choices_Type) < 0) return;
	Py_INCREF(&audio_format_choices_Type);
	PyModule_AddObject(m, "audio_format_choices", (PyObject *)&audio_format_choices_Type);



}

/* ====================== End module ambulant ======================= */

