
/* 
 * @$Id$ 
 */

#include "ambulant/version.h"

#include "ambulant/gui/dx/dx_player.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/asb.h"

#include <iostream>

#pragma comment (lib,"libjpeg.lib")
#pragma comment (lib,"libexpat.lib")

using namespace ambulant;

int
main(int argc, char **argv) {
	std::cout << "Ambulant version " << get_version() << std::endl;
	
	if(argc == 1) {
		std::cerr << "Usage: player_wincon file" << std::endl;
		return 0;
	}
	std::string filename =  lib::win32::resolve_path(argv[1]);
	lib::logger *logger = lib::logger::get_logger();
	
	if(FAILED(CoInitialize(NULL))) {
		logger->error("CoInitialize() failed");
		return 1;
	}
	
	gui::dx::dx_player *player = gui::dx::dx_player::create_player(filename);	
	if(!player->start()) {
		logger->error("Player failed to start playback");
		return 1;
	}
	lib::sleep_msec(500);
	while(!player->is_done()) {
		lib::sleep_msec(500);
	}
	logger->trace("Stoped playing");
	delete player;
	
	CoUninitialize();	
	logger->trace("Normal exit");
	return 0;
}
