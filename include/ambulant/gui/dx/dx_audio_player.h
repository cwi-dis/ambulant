/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
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

#include "ambulant/lib/playable.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/logger.h"

// CLSID_FilterGraph
#include <uuids.h>

#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"amstrmid.lib")

#pragma comment (lib,"uuid.lib")
#pragma comment (lib,"strmiids.lib")

namespace ambulant {

namespace gui {

namespace dx {

using ambulant::lib::win32::win_report_error;
using ambulant::lib::logger;

class audio_player : public lib::playable {
  public:
	audio_player(const std::string& url);
	~audio_player();
		
	void start(double t);
	void stop();
	void pause();
	void resume();
	void seek(double t);
	std::pair<bool, double> get_dur();
	void wantclicks(bool want) {}
	void preroll(double when, double where, double how_much) {}
	const cookie_type& get_cookie() const { return m_cookie;}
	
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
	void release();
	
	std::string m_url;
	cookie_type m_cookie;
	IGraphBuilder *m_graph_builder;
	IMediaControl *m_media_control;
	IMediaPosition *m_media_position;
	IMediaEvent *m_media_event;
	IBasicAudio *m_basic_audio;
};
	
} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_AUDIO_PLAYER_H

