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

PyObject *ambulant_url_New(ambulant::net::url& itself)
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


PyObject *ambulant_screen_rect_New(ambulant::lib::screen_rect_int& itself)
{
    return Py_BuildValue("llll", itself.left(), itself.top(), itself.right(), itself.bottom());
}

int
ambulant_screen_rect_Convert(PyObject *v, ambulant::lib::screen_rect_int *p_itself)
{
    int l, t, r, b;
    
    if (!PyArg_Parse(v, "llll", &l, &t, &r, &b))
        return 0;
    p_itself->set_coord(l, t, r, b);
    return 1;
}

PyObject *ambulant_rect_New(ambulant::lib::rect& itself)
{
    return Py_BuildValue("llll", itself.left(), itself.top(), itself.right(), itself.bottom());
}

int
ambulant_rect_Convert(PyObject *v, ambulant::lib::rect *p_itself)
{
    int l, t, r, b;
    
    if (!PyArg_Parse(v, "llll", &l, &t, &r, &b))
        return 0;
    *p_itself = ambulant::lib::rect(
                ambulant::lib::point(l, t),
                ambulant::lib::size(r-l, b-t));
    return 1;
}

PyObject *ambulant_point_New(ambulant::lib::point& itself)
{
    return Py_BuildValue("ll", itself.x, itself.y);
}

int
ambulant_point_Convert(PyObject *v, ambulant::lib::point *p_itself)
{
    if (!PyArg_Parse(v, "ll", &p_itself->x, &p_itself->y))
        return 0;
    return 1;
}

PyObject *ambulant_size_New(ambulant::lib::size& itself)
{
    return Py_BuildValue("ll", itself.w, itself.h);
}

int
ambulant_size_Convert(PyObject *v, ambulant::lib::size *p_itself)
{
    if (!PyArg_Parse(v, "ll", &p_itself->w, &p_itself->h))
        return 0;
    return 1;
}

