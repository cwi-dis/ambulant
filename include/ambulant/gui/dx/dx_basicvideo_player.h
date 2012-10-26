/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
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

#ifndef AMBULANT_GUI_DX_BASICVIDEO_PLAYER_H
#define AMBULANT_GUI_DX_BASICVIDEO_PLAYER_H

#include "ambulant/config/config.h"

#include <string>
#include <cmath>

#include <objbase.h>
#include <strmif.h>
#include <control.h>

#include "ambulant/common/playable.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/gtypes.h"

#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"amstrmid.lib")
#pragma comment (lib,"strmiids.lib")
#pragma comment (lib,"uuid.lib")


namespace ambulant {

namespace gui {

namespace dx {

using ambulant::lib::win32::win_report_error;
using ambulant::lib::logger;

class basicvideo_player {
  public:
	basicvideo_player(const std::string& url, HWND parent);
	~basicvideo_player();

	void start(double t);
//	void stop();
	bool stop();
	void post_stop() {}
	void init_with_node(const lib::node *n) {}
	void pause(common::pause_display d=common::display_show);
	void resume();
	void seek(double t);
	void endseek(double t);
	common::duration get_dur();
	void wantclicks(bool want) {}
	void preroll(double when, double where, double how_much) {}
	void setrect(const lib::rect& rect);

	bool can_play();
	bool is_playing();
	double get_position();
	const std::string& get_url() const { return m_url;}

	// dx implementation artifacts
	int get_progress();
	void set_progress(int p);

	// -val is the attenuation in decibels
	// can be 0 to 100
	void set_volume(long val);

	// can be -100 to 100
	// 0 sets a neutral balance
	// and 10 sets -10 db to right and -90 db to left
	void set_balance(long val);

  private:
	bool open(const std::string& url, HWND parent);
	void release_player();

	std::string m_url;
	IGraphBuilder *m_graph_builder;
	IMediaControl *m_media_control;
	IMediaPosition *m_media_position;
	IMediaEvent *m_media_event;
	IBasicAudio *m_basic_audio;
	IVideoWindow *m_video_window;
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_DX_BASICVIDEO_PLAYER_H

