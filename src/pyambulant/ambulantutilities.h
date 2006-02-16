/*
** Parse/generate various objects
*/

PyObject *bool_New(bool itself);
int bool_Convert(PyObject *v, bool *p_itself);

PyObject *ambulant_url_New(const ambulant::net::url& itself);
int ambulant_url_Convert(PyObject *v, ambulant::net::url *p_itself);

PyObject *ambulant_region_dim_New(const ambulant::common::region_dim& itself);
int ambulant_region_dim_Convert(PyObject *v, ambulant::common::region_dim *p_itself);

PyObject *ambulant_rect_New(const ambulant::lib::rect& itself);
int ambulant_rect_Convert(PyObject *v, ambulant::lib::rect *p_itself);

PyObject *ambulant_point_New(const ambulant::lib::point& itself);
int ambulant_point_Convert(PyObject *v, ambulant::lib::point *p_itself);

PyObject *ambulant_size_New(const ambulant::lib::size& itself);
int ambulant_size_Convert(PyObject *v, ambulant::lib::size *p_itself);

PyObject *ambulant_attributes_list_New(const ambulant::lib::q_attributes_list& itself);
int ambulant_attributes_list_Convert(PyObject *v, ambulant::lib::q_attributes_list *p_itself);