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

#ifndef AMBULANT_GUI_D2_BASICVIDEO_H
#define AMBULANT_GUI_D2_BASICVIDEO_H

#include "ambulant/config/config.h"
#include "ambulant/lib/event.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/d2/d2_player.h"
#include "ambulant/lib/mtsync.h"

interface IGraphBuilder;
interface IMediaControl;
interface IMediaPosition;
interface IMediaEvent;
interface IBasicAudio;
interface IVideoWindow;
interface IBaseFilter;
interface IMFVideoDisplayControl;

namespace ambulant {

namespace gui {

namespace d2 {

class basicvideo_player;

common::playable_factory *create_d2_basicvideo_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);

class d2_basicvideo_renderer : public common::renderer_playable {
  public:
	d2_basicvideo_renderer(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::factories *fp,
		common::playable_factory_machdep *d2player);
	~d2_basicvideo_renderer();
	void start(double t);
	//void stop();
	bool stop();
	void pause(common::pause_display d=common::display_show);
	void seek(double t);
	void resume();
	bool user_event(const lib::point& pt, int what);
	void redraw(const lib::rect &dirty, common::gui_window *window);
	common::duration get_dur();
	void set_intransition(const lib::transition_info *info) {};
	void start_outtransition(const lib::transition_info *info) {};

  private:
    bool _open(const std::string& url, HWND parent);
	bool _can_play();
	void _start(double t);
	bool _stop();
	void _pause(common::pause_display d);
	void _resume();
	void _seek(double t);
	bool _is_playing();

	void _update_callback();
	void _schedule_update();
	lib::event *m_update_event;
	
	IGraphBuilder *m_graph_builder;
	IMediaControl *m_media_control;
	IMediaPosition *m_media_position;
	IMediaEvent *m_media_event;
	IBasicAudio *m_basic_audio;
	IVideoWindow *m_video_window;
	IBaseFilter *m_evr;
	IMFVideoDisplayControl *m_evr_control;
	HWND m_evr_hwnd;

	d2_player *m_d2player;
	lib::critical_section m_cs;
};

} // namespace d2

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_D2_BASICVIDEO_H
