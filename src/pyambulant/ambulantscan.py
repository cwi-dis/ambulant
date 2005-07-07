# Scan an Apple header file, generating a Python file of generator calls.

import sys
import os
import re
from bgenlocations import TOOLBOXDIR, BGENDIR
sys.path.append(BGENDIR)
from bgenCxxSupport import CxxScanner

AMBULANT="../../include/ambulant/"
DO_SCAN=True

def main():
    input = [
        AMBULANT+ "version.h",
        AMBULANT+ "lib/node.h",
        AMBULANT+ "lib/document.h",
        AMBULANT+ "lib/event.h",
        AMBULANT+ "lib/event_processor.h",
        AMBULANT+ "lib/parser_factory.h",
        AMBULANT+ "lib/sax_handler.h",
        AMBULANT+ "lib/system.h",
        AMBULANT+ "lib/timer.h",
        AMBULANT+ "common/embedder.h",
        AMBULANT+ "common/layout.h",
        AMBULANT+ "common/playable.h",
        AMBULANT+ "common/player.h",
        AMBULANT+ "common/region_info.h",
        AMBULANT+ "gui/none/none_gui.h",
        AMBULANT+ "net/datasource.h",
            ]
    if DO_SCAN:
        output = "ambulantgen.py"
        defsoutput = None
        scanner = MyScanner(input, output, defsoutput)
        scanner.scan()
        scanner.gentypetest("ambulanttypetest.py")
        scanner.close()
        print "=== Testing definitions output code ==="
        if defsoutput: execfile(defsoutput, {}, {})
    print "=== Done scanning and generating, now importing the generated code ==="
    exec "import ambulantsupport"
    print "=== Done.  It's up to you to compile it now! ==="

       
class MyScanner(CxxScanner):
    silent = 0

    def makeblacklistnames(self):
        return [
            # node.h
            "find_nodes_with_name",
            "has_graph_data",
            "create_idmap",
            "to_string",
            "to_trimmed_string",
            "set_attributes",       # string list, too difficult
            "get_read_ptr", # Does not translate to Python
            "get_frame", # Doable, with size in the arglist
            "wakeup", # XXX private/protected
            "wait_event", # XXX private/protected
            "flag_event", # Constructor for unsupported type
            "sax_error", # Constructor for unsupported type
            "audio_format_choices", # Constructor for unsupported type
            "abstract_event_processor", # XXX Constructor for unsupported type
            "none_playable", # XXX Constructor for unsupported type
            "none_background_renderer", # XXX Constructor for unsupported type
            "none_playable_factory",  # XXX Constructor for unsupported type
           
        ]

    def makeblacklisttypes(self):
        return [
            "animation_destination", # XXX
            "audio_datasource", #XXX
            "video_datasource", #XXX
            "Where_we_get_our", # Parser trips over a comment:-)
            "q_attributes_list",
            "q_attributes_list_ref",
            "const_q_attributes_list_ref",
            "flag_event",
            "flag_event_ptr",
            "timer",
            "const_custom_test_map_ptr",
            "node_list", # XXX For now
            "delta_timer", # XXX for now
            "std_queue<event_ptr>_ptr",
            "iterator",
            "const_iterator",
            "char_ptr_ptr",
            "transition_info_ptr",
            "lib_transition_info_ptr",
            "sax_content_handler",
            "sax_content_handler_ptr",
            "sax_error_handler_ptr",
            "sax_error",
            "sax_error_ptr",
            "none_playable",
            "none_playable_ptr",
            "none_background_renderer",
            "none_background_renderer_ptr",
            "audio_format_choices", # XXX For now
            "audio_format_ref", # XXX For now
            "audio_format_choices_ptr", # XXX For now
            "const_audio_format_ref", # XXX for now
            "region_dim", # XXX For now
            "alignment_ptr", # XXX for now
            "common_alignment_ptr", # XXX for now
            "tile_positions",
            
        ]

    def makegreylist(self):
        return [
            ('#ifdef USE_SMIL21',
              [
                'get_top_surface',
                'is_tiled',
                'get_tiles',
                'get_soundalign',
                'get_tiling',
                'initialize',
                'get_region_soundalign',
                'set_region_soundalign',
              ]
            ),
        ]

    def makerepairinstructions(self):
        return [
            # Assume a pair (const char *, size_t) is an input buffer
            (
              [
                ('char_ptr', '*', 'InMode+ConstMode'),
                ('size_t', '*', 'InMode')
              ],[
                ('InBuffer', '*', 'InMode'),
               ]
            ),
            
            # Assume a const char * is an input string
            (
              [
                ('char_ptr', '*', 'InMode+ConstMode'),
              ],[
                ('stringptr', '*', '*'),
               ]
            ),
            
            # Handle const char * return values as strings too
            (
              [
                ('const_char_ptr', '*', 'ReturnMode'),
              ],[
                ('return_stringptr', '*', '*'),
               ]
            ),
            
            # And also for char **
            (
              [
                ('char_ptr_ptr', 'result', 'InMode'),
              ],[
                ('output_stringptr', '*', 'OutMode'),
               ]
            ),
            
            # get_fit_rect got one output parameter wrong.
            ('get_fit_rect',
              [
                ('lib_rect_ptr', 'out_src_rect', 'InMode'),
              ], [
                ('lib_rect', 'out_src_rect', 'OutMode'),
              ]
            )

        ]        
if __name__ == "__main__":
    main()
