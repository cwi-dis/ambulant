#include "qt_mainloop.h"
using namespace ambulant;
using namespace lib;
using namespace gui;
using namespace qt_renderer;
 
void*
qt_mainloop::run(void* view)
{
    qt_gui* qt_view = (qt_gui*) view;
    passive_player *p = new passive_player(qt_view->filename());
    if (!p) return NULL;
 
    qt_window_factory *wf = new qt_window_factory(qt_view);
    qt_renderer_factory* qf = new qt_renderer_factory();
		     
    active_player *a = p->activate
      ((window_factory *) wf, (renderer_factory *) qf);
    if (!a) return NULL;

    event_processor *processor
      = event_processor_factory(realtime_timer_factory());
    
    typedef callback<qt_mainloop,qt_mainloop_callback_arg> callback;
    event *ev = new callback(NULL,  //this
			     &qt_mainloop::player_done_callback, 
			     new qt_mainloop_callback_arg());
    
    a->start(processor, ev);
 //   while (!done())
    while(true)
      sleep(1);
    return (void*) 1;
}
