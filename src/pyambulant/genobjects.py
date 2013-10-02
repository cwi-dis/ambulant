FORMAT="""class %(pname)sObjectDefinition(MyGlobalObjectDefinition):
    %(bname)s
    
%(pname)s_object = %(pname)sObjectDefinition('%(pname)s', '%(pname)sObj', '%(cname)s*')
methods_%(pname)s = []
module.addobject(%(pname)s_object)

%(pname)s_ptr = OpaqueByValueType('%(cname)s*', '%(pname)sObj')
const_%(pname)s_ptr = OpaqueByValueType('const %(cname)s*', '%(pname)sObj')
"""

INCLFORMAT="""extern PyObject *%(pname)sObj_New(%(cname)s* itself);
extern int %(pname)sObj_Convert(PyObject *v, %(cname)s* *p_itself);
"""


OBJECTS=[
    "lib/amstream.h",
    ("ostream", "ambulant::lib::ostream", None),
    
    "lib/logger.h",
    ("logger", "ambulant::lib::logger", None),
    
	"lib/node.h",
	("node_context", "ambulant::lib::node_context", None),
	("node", "ambulant::lib::node", None),
	("node_factory", "ambulant::lib::node_factory", None),
	
	"lib/document.h",
	("document", "ambulant::lib::document", "node_context"),
	
	"lib/event.h",
	("event", "ambulant::lib::event", None),
	
	"lib/event_processor.h",
	("event_processor", "ambulant::lib::event_processor", None),
	
	"lib/parser_factory.h",
	("parser_factory", "ambulant::lib::parser_factory", None),
	("global_parser_factory", "ambulant::lib::global_parser_factory", "parser_factory"),
	
	
	"lib/sax_handler.h",
	("xml_parser", "ambulant::lib::xml_parser", None),
	
	"lib/system.h",
	("system_embedder", "ambulant::lib::system_embedder", None),
	
	"lib/timer.h",
	("timer", "ambulant::lib::timer", None),
	("timer_control", "ambulant::lib::timer_control", "timer"),
	("timer_control_impl", "ambulant::lib::timer_control_impl", "timer_control"),
	("timer_observer", "ambulant::lib::timer_observer", None),
	
	"lib/timer_sync.h",
	("timer_sync", "ambulant::lib::timer_sync", "timer_observer"),
	("timer_sync_factory", "ambulant::lib::timer_sync_factory", None),
	
	"lib/transition_info.h",
	("transition_info", "ambulant::lib::transition_info", None),
	
	"common/embedder.h",
	("embedder", "ambulant::common::embedder", "system_embedder"),
	
	"common/factory.h",
	("factories", "ambulant::common::factories", None),
	
	"common/gui_player.h",
	("gui_screen", "ambulant::common::gui_screen", None),
	("gui_player", "ambulant::common::gui_player", "factories"),
	
	"common/layout.h",
	("alignment", "ambulant::common::alignment", None),
	("animation_notification", "ambulant::common::animation_notification", None),
	("gui_window", "ambulant::common::gui_window", None),
	("gui_events", "ambulant::common::gui_events", None),
	("renderer", "ambulant::common::renderer", "gui_events"),
	("bgrenderer", "ambulant::common::bgrenderer", "gui_events"),
	("surface", "ambulant::common::surface", None),
	("window_factory", "ambulant::common::window_factory", None),
	("surface_template", "ambulant::common::surface_template", "animation_notification"),
	("surface_factory", "ambulant::common::surface_factory", None),
	("layout_manager", "ambulant::common::layout_manager", None),
	
	"common/playable.h",
	("playable", "ambulant::common::playable", None), # XXX Refcounted
	("playable_notification", "ambulant::common::playable_notification", None), 
	("playable_factory", "ambulant::common::playable_factory", None),
	("global_playable_factory", "ambulant::common::global_playable_factory", "playable_factory"),
	
	"common/recorder.h",
	("recorder", "ambulant::common::recorder", None),
	("recorder_factory", "ambulant::common::recorder_factory", None),
	
	"common/renderer_select.h",
	("renderer_select", "ambulant::common::renderer_select", None),
	
	"common/player.h",
	("focus_feedback", "ambulant::common::focus_feedback", None),
	("player_feedback", "ambulant::common::player_feedback", None),
	("player", "ambulant::common::player", None),
	
	"common/region_dim.h",

	"common/region_info.h",
	("region_info", "ambulant::common::region_info", None),
	("animation_destination", "ambulant::common::animation_destination", "region_info"),
	
	"common/state.h",
	("state_test_methods", "ambulant::common::state_test_methods", None),
	("state_change_callback", "ambulant::common::state_change_callback", None),
	("state_component", "ambulant::common::state_component", None),
	("state_component_factory", "ambulant::common::state_component_factory", None),
	("global_state_component_factory", "ambulant::common::global_state_component_factory", "state_component_factory"),
	
	
	"gui/none/none_gui.h",
	("none_window", "ambulant::gui::none::none_window", "gui_window"),
	("none_window_factory", "ambulant::gui::none::none_window_factory", "window_factory"),
	
	"net/datasource.h",
#	("audio_format_choices", "ambulant::net::audio_format_choices", None),
	("datasource", "ambulant::net::datasource", None), # XXX Refcounted
#	("pkt_datasource", "ambulant::net::pkt_datasource", None), # XXX Refcounted
	("audio_datasource", "ambulant::net::audio_datasource", "datasource"), # XXX Refcounted
#	("pkt_datasource", "ambulant::net::pkt_datasource", "pkt_datasource"), # XXX Refcounted
	("video_datasource", "ambulant::net::video_datasource", None), # XXX Refcounted
	("datasource_factory", "ambulant::net::datasource_factory", None),
	("raw_datasource_factory", "ambulant::net::raw_datasource_factory", None),
	("audio_datasource_factory", "ambulant::net::audio_datasource_factory", None),
	("pkt_datasource_factory", "ambulant::net::pkt_datasource_factory", None),
	("video_datasource_factory", "ambulant::net::video_datasource_factory", None),
	("audio_parser_finder", "ambulant::net::audio_parser_finder", None),
	("audio_filter_finder", "ambulant::net::audio_filter_finder", None),
	("audio_decoder_finder", "ambulant::net::audio_decoder_finder", None),
	
	"net/stdio_datasource.h",
	"net/posix_datasource.h",
	
#	"gui/gtk/gtk_factory.h",
	"gui/SDL/sdl_factory.h",
	"net/ffmpeg_factory.h",
]

out = open('ambulantobjgen.py', 'w')
incout = open('ambulantincludegen.py', 'w')
modout = open('ambulantmodule.h', 'w')
for item in OBJECTS:
	if type(item) == type(""):
		print >>out, "# From %s:" % item
		inc = '#include "ambulant/%s"' % item
		print >>incout, 'includestuff = includestuff + \'' + inc + '\\n\''
		continue
	pname, cname, bname = item
	if bname:
		bname = 'basetype = "%s_Type"\n    baseclass = "%s"' % (bname, bname)
	else:
		bname = 'basetype = "pycppbridge_Type"'
	print >>out, FORMAT % locals()
	print >>modout, INCLFORMAT % locals()
	
