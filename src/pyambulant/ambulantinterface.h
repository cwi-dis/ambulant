
/* ==== Declaration of C++ to Python callback module pyambulant ===== */

#include "Python.h"

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

#include "ambulant/lib/node_navigator.h"


/* ============ Glue classes to maintain object identity ============ */

class cpppybridge {
  public:
	virtual ~cpppybridge() {};
};

#if 1
extern PyTypeObject pycppbridge_Type;

extern cpppybridge *pycppbridge_getwrapper(PyObject *o);
extern void pycppbridge_setwrapper(PyObject *o, cpppybridge *w);

inline bool pycppbridge_Check(PyObject *x)
{
	return PyObject_TypeCheck(x, &pycppbridge_Type);
}

#else
inline bool pycppbridge_Check(PyObject *x) { return false; };
inline cpppybridge *pycppbridge_getwrapper(PyObject *o) { return NULL; };
inline void pycppbridge_setwrapper(PyObject *o, cpppybridge *w) {};
#endif

class ostream : public cpppybridge, public ambulant::lib::ostream {
public:
	ostream(PyObject *itself);
	virtual ~ostream();

	bool is_open() const;
	void close();
	int write(const unsigned char * buffer, int nbytes);
	int write(const char* cstr);
	void flush();
  private:
	PyObject *py_ostream;

	friend PyObject *ostreamObj_New(ambulant::lib::ostream *itself);
};
#define BGEN_BACK_SUPPORT_ostream
inline ostream *Py_WrapAs_ostream(PyObject *o)
{
	ostream *rv = dynamic_cast<ostream*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new ostream(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class node_context : public cpppybridge, public ambulant::lib::node_context {
public:
	node_context(PyObject *itself);
	virtual ~node_context();

	void set_prefix_mapping(const std::string& prefix, const std::string& uri);
	const ambulant::lib::xml_string& get_namespace_prefix(const ambulant::lib::xml_string& uri) const;
	bool is_supported_prefix(const ambulant::lib::xml_string& prefix) const;
	ambulant::net::url resolve_url(const ambulant::net::url& rurl) const;
	const ambulant::lib::node* get_root() const;
	const ambulant::lib::node* get_node(const std::string& idd) const;
	ambulant::common::state_component* get_state() const;
	const ambulant::lib::xml_string& apply_avt(const ambulant::lib::node* n, const ambulant::lib::xml_string& attrname, const ambulant::lib::xml_string& attrvalue) const;
	const custom_test_map* get_custom_tests() const { return NULL; }
  private:
	PyObject *py_node_context;
	ambulant::lib::xml_string get_namespace_prefix_rvkeepref;
	ambulant::lib::xml_string apply_avt_rvkeepref;

	friend PyObject *node_contextObj_New(ambulant::lib::node_context *itself);
};
#define BGEN_BACK_SUPPORT_node_context
inline node_context *Py_WrapAs_node_context(PyObject *o)
{
	node_context *rv = dynamic_cast<node_context*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new node_context(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class node : public cpppybridge, public ambulant::lib::node {
public:
	node(PyObject *itself);
	virtual ~node();

	const ambulant::lib::node* down() const;
	const ambulant::lib::node* up() const;
	const ambulant::lib::node* next() const;
	ambulant::lib::node* down();
	ambulant::lib::node* up();
	ambulant::lib::node* next();
	void down(ambulant::lib::node* n);
	void up(ambulant::lib::node* n);
	void next(ambulant::lib::node* n);
	const ambulant::lib::node* previous() const;
	const ambulant::lib::node* get_last_child() const;
	ambulant::lib::node* locate_node(const char* path);
	ambulant::lib::node* get_first_child(const char* name);
	const ambulant::lib::node* get_first_child(const char* name) const;
	ambulant::lib::node* get_root();
	const char * get_container_attribute(const char* name) const;
	ambulant::lib::node* append_child(ambulant::lib::node* child);
	ambulant::lib::node* append_child(const char* name);
	ambulant::lib::node* detach();
	ambulant::lib::node* clone() const;
	void append_data(const char *data__in__, size_t data__len__);
	void append_data(const char* c_str);
	void append_data(const ambulant::lib::xml_string& str);
	void set_attribute(const char* name, const char* value);
	void set_attribute(const char* name, const ambulant::lib::xml_string& value);
	void set_prefix_mapping(const std::string& prefix, const std::string& uri);
	const ambulant::lib::xml_string& get_namespace() const;
	const ambulant::lib::xml_string& get_local_name() const;
	const ambulant::lib::q_name_pair& get_qname() const;
	int get_numid() const;
	const ambulant::lib::xml_string& get_data() const;
	bool is_data_node() const;
	ambulant::lib::xml_string get_trimmed_data() const;
	const char * get_attribute(const char* name) const;
	const char * get_attribute(const std::string& name) const;
	void del_attribute(const char* name);
	ambulant::net::url get_url(const char* attrname) const;
	unsigned int size() const;
	std::string get_xpath() const;
	std::string get_sig() const;
	ambulant::lib::xml_string xmlrepr() const;
	const ambulant::lib::node_context* get_context() const;
	void set_context(ambulant::lib::node_context* c);
	void get_children(const_node_list& l) const {ambulant::lib::node_navigator<const ambulant::lib::node>::get_children(this, l);}
	void set_attributes(const char **attrs) { abort(); }
  private:
	PyObject *py_node;
	ambulant::lib::xml_string get_namespace_rvkeepref;
	ambulant::lib::xml_string get_local_name_rvkeepref;
	ambulant::lib::q_name_pair get_qname_rvkeepref;
	ambulant::lib::xml_string get_data_rvkeepref;

	friend PyObject *nodeObj_New(ambulant::lib::node *itself);
};
#define BGEN_BACK_SUPPORT_node
inline node *Py_WrapAs_node(PyObject *o)
{
	node *rv = dynamic_cast<node*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new node(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class node_factory : public cpppybridge, public ambulant::lib::node_factory {
public:
	node_factory(PyObject *itself);
	virtual ~node_factory();

	ambulant::lib::node* new_node(const ambulant::lib::q_name_pair& qn, const ambulant::lib::q_attributes_list& qattrs, const ambulant::lib::node_context* ctx);
	ambulant::lib::node* new_node(const ambulant::lib::node* other);
	ambulant::lib::node* new_data_node(const char *data__in__, size_t data__len__, const ambulant::lib::node_context* ctx);
	ambulant::lib::node *new_node(const char *local_name, const char **attrs = 0, const ambulant::lib::node_context *ctx = 0) { abort(); return NULL; };
	ambulant::lib::node *new_node(const ambulant::lib::xml_string& local_name, const char **attrs = 0, const ambulant::lib::node_context *ctx = 0) { abort(); return NULL; };
  private:
	PyObject *py_node_factory;

	friend PyObject *node_factoryObj_New(ambulant::lib::node_factory *itself);
};
#define BGEN_BACK_SUPPORT_node_factory
inline node_factory *Py_WrapAs_node_factory(PyObject *o)
{
	node_factory *rv = dynamic_cast<node_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new node_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class event : public cpppybridge, public ambulant::lib::event {
public:
	event(PyObject *itself);
	virtual ~event();

	void fire();
  private:
	PyObject *py_event;

	friend PyObject *eventObj_New(ambulant::lib::event *itself);
};
#define BGEN_BACK_SUPPORT_event
inline event *Py_WrapAs_event(PyObject *o)
{
	event *rv = dynamic_cast<event*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new event(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class event_processor : public cpppybridge, public ambulant::lib::event_processor {
public:
	event_processor(PyObject *itself);
	virtual ~event_processor();

	void add_event(ambulant::lib::event* pe, ambulant::lib::timer::time_type t, ambulant::lib::event_priority priority);
	void cancel_all_events();
	bool cancel_event(ambulant::lib::event* pe, ambulant::lib::event_priority priority);
	ambulant::lib::timer* get_timer() const;
	void set_observer(ambulant::lib::event_processor_observer*) { abort(); }
  private:
	PyObject *py_event_processor;

	friend PyObject *event_processorObj_New(ambulant::lib::event_processor *itself);
};
#define BGEN_BACK_SUPPORT_event_processor
inline event_processor *Py_WrapAs_event_processor(PyObject *o)
{
	event_processor *rv = dynamic_cast<event_processor*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new event_processor(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class parser_factory : public cpppybridge, public ambulant::lib::parser_factory {
public:
	parser_factory(PyObject *itself);
	virtual ~parser_factory();

	std::string get_parser_name();
	ambulant::lib::xml_parser* new_parser(ambulant::lib::sax_content_handler*, ambulant::lib::sax_error_handler*) { abort(); return NULL; }
  private:
	PyObject *py_parser_factory;

	friend PyObject *parser_factoryObj_New(ambulant::lib::parser_factory *itself);
};
#define BGEN_BACK_SUPPORT_parser_factory
inline parser_factory *Py_WrapAs_parser_factory(PyObject *o)
{
	parser_factory *rv = dynamic_cast<parser_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new parser_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class xml_parser : public cpppybridge, public ambulant::lib::xml_parser {
public:
	xml_parser(PyObject *itself);
	virtual ~xml_parser();

	bool parse(const char *buf__in__, size_t buf__len__, bool final);
	void set_content_handler(ambulant::lib::sax_content_handler*) { abort(); }
	void set_error_handler(ambulant::lib::sax_error_handler*) { abort(); }
  private:
	PyObject *py_xml_parser;

	friend PyObject *xml_parserObj_New(ambulant::lib::xml_parser *itself);
};
#define BGEN_BACK_SUPPORT_xml_parser
inline xml_parser *Py_WrapAs_xml_parser(PyObject *o)
{
	xml_parser *rv = dynamic_cast<xml_parser*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new xml_parser(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class system_embedder : public cpppybridge, public ambulant::lib::system_embedder {
public:
	system_embedder(PyObject *itself);
	virtual ~system_embedder();

	void show_file(const ambulant::net::url& href);
  private:
	PyObject *py_system_embedder;

	friend PyObject *system_embedderObj_New(ambulant::lib::system_embedder *itself);
};
#define BGEN_BACK_SUPPORT_system_embedder
inline system_embedder *Py_WrapAs_system_embedder(PyObject *o)
{
	system_embedder *rv = dynamic_cast<system_embedder*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new system_embedder(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class timer : public cpppybridge, public ambulant::lib::timer {
public:
	timer(PyObject *itself);
	virtual ~timer();

	ambulant::lib::timer::time_type elapsed() const;
	double get_realtime_speed() const;
	ambulant::lib::timer::signed_time_type set_drift(ambulant::lib::timer::signed_time_type drift);
	ambulant::lib::timer::signed_time_type get_drift() const;
	void skew(ambulant::lib::timer::signed_time_type skew);
	bool running() const;
	bool is_slaved() const;
  private:
	PyObject *py_timer;

	friend PyObject *timerObj_New(ambulant::lib::timer *itself);
};
#define BGEN_BACK_SUPPORT_timer
inline timer *Py_WrapAs_timer(PyObject *o)
{
	timer *rv = dynamic_cast<timer*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new timer(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class timer_control : public timer, public ambulant::lib::timer_control {
public:
	timer_control(PyObject *itself);
	virtual ~timer_control();

	ambulant::lib::timer::time_type elapsed() const;
	ambulant::lib::timer::time_type elapsed(ambulant::lib::timer::time_type pt) const;
	void start(ambulant::lib::timer::time_type t);
	void stop();
	void pause();
	void resume();
	void set_speed(double speed);
	void set_time(ambulant::lib::timer::time_type t);
	double get_speed() const;
	bool running() const;
	double get_realtime_speed() const;
	ambulant::lib::timer::signed_time_type set_drift(ambulant::lib::timer::signed_time_type drift);
	ambulant::lib::timer::signed_time_type get_drift() const;
	void skew(ambulant::lib::timer::signed_time_type skew);
	void set_observer(ambulant::lib::timer_observer* obs);
	void set_slaved(bool slaved);
	bool is_slaved() const;
  private:
	PyObject *py_timer_control;

	friend PyObject *timer_controlObj_New(ambulant::lib::timer_control *itself);
};
#define BGEN_BACK_SUPPORT_timer_control
inline timer_control *Py_WrapAs_timer_control(PyObject *o)
{
	timer_control *rv = dynamic_cast<timer_control*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new timer_control(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class timer_observer : public cpppybridge, public ambulant::lib::timer_observer {
public:
	timer_observer(PyObject *itself);
	virtual ~timer_observer();

	void started();
	void stopped();
	void paused();
	void resumed();
  private:
	PyObject *py_timer_observer;

	friend PyObject *timer_observerObj_New(ambulant::lib::timer_observer *itself);
};
#define BGEN_BACK_SUPPORT_timer_observer
inline timer_observer *Py_WrapAs_timer_observer(PyObject *o)
{
	timer_observer *rv = dynamic_cast<timer_observer*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new timer_observer(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class timer_sync : public timer_observer, public ambulant::lib::timer_sync {
public:
	timer_sync(PyObject *itself);
	virtual ~timer_sync();

	void initialize(ambulant::lib::timer_control* timer);
	void started();
	void stopped();
	void paused();
	void resumed();
	void clicked(const ambulant::lib::node* n, ambulant::lib::timer::time_type t);
#ifdef WITH_REMOTE_SYNC
	bool uses_external_sync();
#endif
  private:
	PyObject *py_timer_sync;

	friend PyObject *timer_syncObj_New(ambulant::lib::timer_sync *itself);
};
#define BGEN_BACK_SUPPORT_timer_sync
inline timer_sync *Py_WrapAs_timer_sync(PyObject *o)
{
	timer_sync *rv = dynamic_cast<timer_sync*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new timer_sync(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class timer_sync_factory : public cpppybridge, public ambulant::lib::timer_sync_factory {
public:
	timer_sync_factory(PyObject *itself);
	virtual ~timer_sync_factory();

	ambulant::lib::timer_sync* new_timer_sync(ambulant::lib::document* doc);
  private:
	PyObject *py_timer_sync_factory;

	friend PyObject *timer_sync_factoryObj_New(ambulant::lib::timer_sync_factory *itself);
};
#define BGEN_BACK_SUPPORT_timer_sync_factory
inline timer_sync_factory *Py_WrapAs_timer_sync_factory(PyObject *o)
{
	timer_sync_factory *rv = dynamic_cast<timer_sync_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new timer_sync_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class embedder : public system_embedder, public ambulant::common::embedder {
public:
	embedder(PyObject *itself);
	virtual ~embedder();

	void close(ambulant::common::player* p);
	void open(ambulant::net::url newdoc, bool start, ambulant::common::player* old);
	void done(ambulant::common::player* p);
	void starting(ambulant::common::player* p);
	bool aux_open(const ambulant::net::url& href);
	void terminate();
	void show_file(const ambulant::net::url& url) { ::system_embedder::show_file(url); }
  private:
	PyObject *py_embedder;

	friend PyObject *embedderObj_New(ambulant::common::embedder *itself);
};
#define BGEN_BACK_SUPPORT_embedder
inline embedder *Py_WrapAs_embedder(PyObject *o)
{
	embedder *rv = dynamic_cast<embedder*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new embedder(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class factories : public cpppybridge, public ambulant::common::factories {
public:
	factories(PyObject *itself);
	virtual ~factories();

	void init_factories();
	void init_playable_factory();
	void init_window_factory();
	void init_datasource_factory();
	void init_parser_factory();
	void init_node_factory();
	void init_state_component_factory();
#ifdef WITH_REMOTE_SYNC
	void init_timer_sync_factory();
#endif
	void init_recorder_factory();
	ambulant::common::global_playable_factory* get_playable_factory() const;
	ambulant::common::window_factory* get_window_factory() const;
	ambulant::net::datasource_factory* get_datasource_factory() const;
	ambulant::lib::global_parser_factory* get_parser_factory() const;
	ambulant::lib::node_factory* get_node_factory() const;
	ambulant::common::global_state_component_factory* get_state_component_factory() const;
	ambulant::common::recorder_factory* get_recorder_factory() const;
	void set_playable_factory(ambulant::common::global_playable_factory* pf);
	void set_window_factory(ambulant::common::window_factory* wf);
	void set_datasource_factory(ambulant::net::datasource_factory* df);
	void set_parser_factory(ambulant::lib::global_parser_factory* pf);
	void set_node_factory(ambulant::lib::node_factory* nf);
	void set_state_component_factory(ambulant::common::global_state_component_factory* sf);
#ifdef WITH_REMOTE_SYNC
	ambulant::lib::timer_sync_factory* get_timer_sync_factory() const;
#endif
#ifdef WITH_REMOTE_SYNC
	void set_timer_sync_factory(ambulant::lib::timer_sync_factory* tsf);
#endif
	void set_recorder_factory(ambulant::common::recorder_factory* rf);
  private:
	PyObject *py_factories;

	friend PyObject *factoriesObj_New(ambulant::common::factories *itself);
};
#define BGEN_BACK_SUPPORT_factories
inline factories *Py_WrapAs_factories(PyObject *o)
{
	factories *rv = dynamic_cast<factories*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new factories(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class gui_screen : public cpppybridge, public ambulant::common::gui_screen {
public:
	gui_screen(PyObject *itself);
	virtual ~gui_screen();

	void get_size(int* width, int* height);
#ifndef CPP_TO_PYTHON_BRIDGE
	bool get_screenshot(const char* type, char* *out_data__out__, size_t* out_data__len__);
#endif
#ifdef CPP_TO_PYTHON_BRIDGE
	    bool get_screenshot(const char*, char**, size_t*) { return false; }
	    #endif
  private:
	PyObject *py_gui_screen;

	friend PyObject *gui_screenObj_New(ambulant::common::gui_screen *itself);
};
#define BGEN_BACK_SUPPORT_gui_screen
inline gui_screen *Py_WrapAs_gui_screen(PyObject *o)
{
	gui_screen *rv = dynamic_cast<gui_screen*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new gui_screen(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class gui_player : public factories, public ambulant::common::gui_player {
public:
	gui_player(PyObject *itself);
	virtual ~gui_player();

	void init_playable_factory();
	void init_window_factory();
	void init_datasource_factory();
	void init_parser_factory();
	void init_plugins();
	void play();
	void stop();
	void pause();
	void restart(bool reparse);
	void goto_node(const ambulant::lib::node* n);
	bool is_play_enabled() const;
	bool is_stop_enabled() const;
	bool is_pause_enabled() const;
	bool is_play_active() const;
	bool is_stop_active() const;
	bool is_pause_active() const;
	void before_mousemove(int cursor);
	int after_mousemove();
	void on_char(int c);
	void on_focus_advance();
	void on_focus_activate();
	ambulant::lib::document* get_document() const;
	void set_document(ambulant::lib::document* doc);
	ambulant::common::embedder* get_embedder() const;
	void set_embedder(ambulant::common::embedder* em);
	ambulant::common::player* get_player() const;
	void set_player(ambulant::common::player* pl);
	ambulant::net::url get_url() const;
	ambulant::common::gui_screen* get_gui_screen();
#ifdef WITH_REMOTE_SYNC
	void clicked_external(ambulant::lib::node* n, ambulant::lib::timer::time_type t);
#endif
  private:
	PyObject *py_gui_player;

	friend PyObject *gui_playerObj_New(ambulant::common::gui_player *itself);
};
#define BGEN_BACK_SUPPORT_gui_player
inline gui_player *Py_WrapAs_gui_player(PyObject *o)
{
	gui_player *rv = dynamic_cast<gui_player*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new gui_player(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class alignment : public cpppybridge, public ambulant::common::alignment {
public:
	alignment(PyObject *itself);
	virtual ~alignment();

	ambulant::lib::point get_image_fixpoint(ambulant::lib::size image_size) const;
	ambulant::lib::point get_surface_fixpoint(ambulant::lib::size surface_size) const;
  private:
	PyObject *py_alignment;

	friend PyObject *alignmentObj_New(ambulant::common::alignment *itself);
};
#define BGEN_BACK_SUPPORT_alignment
inline alignment *Py_WrapAs_alignment(PyObject *o)
{
	alignment *rv = dynamic_cast<alignment*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new alignment(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class animation_notification : public cpppybridge, public ambulant::common::animation_notification {
public:
	animation_notification(PyObject *itself);
	virtual ~animation_notification();

	void animated();
  private:
	PyObject *py_animation_notification;

	friend PyObject *animation_notificationObj_New(ambulant::common::animation_notification *itself);
};
#define BGEN_BACK_SUPPORT_animation_notification
inline animation_notification *Py_WrapAs_animation_notification(PyObject *o)
{
	animation_notification *rv = dynamic_cast<animation_notification*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new animation_notification(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class gui_window : public cpppybridge, public ambulant::common::gui_window {
public:
	gui_window(PyObject *itself);
	virtual ~gui_window();

	void need_redraw(const ambulant::lib::rect& r);
	void redraw_now();
	void need_events(bool want);
  private:
	PyObject *py_gui_window;

	friend PyObject *gui_windowObj_New(ambulant::common::gui_window *itself);
};
#define BGEN_BACK_SUPPORT_gui_window
inline gui_window *Py_WrapAs_gui_window(PyObject *o)
{
	gui_window *rv = dynamic_cast<gui_window*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new gui_window(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class gui_events : public cpppybridge, public ambulant::common::gui_events {
public:
	gui_events(PyObject *itself);
	virtual ~gui_events();

	void redraw(const ambulant::lib::rect& dirty, ambulant::common::gui_window* window);
	bool user_event(const ambulant::lib::point& where, int what);
	void transition_freeze_end(ambulant::lib::rect area);
  private:
	PyObject *py_gui_events;

	friend PyObject *gui_eventsObj_New(ambulant::common::gui_events *itself);
};
#define BGEN_BACK_SUPPORT_gui_events
inline gui_events *Py_WrapAs_gui_events(PyObject *o)
{
	gui_events *rv = dynamic_cast<gui_events*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new gui_events(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class renderer : public gui_events, public ambulant::common::renderer {
public:
	renderer(PyObject *itself);
	virtual ~renderer();

	void set_surface(ambulant::common::surface* destination);
	void set_alignment(const ambulant::common::alignment* align);
	void set_intransition(const ambulant::lib::transition_info* info);
	void start_outtransition(const ambulant::lib::transition_info* info);
	ambulant::common::surface* get_surface();
	void redraw(const ambulant::lib::rect&, ambulant::common::gui_window*) { abort(); }
	bool user_event(const ambulant::lib::point&, int) { abort(); return false; }
	void transition_freeze_end(ambulant::lib::rect) { abort(); }
  private:
	PyObject *py_renderer;

	friend PyObject *rendererObj_New(ambulant::common::renderer *itself);
};
#define BGEN_BACK_SUPPORT_renderer
inline renderer *Py_WrapAs_renderer(PyObject *o)
{
	renderer *rv = dynamic_cast<renderer*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new renderer(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class bgrenderer : public gui_events, public ambulant::common::bgrenderer {
public:
	bgrenderer(PyObject *itself);
	virtual ~bgrenderer();

	void set_surface(ambulant::common::surface* destination);
	void keep_as_background();
	void highlight(ambulant::common::gui_window* window);
	void redraw(const ambulant::lib::rect&, ambulant::common::gui_window*) { abort(); }
	bool user_event(const ambulant::lib::point&, int) { abort(); return false; }
	void transition_freeze_end(ambulant::lib::rect) { abort(); }
  private:
	PyObject *py_bgrenderer;

	friend PyObject *bgrendererObj_New(ambulant::common::bgrenderer *itself);
};
#define BGEN_BACK_SUPPORT_bgrenderer
inline bgrenderer *Py_WrapAs_bgrenderer(PyObject *o)
{
	bgrenderer *rv = dynamic_cast<bgrenderer*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new bgrenderer(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class surface : public cpppybridge, public ambulant::common::surface {
public:
	surface(PyObject *itself);
	virtual ~surface();

	void show(ambulant::common::gui_events* renderer);
	void renderer_done(ambulant::common::gui_events* renderer);
	void need_redraw(const ambulant::lib::rect& r);
	void need_redraw();
	void need_events(bool want);
	void transition_done();
	void keep_as_background();
	const ambulant::lib::rect& get_rect() const;
	const ambulant::lib::rect& get_clipped_screen_rect() const;
	const ambulant::lib::point& get_global_topleft() const;
	ambulant::lib::rect get_fit_rect(const ambulant::lib::size& src_size, ambulant::lib::rect* out_src_rect, const ambulant::common::alignment* align) const;
	ambulant::lib::rect get_fit_rect(const ambulant::lib::rect& src_crop_rect, const ambulant::lib::size& src_size, ambulant::lib::rect* out_src_rect, const ambulant::common::alignment* align) const;
	ambulant::lib::rect get_crop_rect(const ambulant::lib::size& src_size) const;
	const ambulant::common::region_info* get_info() const;
	ambulant::common::surface* get_top_surface();
	bool is_tiled() const;
	ambulant::common::gui_window* get_gui_window();
	void set_renderer_private_data(ambulant::common::renderer_private_id idd, ambulant::common::renderer_private_data * data);
	ambulant::common::renderer_private_data * get_renderer_private_data(ambulant::common::renderer_private_id idd);
	void highlight(bool on);
	ambulant::common::tile_positions get_tiles(ambulant::lib::size s, ambulant::lib::rect r) const { return surface::get_tiles(s, r); }
  private:
	PyObject *py_surface;
	ambulant::lib::rect get_rect_rvkeepref;
	ambulant::lib::rect get_clipped_screen_rect_rvkeepref;
	ambulant::lib::point get_global_topleft_rvkeepref;

	friend PyObject *surfaceObj_New(ambulant::common::surface *itself);
};
#define BGEN_BACK_SUPPORT_surface
inline surface *Py_WrapAs_surface(PyObject *o)
{
	surface *rv = dynamic_cast<surface*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new surface(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class window_factory : public cpppybridge, public ambulant::common::window_factory {
public:
	window_factory(PyObject *itself);
	virtual ~window_factory();

	ambulant::lib::size get_default_size();
	ambulant::common::gui_window* new_window(const std::string& name, ambulant::lib::size bounds, ambulant::common::gui_events* handler);
	ambulant::common::bgrenderer* new_background_renderer(const ambulant::common::region_info* src);
	void window_done(const std::string& name);
  private:
	PyObject *py_window_factory;

	friend PyObject *window_factoryObj_New(ambulant::common::window_factory *itself);
};
#define BGEN_BACK_SUPPORT_window_factory
inline window_factory *Py_WrapAs_window_factory(PyObject *o)
{
	window_factory *rv = dynamic_cast<window_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new window_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class surface_template : public animation_notification, public ambulant::common::surface_template {
public:
	surface_template(PyObject *itself);
	virtual ~surface_template();

	ambulant::common::surface_template* new_subsurface(const ambulant::common::region_info* info, ambulant::common::bgrenderer* bgrend);
	ambulant::common::surface* activate();
	void animated() { abort(); }
  private:
	PyObject *py_surface_template;

	friend PyObject *surface_templateObj_New(ambulant::common::surface_template *itself);
};
#define BGEN_BACK_SUPPORT_surface_template
inline surface_template *Py_WrapAs_surface_template(PyObject *o)
{
	surface_template *rv = dynamic_cast<surface_template*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new surface_template(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class surface_factory : public cpppybridge, public ambulant::common::surface_factory {
public:
	surface_factory(PyObject *itself);
	virtual ~surface_factory();

	ambulant::common::surface_template* new_topsurface(const ambulant::common::region_info* info, ambulant::common::bgrenderer* bgrend, ambulant::common::window_factory* wf);
  private:
	PyObject *py_surface_factory;

	friend PyObject *surface_factoryObj_New(ambulant::common::surface_factory *itself);
};
#define BGEN_BACK_SUPPORT_surface_factory
inline surface_factory *Py_WrapAs_surface_factory(PyObject *o)
{
	surface_factory *rv = dynamic_cast<surface_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new surface_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class layout_manager : public cpppybridge, public ambulant::common::layout_manager {
public:
	layout_manager(PyObject *itself);
	virtual ~layout_manager();

	ambulant::common::surface* get_surface(const ambulant::lib::node* node);
	ambulant::common::alignment* get_alignment(const ambulant::lib::node* node);
	ambulant::common::animation_notification* get_animation_notification(const ambulant::lib::node* node);
	ambulant::common::animation_destination* get_animation_destination(const ambulant::lib::node* node);
  private:
	PyObject *py_layout_manager;

	friend PyObject *layout_managerObj_New(ambulant::common::layout_manager *itself);
};
#define BGEN_BACK_SUPPORT_layout_manager
inline layout_manager *Py_WrapAs_layout_manager(PyObject *o)
{
	layout_manager *rv = dynamic_cast<layout_manager*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new layout_manager(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class playable : public cpppybridge, public ambulant::common::playable {
public:
	playable(PyObject *itself);
	virtual ~playable();

	void init_with_node(const ambulant::lib::node* n);
	void start(double t);
	bool stop();
	void post_stop();
	void pause(ambulant::common::pause_display d);
	void resume();
	void seek(double t);
	void wantclicks(bool want);
	void preroll(double when, double where, double how_much);
	ambulant::common::duration get_dur();
	ambulant::common::playable::cookie_type get_cookie() const;
	ambulant::common::renderer* get_renderer();
	std::string get_sig() const;
  private:
	PyObject *py_playable;

	friend PyObject *playableObj_New(ambulant::common::playable *itself);
};
#define BGEN_BACK_SUPPORT_playable
inline playable *Py_WrapAs_playable(PyObject *o)
{
	playable *rv = dynamic_cast<playable*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new playable(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class playable_notification : public cpppybridge, public ambulant::common::playable_notification {
public:
	playable_notification(PyObject *itself);
	virtual ~playable_notification();

	void started(ambulant::common::playable::cookie_type n, double t);
	void stopped(ambulant::common::playable::cookie_type n, double t);
	void clicked(ambulant::common::playable::cookie_type n, double t);
	void pointed(ambulant::common::playable::cookie_type n, double t);
	void transitioned(ambulant::common::playable::cookie_type n, double t);
	void marker_seen(ambulant::common::playable::cookie_type n, const char* name, double t);
	void playable_stalled(const ambulant::common::playable* p, const char* reason);
	void playable_unstalled(const ambulant::common::playable* p);
	void playable_started(const ambulant::common::playable* p, const ambulant::lib::node* n, const char* comment);
	void playable_resource(const ambulant::common::playable* p, const char* resource, long amount);
  private:
	PyObject *py_playable_notification;

	friend PyObject *playable_notificationObj_New(ambulant::common::playable_notification *itself);
};
#define BGEN_BACK_SUPPORT_playable_notification
inline playable_notification *Py_WrapAs_playable_notification(PyObject *o)
{
	playable_notification *rv = dynamic_cast<playable_notification*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new playable_notification(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class playable_factory : public cpppybridge, public ambulant::common::playable_factory {
public:
	playable_factory(PyObject *itself);
	virtual ~playable_factory();

	bool supports(ambulant::common::renderer_select* rs);
	ambulant::common::playable* new_playable(ambulant::common::playable_notification* context, ambulant::common::playable::cookie_type cookie, const ambulant::lib::node* node, ambulant::lib::event_processor* evp);
	ambulant::common::playable* new_aux_audio_playable(ambulant::common::playable_notification* context, ambulant::common::playable::cookie_type cookie, const ambulant::lib::node* node, ambulant::lib::event_processor* evp, ambulant::net::audio_datasource* src);
  private:
	PyObject *py_playable_factory;

	friend PyObject *playable_factoryObj_New(ambulant::common::playable_factory *itself);
};
#define BGEN_BACK_SUPPORT_playable_factory
inline playable_factory *Py_WrapAs_playable_factory(PyObject *o)
{
	playable_factory *rv = dynamic_cast<playable_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new playable_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class global_playable_factory : public playable_factory, public ambulant::common::global_playable_factory {
public:
	global_playable_factory(PyObject *itself);
	virtual ~global_playable_factory();

	void add_factory(ambulant::common::playable_factory* rf);
	void preferred_renderer(const char* name);
	bool supports(ambulant::common::renderer_select*) { abort(); return false; }
	ambulant::common::playable* new_playable(ambulant::common::playable_notification*, int, const ambulant::lib::node*, ambulant::lib::event_processor*) { abort(); return NULL; }
	ambulant::common::playable* new_aux_audio_playable(ambulant::common::playable_notification *context, int, const ambulant::lib::node *node, ambulant::lib::event_processor *evp, ambulant::net::audio_datasource *src) { abort(); return NULL; }
  private:
	PyObject *py_global_playable_factory;

	friend PyObject *global_playable_factoryObj_New(ambulant::common::global_playable_factory *itself);
};
#define BGEN_BACK_SUPPORT_global_playable_factory
inline global_playable_factory *Py_WrapAs_global_playable_factory(PyObject *o)
{
	global_playable_factory *rv = dynamic_cast<global_playable_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new global_playable_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class recorder : public cpppybridge, public ambulant::common::recorder {
public:
	recorder(PyObject *itself);
	virtual ~recorder();

	void new_video_data(const char *data__in__, size_t data__len__, ambulant::lib::timer::time_type documenttimestamp);
	void new_audio_data(const char *data__in__, size_t data__len__, ambulant::lib::timer::time_type documenttimestamp);
  private:
	PyObject *py_recorder;

	friend PyObject *recorderObj_New(ambulant::common::recorder *itself);
};
#define BGEN_BACK_SUPPORT_recorder
inline recorder *Py_WrapAs_recorder(PyObject *o)
{
	recorder *rv = dynamic_cast<recorder*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new recorder(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class recorder_factory : public cpppybridge, public ambulant::common::recorder_factory {
public:
	recorder_factory(PyObject *itself);
	virtual ~recorder_factory();

	ambulant::common::recorder* new_recorder(ambulant::net::pixel_order pixel_order, ambulant::lib::size window_size);
  private:
	PyObject *py_recorder_factory;

	friend PyObject *recorder_factoryObj_New(ambulant::common::recorder_factory *itself);
};
#define BGEN_BACK_SUPPORT_recorder_factory
inline recorder_factory *Py_WrapAs_recorder_factory(PyObject *o)
{
	recorder_factory *rv = dynamic_cast<recorder_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new recorder_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class focus_feedback : public cpppybridge, public ambulant::common::focus_feedback {
public:
	focus_feedback(PyObject *itself);
	virtual ~focus_feedback();

	void node_focussed(const ambulant::lib::node* n);
  private:
	PyObject *py_focus_feedback;

	friend PyObject *focus_feedbackObj_New(ambulant::common::focus_feedback *itself);
};
#define BGEN_BACK_SUPPORT_focus_feedback
inline focus_feedback *Py_WrapAs_focus_feedback(PyObject *o)
{
	focus_feedback *rv = dynamic_cast<focus_feedback*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new focus_feedback(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class player_feedback : public cpppybridge, public ambulant::common::player_feedback {
public:
	player_feedback(PyObject *itself);
	virtual ~player_feedback();

	void document_loaded(ambulant::lib::document* doc);
	void document_started();
	void document_stopped();
	void node_started(const ambulant::lib::node* n);
	void node_filled(const ambulant::lib::node* n);
	void node_stopped(const ambulant::lib::node* n);
	void playable_started(const ambulant::common::playable* p, const ambulant::lib::node* n, const char* comment);
	void playable_stalled(const ambulant::common::playable* p, const char* reason);
	void playable_unstalled(const ambulant::common::playable* p);
	void playable_cached(const ambulant::common::playable* p);
	void playable_deleted(const ambulant::common::playable* p);
	void playable_resource(const ambulant::common::playable* p, const char* resource, long amount);
  private:
	PyObject *py_player_feedback;

	friend PyObject *player_feedbackObj_New(ambulant::common::player_feedback *itself);
};
#define BGEN_BACK_SUPPORT_player_feedback
inline player_feedback *Py_WrapAs_player_feedback(PyObject *o)
{
	player_feedback *rv = dynamic_cast<player_feedback*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new player_feedback(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class player : public cpppybridge, public ambulant::common::player {
public:
	player(PyObject *itself);
	virtual ~player();

	void initialize();
	void terminate();
	ambulant::lib::timer* get_timer();
	ambulant::lib::event_processor* get_evp();
	void start();
	void stop();
	void pause();
	void resume();
	bool is_playing() const;
	bool is_pausing() const;
	bool is_done() const;
	int after_mousemove();
	void before_mousemove(int cursor);
	void on_char(int ch);
	void on_state_change(const char* ref);
	ambulant::common::state_component* get_state_engine();
	void on_focus_advance();
	void on_focus_activate();
	void set_focus_feedback(ambulant::common::focus_feedback* fb);
	void set_feedback(ambulant::common::player_feedback* fb);
	ambulant::common::player_feedback* get_feedback();
	bool goto_node(const ambulant::lib::node* n);
	bool highlight(const ambulant::lib::node* n, bool on);
#ifdef WITH_REMOTE_SYNC
	void clicked_external(ambulant::lib::node* n, ambulant::lib::timer::time_type t);
#endif
#ifdef WITH_REMOTE_SYNC
	bool uses_external_sync() const;
#endif
	long add_ref() { return 1; }
	long release() { return 1;}
	long get_ref_count() const { return 1; }
	char *get_read_ptr() { abort(); return NULL; }
  private:
	PyObject *py_player;

	friend PyObject *playerObj_New(ambulant::common::player *itself);
};
#define BGEN_BACK_SUPPORT_player
inline player *Py_WrapAs_player(PyObject *o)
{
	player *rv = dynamic_cast<player*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new player(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class region_info : public cpppybridge, public ambulant::common::region_info {
public:
	region_info(PyObject *itself);
	virtual ~region_info();

	std::string get_name() const;
	ambulant::lib::rect get_rect(const ambulant::lib::rect * default_rect) const;
	ambulant::common::fit_t get_fit() const;
	ambulant::lib::color_t get_bgcolor() const;
	double get_bgopacity() const;
	bool get_transparent() const;
	ambulant::common::zindex_t get_zindex() const;
	bool get_showbackground() const;
	bool is_subregion() const;
	double get_soundlevel() const;
	ambulant::common::sound_alignment get_soundalign() const;
	ambulant::common::tiling get_tiling() const;
	const char * get_bgimage() const;
	ambulant::lib::rect get_crop_rect(const ambulant::lib::size& srcsize) const;
	double get_mediaopacity() const;
	double get_mediabgopacity() const;
	bool is_chromakey_specified() const;
	ambulant::lib::color_t get_chromakey() const;
	ambulant::lib::color_t get_chromakeytolerance() const;
	double get_chromakeyopacity() const;
  private:
	PyObject *py_region_info;

	friend PyObject *region_infoObj_New(ambulant::common::region_info *itself);
};
#define BGEN_BACK_SUPPORT_region_info
inline region_info *Py_WrapAs_region_info(PyObject *o)
{
	region_info *rv = dynamic_cast<region_info*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new region_info(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class animation_destination : public region_info, public ambulant::common::animation_destination {
public:
	animation_destination(PyObject *itself);
	virtual ~animation_destination();

	ambulant::common::region_dim get_region_dim(const std::string& which, bool fromdom) const;
	ambulant::lib::color_t get_region_color(const std::string& which, bool fromdom) const;
	ambulant::common::zindex_t get_region_zindex(bool fromdom) const;
	double get_region_soundlevel(bool fromdom) const;
	ambulant::common::sound_alignment get_region_soundalign(bool fromdom) const;
	double get_region_opacity(const std::string& which, bool fromdom) const;
	void set_region_dim(const std::string& which, const ambulant::common::region_dim& rd);
	void set_region_color(const std::string& which, ambulant::lib::color_t clr);
	void set_region_zindex(ambulant::common::zindex_t z);
	void set_region_soundlevel(double level);
	void set_region_soundalign(ambulant::common::sound_alignment sa);
	void set_region_opacity(const std::string& which, double level);
	std::string get_name() const { return ::region_info::get_name(); }
	ambulant::lib::rect get_rect(const ambulant::lib::rect* dft=NULL) const { return ::region_info::get_rect(dft); }
	ambulant::common::fit_t get_fit() const { return ::region_info::get_fit(); }
	ambulant::lib::color_t get_bgcolor() const { return ::region_info::get_bgcolor(); }
	ambulant::common::zindex_t get_zindex() const { return ::region_info::get_zindex(); }
	bool get_showbackground() const { return ::region_info::get_showbackground(); }
	bool is_subregion() const { return ::region_info::is_subregion(); }
	double get_soundlevel() const { return ::region_info::get_soundlevel(); }
	ambulant::common::sound_alignment get_soundalign() const { return ::region_info::get_soundalign(); }
	ambulant::common::tiling get_tiling() const { return ::region_info::get_tiling(); }
	const char* get_bgimage() const { return ::region_info::get_bgimage(); }
	double get_bgopacity() const { return ::region_info::get_bgopacity(); }
	bool get_transparent() const { return ::region_info::get_transparent(); }
	double get_mediaopacity() const { return ::region_info::get_mediaopacity(); }
	double get_mediabgopacity() const { return ::region_info::get_mediabgopacity(); }
	ambulant::lib::rect get_crop_rect(const ambulant::lib::size& srcsize) const { return ::region_info::get_crop_rect(srcsize); }
	const ambulant::common::region_dim_spec& get_region_panzoom(bool fromdom) const { abort(); static ambulant::common::region_dim_spec dummy; return dummy; }
	void set_region_panzoom(const ambulant::common::region_dim_spec& rds) { abort(); }
	bool is_chromakey_specified() const { return ::region_info::is_chromakey_specified(); }
	ambulant::lib::color_t get_chromakey() const { return ::region_info::get_chromakey(); }
	ambulant::lib::color_t get_chromakeytolerance() const { return ::region_info::get_chromakeytolerance(); }
	double get_chromakeyopacity() const { return ::region_info::get_chromakeyopacity(); }
  private:
	PyObject *py_animation_destination;

	friend PyObject *animation_destinationObj_New(ambulant::common::animation_destination *itself);
};
#define BGEN_BACK_SUPPORT_animation_destination
inline animation_destination *Py_WrapAs_animation_destination(PyObject *o)
{
	animation_destination *rv = dynamic_cast<animation_destination*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new animation_destination(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class state_test_methods : public cpppybridge, public ambulant::common::state_test_methods {
public:
	state_test_methods(PyObject *itself);
	virtual ~state_test_methods();

	bool smil_audio_desc() const;
	int smil_bitrate() const;
	bool smil_captions() const;
	bool smil_component(std::string uri) const;
	bool smil_custom_test(std::string name) const;
	std::string smil_cpu() const;
	float smil_language(std::string lang) const;
	std::string smil_operating_system() const;
	std::string smil_overdub_or_subtitle() const;
	bool smil_required(std::string uri) const;
	int smil_screen_depth() const;
	int smil_screen_height() const;
	int smil_screen_width() const;
  private:
	PyObject *py_state_test_methods;

	friend PyObject *state_test_methodsObj_New(ambulant::common::state_test_methods *itself);
};
#define BGEN_BACK_SUPPORT_state_test_methods
inline state_test_methods *Py_WrapAs_state_test_methods(PyObject *o)
{
	state_test_methods *rv = dynamic_cast<state_test_methods*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new state_test_methods(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class state_change_callback : public cpppybridge, public ambulant::common::state_change_callback {
public:
	state_change_callback(PyObject *itself);
	virtual ~state_change_callback();

	void on_state_change(const char* ref);
  private:
	PyObject *py_state_change_callback;

	friend PyObject *state_change_callbackObj_New(ambulant::common::state_change_callback *itself);
};
#define BGEN_BACK_SUPPORT_state_change_callback
inline state_change_callback *Py_WrapAs_state_change_callback(PyObject *o)
{
	state_change_callback *rv = dynamic_cast<state_change_callback*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new state_change_callback(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class state_component : public cpppybridge, public ambulant::common::state_component {
public:
	state_component(PyObject *itself);
	virtual ~state_component();

	void register_state_test_methods(ambulant::common::state_test_methods* stm);
	void declare_state(const ambulant::lib::node* state);
	bool bool_expression(const char* expr);
	void set_value(const char* var, const char* expr);
	void new_value(const char* ref, const char* where, const char* name, const char* expr);
	void del_value(const char* ref);
	void send(const ambulant::lib::node* submission);
	std::string string_expression(const char* expr);
	void want_state_change(const char* ref, ambulant::common::state_change_callback* cb);
	std::string getsubtree(const char* ref, bool as_query);
  private:
	PyObject *py_state_component;

	friend PyObject *state_componentObj_New(ambulant::common::state_component *itself);
};
#define BGEN_BACK_SUPPORT_state_component
inline state_component *Py_WrapAs_state_component(PyObject *o)
{
	state_component *rv = dynamic_cast<state_component*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new state_component(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class state_component_factory : public cpppybridge, public ambulant::common::state_component_factory {
public:
	state_component_factory(PyObject *itself);
	virtual ~state_component_factory();

	ambulant::common::state_component* new_state_component(const char* uri);
  private:
	PyObject *py_state_component_factory;

	friend PyObject *state_component_factoryObj_New(ambulant::common::state_component_factory *itself);
};
#define BGEN_BACK_SUPPORT_state_component_factory
inline state_component_factory *Py_WrapAs_state_component_factory(PyObject *o)
{
	state_component_factory *rv = dynamic_cast<state_component_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new state_component_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class global_state_component_factory : public state_component_factory, public ambulant::common::global_state_component_factory {
public:
	global_state_component_factory(PyObject *itself);
	virtual ~global_state_component_factory();

	void add_factory(ambulant::common::state_component_factory* sf);
	ambulant::common::state_component* new_state_component(const char*) { abort(); return NULL; }
  private:
	PyObject *py_global_state_component_factory;

	friend PyObject *global_state_component_factoryObj_New(ambulant::common::global_state_component_factory *itself);
};
#define BGEN_BACK_SUPPORT_global_state_component_factory
inline global_state_component_factory *Py_WrapAs_global_state_component_factory(PyObject *o)
{
	global_state_component_factory *rv = dynamic_cast<global_state_component_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new global_state_component_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class datasource : public cpppybridge, public ambulant::net::datasource {
public:
	datasource(PyObject *itself);
	virtual ~datasource();

	void start(ambulant::lib::event_processor* evp, ambulant::lib::event* callback);
	void start_prefetch(ambulant::lib::event_processor* evp);
	void stop();
	bool end_of_file();
	size_t size() const;
	void readdone(size_t len);
	long add_ref() { return 1; }
	long release() { return 1;}
	long get_ref_count() const { return 1; }
	long get_bandwidth_usage_data(const char **resource) { return -1; }
	char *get_read_ptr() { abort(); return NULL; }
  private:
	PyObject *py_datasource;

	friend PyObject *datasourceObj_New(ambulant::net::datasource *itself);
};
#define BGEN_BACK_SUPPORT_datasource
inline datasource *Py_WrapAs_datasource(PyObject *o)
{
	datasource *rv = dynamic_cast<datasource*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new datasource(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class raw_datasource_factory : public cpppybridge, public ambulant::net::raw_datasource_factory {
public:
	raw_datasource_factory(PyObject *itself);
	virtual ~raw_datasource_factory();

	ambulant::net::datasource* new_raw_datasource(const ambulant::net::url& url);
  private:
	PyObject *py_raw_datasource_factory;

	friend PyObject *raw_datasource_factoryObj_New(ambulant::net::raw_datasource_factory *itself);
};
#define BGEN_BACK_SUPPORT_raw_datasource_factory
inline raw_datasource_factory *Py_WrapAs_raw_datasource_factory(PyObject *o)
{
	raw_datasource_factory *rv = dynamic_cast<raw_datasource_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new raw_datasource_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class audio_datasource_factory : public cpppybridge, public ambulant::net::audio_datasource_factory {
public:
	audio_datasource_factory(PyObject *itself);
	virtual ~audio_datasource_factory();

	ambulant::net::audio_datasource* new_audio_datasource(const ambulant::net::url& url, const ambulant::net::audio_format_choices& fmt, ambulant::net::timestamp_t clip_begin, ambulant::net::timestamp_t clip_end);
  private:
	PyObject *py_audio_datasource_factory;

	friend PyObject *audio_datasource_factoryObj_New(ambulant::net::audio_datasource_factory *itself);
};
#define BGEN_BACK_SUPPORT_audio_datasource_factory
inline audio_datasource_factory *Py_WrapAs_audio_datasource_factory(PyObject *o)
{
	audio_datasource_factory *rv = dynamic_cast<audio_datasource_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new audio_datasource_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class video_datasource_factory : public cpppybridge, public ambulant::net::video_datasource_factory {
public:
	video_datasource_factory(PyObject *itself);
	virtual ~video_datasource_factory();

	ambulant::net::video_datasource* new_video_datasource(const ambulant::net::url& url, ambulant::net::timestamp_t clip_begin, ambulant::net::timestamp_t clip_end);
  private:
	PyObject *py_video_datasource_factory;

	friend PyObject *video_datasource_factoryObj_New(ambulant::net::video_datasource_factory *itself);
};
#define BGEN_BACK_SUPPORT_video_datasource_factory
inline video_datasource_factory *Py_WrapAs_video_datasource_factory(PyObject *o)
{
	video_datasource_factory *rv = dynamic_cast<video_datasource_factory*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new video_datasource_factory(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class audio_parser_finder : public cpppybridge, public ambulant::net::audio_parser_finder {
public:
	audio_parser_finder(PyObject *itself);
	virtual ~audio_parser_finder();

	ambulant::net::audio_datasource* new_audio_parser(const ambulant::net::url& url, const ambulant::net::audio_format_choices& hint, ambulant::net::audio_datasource* src);
  private:
	PyObject *py_audio_parser_finder;

	friend PyObject *audio_parser_finderObj_New(ambulant::net::audio_parser_finder *itself);
};
#define BGEN_BACK_SUPPORT_audio_parser_finder
inline audio_parser_finder *Py_WrapAs_audio_parser_finder(PyObject *o)
{
	audio_parser_finder *rv = dynamic_cast<audio_parser_finder*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new audio_parser_finder(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

class audio_filter_finder : public cpppybridge, public ambulant::net::audio_filter_finder {
public:
	audio_filter_finder(PyObject *itself);
	virtual ~audio_filter_finder();

	ambulant::net::audio_datasource* new_audio_filter(ambulant::net::audio_datasource* src, const ambulant::net::audio_format_choices& fmts);
  private:
	PyObject *py_audio_filter_finder;

	friend PyObject *audio_filter_finderObj_New(ambulant::net::audio_filter_finder *itself);
};
#define BGEN_BACK_SUPPORT_audio_filter_finder
inline audio_filter_finder *Py_WrapAs_audio_filter_finder(PyObject *o)
{
	audio_filter_finder *rv = dynamic_cast<audio_filter_finder*>(pycppbridge_getwrapper(o));
	if (rv) return rv;
	rv = new audio_filter_finder(o);
	pycppbridge_setwrapper(o, rv);
	return rv;
}

