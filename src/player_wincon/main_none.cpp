
/* 
 * @$Id$ 
 */

#include "ambulant/version.h"

#include "ambulant/lib/player.h"
#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/logger.h"

#include "ambulant/gui/none/none_gui.h"

using namespace ambulant;

int
main(int argc, char **argv) {
	std::cout << "Ambulant version " << get_version() << std::endl;
	
	if(argc == 1) {
		std::cerr << "Usage: player_wincon file" << std::endl;
		return 0;
	}
	char *filename = argv[1];
	
	lib::logger *logger = lib::logger::get_logger();
	logger->trace("Play: %s", filename);
	
	// Create passive_player from filename
	lib::passive_player *pplayer = new lib::passive_player(filename);
	if (!pplayer) {
		logger->error("Failed to construct passive_player from file %s", filename);
		return 1;
	}
	
	// Create GUI window_factory and renderer_factory
	lib::window_factory *wf = new gui::none::none_window_factory();
	lib::renderer_factory *rf = new ambulant::gui::none::none_renderer_factory();
	
	// Request an active_player for the provided factories 
	lib::active_player *aplayer = pplayer->activate(wf, rf);
	if (!aplayer) {
		logger->error("passive_player::activate() failed to create active_player");
		return 1;
	}
	
	// Create an event_processor
	lib::event_processor *processor = 
		lib::event_processor_factory(lib::realtime_timer_factory());
	
	// Start event_processor. 
	// Pass a flag_event to be set when done.
	bool done = false;
	logger->trace("Start playing");
	aplayer->start(processor, new lib::flag_event(done));
	while(!done)
		lib::sleep_msec(50);
	logger->trace("Finished playing");
	
	// cleanup
	delete processor;
	delete pplayer;
	aplayer = lib::release(aplayer);
	// verify:
	if(aplayer != 0)
		logger->warn("active_player ref_count: " + aplayer->get_ref_count());
	delete rf;
	delete wf;
	
	logger->trace("Normal exit");
	return 0;
}
