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
 
#include "ambulant/gui/dg/dg_audio_player.h"
#include "ambulant/gui/dg/dg_mp3_decoder.h"
#include "ambulant/gui/dg/dg_audio_renderer.h"
#include "ambulant/lib/win32/win32_fstream.h"
#include "ambulant/lib/logger.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dg::audio_player::audio_player(const std::string& url) 
:	m_bbuf(read_size),
	m_decbuf(0), 
	m_renderer(0) {
	memset(&m_wfx, 0, sizeof(WAVEFORMATEX));
	if(!m_ifs.open(url)) {
		lib::logger::get_logger()->error("Failed to open: %s", url);
	} 
	m_ifs.read(m_bbuf);
	m_bbuf.flip();
	m_decoder.get_wave_format(m_bbuf, m_wfx);
	m_renderer = new gui::dg::audio_renderer();
	m_renderer->open(m_wfx);
	render();
}

gui::dg::audio_player::~audio_player() {
	stop();
	if(m_ifs.is_open()) m_ifs.close();
	if(m_decbuf != 0) delete m_decbuf;
}

bool gui::dg::audio_player::can_play() const {
	return m_ifs.is_open() && m_renderer;
}
	
void gui::dg::audio_player::start(double t) {
	if(m_renderer) m_renderer->start();
}

void gui::dg::audio_player::pause() {
	if(m_renderer) m_renderer->pause();
}
	
void gui::dg::audio_player::resume() {
	if(m_renderer) m_renderer->resume();
}
	
void gui::dg::audio_player::stop() {
	gui::dg::audio_renderer *dummy = m_renderer;
	m_renderer = 0;
	if(dummy) {
		// audio_renderer::stop() will delete after 
		// a while the audio_renderer object
		dummy->stop(); 
		
		// Due to the audio_renderer::stop() arrangement the following code is not necessary 
		//if(WaitForSingleObject(dummy->get_done_event(), 3000) != WAIT_OBJECT_0) {
		//	lib::logger::get_logger()->warn("Wait for audio renderer done event failed");
		//}
	}
}
	
std::pair<bool, double> 
gui::dg::audio_player::get_dur() {
	return std::pair<bool, double>(false, 0.0);
}

bool gui::dg::audio_player::is_playing() {
	return m_renderer != 0 && 
		WaitForSingleObject(m_renderer->get_done_event(), 0) != WAIT_OBJECT_0;
}
	
void gui::dg::audio_player::render() {
	if(!m_renderer) return;
	if(m_renderer->get_audio_data_size() > lo_limit) return;
	while(m_ifs.is_open() && 
		m_renderer->get_audio_data_size()<hi_limit) {
		// Decode
		if(m_decbuf == 0) {
			m_decbuf = new std::basic_string<char>();
			m_decbuf->reserve(dec_size_estim);
		}
		m_decoder.decode(m_bbuf, m_decbuf);
		m_bbuf.compact();
				
		// Render buffer
		if(m_decbuf->size() > 0) {
			m_renderer->write(m_decbuf);
			m_decbuf = 0;
		}
		
		// fill read buffer
		int ppos = m_bbuf.get_position();
		m_ifs.read(m_bbuf);
		if(ppos == m_bbuf.get_position())
			m_ifs.close();
		m_bbuf.flip();
	}
}

void gui::dg::audio_player::resample() {
	bool resample = false;
	if(m_wfx.nSamplesPerSec == 48000) {
		m_wfx.nSamplesPerSec = 44100;
		resample = true;
	} else if(m_wfx.nSamplesPerSec != 22050) {
		m_wfx.nSamplesPerSec = 22050;
		resample = true;
	}
	if(resample) {
		m_decoder.reset();
		m_decoder.get_wave_format(m_bbuf, m_wfx);
	}
}


