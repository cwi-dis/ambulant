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
        AMBULANT+ "lib/transition_info.h",
        AMBULANT+ "common/embedder.h",
        AMBULANT+ "common/layout.h",
        AMBULANT+ "common/playable.h",
        AMBULANT+ "common/player.h",
        AMBULANT+ "common/region_info.h",
        AMBULANT+ "gui/none/none_gui.h",
        AMBULANT+ "net/datasource.h",
        AMBULANT+ "net/posix_datasource.h",
        AMBULANT+ "net/stdio_datasource.h",
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
    silent = 1

    def makeblacklistnames(self):
        return [
            # node.h
#            "set_attributes",       # string list, too difficult
            "get_read_ptr", # Does not translate to Python
            "get_frame", # Doable, with size in the arglist
#            "wakeup", # XXX private/protected
#            "wait_event", # XXX private/protected
            "flag_event", # Constructor for unsupported type
            "sax_error", # Constructor for unsupported type
#            "audio_format_choices", # Constructor for unsupported type
            "none_playable", # XXX Constructor for unsupported type
            "none_background_renderer", # XXX Constructor for unsupported type
            "none_playable_factory",  # XXX Constructor for unsupported type
            "event_processor_impl", # Constructor for unsupported type
           
        ]

    def makeblacklisttypes(self):
        return [
            "event_processor_impl",  # Concrete version of event_processor, ignore.
            "audio_datasource", #XXX
            "video_datasource", #XXX
            "abstract_demux", #XXX
            "demux_datasink", #XXX
            "demux_datasink_ptr", #XXX
            "posix_datasource", # We want only the factory and (abstract) interface
            "posix_datasource_ptr", # Ditto
            "posix_datasource_factory", # Ditto
            "stdio_datasource", # We want only the factory and (abstract) interface
            "stdio_datasource_ptr", # Ditto
            "stdio_datasource_factory", # Ditto
            "Where_we_get_our", # Parser trips over a comment:-)
            "q_attributes_list",    # We don't do lists, for now
            "q_attributes_list_ref",    # We don't do lists, for now
            "const_q_attributes_list_ref",    # We don't do lists, for now
            "flag_event",  # Holds a reference to a bool, not useful for Python
            "flag_event_ptr",  # Holds a reference to a bool, not useful for Python
            "const_custom_test_map_ptr", # We don't do maps for now
            "node_list",     # We don't do lists, for now
            "delta_timer", # XXX for now
            "iterator", # Not needed in Python
            "const_iterator", # Not needed in Python
            "char_ptr_ptr",
            "sax_content_handler",
            "sax_content_handler_ptr",
            "sax_error_handler_ptr",
            "sax_error",
            "sax_error_ptr",
            "none_playable",
            "none_playable_ptr",
            "none_background_renderer",
            "none_background_renderer_ptr",
            "tile_positions",    # We don't do lists, for now
            
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
                'get_bgimage',
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
            
            # Assume a const uint8_t * is an input string
            (
              [
                ('uint8_t_ptr', '*', 'InMode+ConstMode'),
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
