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
	"lib/node.h",
	("node_context", "ambulant::lib::node_context", None),
	("node", "ambulant::lib::node", None),
	
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
	("timer_events", "ambulant::lib::timer_events", None),
	("abstract_timer", "ambulant::lib::abstract_timer", None),
	
	"lib/transition_info.h",
	("transition_info", "ambulant::lib::transition_info", None),
	
	"common/embedder.h",
	("embedder", "ambulant::common::embedder", "system_embedder"),
	
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
	
	"common/player.h",
	("player_feedback", "ambulant::common::player_feedback", None),
	("player", "ambulant::common::player", None),
	
	"common/region_info.h",
	("region_info", "ambulant::common::region_info", None),
	("animation_destination", "ambulant::common::animation_destination", "region_info"),
	
	"gui/none/none_gui.h",
	("none_window", "ambulant::gui::none::none_window", "gui_window"),
	("none_window_factory", "ambulant::gui::none::none_window_factory", "window_factory"),
	
	"net/datasource.h",
	("datasource", "ambulant::net::datasource", None), # XXX Refcounted
	("audio_datasource", "ambulant::net::audio_datasource", "datasource"), # XXX Refcounted
	("video_datasource", "ambulant::net::video_datasource", None), # XXX Refcounted
	("datasource_factory", "ambulant::net::datasource_factory", None),
	("raw_datasource_factory", "ambulant::net::raw_datasource_factory", None),
	("audio_datasource_factory", "ambulant::net::audio_datasource_factory", None),
	("video_datasource_factory", "ambulant::net::video_datasource_factory", None),
	("audio_parser_finder", "ambulant::net::audio_parser_finder", None),
	("audio_filter_finder", "ambulant::net::audio_filter_finder", None),
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
		bname = 'baseclass = None'
	print >>out, FORMAT % locals()
	print >>modout, INCLFORMAT % locals()
	
