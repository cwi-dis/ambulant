/* 
 * @$Id$ 
 */


// Environment for testing design classes

#include <iostream>
#include "mainloop.h"

void
usage()
{
	std::cerr << "Usage: demoplayer file" << std::endl;
	std::cerr << "Options: --version (-v) prints version info" << std::endl;
	exit(1);
}

void
mainloop::run(char *filename)
{
	using namespace ambulant;
	
	lib::passive_player *p = new lib::passive_player(filename);
	if (!p) return;

	lib::active_player *a = p->activate();
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
