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

#ifndef AMBULANT_GUI_DX_AUDIO_PLAYER_H
#define AMBULANT_GUI_DX_AUDIO_PLAYER_H

#include "ambulant/config/config.h"

#include <string>
#include <cmath>

#include <objbase.h>
#include <strmif.h>
#include <control.h>

#include "ambulant/common/playable.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/logger.h"

#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"amstrmid.lib")
#pragma comment (lib,"strmiids.lib")
#pragma comment (lib,"uuid.lib")

//#define WITH_TPB_AUDIO_SPEEDUP

#ifdef WITH_TPB_AUDIO_SPEEDUP
// If this option is active during build (which is true when building
// for AmisAmbulant) we try to insert the TPB Audio Speedup/Slowdown
// filter into our filtergraph. This allows for changing audio playback
// speed without altering pitch.
// If the TPB DirectShow filter is not available we do nothing.
#include <set>

DEFINE_GUID(CLSID_TPBVupp10,
	0x66172967, 0x56c5, 0x4b89, 0xaa, 0x92, 0xc9, 0xef, 0xec, 0x56, 0x46, 0x7b);

// {A33E626E-D6C4-4559-A1D6-9F1D95F0D8E2}
DEFINE_GUID(IID_IVuppInterface,
0xa33e626e, 0xd6c4, 0x4559, 0xa1, 0xd6, 0x9f, 0x1d, 0x95, 0xf0, 0xd8, 0xe2);

DECLARE_INTERFACE_(IVuppInterface, IUnknown) {

	//Deklarera metoder:
	STDMETHOD(setCrossFadeSpeed)
		( THIS_
			double speed
		) PURE;

	STDMETHOD(setWindowLength)
		( THIS_
			int length
		) PURE;

	STDMETHOD(setCycleSpeed)
		( THIS_
			short speed
		) PURE;

	STDMETHOD(setSilenceLoudnessThreshold)
		( THIS_
			short threshold
		) PURE;

	STDMETHOD(setSilenceRemainderLength)
		( THIS_
			short length
		) PURE;

	STDMETHOD(setSilenceSpeed)
		( THIS_
			short speed
		) PURE;

	STDMETHOD(getPosition)
		( THIS_
			LONGLONG &position
		) PURE;
};
#endif // WITH_TPB_AUDIO_SPEEDUP

namespace ambulant {

namespace gui {

namespace dx {

using ambulant::lib::win32::win_report_error;
using ambulant::lib::logger;

class audio_player {
  public:
	audio_player(const std::string& url);
	~audio_player();

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
	bool open(const std::string& url);
	void release_player();

	std::string m_url;
	IGraphBuilder *m_graph_builder;
	IMediaControl *m_media_control;
	IMediaPosition *m_media_position;
	IMediaEvent *m_media_event;
	IBasicAudio *m_basic_audio;
#ifdef WITH_TPB_AUDIO_SPEEDUP
  public:
	void set_rate(double rate);
	static void set_global_rate(double rate);
	static double change_global_rate(double adjustment);
  private:
	IVuppInterface *m_audio_speedup;
	void initialize_speedup_filter();
	static void register_player(audio_player *cur);
	static void unregister_player(audio_player *cur);
	static std::set<audio_player *> s_active_players;
	static double s_current_playback_rate;
#endif
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_DX_AUDIO_PLAYER_H

