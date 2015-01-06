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

#ifndef __GSTREAMER_AUDIO_RENDERER_H
#define __GSTREAMER_AUDIO_RENDERER_H

#include "gstreamer_player.h"
#include <iostream>

#include "ambulant/common/factory.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/lib/logger.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/url.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/smil2/transition.h"
#include "ambulant/lib/transition_info.h"
#include "ambulant/lib/asb.h"



#define AMBULANT_MAX_CHANNELS 2

namespace ambulant {
namespace gui {
namespace gstreamer {

class gstreamer_audio_renderer : public common::renderer_playable {

  public:
	gstreamer_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp);

	gstreamer_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory,
	net::audio_datasource *ds);

	~gstreamer_audio_renderer();

	static bool is_supported(const lib::node *node);

	common::duration get_dur();
	void start(double where);
	bool stop();
//	void post_stop();
//	void init_with_node(const lib::node *n);
//	void preroll(double when, double where, double how_much);
	void stopped();
	void seek(double t);
	void pause(common::pause_display d=common::display_show);
	void resume();
//	void freeze() {};
//	void speed_changed() {};

//	void set_surface(common::surface *dest) { abort(); }
//	common::surface *get_surface() { abort(); }
//	void set_alignment(common::alignment *align) { /* Ignore, for now */ }
//	void transition_freeze_end(lib::rect area) {}
	void redraw(const lib::rect &dirty, common::gui_window *window) {}
	void set_intransition(const lib::transition_info* info);
	void start_outtransition(const lib::transition_info* info);
	GstElement* m_pipeline;

  private:
	void _start(double where);
	bool _stop();
	void _stopped();
	void _seek(double t);
	void _pause();
	void _resume();
	void init_player( const lib::node *node);
	gstreamer_player* m_player;
	net::url m_url;
	lib::critical_section m_lock;

	bool m_is_playing;
	bool m_is_paused;
	bool m_read_ptr_called;
	bool m_audio_started;
	int m_volcount;
	float m_volumes[AMBULANT_MAX_CHANNELS];
	const lib::transition_info* m_intransition;
	const lib::transition_info* m_outtransition;
	smil2::audio_transition_engine* m_transition_engine;
	// class methods and attributes:
};

} // end namespace gstreamer
} // end namespace gui
} // end namespace ambulant


#endif // __GSTREAMER_AUDIO_RENDERER_H

