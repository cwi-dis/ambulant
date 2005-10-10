// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

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
