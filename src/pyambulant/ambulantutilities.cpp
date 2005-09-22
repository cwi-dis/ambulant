#include "ambulantinterface.h"
#include "ambulantutilities.h"

/*
** Parse/generate various objects
*/
PyObject *bool_New(bool itself)
{
    if (itself) {
        Py_RETURN_TRUE;
    } 
    Py_RETURN_FALSE;
}

int
bool_Convert(PyObject *v, bool *p_itself)
{
    int istrue = PyObject_IsTrue(v);
    if (istrue < 0) return 0;
    *p_itself = (istrue > 0);
    return 1;
}

PyObject *ambulant_url_New(const ambulant::net::url& itself)
{
    return PyString_FromString(itself.get_url().c_str());
}

int
ambulant_url_Convert(PyObject *v, ambulant::net::url *p_itself)
{
    char *cstr = PyString_AsString(v);
    if (cstr == NULL) return 0;
    std::string cxxstr = cstr;
    ambulant::net::url url(cxxstr);
    *p_itself = url;
    return 1;
}


PyObject *ambulant_region_dim_New(const ambulant::common::region_dim& itself)
{
    if (itself.absolute())
        return Py_BuildValue("sl", "auto", 0);
    else if (itself.relative())
        return Py_BuildValue("sd", "relative", itself.get_as_dbl());
    else
        return Py_BuildValue("sl", "absolute", itself.get_as_int());
}

int
ambulant_region_dim_Convert(PyObject *v, ambulant::common::region_dim *p_itself)
{
    char *tp;
    double dv;
    int iv;
    
    if (PyArg_ParseTuple(v, "sd", &tp, &dv)) {
        if (strcmp(tp, "auto") == 0) {
            *p_itself = ambulant::common::region_dim();
            return 1;
        }
        if (strcmp(tp, "relative") == 0) {
            *p_itself = ambulant::common::region_dim(dv);
            return 1;
        }
        if (strcmp(tp, "absolute") == 0) {
            *p_itself = ambulant::common::region_dim((int)dv);
            return 1;
        }
        PyErr_SetString(PyExc_TypeError, "region_dim type should be auto, relative or absolute");
        return 0;
    }
    if (PyArg_Parse(v, "d", &dv)) {
        *p_itself = ambulant::common::region_dim(dv);
        return 1;
    }
    if (PyArg_Parse(v, "l", &iv)) {
        *p_itself = ambulant::common::region_dim(iv);
        return 1;
    }
    PyErr_SetString(PyExc_TypeError, "region_dim should be int, double or (type, value) tuple");
    return 0;
}


PyObject *ambulant_rect_New(const ambulant::lib::rect& itself)
{
    return Py_BuildValue("llll", itself.left(), itself.top(), itself.right(), itself.bottom());
}

int
ambulant_rect_Convert(PyObject *v, ambulant::lib::rect *p_itself)
{
    int l, t, r, b;
    
    if (!PyArg_ParseTuple(v, "llll", &l, &t, &r, &b))
        return 0;
    *p_itself = ambulant::lib::rect(
                ambulant::lib::point(l, t),
                ambulant::lib::size(r-l, b-t));
    return 1;
}

PyObject *ambulant_point_New(const ambulant::lib::point& itself)
{
    return Py_BuildValue("ll", itself.x, itself.y);
}

int
ambulant_point_Convert(PyObject *v, ambulant::lib::point *p_itself)
{
    if (!PyArg_ParseTuple(v, "ll", &p_itself->x, &p_itself->y))
        return 0;
    return 1;
}

PyObject *ambulant_size_New(const ambulant::lib::size& itself)
{
    return Py_BuildValue("ll", itself.w, itself.h);
}

int
ambulant_size_Convert(PyObject *v, ambulant::lib::size *p_itself)
{
    if (!PyArg_ParseTuple(v, "ll", &p_itself->w, &p_itself->h))
        return 0;
    return 1;
}

