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
 * @$Id$
 */

#ifndef AMBULANT_GUI_DX_VIDEO_PLAYER_H
#define AMBULANT_GUI_DX_VIDEO_PLAYER_H

#include "ambulant/config/config.h"

#include <string>

#include <objbase.h>
#include <strmif.h>
#include <control.h>
#include <mmstream.h>
#include <amstream.h>
#include <ddstream.h>

#include "ambulant/lib/gtypes.h"
#include "ambulant/common/playable.h"

#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"amstrmid.lib")
#pragma comment (lib,"strmiids.lib")
#pragma comment (lib,"uuid.lib")

namespace ambulant {
	namespace lib {
		class event_processor;
		class event;
	}
}

namespace ambulant {

namespace gui {

namespace dx {

class video_player {
  public:
	video_player(const std::string& url, IDirectDraw* ddraw);
	~video_player();

	void start(double t);
//	void stop();
	bool stop();
	void post_stop() {}
	void init_with_node(const lib::node *n) {}
	void pause(common::pause_display d=common::display_show);
	void resume();
	void seek(double t);
	common::duration get_dur();
	void wantclicks(bool want) { m_wantclicks = want;}

	void preroll(double when, double where, double how_much) {}
	int ms_per_frame();

	bool can_play();
	bool is_playing();
	double get_position();
	const std::string& get_url() const { return m_url;}

	// dx implementation artifacts
	bool update();
	IDirectDrawSurface *get_ddsurf() { return m_ddsurf;}
	lib::size get_size() const {
		return lib::size(m_rcsurf.right, m_rcsurf.bottom);
	}

  private:
	bool open(const std::string& url, IDirectDraw* dd);
	void release_player();

	std::string m_url;

	IMultiMediaStream *m_mmstream;
	IMediaStream *m_vidstream;
	IDirectDrawMediaStream *m_ddstream;
	IDirectDrawStreamSample *m_ddsample;
	IDirectDrawSurface* m_ddsurf;
	RECT m_rcsurf;
	bool m_wantclicks;
	bool m_update_busy;
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_DX_VIDEO_PLAYER_H

