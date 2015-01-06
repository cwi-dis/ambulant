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

#ifndef AMBULANT_GUI_GSTREAMER_GSTREAMER_RENDERER_FACTORY_H
#define AMBULANT_GUI_GSTREAMER_GSTREAMER_RENDERER_FACTORY_H

#include "ambulant/common/factory.h"

#define AMBULANT_MAX_CHANNELS 2

// to be called from main thread before any other thread, otherwise unsafe
extern void
gstreamer_player_initialize(int* argcp, char*** argvp);

// to be called from main thread all other thread finished, otherwise unsafe
extern void
gstreamer_player_finalize();

namespace ambulant {
namespace gui {
namespace gstreamer {

class gstreamer_renderer_factory : public common::playable_factory {
  public:

	gstreamer_renderer_factory(common::factories *factory)
	:   m_factory(factory) {}
	~gstreamer_renderer_factory();

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

AMBULANTAPI common::playable_factory *create_gstreamer_renderer_factory(common::factories *factory);

} // end namespace gstreamer
} // end namespace gui
} // end namespace ambulant

#endif // AMBULANT_GUI_GSTREAMER_GSTREAMER_RENDERER_FACTORY_H
