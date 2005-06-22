/*
** Parse/generate various objects
*/

PyObject *bool_New(bool itself);
int bool_Convert(PyObject *v, bool *p_itself);

PyObject *cxx_std_string_New(std::string& itself);
int cxx_std_string_Convert(PyObject *v, std::string *p_itself);

PyObject *ambulant_url_New(ambulant::net::url& itself);
int ambulant_url_Convert(PyObject *v, ambulant::net::url *p_itself);

PyObject *ambulant_screen_rect_New(ambulant::lib::screen_rect_int& itself);
int ambulant_screen_rect_Convert(PyObject *v, ambulant::lib::screen_rect_int *p_itself);

PyObject *ambulant_rect_New(ambulant::lib::rect& itself);
int ambulant_rect_Convert(PyObject *v, ambulant::lib::rect *p_itself);

PyObject *ambulant_point_New(ambulant::lib::point& itself);
int ambulant_point_Convert(PyObject *v, ambulant::lib::point *p_itself);

PyObject *ambulant_size_New(ambulant::lib::size& itself);
int ambulant_size_Convert(PyObject *v, ambulant::lib::size *p_itself);
