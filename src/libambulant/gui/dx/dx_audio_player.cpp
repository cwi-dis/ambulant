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

#include "ambulant/gui/dx/dx_audio_player.h"

#include "ambulant/lib/logger.h"

using namespace ambulant;

using ambulant::lib::win32::win_report_error;
using ambulant::lib::logger;
const ULONGLONG MILLIS_FACT = 10000;

gui::dx::audio_player::audio_player(const std::string& url)
:	m_url(url),
	m_graph_builder(0),
	m_media_control(0),
	m_media_position(0),
	m_media_event(0),
	m_basic_audio(0) {
	open(m_url);
}
	
gui::dx::audio_player::~audio_player() {
	stop();
}
		
void gui::dx::audio_player::start(double t) {
	if(is_playing()) pause();
	seek(t);
	resume();
}

void gui::dx::audio_player::stop() {
	if(m_media_control == 0) return;
	HRESULT hr = m_media_control->Stop();
	if(FAILED(hr)) {
		win_report_error("IMediaControl::stop()", hr);	
	}
	release();
}

void gui::dx::audio_player::pause() {
	if(m_media_control == 0) return;
	HRESULT hr = m_media_control->Pause();
	if(FAILED(hr)) {
		win_report_error("IMediaControl::pause()", hr);	
	}
}

void gui::dx::audio_player::resume() {
	if(m_media_control == 0) {
		logger::get_logger()->warn("Invalid call to audio_player::run");
		return;
	}
	HRESULT hr = m_media_control->Run();
	if(FAILED(hr)) {
		win_report_error("IMediaControl::run()", hr);	
	}
}

void gui::dx::audio_player::seek(double t) {
	if(m_media_position == 0) return;
	HRESULT hr = m_media_position->put_CurrentPosition(REFTIME(t));
	if(FAILED(hr))
		win_report_error("IMediaPosition::put_CurrentPosition()", hr);	
}

std::pair<bool, double> gui::dx::audio_player::get_dur() {
	if(m_media_position == 0) {
		logger::get_logger()->warn("Invalid call to audio_player::get_duration");
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

bool gui::dx::audio_player::can_play() {
	return m_graph_builder && 
		m_media_event &&
		m_media_position &&
		m_media_control &&
		m_media_event;
}

bool gui::dx::audio_player::is_playing() {
	if(m_media_event == 0) return false;
	long msTimeout = 50;
	long evCode = 0;
	HRESULT hr = m_media_event->WaitForCompletion(msTimeout, &evCode);
	if(hr == E_ABORT) return true;
	else if(hr == S_OK) return false;
	else if(FAILED(hr)) {
		win_report_error("IMediaEvent::WaitForCompletion()", hr);	
		return false;
	}
	return evCode == 0;
}

double gui::dx::audio_player::get_position() {
	if(m_media_position == 0) {
		logger::get_logger()->warn("Invalid call to audio_player::get_current_position");
		return 0.0;
	}
	REFTIME pos = 0.0;
	HRESULT hr = m_media_position->get_CurrentPosition(&pos);
	if(FAILED(hr)) {
		win_report_error("IMediaPosition::get_CurrentPosition()", hr);	
		return 0.0;
	}
	return pos;
}	
	
//////////////////////////
		
bool gui::dx::audio_player::open(const std::string& url) {
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
		win_report_error("IGraphBuilder::RenderFile()", hr);	
		return false;
	}
		
	hr = m_graph_builder->QueryInterface(IID_IMediaControl, (void **) &m_media_control);
	if(FAILED(hr)) {
		win_report_error("QueryInterface(IID_IMediaControl, ...)", hr);	
		return false;
	}
	m_graph_builder->QueryInterface(IID_IMediaPosition, (void **) &m_media_position);
	if(FAILED(hr)) {
		win_report_error("QueryInterface(IID_IMediaPosition, ...)", hr);	
		return false;
	}
	m_graph_builder->QueryInterface(IID_IMediaEvent, (void **) &m_media_event);
	if(FAILED(hr)) {
		win_report_error("QueryInterface(IID_IMediaEvent, ...)", hr);	
		return false;
	}
			
	m_graph_builder->QueryInterface(IID_IBasicAudio, (void **) &m_basic_audio);
	if(FAILED(hr)) {
		win_report_error("QueryInterface(IID_IBasicAudio, ...)", hr);	
	}
	return true;
}

void gui::dx::audio_player::release() {
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
		m_graph_builder->Release();
		m_graph_builder = 0;
	}
}
		
int gui::dx::audio_player::get_progress() {
	return (int)floor(0.5 + 100.0*get_position()/get_dur().second);
}
		
void gui::dx::audio_player::set_progress(int p) {
	seek(get_dur().second*(double(p)/100.00));
}
			
// -val is the attenuation in decibels 
// can be 0 to 100
void gui::dx::audio_player::set_volume(long val) {
	if(m_basic_audio == 0) return;
	val = (val>=0)?val:0;
	val = (val<=100)?val:100;
	long cdb = -(100-val)*100;
	m_basic_audio->put_Volume(cdb);
}
		
// can be -100 to 100
// 0 sets a neutral balance
// and 10 sets -10 db to right and -90 db to left
void gui::dx::audio_player::set_balance(long val) {
	if(m_basic_audio == 0) return;
	val = (val>=-100)?val:-100;
	val = (val<=100)?val:100;
	long cdb = val*100;
	m_basic_audio->put_Volume(cdb);
}

