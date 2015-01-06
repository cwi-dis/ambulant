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
/*
 * The contents of this file are subject to the Public
 * License XXX.
 *
 */
/* from: renderer.h,v 1.1 2003/09/15 10:36:17 jack Exp */

#ifndef AMBULANT_GUI_SDL_TEXT_RENDERER_H
#define AMBULANT_GUI_SDL_TEXT_RENDERER_H

#ifdef  WITH_SDL_IMAGE

#include "ambulant/common/factory.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/region_info.h"
#include "ambulant/gui/none/none_gui.h"

#include "ambulant/gui/SDL/sdl_renderer.h"

// We prefer building with SDL_Pango, otherwise SDL_ttf is used.
#if defined(WITH_SDL_PANGO)
#include <pango-1.0/pango/pango.h>
#define __PANGO_H__ // this reveals some useful functions in SDL_Pango we want
#include <SDL_Pango.h>
#elif defined(WITH_SDL_TTF)
#include "SDL_ttf.h"
#else
#warning "No sdl_text_renderer available due to missing third party packages"
#endif 

namespace ambulant {

namespace gui {

namespace sdl {

class sdl_text_renderer : public sdl_renderer<renderer_playable_dsall> {
  public:
	sdl_text_renderer(common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp);
	~sdl_text_renderer();

	void redraw_body(const lib::rect &r,
		common::gui_window* w);

  private:
	char* m_text_storage;
	lib::color_t m_text_color;
	float m_text_size;
	const char* m_text_font;
	lib::critical_section m_lock;

#if defined(WITH_SDL_TTF)
	TTF_Font* m_ttf_font;       // font to be used for ttf rendering
	int m_ttf_style;            // style to be used for ttf rendering
#endif//defined(WITH_SDL_TTF)
	SDL_Surface* m_sdl_surface; // surface that was rendered from the text 
};

} // namespace sdl

} // namespace gui

} // namespace ambulant

#endif // WITH_SDL_IMAGE

#endif/*AMBULANT_GUI_SDL_TEXT_RENDERER_H*/
