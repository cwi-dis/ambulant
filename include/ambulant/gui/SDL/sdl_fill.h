/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

#ifdef  WITH_SDL22XX // TBD

#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/smil2/smiltext.h"
//#include "sdl_renderer.h"
#include "SDL.h"

namespace ambulant {
  using namespace common;
  using namespace lib;

namespace gui {

namespace sdl {

class sdl_fill_renderer : public  renderer_playable {
  public:
	sdl_fill_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
	:	renderer_playable(context, cookie, node, evp, factory, mdp),
		m_is_showing(false),
		m_intransition(NULL),
		m_outtransition(NULL),
		m_trans_engine(NULL) {};

	~sdl_fill_renderer();

	//	void freeze() {}
	void start(double where);
	bool stop();
	void seek(double t) {}

	void set_intransition(lib::transition_info *info) { m_intransition = info; }
	void start_outtransition(lib::transition_info *info);
	bool user_event(const point &where, int what = 0);
	void redraw(const rect &dirty, gui_window *window);
	void redraw_body(const lib::rect &dirty, common::gui_window *window);
	void set_intransition(const lib::transition_info *info) {};
	void start_outtransition(const lib::transition_info *info) {};
  private:
	void transition_step();

	bool m_is_showing;
	lib::transition_info *m_intransition, *m_outtransition;
	// TBD smil2::transition_engine *m_trans_engine;
	void *m_trans_engine;
	critical_section m_lock;
};

class sdl_background_renderer : public common::background_renderer {
  public:
	sdl_background_renderer(const common::region_info *src)
	:	common::background_renderer(src),
		m_background_pixmap(NULL) {}

	void redraw(const lib::rect &dirty, common::gui_window *windo);
	void highlight(gui_window *window);
	void keep_as_background();
  private:
//X	GdkPixmap *m_background_pixmap;
	void* m_background_pixmap;
};

} // namespace sdl

} // namespace gui

} // namespace ambulant

#endif // WITH_SDL2

#endif  /*AMBULANT_GUI_SDL_SDL_FILL_H*/
