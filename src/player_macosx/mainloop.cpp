/* 
 * @$Id$ 
 */


// Environment for testing design classes

#include <iostream>
#include "mainloop.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"

void
usage()
{
	std::cerr << "Usage: demoplayer file" << std::endl;
	std::cerr << "Options: --version (-v) prints version info" << std::endl;
	exit(1);
}

void
mainloop::run(const char *filename, ambulant::lib::window_factory *wf)
{
	using namespace ambulant;
	
	lib::passive_player *p = new lib::passive_player(filename);
	if (!p) return;

	lib::active_player *a = p->activate(
		wf,
		(lib::renderer_factory *)new ambulant::gui::cocoa::cocoa_renderer_factory());
	if (!a) return;
	
	lib::event_processor *processor = lib::event_processor_factory();

	typedef lib::callback<mainloop,mainloop_callback_arg> callback;
	lib::event *ev = new callback(this, 
		&mainloop::player_done_callback, 
		new mainloop_callback_arg());

	a->start(processor, ev);

	while (!m_done)
		sleep(1);
}
