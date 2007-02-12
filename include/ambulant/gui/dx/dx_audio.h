/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_DX_AUDIO_H
#define AMBULANT_GUI_DX_AUDIO_H

#include "ambulant/config/config.h"
#include "ambulant/lib/event.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/smil2/transition.h"

namespace ambulant {

namespace gui {

namespace dx {

class audio_player;

class dx_audio_renderer : public common::renderer_playable {
  public:
	dx_audio_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp);
	~dx_audio_renderer();
	void start(double t);
	void stop();
	void seek(double t);
	void pause(common::pause_display d=common::display_show);
	void resume();
	void redraw(const lib::rect &dirty, common::gui_window *window);
	common::duration get_dur();
	void set_intransition(const lib::transition_info* info);
	void start_outtransition(const lib::transition_info* info);
  private:
    void update_levels();
	void update_callback();
	void schedule_update();
	audio_player *m_player;
 	lib::event *m_update_event;
	double m_level;
	int m_balance;
	const lib::transition_info* m_intransition;
	const lib::transition_info* m_outtransition;
	smil2::audio_transition_engine* m_transition_engine;
 	lib::critical_section m_cs;
};

/// Change the overall soundlevel
AMBULANTAPI double change_global_level(double factor);
} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_AUDIO_H
