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

#ifndef AMBULANT_GUI_SDL_FACTORY_H
#define AMBULANT_GUI_SDL_FACTORY_H

#include "ambulant/common/factory.h"
#include "ambulant/common/gui_player.h"
//X #include "ambulant/common/playable.h"
//X #include "ambulant/gui/SDL/sdl_window.h"

namespace ambulant {
namespace gui {
namespace sdl {

/// SDL implementation of window_factory
class sdl_window_factory : public common::window_factory {
  public:
//X	sdl_window_factory(sdl_ambulant_window* sdl_window, common::gui_player* gpl);
	sdl_window_factory(void* sdl_window, common::gui_player* gpl);
	~sdl_window_factory();
	common::gui_window* new_window(const std::string &name, lib::size bounds, common::gui_events *region);
	common::bgrenderer *new_background_renderer(const common::region_info *src);


  private:
	void* m_parent_window;
	lib::point m_p;
	common::gui_player* m_gui_player;
//X	GdkCursor* m_arrow_cursor;
//X	GdkCursor* m_hand1_cursor;
//X	GdkCursor* m_hand2_cursor;
};  // class sdl_window_factory

AMBULANTAPI common::playable_factory *create_sdl_playable_factory(common::factories *factory);

// XXXX Needs to be implemented:
// Create sdl_ambulant_window inside sdl_parent_window, call create_sdl_window_factory.
AMBULANTAPI common::window_factory *create_sdl_window_factory(void* sdl_parent_window, common::gui_player* gpl);
//X AMBULANTAPI common::window_factory *create_sdl_window_factory(sdl_ambulant_window* sdl_window, common::gui_player* gpl);
#ifdef WITH_SDL_IMAGE
// Playable factories
AMBULANTAPI common::playable_factory *create_sdl_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
AMBULANTAPI common::playable_factory *create_sdl_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
AMBULANTAPI common::playable_factory *create_sdl_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
AMBULANTAPI common::playable_factory *create_sdl_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
AMBULANTAPI common::playable_factory *create_sdl_fill_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
#endif//WITH_SDL_IMAGE

} // end namespace sdl
} // end namespace gui
} // end namespace ambulant


#endif // AMBULANT_GUI_SDL_FACTORY_H
