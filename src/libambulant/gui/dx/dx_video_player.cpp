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

#include "ambulant/gui/dx/dx_video_player.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"

#include "ambulant/lib/logger.h"

using namespace ambulant;

using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_report_last_error;
using ambulant::lib::logger;
const ULONGLONG MILLIS_FACT = 10000;

gui::dx::video_player::video_player(const std::string& url, viewport* v, lib::event_processor* evp)
:	m_url(url),
	m_viewport(v),
	m_evp(evp), 
	m_mmstream(0),
	m_vidstream(0),
	m_ddstream(0),
	m_ddsample(0),
	m_ddsurf(0),
	m_wantclicks(false),
	m_update_event(0) {
	HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
		win_report_error("CoInitialize() failed", hr);
	open(m_url, m_viewport->get_direct_draw());
}

gui::dx::video_player::~video_player() {
	if(is_playing()) stop();
	CoUninitialize();		
}

void gui::dx::video_player::start(double t) {
	if(!m_mmstream) return;
	if(is_playing()) pause();
	seek(t);
	resume();
}

void gui::dx::video_player::stop() {
	if(!m_mmstream) return;
	pause();
	Sleep(50);
	release();
	// remove any effects
}

void gui::dx::video_player::pause() {
	if(!m_mmstream) return;
	cancel_update();
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
	schedule_update();
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
		win_report_error("IMultiMediaStream::SetState()", hr);	
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
	return state == STREAMSTATE_RUN;
}

double gui::dx::video_player::get_position() {
	if(!m_mmstream) return 0;
	STREAM_TIME st;
	HRESULT hr = m_mmstream->GetTime(&st);
	if(FAILED(hr)) {
		win_report_error("IMultiMediaStream::GetTime()", hr);	
		return 0.0;
	}
	return  0.001*double(st / MILLIS_FACT);
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
		win_report_error("IAMMultiMediaStream::OpenFile()", hr);	
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

#define RELEASE(x, s) if(x) {logger::get_logger()->trace(s, x->Release());x=NULL;}

void gui::dx::video_player::release() {
	if(m_mmstream) {
		/*
		if(m_ddsurf) {
			m_ddsurf->Release();
			m_ddsurf = 0;
		}
		if(m_ddsample) {
			m_ddsample->Release();
			m_ddsample = 0;
		}*/
		if(m_ddstream)  {
			m_ddstream->Release();
			m_ddstream = 0;
		}
		if(m_vidstream)  {
			m_vidstream->Release();
			m_vidstream = 0;
		}
		m_mmstream->Release();
		m_mmstream = 0;
	}
}

bool gui::dx::video_player::update() {
	if(!m_mmstream || !m_ddsample) return false;
	return m_ddsample->Update(0, NULL, NULL, 0) == S_OK;
}

void gui::dx::video_player::update_callback() {
	if(!m_update_event) return;
	m_update_event = 0;
	if(update()) {
		m_viewport->redraw();
		schedule_update();
	}
	
}

void gui::dx::video_player::schedule_update() {
	if(m_update_event) return;
	m_update_event = new lib::no_arg_callback_event<video_player>(this, 
		&video_player::update_callback);
	m_evp->add_event(m_update_event, 50);
}

void gui::dx::video_player::cancel_update() {
	if(m_update_event) {
		m_evp->cancel_event(m_update_event);
		m_update_event = 0;
	}
}
