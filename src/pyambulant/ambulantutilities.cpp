#include "ambulantinterface.h"
#include "ambulantutilities.h"

/*
** Parse/generate various objects
*/
PyObject *
bool_New(bool itself)
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

int
cobject_Convert(PyObject *v, void **p_itself)
{
	if (!PyCObject_Check(v)) return 0;
	*p_itself = PyCObject_AsVoidPtr(v);
	return 1;
}
PyObject *
ambulant_url_New(const ambulant::net::url& itself)
{
	return PyString_FromString(itself.get_url().c_str());
}

int
ambulant_url_Convert(PyObject *v, ambulant::net::url *p_itself)
{
	char *cstr = PyString_AsString(v);
	if (cstr == NULL) return 0;
	std::string cxxstr = cstr;
	ambulant::net::url url = ambulant::net::url::from_url(cxxstr);
	*p_itself = url;
	return 1;
}


PyObject *
ambulant_region_dim_New(const ambulant::common::region_dim& itself)
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


PyObject *
ambulant_rect_New(const ambulant::lib::rect& itself)
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

PyObject *
ambulant_point_New(const ambulant::lib::point& itself)
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

PyObject *
ambulant_size_New(const ambulant::lib::size& itself)
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

PyObject *
ambulant_attributes_list_New(const ambulant::lib::q_attributes_list& itself)
{
	int nitems = itself.size();
	PyObject *rv = PyList_New(nitems);
	if (!rv) return NULL;
	int pos = 0;
	ambulant::lib::q_attributes_list::const_iterator i;
	for(i=itself.begin(); i!= itself.end(); i++) {
		const ambulant::lib::q_attribute_pair& item = *i;
		PyObject *pitem = Py_BuildValue("(ss)s",
			item.first.first.c_str(),
			item.first.second.c_str(),
			item.second.c_str());
		if (!pitem) goto fail;
		if (PyList_SetItem(rv, pos, pitem) < 0) {
			Py_DECREF(pitem);
			goto fail;
		}
		pos++;
	}
	return rv;
fail:
	Py_DECREF(rv);
	return NULL;
}

int
ambulant_attributes_list_Convert(PyObject *v, ambulant::lib::q_attributes_list *p_itself)
{
	if (!PySequence_Check(v)) return 0;
	int i;
	int size = PySequence_Size(v);
	for (i=0; i<size; i++) {
		PyObject *item = PySequence_GetItem(v, i);
		if (item==NULL) return 0;
		char *attrns=NULL, *attrname=NULL, *attrvalue=NULL;
		if (!PyArg_ParseTuple(item, "(ss)s", &attrns, &attrname, &attrvalue)) {
			Py_DECREF(item);
			return 0;
		}
		Py_DECREF(item);
		ambulant::lib::q_attribute_pair citem =
			ambulant::lib::q_attribute_pair(
				ambulant::lib::q_name_pair(
					ambulant::lib::xml_string(attrns),
					ambulant::lib::xml_string(attrname)),
				ambulant::lib::xml_string(attrvalue));
		p_itself->push_back(citem);
	}
	return 1;
}

