// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#include "ambulant/gui/dx/dx_video_player.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/win32/win32_error.h"

#include "ambulant/lib/logger.h"

#include <vfwmsgs.h>

using namespace ambulant;

using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_report_last_error;
using ambulant::lib::logger;
const ULONGLONG MILLIS_FACT = 10000;

gui::dx::video_player::video_player(const std::string& url, IDirectDraw* ddraw)
:	m_url(url),
	m_mmstream(0),
	m_vidstream(0),
	m_ddstream(0),
	m_ddsample(0),
	m_ddsurf(0),
	m_wantclicks(false),
	m_update_busy(false)
{
	open(m_url, ddraw);
}

gui::dx::video_player::~video_player() {
	stop();
}

void gui::dx::video_player::start(double t) {
	if(!m_mmstream) return;
	if(is_playing()) pause();
	seek(t);
	resume();
}

bool gui::dx::video_player::stop() {
	if(!m_mmstream) return true;
	HRESULT hr = m_mmstream->SetState(STREAMSTATE_STOP);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::SetState()", hr);
	}
	release_player();
	return false;
}

void gui::dx::video_player::pause(common::pause_display d) {
	if(!m_mmstream) return;
	HRESULT hr = m_mmstream->SetState(STREAMSTATE_STOP);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::SetState()", hr);
	}
}

void gui::dx::video_player::resume() {
	if(!m_mmstream) return;
	HRESULT hr = m_mmstream->SetState(STREAMSTATE_RUN);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::SetState()", hr);
	}
}

void gui::dx::video_player::seek(double t) {
	if(!m_mmstream) return;
	STREAM_TIME st = MILLIS_FACT*STREAM_TIME(t*1000 + 0.5);
	HRESULT hr = m_mmstream->Seek(st);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::Seek()", hr);
	}
}

std::pair<bool, double> gui::dx::video_player::get_dur() {
	if(!m_mmstream) return std::pair<bool, double>(false, 0);
	STREAM_TIME stdur;
	HRESULT hr;
	hr = m_mmstream->GetDuration(&stdur);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::GetDuration()", hr);
		return std::pair<bool, double>(false, 0);
	}
	double dur = 0.001*double(stdur / MILLIS_FACT);
	return std::pair<bool, double>((dur>0), dur);
}

bool gui::dx::video_player::can_play() {
	return m_mmstream && m_vidstream && m_ddstream;
}

bool gui::dx::video_player::is_playing() {
	if(!m_mmstream) return false;
	STREAM_STATE state;
	HRESULT hr = m_mmstream->GetState(&state);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::GetState()", hr);
		return false;
	}
	if(state != STREAMSTATE_RUN)
		return false;

	STREAM_TIME stdur;
	hr = m_mmstream->GetDuration(&stdur);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::GetDuration()", hr);
		return false;
	}

	if (hr == S_FALSE) return true;
	STREAM_TIME st;
	hr = m_mmstream->GetTime(&st);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::GetTime()", hr);
		return false;
	}
	return st<stdur;
}

double gui::dx::video_player::get_position() {
	if(!m_mmstream) return 0;
	STREAM_TIME st;
	HRESULT hr = m_mmstream->GetTime(&st);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::GetTime()", hr);
		return 0.0;
	}
	return  0.001*double(__int64(st / MILLIS_FACT));
}

////////////////

bool gui::dx::video_player::open(const std::string& url, IDirectDraw* dd) {
	m_url = url;
	HRESULT hr = S_OK;

	IAMMultiMediaStream *mmstream = 0;
	hr = CoCreateInstance(CLSID_AMMultiMediaStream, NULL, CLSCTX_INPROC_SERVER,
		IID_IAMMultiMediaStream, (void**)&mmstream);
	if(FAILED(hr)) {
		win_report_error("CoCreateInstance(CLSID_AMMultiMediaStream, ...)", hr);
		return false;
	}
	IGraphBuilder *graph_builder = 0;
	hr = mmstream->Initialize(STREAMTYPE_READ, 0, graph_builder);
	if(FAILED(hr)) {
		mmstream->Release();
		win_report_error("IAMMultiMediaStream::Initialize()", hr);
		return false;
	}
	hr = mmstream->AddMediaStream(dd, &MSPID_PrimaryVideo, 0, NULL);
	if(FAILED(hr)) {
		mmstream->Release();
		win_report_error("IAMMultiMediaStream::AddMediaStream(..., &MSPID_PrimaryVideo,...)", hr);
		return false;
	}
	hr = mmstream->AddMediaStream(NULL, &MSPID_PrimaryAudio, AMMSF_ADDDEFAULTRENDERER, NULL);
	if(FAILED(hr)) {
		mmstream->Release();
		win_report_error("IAMMultiMediaStream::AddMediaStream(..., &MSPID_PrimaryAudio,...)", hr);
		return false;
	}

	WCHAR wsz[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, url.c_str(), -1, wsz, MAX_PATH);
	hr = mmstream->OpenFile(wsz, 0);
	if(FAILED(hr)) {
		mmstream->Release();
		if (hr == 0x800c000d)  // XXX This value experimentally determined:-)
			logger::get_logger()->error("%s: Unsupported URL protocol", url.c_str());
		else if (hr == VFW_E_CANNOT_CONNECT)
			logger::get_logger()->error("%s: Unsupported video format", url.c_str());
		else
			logger::get_logger()->error("%s: DirectX error 0x%x", url.c_str(), hr);
		return false;
	}

	m_mmstream = mmstream;
	hr = m_mmstream->GetMediaStream(MSPID_PrimaryVideo, &m_vidstream);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::GetMediaStream()", hr);
		return false;
	}
	hr = m_vidstream->QueryInterface(IID_IDirectDrawMediaStream, (void**)&m_ddstream);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::QueryInterface(IID_IDirectDrawMediaStream, ...)", hr);
		return false;
	}
	hr = m_ddstream->CreateSample(NULL, NULL, 0, &m_ddsample);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::CreateSample()", hr);
		return false;
	}
	hr = m_ddsample->GetSurface(&m_ddsurf, &m_rcsurf);
	if(FAILED(hr)) {
		win_report_error("IDirectDrawStreamSample::GetSurface()", hr);
		return false;
	}
	return true;
}

void gui::dx::video_player::release_player() {
	// avoid an update during release
	IMultiMediaStream *mmstream = m_mmstream;
	m_mmstream = 0;
	if(mmstream) {
		if(m_ddsurf) {
			m_ddsurf->Release();
			m_ddsurf = 0;
		}
		if(m_ddsample) {
			m_ddsample->Release();
			m_ddsample = 0;
		}
		if(m_ddstream)  {
			m_ddstream->Release();
			m_ddstream = 0;
		}
		if(m_vidstream)  {
			m_vidstream->Release();
			m_vidstream = 0;
		}
		mmstream->Release();
	}
}

int gui::dx::video_player::ms_per_frame() {
	if (m_ddstream == NULL) return 50;
	STREAM_TIME frametime = 0;
	HRESULT hr = m_ddstream->GetTimePerFrame(&frametime);
	if (hr != S_OK) return 50;
	// lib::logger::get_logger()->debug("dx_video_player: %lld frame duration", frametime);
	return (int)(frametime / 10000);
}

bool gui::dx::video_player::update() {
	if(!m_mmstream || !m_ddsample) return false;
	HRESULT hr = S_OK;
	bool got_sample = false;
#if 0
	AM_DBG static int count; lib::logger::get_logger()->debug("Update: %d", count++);
	// First check what happened to the previous update (if any)
	if (m_update_busy) {
		hr = m_ddsample->CompletionStatus(COMPSTAT_NOUPDATEOK, 0);
		if (hr != MS_S_PENDING) m_update_busy = false;
		got_sample = (hr == S_OK);
	}
	// And schedule a new sample
	if (!m_update_busy && hr != MS_S_ENDOFSTREAM) {
		HRESULT hr = m_ddsample->Update(SSUPDATE_ASYNC, NULL, NULL, 0);
		if (hr == MS_S_PENDING)
			m_update_busy = true;
		if (hr == S_OK)
			got_sample = true;
	}
#else
	hr = m_ddsample->Update(0, NULL, NULL, 0);
	if (hr == S_OK)
		got_sample = true;
#endif
	return got_sample;
}
