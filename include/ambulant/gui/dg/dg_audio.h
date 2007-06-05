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
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/renderer_impl.h"

namespace ambulant {

namespace gui {

namespace dg {

class audio_player;

class dg_audio_renderer : public common::renderer_playable {
  public:
	dg_audio_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		lib::event_processor* worker);
	~dg_audio_renderer();
	void start(double t);
	void stop();
	void pause(common::pause_display d=common::display_show);
	void resume();
	void seek(double t);
	void redraw(const lib::rect &dirty, common::gui_window *window);
	void set_intransition(const lib::transition_info *info) {};
	void start_outtransition(const lib::transition_info *info) {};
	std::pair<bool, double> get_dur();
  private:
	void update_callback();
	void schedule_update();
	audio_player *m_player;
 	lib::event *m_update_event;
 	lib::event_processor* m_worker;
 	lib::critical_section m_cs;
};

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DG_AUDIO_H
