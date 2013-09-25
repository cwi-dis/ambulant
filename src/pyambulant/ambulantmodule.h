extern PyObject *ostreamObj_New(ambulant::lib::ostream* itself);
extern int ostreamObj_Convert(PyObject *v, ambulant::lib::ostream* *p_itself);

extern PyObject *loggerObj_New(ambulant::lib::logger* itself);
extern int loggerObj_Convert(PyObject *v, ambulant::lib::logger* *p_itself);

extern PyObject *node_contextObj_New(ambulant::lib::node_context* itself);
extern int node_contextObj_Convert(PyObject *v, ambulant::lib::node_context* *p_itself);

extern PyObject *nodeObj_New(ambulant::lib::node* itself);
extern int nodeObj_Convert(PyObject *v, ambulant::lib::node* *p_itself);

extern PyObject *node_factoryObj_New(ambulant::lib::node_factory* itself);
extern int node_factoryObj_Convert(PyObject *v, ambulant::lib::node_factory* *p_itself);

extern PyObject *documentObj_New(ambulant::lib::document* itself);
extern int documentObj_Convert(PyObject *v, ambulant::lib::document* *p_itself);

extern PyObject *eventObj_New(ambulant::lib::event* itself);
extern int eventObj_Convert(PyObject *v, ambulant::lib::event* *p_itself);

extern PyObject *event_processorObj_New(ambulant::lib::event_processor* itself);
extern int event_processorObj_Convert(PyObject *v, ambulant::lib::event_processor* *p_itself);

extern PyObject *parser_factoryObj_New(ambulant::lib::parser_factory* itself);
extern int parser_factoryObj_Convert(PyObject *v, ambulant::lib::parser_factory* *p_itself);

extern PyObject *global_parser_factoryObj_New(ambulant::lib::global_parser_factory* itself);
extern int global_parser_factoryObj_Convert(PyObject *v, ambulant::lib::global_parser_factory* *p_itself);

extern PyObject *xml_parserObj_New(ambulant::lib::xml_parser* itself);
extern int xml_parserObj_Convert(PyObject *v, ambulant::lib::xml_parser* *p_itself);

extern PyObject *system_embedderObj_New(ambulant::lib::system_embedder* itself);
extern int system_embedderObj_Convert(PyObject *v, ambulant::lib::system_embedder* *p_itself);

extern PyObject *timerObj_New(ambulant::lib::timer* itself);
extern int timerObj_Convert(PyObject *v, ambulant::lib::timer* *p_itself);

extern PyObject *timer_controlObj_New(ambulant::lib::timer_control* itself);
extern int timer_controlObj_Convert(PyObject *v, ambulant::lib::timer_control* *p_itself);

extern PyObject *timer_control_implObj_New(ambulant::lib::timer_control_impl* itself);
extern int timer_control_implObj_Convert(PyObject *v, ambulant::lib::timer_control_impl* *p_itself);

extern PyObject *timer_observerObj_New(ambulant::lib::timer_observer* itself);
extern int timer_observerObj_Convert(PyObject *v, ambulant::lib::timer_observer* *p_itself);

extern PyObject *timer_syncObj_New(ambulant::lib::timer_sync* itself);
extern int timer_syncObj_Convert(PyObject *v, ambulant::lib::timer_sync* *p_itself);

extern PyObject *timer_sync_factoryObj_New(ambulant::lib::timer_sync_factory* itself);
extern int timer_sync_factoryObj_Convert(PyObject *v, ambulant::lib::timer_sync_factory* *p_itself);

extern PyObject *transition_infoObj_New(ambulant::lib::transition_info* itself);
extern int transition_infoObj_Convert(PyObject *v, ambulant::lib::transition_info* *p_itself);

extern PyObject *embedderObj_New(ambulant::common::embedder* itself);
extern int embedderObj_Convert(PyObject *v, ambulant::common::embedder* *p_itself);

extern PyObject *factoriesObj_New(ambulant::common::factories* itself);
extern int factoriesObj_Convert(PyObject *v, ambulant::common::factories* *p_itself);

extern PyObject *gui_screenObj_New(ambulant::common::gui_screen* itself);
extern int gui_screenObj_Convert(PyObject *v, ambulant::common::gui_screen* *p_itself);

extern PyObject *gui_playerObj_New(ambulant::common::gui_player* itself);
extern int gui_playerObj_Convert(PyObject *v, ambulant::common::gui_player* *p_itself);

extern PyObject *alignmentObj_New(ambulant::common::alignment* itself);
extern int alignmentObj_Convert(PyObject *v, ambulant::common::alignment* *p_itself);

extern PyObject *animation_notificationObj_New(ambulant::common::animation_notification* itself);
extern int animation_notificationObj_Convert(PyObject *v, ambulant::common::animation_notification* *p_itself);

extern PyObject *gui_windowObj_New(ambulant::common::gui_window* itself);
extern int gui_windowObj_Convert(PyObject *v, ambulant::common::gui_window* *p_itself);

extern PyObject *gui_eventsObj_New(ambulant::common::gui_events* itself);
extern int gui_eventsObj_Convert(PyObject *v, ambulant::common::gui_events* *p_itself);

extern PyObject *rendererObj_New(ambulant::common::renderer* itself);
extern int rendererObj_Convert(PyObject *v, ambulant::common::renderer* *p_itself);

extern PyObject *bgrendererObj_New(ambulant::common::bgrenderer* itself);
extern int bgrendererObj_Convert(PyObject *v, ambulant::common::bgrenderer* *p_itself);

extern PyObject *surfaceObj_New(ambulant::common::surface* itself);
extern int surfaceObj_Convert(PyObject *v, ambulant::common::surface* *p_itself);

extern PyObject *window_factoryObj_New(ambulant::common::window_factory* itself);
extern int window_factoryObj_Convert(PyObject *v, ambulant::common::window_factory* *p_itself);

extern PyObject *surface_templateObj_New(ambulant::common::surface_template* itself);
extern int surface_templateObj_Convert(PyObject *v, ambulant::common::surface_template* *p_itself);

extern PyObject *surface_factoryObj_New(ambulant::common::surface_factory* itself);
extern int surface_factoryObj_Convert(PyObject *v, ambulant::common::surface_factory* *p_itself);

extern PyObject *layout_managerObj_New(ambulant::common::layout_manager* itself);
extern int layout_managerObj_Convert(PyObject *v, ambulant::common::layout_manager* *p_itself);

extern PyObject *playableObj_New(ambulant::common::playable* itself);
extern int playableObj_Convert(PyObject *v, ambulant::common::playable* *p_itself);

extern PyObject *playable_notificationObj_New(ambulant::common::playable_notification* itself);
extern int playable_notificationObj_Convert(PyObject *v, ambulant::common::playable_notification* *p_itself);

extern PyObject *playable_factoryObj_New(ambulant::common::playable_factory* itself);
extern int playable_factoryObj_Convert(PyObject *v, ambulant::common::playable_factory* *p_itself);

extern PyObject *global_playable_factoryObj_New(ambulant::common::global_playable_factory* itself);
extern int global_playable_factoryObj_Convert(PyObject *v, ambulant::common::global_playable_factory* *p_itself);

extern PyObject *recorderObj_New(ambulant::common::recorder* itself);
extern int recorderObj_Convert(PyObject *v, ambulant::common::recorder* *p_itself);

extern PyObject *recorder_factoryObj_New(ambulant::common::recorder_factory* itself);
extern int recorder_factoryObj_Convert(PyObject *v, ambulant::common::recorder_factory* *p_itself);

extern PyObject *renderer_selectObj_New(ambulant::common::renderer_select* itself);
extern int renderer_selectObj_Convert(PyObject *v, ambulant::common::renderer_select* *p_itself);

extern PyObject *focus_feedbackObj_New(ambulant::common::focus_feedback* itself);
extern int focus_feedbackObj_Convert(PyObject *v, ambulant::common::focus_feedback* *p_itself);

extern PyObject *player_feedbackObj_New(ambulant::common::player_feedback* itself);
extern int player_feedbackObj_Convert(PyObject *v, ambulant::common::player_feedback* *p_itself);

extern PyObject *playerObj_New(ambulant::common::player* itself);
extern int playerObj_Convert(PyObject *v, ambulant::common::player* *p_itself);

extern PyObject *region_infoObj_New(ambulant::common::region_info* itself);
extern int region_infoObj_Convert(PyObject *v, ambulant::common::region_info* *p_itself);

extern PyObject *animation_destinationObj_New(ambulant::common::animation_destination* itself);
extern int animation_destinationObj_Convert(PyObject *v, ambulant::common::animation_destination* *p_itself);

extern PyObject *state_test_methodsObj_New(ambulant::common::state_test_methods* itself);
extern int state_test_methodsObj_Convert(PyObject *v, ambulant::common::state_test_methods* *p_itself);

extern PyObject *state_change_callbackObj_New(ambulant::common::state_change_callback* itself);
extern int state_change_callbackObj_Convert(PyObject *v, ambulant::common::state_change_callback* *p_itself);

extern PyObject *state_componentObj_New(ambulant::common::state_component* itself);
extern int state_componentObj_Convert(PyObject *v, ambulant::common::state_component* *p_itself);

extern PyObject *state_component_factoryObj_New(ambulant::common::state_component_factory* itself);
extern int state_component_factoryObj_Convert(PyObject *v, ambulant::common::state_component_factory* *p_itself);

extern PyObject *global_state_component_factoryObj_New(ambulant::common::global_state_component_factory* itself);
extern int global_state_component_factoryObj_Convert(PyObject *v, ambulant::common::global_state_component_factory* *p_itself);

extern PyObject *none_windowObj_New(ambulant::gui::none::none_window* itself);
extern int none_windowObj_Convert(PyObject *v, ambulant::gui::none::none_window* *p_itself);

extern PyObject *none_window_factoryObj_New(ambulant::gui::none::none_window_factory* itself);
extern int none_window_factoryObj_Convert(PyObject *v, ambulant::gui::none::none_window_factory* *p_itself);

extern PyObject *datasourceObj_New(ambulant::net::datasource* itself);
extern int datasourceObj_Convert(PyObject *v, ambulant::net::datasource* *p_itself);

extern PyObject *audio_datasourceObj_New(ambulant::net::audio_datasource* itself);
extern int audio_datasourceObj_Convert(PyObject *v, ambulant::net::audio_datasource* *p_itself);

extern PyObject *video_datasourceObj_New(ambulant::net::video_datasource* itself);
extern int video_datasourceObj_Convert(PyObject *v, ambulant::net::video_datasource* *p_itself);

extern PyObject *datasource_factoryObj_New(ambulant::net::datasource_factory* itself);
extern int datasource_factoryObj_Convert(PyObject *v, ambulant::net::datasource_factory* *p_itself);

extern PyObject *raw_datasource_factoryObj_New(ambulant::net::raw_datasource_factory* itself);
extern int raw_datasource_factoryObj_Convert(PyObject *v, ambulant::net::raw_datasource_factory* *p_itself);

extern PyObject *audio_datasource_factoryObj_New(ambulant::net::audio_datasource_factory* itself);
extern int audio_datasource_factoryObj_Convert(PyObject *v, ambulant::net::audio_datasource_factory* *p_itself);

extern PyObject *pkt_datasource_factoryObj_New(ambulant::net::pkt_datasource_factory* itself);
extern int pkt_datasource_factoryObj_Convert(PyObject *v, ambulant::net::pkt_datasource_factory* *p_itself);

extern PyObject *video_datasource_factoryObj_New(ambulant::net::video_datasource_factory* itself);
extern int video_datasource_factoryObj_Convert(PyObject *v, ambulant::net::video_datasource_factory* *p_itself);

extern PyObject *audio_parser_finderObj_New(ambulant::net::audio_parser_finder* itself);
extern int audio_parser_finderObj_Convert(PyObject *v, ambulant::net::audio_parser_finder* *p_itself);

extern PyObject *audio_filter_finderObj_New(ambulant::net::audio_filter_finder* itself);
extern int audio_filter_finderObj_Convert(PyObject *v, ambulant::net::audio_filter_finder* *p_itself);

extern PyObject *audio_decoder_finderObj_New(ambulant::net::audio_decoder_finder* itself);
extern int audio_decoder_finderObj_Convert(PyObject *v, ambulant::net::audio_decoder_finder* *p_itself);

