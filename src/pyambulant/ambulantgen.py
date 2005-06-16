# Generated from '/usr/local/include/ambulant/lib/node.h'

f = CxxMethodGenerator(void, 'down',
    (node_interface_ptr, 'n', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'up',
    (node_interface_ptr, 'n', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'next',
    (node_interface_ptr, 'n', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(const_node_interface_ptr, 'previous',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(const_node_interface_ptr, 'get_last_child',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(node_interface_ptr, 'locate_node',
    (stringptr, 'path', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(node_interface_ptr, 'get_root',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(node_interface_ptr, 'append_child',
    (node_interface_ptr, 'child', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(node_interface_ptr, 'detach',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(node_interface_ptr, 'clone',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'append_data',
    (InBuffer, 'data', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'set_attribute',
    (stringptr, 'name', InMode),
    (stringptr, 'value', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(int, 'get_numid',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(net_url, 'get_url',
    (stringptr, 'attrname', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(unsigned_int, 'size',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(std_string, 'get_path_display_desc',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(std_string, 'get_sig',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(const_node_context_ptr, 'get_context',
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'set_context',
    (node_context_ptr, 'c', InMode),
)
methods_node_interface.append(f)

f = CxxMethodGenerator(void, 'set_prefix_mapping',
    (std_string, 'prefix', InMode+RefMode),
    (std_string, 'uri', InMode+RefMode),
)
methods_node_context.append(f)

f = CxxMethodGenerator(net_url, 'resolve_url',
    (net_url, 'rurl', InMode+RefMode),
)
methods_node_context.append(f)

f = CxxMethodGenerator(const_node_ptr, 'get_node',
    (std_string, 'idd', InMode+RefMode),
)
methods_node_context.append(f)

f = CxxMethodGenerator(void, 'fire',
)
methods_event.append(f)

f = CxxMethodGenerator(void, 'add_event',
    (event_ptr, 'pe', InMode),
    (time_type, 't', InMode),
    (event_priority, 'priority', InMode),
)
methods_event_processor.append(f)

f = CxxMethodGenerator(void, 'cancel_all_events',
)
methods_event_processor.append(f)

f = CxxMethodGenerator(bool, 'cancel_event',
    (event_ptr, 'pe', InMode),
    (event_priority, 'priority', InMode),
)
methods_event_processor.append(f)

f = CxxMethodGenerator(void, 'serve_events',
)
methods_event_processor.append(f)

f = CxxMethodGenerator(void, 'stop_processor_thread',
)
methods_event_processor.append(f)

f = CxxMethodGenerator(std_string, 'get_parser_name',
)
methods_parser_factory.append(f)

f = CxxMethodGenerator(bool, 'parse',
    (InBuffer, 'buf', InMode),
    (bool, 'final', InMode),
)
methods_xml_parser.append(f)

f = CxxMethodGenerator(void, 'show_file',
    (net_url, 'href', InMode+RefMode),
)
methods_system.append(f)

f = CxxMethodGenerator(void, 'speed_changed',
)
methods_timer_events.append(f)

f = CxxMethodGenerator(time_type, 'elapsed',
)
methods_abstract_timer.append(f)

f = CxxMethodGenerator(double, 'get_realtime_speed',
)
methods_abstract_timer.append(f)

f = CxxMethodGenerator(lib_point, 'get_image_fixpoint',
    (lib_size, 'image_size', InMode),
)
methods_alignment.append(f)

f = CxxMethodGenerator(lib_point, 'get_surface_fixpoint',
    (lib_size, 'surface_size', InMode),
)
methods_alignment.append(f)

f = CxxMethodGenerator(void, 'animated',
)
methods_animation_notification.append(f)

f = CxxMethodGenerator(void, 'need_redraw',
    (lib_screen_rect_int, 'r', InMode+RefMode),
)
methods_gui_window.append(f)

f = CxxMethodGenerator(void, 'redraw_now',
)
methods_gui_window.append(f)

f = CxxMethodGenerator(void, 'need_events',
    (bool, 'want', InMode),
)
methods_gui_window.append(f)

f = CxxMethodGenerator(void, 'redraw',
    (lib_screen_rect_int, 'dirty', InMode+RefMode),
    (gui_window_ptr, 'window', InMode),
)
methods_gui_events.append(f)

f = CxxMethodGenerator(void, 'user_event',
    (lib_point, 'where', InMode+RefMode),
    (int, 'what', InMode),
)
methods_gui_events.append(f)

f = CxxMethodGenerator(void, 'transition_freeze_end',
    (lib_screen_rect_int, 'area', InMode),
)
methods_gui_events.append(f)

f = CxxMethodGenerator(void, 'set_surface',
    (surface_ptr, 'destination', InMode),
)
methods_renderer.append(f)

f = CxxMethodGenerator(void, 'set_alignment',
    (alignment_ptr, 'align', InMode),
)
methods_renderer.append(f)

f = CxxMethodGenerator(void, 'keep_as_background',
)
methods_bgrenderer.append(f)

f = CxxMethodGenerator(void, 'show',
    (gui_events_ptr, 'renderer', InMode),
)
methods_surface.append(f)

f = CxxMethodGenerator(void, 'renderer_done',
    (gui_events_ptr, 'renderer', InMode),
)
methods_surface.append(f)

f = CxxMethodGenerator(void, 'transition_done',
)
methods_surface.append(f)

f = CxxMethodGenerator(screen_rect_int, 'get_rect',
)
methods_surface.append(f)

f = CxxMethodGenerator(lib_screen_rect_int, 'get_fit_rect',
    (lib_size, 'src_size', InMode+RefMode),
    (lib_rect, 'out_src_rect', OutMode),
    (alignment_ptr, 'align', InMode),
)
methods_surface.append(f)

f = CxxMethodGenerator(bool, 'is_tiled',
    condition='#ifdef USE_SMIL21',
)
methods_surface.append(f)

f = CxxMethodGenerator(void, 'window_done',
    (std_string, 'name', InMode+RefMode),
)
methods_window_factory.append(f)

f = CxxMethodGenerator(void, 'start',
    (double, 't', InMode),
)
methods_playable.append(f)

f = CxxMethodGenerator(void, 'stop',
)
methods_playable.append(f)

f = CxxMethodGenerator(void, 'pause',
)
methods_playable.append(f)

f = CxxMethodGenerator(void, 'resume',
)
methods_playable.append(f)

f = CxxMethodGenerator(void, 'seek',
    (double, 't', InMode),
)
methods_playable.append(f)

f = CxxMethodGenerator(void, 'wantclicks',
    (bool, 'want', InMode),
)
methods_playable.append(f)

f = CxxMethodGenerator(void, 'preroll',
    (double, 'when', InMode),
    (double, 'where', InMode),
    (double, 'how_much', InMode),
)
methods_playable.append(f)

f = CxxMethodGenerator(const_cookie_type_ref, 'get_cookie',
)
methods_playable.append(f)

f = CxxMethodGenerator(void, 'started',
    (cookie_type, 'n', InMode),
    (double, 't', InMode),
)
methods_playable_notification.append(f)

f = CxxMethodGenerator(void, 'stopped',
    (cookie_type, 'n', InMode),
    (double, 't', InMode),
)
methods_playable_notification.append(f)

f = CxxMethodGenerator(void, 'stalled',
    (cookie_type, 'n', InMode),
    (double, 't', InMode),
)
methods_playable_notification.append(f)

f = CxxMethodGenerator(void, 'unstalled',
    (cookie_type, 'n', InMode),
    (double, 't', InMode),
)
methods_playable_notification.append(f)

f = CxxMethodGenerator(void, 'clicked',
    (cookie_type, 'n', InMode),
    (double, 't', InMode),
)
methods_playable_notification.append(f)

f = CxxMethodGenerator(void, 'pointed',
    (cookie_type, 'n', InMode),
    (double, 't', InMode),
)
methods_playable_notification.append(f)

f = CxxMethodGenerator(void, 'transitioned',
    (cookie_type, 'n', InMode),
    (double, 't', InMode),
)
methods_playable_notification.append(f)

f = CxxMethodGenerator(void, 'node_started',
    (lib_node_ptr, 'n', InMode),
)
methods_player_feedback.append(f)

f = CxxMethodGenerator(void, 'node_stopped',
    (lib_node_ptr, 'n', InMode),
)
methods_player_feedback.append(f)

f = CxxMethodGenerator(void, 'initialize',
    condition='#ifdef USE_SMIL21',
)
methods_player.append(f)

f = CxxMethodGenerator(lib_timer_ptr, 'get_timer',
)
methods_player.append(f)

f = CxxMethodGenerator(lib_event_processor_ptr, 'get_evp',
)
methods_player.append(f)

f = CxxMethodGenerator(bool, 'is_playing',
)
methods_player.append(f)

f = CxxMethodGenerator(bool, 'is_pausing',
)
methods_player.append(f)

f = CxxMethodGenerator(bool, 'is_done',
)
methods_player.append(f)

f = CxxMethodGenerator(int, 'get_cursor',
)
methods_player.append(f)

f = CxxMethodGenerator(void, 'set_cursor',
    (int, 'cursor', InMode),
)
methods_player.append(f)

f = CxxMethodGenerator(void, 'set_feedback',
    (player_feedback_ptr, 'fb', InMode),
)
methods_player.append(f)

f = CxxMethodGenerator(bool, 'goto_node',
    (lib_node_ptr, 'n', InMode),
)
methods_player.append(f)

f = CxxMethodGenerator(std_string, 'get_name',
)
methods_region_info.append(f)

f = CxxMethodGenerator(screen_rect_int, 'get_screen_rect',
)
methods_region_info.append(f)

f = CxxMethodGenerator(fit_t, 'get_fit',
)
methods_region_info.append(f)

f = CxxMethodGenerator(color_t, 'get_bgcolor',
)
methods_region_info.append(f)

f = CxxMethodGenerator(bool, 'get_transparent',
)
methods_region_info.append(f)

f = CxxMethodGenerator(zindex_t, 'get_zindex',
)
methods_region_info.append(f)

f = CxxMethodGenerator(bool, 'get_showbackground',
)
methods_region_info.append(f)

f = CxxMethodGenerator(bool, 'is_subregion',
)
methods_region_info.append(f)

f = CxxMethodGenerator(double, 'get_soundlevel',
)
methods_region_info.append(f)

f = CxxMethodGenerator(sound_alignment, 'get_soundalign',
    condition='#ifdef USE_SMIL21',
)
methods_region_info.append(f)

f = CxxMethodGenerator(tiling, 'get_tiling',
    condition='#ifdef USE_SMIL21',
)
methods_region_info.append(f)

f = CxxMethodGenerator(color_t, 'get_region_color',
    (std_string, 'which', InMode+RefMode),
    (bool, 'fromdom', InMode),
)
methods_animation_destination.append(f)

f = CxxMethodGenerator(zindex_t, 'get_region_zindex',
    (bool, 'fromdom', InMode),
)
methods_animation_destination.append(f)

f = CxxMethodGenerator(double, 'get_region_soundlevel',
    (bool, 'fromdom', InMode),
)
methods_animation_destination.append(f)

f = CxxMethodGenerator(sound_alignment, 'get_region_soundalign',
    (bool, 'fromdom', InMode),
    condition='#ifdef USE_SMIL21',
)
methods_animation_destination.append(f)

f = CxxMethodGenerator(void, 'set_region_color',
    (std_string, 'which', InMode+RefMode),
    (lib_color_t, 'clr', InMode),
)
methods_animation_destination.append(f)

f = CxxMethodGenerator(void, 'set_region_zindex',
    (common_zindex_t, 'z', InMode),
)
methods_animation_destination.append(f)

f = CxxMethodGenerator(void, 'set_region_soundlevel',
    (double, 'level', InMode),
)
methods_animation_destination.append(f)

f = CxxMethodGenerator(void, 'set_region_soundalign',
    (sound_alignment, 'sa', InMode),
    condition='#ifdef USE_SMIL21',
)
methods_animation_destination.append(f)

f = CxxMethodGenerator(bool, 'end_of_file',
)
methods_datasource.append(f)

f = CxxMethodGenerator(void, 'readdone',
    (int, 'len', InMode),
)
methods_datasource.append(f)

f = CxxMethodGenerator(void, 'read_ahead',
    (timestamp_t, 'time', InMode),
)
methods_audio_datasource.append(f)

f = CxxMethodGenerator(timestamp_t, 'get_clip_end',
)
methods_audio_datasource.append(f)

f = CxxMethodGenerator(timestamp_t, 'get_clip_begin',
)
methods_audio_datasource.append(f)

f = CxxMethodGenerator(bool, 'has_audio',
)
methods_video_datasource.append(f)

f = CxxMethodGenerator(void, 'start_frame',
    (lib_event_processor_ptr, 'evp', InMode),
    (lib_event_ptr, 'callback', InMode),
    (timestamp_t, 'pts', InMode),
)
methods_video_datasource.append(f)

f = CxxMethodGenerator(int, 'width',
)
methods_video_datasource.append(f)

f = CxxMethodGenerator(int, 'height',
)
methods_video_datasource.append(f)

f = CxxMethodGenerator(void, 'frame_done',
    (timestamp_t, 'timestamp', InMode),
    (bool, 'keepdata', InMode),
)
methods_video_datasource.append(f)

f = CxxMethodGenerator(datasource_ptr, 'new_raw_datasource',
    (net_url, 'url', InMode+RefMode),
)
methods_raw_datasource_factory.append(f)

f = CxxMethodGenerator(video_datasource_ptr, 'new_video_datasource',
    (net_url, 'url', InMode+RefMode),
)
methods_video_datasource_factory.append(f)

