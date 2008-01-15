/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
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

#ifndef AMBULANT_GUI_DG_AUDIO_RENDERER_H
#define AMBULANT_GUI_DG_AUDIO_RENDERER_H

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifndef _INC_MMSYSTEM
#include <mmsystem.h>
#endif

#ifndef AMBULANT_PLATFORM_WIN32_WCE
#pragma comment (lib,"winmm.lib")
#endif

#include "ambulant/config/config.h"
#include <string>
#include <deque>

namespace ambulant {

namespace gui {

namespace dg {

class audio_renderer {
  public:
	typedef std::basic_string<char> audio_buffer;
	
	audio_renderer();	
	~audio_renderer();
	
	static bool can_play(WAVEFORMATEX& wfx);
	
	bool open(int nSamplesPerSec, int nChannels, int depth);

	bool open(WAVEFORMATEX& wfx);

	void close();

	void start();
	
	void pause();

	void resume();

	void stop();

	void update();
	
	static void seterror(const char *funcname, MMRESULT mmres);
	static void seterror(const char *funcname, const char *msg);
	static void seterror(const char *funcname);

	bool is_open() const { return (m_hWaveOut != NULL);}
	operator HWAVEOUT() { return m_hWaveOut;}

	bool write(audio_buffer* p);
	
	bool has_audio_data() const { return !m_audio_data.empty();}
	size_t get_audio_data_size() const { return m_audio_data.size();}
	
	void set_done_event(HANDLE hDoneEvent);
	void unprepare_front();
	void clear_data();
	
  private:
	HWAVEOUT m_hWaveOut;
	std::deque<WAVEHDR> m_audio_data;
};

} // namespace dg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DG_AUDIO_RENDERER_H
