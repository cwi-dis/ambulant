// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
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

#include "ambulant/gui/dx/dx_basicvideo_player.h"

#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"
#include <math.h>
#include <vfwmsgs.h>
// CLSID_FilterGraph
#include <uuids.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

using ambulant::lib::win32::win_report_error;
using ambulant::lib::win32::win_trace_error;
using ambulant::lib::logger;
const ULONGLONG MILLIS_FACT = 10000;


gui::dx::basicvideo_player::basicvideo_player(const std::string& url, HWND parent)
:	m_url(url),
	m_graph_builder(0),
	m_media_control(0),
	m_media_position(0),
	m_media_event(0),
	m_basic_audio(0) {
	open(m_url, parent);
}

gui::dx::basicvideo_player::~basicvideo_player() {
	stop();
}

void gui::dx::basicvideo_player::start(double t) {
	if(is_playing()) pause();
	seek(t);
	resume();
}

bool gui::dx::basicvideo_player::stop() {
	if(m_media_control == 0) return true;
	HRESULT hr = m_media_control->Stop();
	if(FAILED(hr)) {
		win_report_error("IMediaControl::stop()", hr);
	}
	release_player();
	return false;
}

void gui::dx::basicvideo_player::pause(common::pause_display d) {
	if(m_media_control == 0) return;
	HRESULT hr = m_media_control->Pause();
	if(FAILED(hr)) {
		win_report_error("IMediaControl::pause()", hr);
	}
}

void gui::dx::basicvideo_player::resume() {
	if(m_media_control == 0) {
		logger::get_logger()->debug("Invalid call to basicvideo_player::run");
		return;
	}
	HRESULT hr = m_media_control->Run();
	if(FAILED(hr)) {
		win_report_error("IMediaControl::run()", hr);
	}
}

void gui::dx::basicvideo_player::seek(double t) {
	if(m_media_position == 0) return;
	HRESULT hr = m_media_position->put_CurrentPosition(REFTIME(t));
	if(FAILED(hr))
		win_report_error("IMediaPosition::put_CurrentPosition()", hr);
}

void gui::dx::basicvideo_player::endseek(double t) {
	if(m_media_position == 0) return;
	HRESULT hr = m_media_position->put_StopTime(REFTIME(t));
	if(FAILED(hr))
		win_report_error("IMediaPosition::put_StopTime()", hr);
}

std::pair<bool, double> gui::dx::basicvideo_player::get_dur() {
	if(m_media_position == 0) {
		logger::get_logger()->debug("Invalid call to basicvideo_player::get_duration");
		return std::pair<bool, double>(false, 0);
	}
	REFTIME dur = 0.0;
	HRESULT hr = m_media_position->get_Duration(&dur);
	if(FAILED(hr)) {
		win_report_error("IMediaPosition::get_Duration()", hr);
		return std::pair<bool, double>(false, 0);
	}
	return std::pair<bool, double>(dur>0, dur);
}

bool gui::dx::basicvideo_player::can_play() {
	return m_graph_builder &&
		m_media_event &&
		m_media_position &&
		m_media_control &&
		m_media_event;
}

bool gui::dx::basicvideo_player::is_playing() {
	if(m_media_event == 0) return false;
	long msTimeout = 0;
	long evCode = 0;
	HRESULT hr = m_media_event->WaitForCompletion(msTimeout, &evCode);
	if(hr == E_ABORT) return true;
	else if(hr == S_OK) return false;
	else if(FAILED(hr)) {
		// XXXJack: this error occurs all the time...
		if (hr == 0x80040227) return false;
		win_trace_error("IMediaEvent::WaitForCompletion()", hr);
		return false;
	}
	return evCode == 0;
}

//////////////////////////

bool gui::dx::basicvideo_player::open(const std::string& url, HWND parent) {
	m_url = url;
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph,0,CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder,(void**)&m_graph_builder);
	if(FAILED(hr)) {
		win_report_error("CoCreateInstance(CLSID_FilterGraph, ...)", hr);
		return false;
	}

	WCHAR wsz[MAX_PATH];
	MultiByteToWideChar(CP_ACP,0, url.c_str(), -1, wsz, MAX_PATH);
	hr = m_graph_builder->RenderFile(wsz, 0);
	if(FAILED(hr)){
		if (hr == 0x800c000d)  // XXX This value experimentally determined:-)
			logger::get_logger()->error("%s: Unsupported URL protocol", url.c_str());
		else if (hr == VFW_E_CANNOT_CONNECT)
			logger::get_logger()->error("%s: Unsupported video format", url.c_str());
		else
			logger::get_logger()->error("%s: DirectX error 0x%x", url.c_str(), hr);
		return false;
	}

	hr = m_graph_builder->QueryInterface(IID_IMediaControl, (void **) &m_media_control);
	if(FAILED(hr)) {
		win_report_error("QueryInterface(IID_IMediaControl, ...)", hr);
		return false;
	}
	hr = m_graph_builder->QueryInterface(IID_IMediaPosition, (void **) &m_media_position);
	if(FAILED(hr)) {
		win_report_error("QueryInterface(IID_IMediaPosition, ...)", hr);
		return false;
	}
	hr = m_graph_builder->QueryInterface(IID_IMediaEvent, (void **) &m_media_event);
	if(FAILED(hr)) {
		win_report_error("QueryInterface(IID_IMediaEvent, ...)", hr);
		return false;
	}

	hr = m_graph_builder->QueryInterface(IID_IBasicAudio, (void **) &m_basic_audio);
	if(FAILED(hr)) {
		win_report_error("QueryInterface(IID_IBasicAudio, ...)", hr);
	}
	hr = m_graph_builder->QueryInterface(IID_IVideoWindow, (void **) &m_video_window);
	if(FAILED(hr)) {
		win_report_error("QueryInterface(IID_IVideoWindow, ...)", hr);
	}

	// Reposition output window
	if (m_video_window) {
		hr = m_video_window->put_Owner((OAHWND)parent);
		if (FAILED(hr)) {
			win_report_error("put_Owner()", hr);
		}
		long style;
		hr = m_video_window->get_WindowStyle(&style);
		if (FAILED(hr)) {
			win_report_error("get_WindowStyle()", hr);
		}
		lib::logger::get_logger()->debug("basicvideo_player: windowStyle was 0x%x", style);
		style |= WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
		style &= ~(WS_CAPTION|WS_BORDER|WS_DLGFRAME|WS_SYSMENU|WS_SIZEBOX|WS_MINIMIZEBOX);
		hr = m_video_window->put_WindowStyle(style);
		if (FAILED(hr)) {
			win_report_error("put_WindowStyle()", hr);
		}
		hr = m_video_window->put_MessageDrain((OAHWND)parent);
		if (FAILED(hr)) {
			win_report_error("put_MessageDrain()", hr);
		}
	}
	return true;
}

void gui::dx::basicvideo_player::setrect(const lib::rect &r) {
	if (!m_video_window) return;
	HRESULT hr = m_video_window->SetWindowPosition(r.x, r.y, r.w, r.h);
	if (FAILED(hr)) {
		win_report_error("SetWindowPosition()", hr);
	}
}

void gui::dx::basicvideo_player::release_player() {
	if(m_graph_builder) {
		if(m_media_event) {
			m_media_event->Release();
			m_media_event = 0;
		}
		if(m_media_position) {
			m_media_position->Release();
			m_media_position = 0;
		}
		if(m_media_control) {
			m_media_control->Release();
			m_media_control = 0;
		}
		if(m_basic_audio) {
			m_basic_audio->Release();
			m_basic_audio = 0;
		}
		if (m_video_window) {
			m_video_window->Release();
			m_video_window = 0;
		}
		m_graph_builder->Release();
		m_graph_builder = 0;
	}
}


// -val is the attenuation in decibels
// can be 0 to 100
void gui::dx::basicvideo_player::set_volume(long val) {
	if(m_basic_audio == 0) return;
	if (val < 0) val = 0;
	if (val > 100) val = 100;
	long cdb = (long)(20.0*log10((double)val/100.0)*100);
	m_basic_audio->put_Volume(cdb);
}

// can be -100 to 100
// 0 sets a neutral balance
// and 10 sets -10 db to right and -90 db to left
void gui::dx::basicvideo_player::set_balance(long val) {
	if(m_basic_audio == 0) return;
	val = (val>=-100)?val:-100;
	val = (val<=100)?val:100;
	long cdb = val*100;
	m_basic_audio->put_Balance(cdb);
}

