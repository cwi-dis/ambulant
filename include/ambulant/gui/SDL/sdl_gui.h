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

#ifndef AMBULANT_GUI_SDL_GUI_H
#define AMBULANT_GUI_SDL_GUI_H

#include <iostream>


#include "ambulant/common/factory.h"
#include "ambulant/net/datasource.h"
#include "ambulant/common/playable.h"

namespace ambulant {
namespace gui {
namespace sdl {

class sdl_renderer_factory : public common::playable_factory {
  public:

	sdl_renderer_factory(common::factories *factory)
	:   m_factory(factory) {}
	~sdl_renderer_factory();

	bool supports(common::renderer_select *);

	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);

	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src);

  private:
	common::factories *m_factory;

};



} // end namespace sdl
} // end namespace gui
} // end namespace ambulant


#endif // AMBULANT_GUI_SDL_GUI_H
