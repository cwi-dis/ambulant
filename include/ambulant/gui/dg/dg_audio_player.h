/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
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

#ifndef AMBULANT_GUI_DG_AUDIO_PLAYER_H
#define AMBULANT_GUI_DG_AUDIO_PLAYER_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/lib/win32/win32_fstream.h"
#include "ambulant/gui/dg/dg_mp3_decoder.h"
#include "ambulant/gui/dg/dg_audio_renderer.h"

namespace ambulant {

namespace gui {

namespace dg {

class audio_player {	
  public:
	audio_player(const std::string& url);
	
	~audio_player();
	
	bool can_play() const;
	void start(double t);
	void pause(common::pause_display d=common::display_show);
	void resume();
	void stop();	
	std::pair<bool, double> get_dur();
	bool is_playing();
	void update();	
	
  private:
	void resample();
	
	enum { read_size = 8192, dec_size_estim = 98304};
	enum { lo_limit = 8, hi_limit = 12};
	enum { read_sync_size = 200000};
	
	lib::byte_buffer m_bbuf;
	std::basic_string<char> *m_decbuf;
	lib::win32::fstream m_ifs;
	gui::dg::mp3_decoder m_decoder;
	WAVEFORMATEX m_wfx;  
};


} // namespace dg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DG_AUDIO_PLAYER_H
