/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __SDL_PLAYER_H__
#define __SDL_PLAYER_H__

/*
 * sdl_gui_player.h -- sdl interface to ambulant
 *		       functions to drive the common SMIL document player
 */

#include <iostream>
#include <string>
#include <map>
#include <stack>

#include "ambulant/version.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/common/embedder.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/player.h"
#include "ambulant/gui/none/none_gui.h"
//X #include "ambulant/gui/SDL/sdl_renderer.h"
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_window.h"
#include "ambulant/smil2/smil_player.h"

#include "SDL.h"

#include "sdl_gui.h"

using namespace ambulant;
using namespace common;
using namespace lib;
using namespace gui;
using namespace sdl;

void open_web_browser(const std::string &href);

class sdl_player_callback_arg {
};
class sdl_gui;

class sdl_gui_player :
	public common::gui_player,
	public ambulant::common::embedder
{
  public:
	sdl_gui_player(sdl_gui* parent);
	~sdl_gui_player();

	void init_window_factory();
	void init_playable_factory();
	void init_datasource_factory();
	void init_parser_factory();

	bool is_open() const { return m_player != NULL; }

	void show_file(const ambulant::net::url&);
	void close(common::player *p) {} //TBD
	void open(net::url newdoc, bool start, common::player *old=NULL) {} //TBD
	void redraw();
	void redraw(void* /*ambulant_sdl_window* */, void* /*ambulant::lib::rect* */);
	SDL_Window* get_window() { return m_gui->get_window(); }
	bool user_event (SDL_Point &p, int what = 0);
	// helper: convert a point in screen space to a point in ambulant space
	void resize_window (int w, int h);
  private:
	ambulant::common::player* create_player(const char* filename);
	bool user_event (const point &p, int what = 0);
	// top level drawing window
	void create_top_window (const char* filename);
	lib::point m_origin;
	lib::size m_size;
	lib::rect m_rect;
	// gui, logger and drawing surface
	sdl_gui *m_gui;
	lib::logger *m_logger;
	bool m_running;
	ambulant::gui::sdl::sdl_ambulant_window *m_sdl_ambulant_window;
	ambulant::gui::sdl::ambulant_sdl_window *m_ambulant_sdl_window;
};
#endif/*__SDL_PLAYER_H__*/
