/* 
 * @$Id$ 
 */


// Environment for testing design classes

#include <iostream>
#include "mainloop.h"
#include "ambulant/lib/timer.h"
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

	m_active_player = p->activate(
		wf,
		(lib::renderer_factory *)new ambulant::gui::cocoa::cocoa_renderer_factory());
	if (!m_active_player) return;
	
	lib::event_processor *processor = lib::event_processor_factory(lib::realtime_timer_factory());

	typedef lib::no_arg_callback<mainloop> callback;
	lib::event *ev = new callback(this, &mainloop::player_done_callback);

	m_running = true;
	m_active_player->start(processor, ev);

	while (m_running)
		sleep(1);
//	delete m_active_player;
	m_active_player = NULL;
}

void
mainloop::set_speed(double speed)
{
	m_speed = speed;
	if (m_active_player)
		m_active_player->set_speed(speed);
}
