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

#include "ambulant/gui/dg/dg_audio_renderer.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/logger.h"


#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dg::audio_renderer::audio_renderer()
:	m_hWaveOut(NULL),
	m_hDoneEvent(CreateEvent(NULL, TRUE, FALSE, NULL)) {
	if(!m_hDoneEvent) 
		lib::win32::win_report_last_error("CreateEvent");
}
	
gui::dg::audio_renderer::~audio_renderer() {
	if(m_hDoneEvent) 
		CloseHandle(m_hDoneEvent);
}

//static 
bool gui::dg::audio_renderer::can_play(WAVEFORMATEX& wfx) {
	DWORD flags = WAVE_FORMAT_QUERY;
	MMRESULT mmres = waveOutOpen(NULL, WAVE_MAPPER, &wfx, 0, 0, flags);
	if(mmres != MMSYSERR_NOERROR) {
		seterror("waveOutOpen()", mmres);
		return false;
	}
	return true;
}

bool gui::dg::audio_renderer::open(int nSamplesPerSec, int nChannels) {
	int wBitsPerSample = 16; 
	int nBlockAlign = nChannels*wBitsPerSample/8; 
	long nAvgBytesPerSec = nBlockAlign*nSamplesPerSec;
	WAVEFORMATEX wfx = {WAVE_FORMAT_PCM, // format type
		WORD(nChannels), // number of channels
		DWORD(nSamplesPerSec), // sample rate
		DWORD(nAvgBytesPerSec), // for buffer estimation
		WORD(nBlockAlign), // block size of data
		WORD(wBitsPerSample), //number of bits per sample of mono data
		WORD(0) // the count in bytes of the size of the extra info after this field
		};
	return open(wfx);
}

bool gui::dg::audio_renderer::open(WAVEFORMATEX& wfx) {
	MMRESULT mmres = waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &wfx, 
		reinterpret_cast<DWORD_PTR>(audio_renderer::callback), 
		reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
	if(mmres != MMSYSERR_NOERROR) {
		seterror("waveOutOpen()", mmres);
		return false;
	}
	mmres = waveOutPause(m_hWaveOut);
	if(mmres != MMSYSERR_NOERROR) {
		seterror("waveOutPause()", mmres);
		return false;
	}
	return true;
}

void gui::dg::audio_renderer::close() {
	stop();
}

void gui::dg::audio_renderer::start() {
	resume();
}
	
void gui::dg::audio_renderer::pause() { 
	if(m_hWaveOut == NULL) return;
	MMRESULT mmres = waveOutPause(m_hWaveOut);
	if(mmres != MMSYSERR_NOERROR)
		seterror("waveOutPause()", mmres);		
}

void gui::dg::audio_renderer::resume() { 
	if(m_hWaveOut == NULL) return;
	MMRESULT mmres = waveOutRestart(m_hWaveOut);
	if(mmres != MMSYSERR_NOERROR)
		seterror("waveOutRestart()", mmres);
}

void gui::dg::audio_renderer::stop() {
	if(m_hWaveOut == NULL) return;
	waveOutPause(m_hWaveOut);
	waveOutClose(m_hWaveOut);
}

// static 
void gui::dg::audio_renderer::seterror(const char *funcname, MMRESULT mmres) {
	if(mmres != MMSYSERR_NOERROR) {
		if(mmres == MMSYSERR_INVALHANDLE)
			seterror(funcname, "MMSYSERR_INVALHANDLE, Specified device handle is invalid.");
		else if(mmres == MMSYSERR_BADDEVICEID)
			seterror(funcname, "MMSYSERR_BADDEVICEID, Specified device identifier is out of range.");
		else if(mmres == MMSYSERR_NODRIVER)
			seterror(funcname, "MMSYSERR_NODRIVER, No device driver is present");
		else if(mmres == MMSYSERR_NOMEM)
			seterror(funcname, "MMSYSERR_NOMEM, Unable to allocate or lock memory.");
		else if(mmres == WAVERR_BADFORMAT)
			seterror(funcname, "WAVERR_BADFORMAT, Attempted to open with an unsupported waveform-audio format.");
		else if(mmres == WAVERR_SYNC)
			seterror(funcname, "WAVERR_SYNC, Device is synchronous but waveOutOpen was called without using the WAVE_ALLOWSYNC flag");
	}
}

// static 
void gui::dg::audio_renderer::seterror(const char *funcname, const char *msg) {
	lib::logger::get_logger()->error("%s failed, %s", funcname, msg);
}

// static 
void gui::dg::audio_renderer::seterror(const char *funcname) {
	lib::logger::get_logger()->error("%s failed", funcname);
}

// Destructs done audio chunks
// Deletes the audio_renderer instance on WOM_CLOSE
// Therefore we don't need to wait until a WOM_CLOSE to delete the audio_renderer instance.
//
// static 
void __stdcall gui::dg::audio_renderer::callback(HWAVEOUT hwo, UINT uMsg, 
	DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam2) {
	audio_renderer *wout = 
		reinterpret_cast<audio_renderer*>(dwInstance);
	if(uMsg == WOM_OPEN) {
		AM_DBG lib::logger::get_logger()->trace("WOM_OPEN");
	} else if(uMsg == WOM_DONE) {
		AM_DBG lib::logger::get_logger()->trace("WOM_DONE");
		wout->unprepare_front();
		if(!wout->has_audio_data())
			SetEvent(wout->m_hDoneEvent);
	} else if(uMsg == WOM_CLOSE) {
		AM_DBG lib::logger::get_logger()->trace("WOM_CLOSE");
		wout->clear_data();
		SetEvent(wout->m_hDoneEvent);
		wout->m_hWaveOut = 0;
		delete wout;
	}
}

bool gui::dg::audio_renderer::write(audio_buffer* p) {
	m_audio_data.push_back(WAVEHDR());
	WAVEHDR& waveHdr = m_audio_data.back();
	memset(&waveHdr, 0, sizeof(WAVEHDR));
	waveHdr.lpData = const_cast<char*>(p->data());
	waveHdr.dwBufferLength = DWORD(p->length());
	waveHdr.dwUser = reinterpret_cast<DWORD_PTR>(p);
	MMRESULT mmres = waveOutPrepareHeader(m_hWaveOut, &waveHdr, sizeof(WAVEHDR));
	if(mmres != MMSYSERR_NOERROR)
		seterror("waveOutPrepareHeader()");
	mmres = waveOutWrite(m_hWaveOut, &waveHdr, sizeof(WAVEHDR));
	if(mmres != MMSYSERR_NOERROR)
		seterror("waveOutWrite()");
	return (mmres == MMSYSERR_NOERROR);
}
	
void gui::dg::audio_renderer::unprepare_front() {
	if(!m_audio_data.empty()) {
		WAVEHDR& waveHdr = m_audio_data.front();
		if(m_hWaveOut) {
			MMRESULT mmres = waveOutUnprepareHeader(m_hWaveOut, &waveHdr, sizeof(WAVEHDR));
			if(mmres != MMSYSERR_NOERROR)
				seterror("waveOutUnprepareHeader()");
		}
		audio_buffer *p =  reinterpret_cast<audio_buffer*>(waveHdr.dwUser);
		delete p;
		m_audio_data.pop_front();
	}
}

void gui::dg::audio_renderer::clear_data() {
	while(has_audio_data()) unprepare_front();
}

