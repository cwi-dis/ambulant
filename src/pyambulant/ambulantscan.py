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
            
        ]

    def makeblacklisttypes(self):
        return [
            "q_attributes_list",
            "q_attributes_list_ref",
            "const_q_name_pair",
            "const_q_name_pair_ref",
            "q_name_pair",
            "flag_event",
            "timer",
            "node_list", # XXX For now
            "xml_string", # XXX For now
            "const_xml_string_ref", # XXX For now
            "const_custom_test_map_ptr", # XXX For now 
            "const_q_attributes_list_ref",  # XXX For now
            "const_lib_screen_rect_int_ref", # XXX For now
            "const_lib_point_ref", # XXX For now
            "common_factories_ptr", # XXX For now
            "const_ambulant_net_url_ref", # XXX for now
            "const_audio_format_ref", # XXX for now
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
            "audio_format_choices", # XXX For now
            "audio_format_ref", # XXX For now
            "region_dim", # XXX For now
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
##            ('set_attribute',
##              [
##                ('char_ptr', '*', '*'),
##                ('char_ptr', '*', '*')
##              ],[
##                ('stringptr', '*', '*'),
##                ('stringptr', '*', '*')
##              ]
##            ),
##            ('locate_node',
##              [
##                ('char_ptr', '*', '*')
##              ],[
##                ('stringptr', '*', '*')
##              ]
##            ),
##            ('get_url',
##              [
##                ('char_ptr', '*', '*')
##              ],[
##                ('stringptr', '*', '*')
##              ]
##            ),
            (
              [
                ('char_ptr', '*', 'InMode+ConstMode'),
                ('size_t', '*', 'InMode')
              ],[
                ('InBuffer', '*', 'InMode'),
               ]
            ),
            (
              [
                ('char_ptr', '*', 'InMode+ConstMode'),
              ],[
                ('stringptr', '*', '*'),
               ]
            ),
            (
              [
                ('const_char_ptr', '*', 'ReturnMode'),
              ],[
                ('return_stringptr', '*', '*'),
               ]
            ),
            (
              [
                ('char_ptr_ptr', 'result', 'InMode'),
              ],[
                ('output_stringptr', '*', 'OutMode'),
               ]
            ),
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
