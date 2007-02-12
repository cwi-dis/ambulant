// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* 
 * @$Id$ 
 */

#include "ambulant/gui/dg/dg_audio_renderer.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/logger.h"

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dg::audio_renderer::audio_renderer()
:	m_hWaveOut(NULL) {
}
	
gui::dg::audio_renderer::~audio_renderer() {
	if(m_hWaveOut) stop();
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

bool gui::dg::audio_renderer::open(int nSamplesPerSec, int nChannels, int depth) {
	int wBitsPerSample = depth; 
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
	// waveOutReset() blocks with callbacks 
	//MMRESULT mmres = waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &wfx, 
	//	reinterpret_cast<DWORD_PTR>(audio_renderer::callback), 
	//	reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
	if(m_hWaveOut) stop();
	MMRESULT mmres = waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &wfx, 
		0, 0, CALLBACK_NULL);
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
	if(m_hWaveOut == NULL) return;
	MMRESULT mmres = waveOutClose(m_hWaveOut);
	if(mmres != MMSYSERR_NOERROR)
		seterror("waveOutClose()", mmres);
	m_hWaveOut = NULL;
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
	MMRESULT mmres = waveOutReset(m_hWaveOut);
	if(mmres != MMSYSERR_NOERROR)
		seterror("waveOutReset()", mmres);
	update();
	if(!m_audio_data.empty())
		lib::logger::get_logger()->warn("NOT DONE BUFFERS: %u", m_audio_data.size());
	clear_data();
	close();
}

// static 
void gui::dg::audio_renderer::seterror(const char *funcname, MMRESULT mmres) {
	if(mmres == MMSYSERR_NOERROR) return;
	switch(mmres) {
		case MMSYSERR_INVALHANDLE:
			seterror(funcname, "MMSYSERR_INVALHANDLE, Specified device handle is invalid.");
			break;
		case MMSYSERR_BADDEVICEID:
			seterror(funcname, "MMSYSERR_BADDEVICEID, Specified device identifier is out of range.");
			break;
		case MMSYSERR_NODRIVER:
			seterror(funcname, "MMSYSERR_NODRIVER, No device driver is present");
			break;
		case MMSYSERR_NOMEM:
			seterror(funcname, "MMSYSERR_NOMEM, Unable to allocate or lock memory.");
			break;
		case WAVERR_BADFORMAT:
			seterror(funcname, "WAVERR_BADFORMAT, Attempted to open with an unsupported waveform-audio format.");
			break;
		case WAVERR_SYNC:
			seterror(funcname, "WAVERR_SYNC, Device is synchronous but waveOutOpen was called without using the WAVE_ALLOWSYNC flag");
			break;
		case WAVERR_STILLPLAYING:
			seterror(funcname, "WAVERR_STILLPLAYING, There are still buffers in the queue");
			break;
		default:
			lib::logger::get_logger()->error("%s failed, Error: %u", funcname, mmres);
			break;
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

void gui::dg::audio_renderer::update() {
	while(!m_audio_data.empty()) {
		WAVEHDR& waveHdr = m_audio_data.front();
		if((waveHdr.dwFlags & WHDR_DONE) == WHDR_DONE) {
			if(m_hWaveOut) {
				MMRESULT mmres = waveOutUnprepareHeader(m_hWaveOut, &waveHdr, sizeof(WAVEHDR));
				if(mmres != MMSYSERR_NOERROR)
					seterror("waveOutUnprepareHeader()");
			}
			audio_buffer *p =  reinterpret_cast<audio_buffer*>(waveHdr.dwUser);
			delete p;
			m_audio_data.pop_front();
		} else break;
	}
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
	while(has_audio_data()) {
		WAVEHDR& waveHdr = m_audio_data.front();
		audio_buffer *p =  reinterpret_cast<audio_buffer*>(waveHdr.dwUser);
		delete p;
		m_audio_data.pop_front();
	}
}

