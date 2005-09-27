
/* ==== Declaration of C++ to Python callback module pyambulant ===== */

#include "Python.h"


#define WITH_EXTERNAL_DOM 1
#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/parser_factory.h"
#include "ambulant/lib/sax_handler.h"
#include "ambulant/lib/system.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/transition_info.h"
#include "ambulant/common/embedder.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/player.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/common/region_info.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/stdio_datasource.h"
#include "ambulant/net/posix_datasource.h"

class node_context : public ambulant::lib::node_context {
public:
	node_context(PyObject *itself);
	virtual ~node_context();

	void set_prefix_mapping(const std::string& prefix, const std::string& uri);
	const char * get_namespace_prefix(const ambulant::lib::xml_string& uri) const;
	ambulant::net::url resolve_url(const ambulant::net::url& rurl) const;
	const ambulant::lib::node* get_root() const;
	const ambulant::lib::node* get_node(const std::string& idd) const;
	const custom_test_map* get_custom_tests() const { return NULL; }
  private:
	PyObject *py_node_context;

	friend PyObject *node_contextObj_New(ambulant::lib::node_context *itself);
};
#define BGEN_BACK_SUPPORT_node_context
inline node_context *Py_WrapAs_node_context(PyObject *o) { return new node_context(o); }

class node : public ambulant::lib::node {
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
	void append_data(char *data__in__, long data__len__);
	void append_data(const char* c_str);
	void append_data(const ambulant::lib::xml_string& str);
	void set_attribute(const char* name, const char* value);
	void set_attribute(const char* name, const ambulant::lib::xml_string& value);
	void set_namespace(const ambulant::lib::xml_string& ns);
	const ambulant::lib::xml_string& get_namespace() const;
	const ambulant::lib::xml_string& get_local_name() const;
	const ambulant::lib::q_name_pair& get_qname() const;
	int get_numid() const;
	const ambulant::lib::xml_string& get_data() const;
	ambulant::lib::xml_string get_trimmed_data() const;
	const char * get_attribute(const char* name) const;
	const char * get_attribute(const std::string& name) const;
	ambulant::net::url get_url(const char* attrname) const;
	unsigned int size() const;
	std::string get_path_display_desc() const;
	std::string get_sig() const;
	ambulant::lib::xml_string xmlrepr() const;
	const ambulant::lib::node_context* get_context() const;
	void set_context(ambulant::lib::node_context* c);
	void get_children(const_node_list& l) const {}
	void append_data(const char *data, size_t len) { abort(); }
	void set_attributes(const char **attrs) { abort(); }
  private:
	PyObject *py_node;
	ambulant::lib::xml_string get_namespace_rv;
	ambulant::lib::xml_string get_local_name_rv;
	ambulant::lib::q_name_pair get_qname_rv;
	ambulant::lib::xml_string get_data_rv;

	friend PyObject *nodeObj_New(ambulant::lib::node *itself);
};
#define BGEN_BACK_SUPPORT_node
inline node *Py_WrapAs_node(PyObject *o) { return new node(o); }

class event : public ambulant::lib::event {
public:
	event(PyObject *itself);
	virtual ~event();

	void fire();
  private:
	PyObject *py_event;

	friend PyObject *eventObj_New(ambulant::lib::event *itself);
};
#define BGEN_BACK_SUPPORT_event
inline event *Py_WrapAs_event(PyObject *o) { return new event(o); }

class event_processor : public ambulant::lib::event_processor {
public:
	event_processor(PyObject *itself);
	virtual ~event_processor();

	void add_event(ambulant::lib::event* pe, ambulant::lib::timer::time_type t, ambulant::lib::event_processor::event_priority priority);
	void cancel_all_events();
	bool cancel_event(ambulant::lib::event* pe, ambulant::lib::event_processor::event_priority priority);
	void serve_events();
	ambulant::lib::timer* get_timer() const;
	void stop_processor_thread();
  private:
	PyObject *py_event_processor;

	friend PyObject *event_processorObj_New(ambulant::lib::event_processor *itself);
};
#define BGEN_BACK_SUPPORT_event_processor
inline event_processor *Py_WrapAs_event_processor(PyObject *o) { return new event_processor(o); }

class parser_factory : public ambulant::lib::parser_factory {
public:
	parser_factory(PyObject *itself);
	virtual ~parser_factory();

	std::string get_parser_name();
	ambulant::lib::xml_parser* new_parser(ambulant::lib::sax_content_handler*, ambulant::lib::sax_error_handler*) { abort(); }
  private:
	PyObject *py_parser_factory;

	friend PyObject *parser_factoryObj_New(ambulant::lib::parser_factory *itself);
};
#define BGEN_BACK_SUPPORT_parser_factory
inline parser_factory *Py_WrapAs_parser_factory(PyObject *o) { return new parser_factory(o); }

class xml_parser : public ambulant::lib::xml_parser {
public:
	xml_parser(PyObject *itself);
	virtual ~xml_parser();

	bool parse(char *buf__in__, long buf__len__, bool final);
	bool parse(const char*, long unsigned int, bool) { abort(); }
	void set_content_handler(ambulant::lib::sax_content_handler*) { abort(); }
	void set_error_handler(ambulant::lib::sax_error_handler*) { abort(); }
  private:
	PyObject *py_xml_parser;

	friend PyObject *xml_parserObj_New(ambulant::lib::xml_parser *itself);
};
#define BGEN_BACK_SUPPORT_xml_parser
inline xml_parser *Py_WrapAs_xml_parser(PyObject *o) { return new xml_parser(o); }

class system_embedder : public ambulant::lib::system_embedder {
public:
	system_embedder(PyObject *itself);
	virtual ~system_embedder();

	void show_file(const ambulant::net::url& href);
  private:
	PyObject *py_system_embedder;

	friend PyObject *system_embedderObj_New(ambulant::lib::system_embedder *itself);
};
#define BGEN_BACK_SUPPORT_system_embedder
inline system_embedder *Py_WrapAs_system_embedder(PyObject *o) { return new system_embedder(o); }

class timer_events : public ambulant::lib::timer_events {
public:
	timer_events(PyObject *itself);
	virtual ~timer_events();

	void speed_changed();
  private:
	PyObject *py_timer_events;

	friend PyObject *timer_eventsObj_New(ambulant::lib::timer_events *itself);
};
#define BGEN_BACK_SUPPORT_timer_events
inline timer_events *Py_WrapAs_timer_events(PyObject *o) { return new timer_events(o); }

class timer : public ambulant::lib::timer {
public:
	timer(PyObject *itself);
	virtual ~timer();

	ambulant::lib::timer::time_type elapsed() const;
	double get_realtime_speed() const;
  private:
	PyObject *py_timer;

	friend PyObject *timerObj_New(ambulant::lib::timer *itself);
};
#define BGEN_BACK_SUPPORT_timer
inline timer *Py_WrapAs_timer(PyObject *o) { return new timer(o); }

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
  private:
	PyObject *py_timer_control;

	friend PyObject *timer_controlObj_New(ambulant::lib::timer_control *itself);
};
#define BGEN_BACK_SUPPORT_timer_control
inline timer_control *Py_WrapAs_timer_control(PyObject *o) { return new timer_control(o); }

class embedder : public system_embedder, public ambulant::common::embedder {
public:
	embedder(PyObject *itself);
	virtual ~embedder();

	void close(ambulant::common::player* p);
	void open(ambulant::net::url newdoc, bool start, ambulant::common::player* old);
	void done(ambulant::common::player* p);
	void show_file(const ambulant::net::url& url) { system_embedder::show_file(url); }
  private:
	PyObject *py_embedder;

	friend PyObject *embedderObj_New(ambulant::common::embedder *itself);
};
#define BGEN_BACK_SUPPORT_embedder
inline embedder *Py_WrapAs_embedder(PyObject *o) { return new embedder(o); }

class alignment : public ambulant::common::alignment {
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
inline alignment *Py_WrapAs_alignment(PyObject *o) { return new alignment(o); }

class animation_notification : public ambulant::common::animation_notification {
public:
	animation_notification(PyObject *itself);
	virtual ~animation_notification();

	void animated();
  private:
	PyObject *py_animation_notification;

	friend PyObject *animation_notificationObj_New(ambulant::common::animation_notification *itself);
};
#define BGEN_BACK_SUPPORT_animation_notification
inline animation_notification *Py_WrapAs_animation_notification(PyObject *o) { return new animation_notification(o); }

class gui_window : public ambulant::common::gui_window {
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
inline gui_window *Py_WrapAs_gui_window(PyObject *o) { return new gui_window(o); }

class gui_events : public ambulant::common::gui_events {
public:
	gui_events(PyObject *itself);
	virtual ~gui_events();

	void redraw(const ambulant::lib::rect& dirty, ambulant::common::gui_window* window);
	void user_event(const ambulant::lib::point& where, int what);
	void transition_freeze_end(ambulant::lib::rect area);
  private:
	PyObject *py_gui_events;

	friend PyObject *gui_eventsObj_New(ambulant::common::gui_events *itself);
};
#define BGEN_BACK_SUPPORT_gui_events
inline gui_events *Py_WrapAs_gui_events(PyObject *o) { return new gui_events(o); }

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
	void user_event(const ambulant::lib::point&, int) { abort(); }
	void transition_freeze_end(ambulant::lib::rect) { abort(); }
  private:
	PyObject *py_renderer;

	friend PyObject *rendererObj_New(ambulant::common::renderer *itself);
};
#define BGEN_BACK_SUPPORT_renderer
inline renderer *Py_WrapAs_renderer(PyObject *o) { return new renderer(o); }

class bgrenderer : public gui_events, public ambulant::common::bgrenderer {
public:
	bgrenderer(PyObject *itself);
	virtual ~bgrenderer();

	void set_surface(ambulant::common::surface* destination);
	void keep_as_background();
	void redraw(const ambulant::lib::rect&, ambulant::common::gui_window*) { abort(); }
	void user_event(const ambulant::lib::point&, int) { abort(); }
	void transition_freeze_end(ambulant::lib::rect) { abort(); }
  private:
	PyObject *py_bgrenderer;

	friend PyObject *bgrendererObj_New(ambulant::common::bgrenderer *itself);
};
#define BGEN_BACK_SUPPORT_bgrenderer
inline bgrenderer *Py_WrapAs_bgrenderer(PyObject *o) { return new bgrenderer(o); }

class surface : public ambulant::common::surface {
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
	const ambulant::lib::point& get_global_topleft() const;
	ambulant::lib::rect get_fit_rect(const ambulant::lib::size& src_size, ambulant::lib::rect out_src_rect, const ambulant::common::alignment* align) const;
	const ambulant::common::region_info* get_info() const;
#ifdef USE_SMIL21
	ambulant::common::surface* get_top_surface();
#endif
#ifdef USE_SMIL21
	bool is_tiled() const;
#endif
	ambulant::common::gui_window* get_gui_window();
	ambulant::lib::rect get_fit_rect(const ambulant::lib::size&, ambulant::lib::rect*, const ambulant::common::alignment*) const { abort(); }
	ambulant::common::tile_positions get_tiles(ambulant::lib::size s, ambulant::lib::rect r) const { return surface::get_tiles(s, r); }
  private:
	PyObject *py_surface;
	ambulant::lib::rect get_rect_rv;
	ambulant::lib::point get_global_topleft_rv;

	friend PyObject *surfaceObj_New(ambulant::common::surface *itself);
};
#define BGEN_BACK_SUPPORT_surface
inline surface *Py_WrapAs_surface(PyObject *o) { return new surface(o); }

class window_factory : public ambulant::common::window_factory {
public:
	window_factory(PyObject *itself);
	virtual ~window_factory();

	ambulant::common::gui_window* new_window(const std::string& name, ambulant::lib::size bounds, ambulant::common::gui_events* handler);
	ambulant::common::bgrenderer* new_background_renderer(const ambulant::common::region_info* src);
	void window_done(const std::string& name);
  private:
	PyObject *py_window_factory;

	friend PyObject *window_factoryObj_New(ambulant::common::window_factory *itself);
};
#define BGEN_BACK_SUPPORT_window_factory
inline window_factory *Py_WrapAs_window_factory(PyObject *o) { return new window_factory(o); }

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
inline surface_template *Py_WrapAs_surface_template(PyObject *o) { return new surface_template(o); }

class surface_factory : public ambulant::common::surface_factory {
public:
	surface_factory(PyObject *itself);
	virtual ~surface_factory();

	ambulant::common::surface_template* new_topsurface(const ambulant::common::region_info* info, ambulant::common::bgrenderer* bgrend, ambulant::common::window_factory* wf);
  private:
	PyObject *py_surface_factory;

	friend PyObject *surface_factoryObj_New(ambulant::common::surface_factory *itself);
};
#define BGEN_BACK_SUPPORT_surface_factory
inline surface_factory *Py_WrapAs_surface_factory(PyObject *o) { return new surface_factory(o); }

class layout_manager : public ambulant::common::layout_manager {
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
inline layout_manager *Py_WrapAs_layout_manager(PyObject *o) { return new layout_manager(o); }

class playable : public ambulant::common::playable {
public:
	playable(PyObject *itself);
	virtual ~playable();

	void start(double t);
	void stop();
	void pause();
	void resume();
	void seek(double t);
	void wantclicks(bool want);
	void preroll(double when, double where, double how_much);
	ambulant::common::duration get_dur();
	ambulant::common::playable::cookie_type get_cookie() const;
	ambulant::common::renderer* get_renderer();
  private:
	PyObject *py_playable;

	friend PyObject *playableObj_New(ambulant::common::playable *itself);
};
#define BGEN_BACK_SUPPORT_playable
inline playable *Py_WrapAs_playable(PyObject *o) { return new playable(o); }

class playable_notification : public ambulant::common::playable_notification {
public:
	playable_notification(PyObject *itself);
	virtual ~playable_notification();

	void started(ambulant::common::playable::cookie_type n, double t);
	void stopped(ambulant::common::playable::cookie_type n, double t);
	void stalled(ambulant::common::playable::cookie_type n, double t);
	void unstalled(ambulant::common::playable::cookie_type n, double t);
	void clicked(ambulant::common::playable::cookie_type n, double t);
	void pointed(ambulant::common::playable::cookie_type n, double t);
	void transitioned(ambulant::common::playable::cookie_type n, double t);
  private:
	PyObject *py_playable_notification;

	friend PyObject *playable_notificationObj_New(ambulant::common::playable_notification *itself);
};
#define BGEN_BACK_SUPPORT_playable_notification
inline playable_notification *Py_WrapAs_playable_notification(PyObject *o) { return new playable_notification(o); }

class playable_factory : public ambulant::common::playable_factory {
public:
	playable_factory(PyObject *itself);
	virtual ~playable_factory();

	ambulant::common::playable* new_playable(ambulant::common::playable_notification* context, ambulant::common::playable::cookie_type cookie, const ambulant::lib::node* node, ambulant::lib::event_processor* evp);
	ambulant::common::playable* new_aux_audio_playable(ambulant::common::playable_notification* context, ambulant::common::playable::cookie_type cookie, const ambulant::lib::node* node, ambulant::lib::event_processor* evp, ambulant::net::audio_datasource* src);
  private:
	PyObject *py_playable_factory;

	friend PyObject *playable_factoryObj_New(ambulant::common::playable_factory *itself);
};
#define BGEN_BACK_SUPPORT_playable_factory
inline playable_factory *Py_WrapAs_playable_factory(PyObject *o) { return new playable_factory(o); }

class global_playable_factory : public playable_factory, public ambulant::common::global_playable_factory {
public:
	global_playable_factory(PyObject *itself);
	virtual ~global_playable_factory();

	void add_factory(ambulant::common::playable_factory* rf);
	ambulant::common::playable* new_playable(ambulant::common::playable_notification*, int, const ambulant::lib::node*, ambulant::lib::event_processor*) { abort(); }
	ambulant::common::playable* new_aux_audio_playable(ambulant::common::playable_notification *context, int, const ambulant::lib::node *node, ambulant::lib::event_processor *evp, ambulant::net::audio_datasource *src) { abort(); }
  private:
	PyObject *py_global_playable_factory;

	friend PyObject *global_playable_factoryObj_New(ambulant::common::global_playable_factory *itself);
};
#define BGEN_BACK_SUPPORT_global_playable_factory
inline global_playable_factory *Py_WrapAs_global_playable_factory(PyObject *o) { return new global_playable_factory(o); }

class player_feedback : public ambulant::common::player_feedback {
public:
	player_feedback(PyObject *itself);
	virtual ~player_feedback();

	void node_started(const ambulant::lib::node* n);
	void node_stopped(const ambulant::lib::node* n);
  private:
	PyObject *py_player_feedback;

	friend PyObject *player_feedbackObj_New(ambulant::common::player_feedback *itself);
};
#define BGEN_BACK_SUPPORT_player_feedback
inline player_feedback *Py_WrapAs_player_feedback(PyObject *o) { return new player_feedback(o); }

class player : public ambulant::common::player {
public:
	player(PyObject *itself);
	virtual ~player();

#ifdef USE_SMIL21
	void initialize();
#endif
	ambulant::lib::timer* get_timer();
	ambulant::lib::event_processor* get_evp();
	void start();
	void stop();
	void pause();
	void resume();
	bool is_playing() const;
	bool is_pausing() const;
	bool is_done() const;
	int get_cursor() const;
	void set_cursor(int cursor);
	void set_feedback(ambulant::common::player_feedback* fb);
	bool goto_node(const ambulant::lib::node* n);
  private:
	PyObject *py_player;

	friend PyObject *playerObj_New(ambulant::common::player *itself);
};
#define BGEN_BACK_SUPPORT_player
inline player *Py_WrapAs_player(PyObject *o) { return new player(o); }

class region_info : public ambulant::common::region_info {
public:
	region_info(PyObject *itself);
	virtual ~region_info();

	std::string get_name() const;
	ambulant::lib::rect get_rect() const;
	ambulant::common::fit_t get_fit() const;
	ambulant::lib::color_t get_bgcolor() const;
	bool get_transparent() const;
	ambulant::common::zindex_t get_zindex() const;
	bool get_showbackground() const;
	bool is_subregion() const;
	double get_soundlevel() const;
#ifdef USE_SMIL21
	ambulant::common::sound_alignment get_soundalign() const;
#endif
#ifdef USE_SMIL21
	ambulant::common::tiling get_tiling() const;
#endif
#ifdef USE_SMIL21
	const char * get_bgimage() const;
#endif
  private:
	PyObject *py_region_info;

	friend PyObject *region_infoObj_New(ambulant::common::region_info *itself);
};
#define BGEN_BACK_SUPPORT_region_info
inline region_info *Py_WrapAs_region_info(PyObject *o) { return new region_info(o); }

class animation_destination : public region_info, public ambulant::common::animation_destination {
public:
	animation_destination(PyObject *itself);
	virtual ~animation_destination();

	ambulant::common::region_dim get_region_dim(const std::string& which, bool fromdom) const;
	ambulant::lib::color_t get_region_color(const std::string& which, bool fromdom) const;
	ambulant::common::zindex_t get_region_zindex(bool fromdom) const;
	double get_region_soundlevel(bool fromdom) const;
#ifdef USE_SMIL21
	ambulant::common::sound_alignment get_region_soundalign(bool fromdom) const;
#endif
	void set_region_dim(const std::string& which, const ambulant::common::region_dim& rd);
	void set_region_color(const std::string& which, ambulant::lib::color_t clr);
	void set_region_zindex(ambulant::common::zindex_t z);
	void set_region_soundlevel(double level);
#ifdef USE_SMIL21
	void set_region_soundalign(ambulant::common::sound_alignment sa);
#endif
	std::string get_name() const { return region_info::get_name(); }
	ambulant::lib::rect get_rect() const { return region_info::get_rect(); }
	ambulant::common::fit_t get_fit() const { return region_info::get_fit(); }
	ambulant::lib::color_t get_bgcolor() const { return region_info::get_bgcolor(); }
	bool get_transparent() const { return region_info::get_transparent(); }
	ambulant::common::zindex_t get_zindex() const { return region_info::get_zindex(); }
	bool get_showbackground() const { return region_info::get_showbackground(); }
	bool is_subregion() const { return region_info::is_subregion(); }
	double get_soundlevel() const { return region_info::get_soundlevel(); }
	ambulant::common::sound_alignment get_soundalign() const { return region_info::get_soundalign(); }
	ambulant::common::tiling get_tiling() const { return region_info::get_tiling(); }
	const char* get_bgimage() const { return region_info::get_bgimage(); }
  private:
	PyObject *py_animation_destination;

	friend PyObject *animation_destinationObj_New(ambulant::common::animation_destination *itself);
};
#define BGEN_BACK_SUPPORT_animation_destination
inline animation_destination *Py_WrapAs_animation_destination(PyObject *o) { return new animation_destination(o); }

class datasource : public ambulant::net::datasource {
public:
	datasource(PyObject *itself);
	virtual ~datasource();

	void start(ambulant::lib::event_processor* evp, ambulant::lib::event* callback);
	void stop();
	bool end_of_file();
	int size() const;
	void readdone(int len);
	long add_ref() { return 1; }
	long release() { return 1;}
	long get_ref_count() const { return 1; }
	char *get_read_ptr() { abort(); return NULL;}
  private:
	PyObject *py_datasource;

	friend PyObject *datasourceObj_New(ambulant::net::datasource *itself);
};
#define BGEN_BACK_SUPPORT_datasource
inline datasource *Py_WrapAs_datasource(PyObject *o) { return new datasource(o); }

class raw_datasource_factory : public ambulant::net::raw_datasource_factory {
public:
	raw_datasource_factory(PyObject *itself);
	virtual ~raw_datasource_factory();

	ambulant::net::datasource* new_raw_datasource(const ambulant::net::url& url);
  private:
	PyObject *py_raw_datasource_factory;

	friend PyObject *raw_datasource_factoryObj_New(ambulant::net::raw_datasource_factory *itself);
};
#define BGEN_BACK_SUPPORT_raw_datasource_factory
inline raw_datasource_factory *Py_WrapAs_raw_datasource_factory(PyObject *o) { return new raw_datasource_factory(o); }

class audio_datasource_factory : public ambulant::net::audio_datasource_factory {
public:
	audio_datasource_factory(PyObject *itself);
	virtual ~audio_datasource_factory();

	ambulant::net::audio_datasource* new_audio_datasource(const ambulant::net::url& url, const ambulant::net::audio_format_choices& fmt, ambulant::net::timestamp_t clip_begin, ambulant::net::timestamp_t clip_end);
  private:
	PyObject *py_audio_datasource_factory;

	friend PyObject *audio_datasource_factoryObj_New(ambulant::net::audio_datasource_factory *itself);
};
#define BGEN_BACK_SUPPORT_audio_datasource_factory
inline audio_datasource_factory *Py_WrapAs_audio_datasource_factory(PyObject *o) { return new audio_datasource_factory(o); }

class video_datasource_factory : public ambulant::net::video_datasource_factory {
public:
	video_datasource_factory(PyObject *itself);
	virtual ~video_datasource_factory();

	ambulant::net::video_datasource* new_video_datasource(const ambulant::net::url& url, ambulant::net::timestamp_t clip_begin, ambulant::net::timestamp_t clip_end);
  private:
	PyObject *py_video_datasource_factory;

	friend PyObject *video_datasource_factoryObj_New(ambulant::net::video_datasource_factory *itself);
};
#define BGEN_BACK_SUPPORT_video_datasource_factory
inline video_datasource_factory *Py_WrapAs_video_datasource_factory(PyObject *o) { return new video_datasource_factory(o); }

class audio_parser_finder : public ambulant::net::audio_parser_finder {
public:
	audio_parser_finder(PyObject *itself);
	virtual ~audio_parser_finder();

	ambulant::net::audio_datasource* new_audio_parser(const ambulant::net::url& url, const ambulant::net::audio_format_choices& hint, ambulant::net::audio_datasource* src);
  private:
	PyObject *py_audio_parser_finder;

	friend PyObject *audio_parser_finderObj_New(ambulant::net::audio_parser_finder *itself);
};
#define BGEN_BACK_SUPPORT_audio_parser_finder
inline audio_parser_finder *Py_WrapAs_audio_parser_finder(PyObject *o) { return new audio_parser_finder(o); }

class audio_filter_finder : public ambulant::net::audio_filter_finder {
public:
	audio_filter_finder(PyObject *itself);
	virtual ~audio_filter_finder();

	ambulant::net::audio_datasource* new_audio_filter(ambulant::net::audio_datasource* src, const ambulant::net::audio_format_choices& fmts);
  private:
	PyObject *py_audio_filter_finder;

	friend PyObject *audio_filter_finderObj_New(ambulant::net::audio_filter_finder *itself);
};
#define BGEN_BACK_SUPPORT_audio_filter_finder
inline audio_filter_finder *Py_WrapAs_audio_filter_finder(PyObject *o) { return new audio_filter_finder(o); }

