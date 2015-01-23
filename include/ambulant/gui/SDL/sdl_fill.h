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

/*
 * $Id$
 */

#ifndef AMBULANT_GUI_SDL_SDL_FILL_H
#define AMBULANT_GUI_SDL_SDL_FILL_H

#ifdef  WITH_SDL_IMAGE

#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/region_info.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/smil2/smiltext.h"
#include "ambulant/smil2/transition.h"
#include "sdl_renderer.h"
#include "sdl_window.h"
#include "SDL.h"

namespace ambulant {
  using namespace common;
  using namespace lib;

namespace gui {

namespace sdl {

common::playable_factory *
create_sdl_fill_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);

  class sdl_fill_renderer : public sdl_renderer<renderer_playable> {
  public:
	sdl_fill_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
	  :	sdl_renderer<renderer_playable>(context, cookie, node, evp, factory, mdp), 
	m_surface(NULL),
	m_W(0),
	m_H(0)
	{};
	~sdl_fill_renderer();

	void start(double where);
	void seek(double t) {} // ignore seek()

	void redraw_body(const lib::rect &dirty, common::gui_window *window);
  private:
	SDL_Surface* m_surface;
	int m_W, m_H;
	critical_section m_lock;
};

class sdl_background_renderer : public common::background_renderer {
  public:
	sdl_background_renderer(const common::region_info *src)
	  :	common::background_renderer(src)
#ifndef WITH_SDL_TEXTURE
      , m_bgcolor(lib::to_color("black")),
	m_sdl_color(SDL_Color_from_ambulant_color(m_bgcolor)),
	m_map(0)
#endif//! WITH_SDL_TEXTURE
	{};
	~sdl_background_renderer();

	void redraw(const lib::rect &dirty, common::gui_window *window);
	void highlight(gui_window *window);
	void keep_as_background();
#ifndef WITH_SDL_TEXTURE
  private:
	lib::color_t m_bgcolor;
	SDL_Color m_sdl_color;
	Uint32 m_map;
#endif//! WITH_SDL_TEXTURE
};

} // namespace sdl

} // namespace gui

} // namespace ambulant

#endif // WITH_SDL_IMAGE

#endif  /*AMBULANT_GUI_SDL_SDL_FILL_H*/
