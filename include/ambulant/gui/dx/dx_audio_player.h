
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_GUI_DX_AUDIO_PLAYER_H
#define AMBULANT_GUI_DX_AUDIO_PLAYER_H

#include <string>

#include <objbase.h>
#include <strmif.h>
#include <control.h>

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

// Context template argument: 
//	implements get_url()

template <class Context>
class audio_player {
  public:
	audio_player()
	:	m_context(0),
		m_graph_builder(0),
		m_media_control(0),
		m_media_position(0),
		m_media_event(0),
		m_basic_audio(0) {
		HRESULT hr = CoInitialize(NULL);
		if(FAILED(hr))
			win_report_error("CoInitialize() failed", hr);
	}
		
	audio_player(const Context *ctx)
	:	m_context(ctx),
		m_graph_builder(0),
		m_media_control(0),
		m_media_position(0),
		m_media_event(0),
		m_basic_audio(0) {
		HRESULT hr = CoInitialize(NULL);
		if(FAILED(hr))
			win_report_error("CoInitialize() failed", hr);
		open(ctx->get_url());
	}
	
	~audio_player() {
		if(m_graph_builder) {
			if(m_media_event) 
				m_media_event->Release();
			if(m_media_position) 
				m_media_position->Release();
			if(m_media_control) 
				m_media_control->Release();
			if(m_basic_audio)
				m_basic_audio->Release();
			m_graph_builder->Release();
		}
		CoUninitialize();		
	}
		
	bool can_play() {
		return m_graph_builder && 
			m_media_event &&
			m_media_position &&
			m_media_control &&
			m_media_event &&
			get_duration() > 1.0;
	}	
		
	bool open(const std::string& url) {
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
			
	int get_progress() {
		return floor(0.5 + 100.0*get_position()/get_duration());
	}
		
	bool set_progress(int p) {
		set_position(get_duration()*(double(p)/100.00));
	}
	
	bool play() {
		if(m_media_control == 0) {
			logger::get_logger()->warn("Invalid call to audio_player::run");
			return false;
		}
		HRESULT hr = m_media_control->Run();
		if(FAILED(hr)) {
			win_report_error("IMediaControl::run()", hr);	
			return false;
		}
		return true;
	}
			
	bool pause() {
		if(m_media_control == 0) return true;
		HRESULT hr = m_media_control->Pause();
		if(FAILED(hr)) {
			win_report_error("IMediaControl::pause()", hr);	
			return false;
		}
		return true;
	}	
		
	bool stop() {
		if(m_media_control == 0) return true;
		HRESULT hr = m_media_control->Stop();
		if(FAILED(hr)) {
			win_report_error("IMediaControl::stop()", hr);	
			return false;
		}
		return true;
	}
			
	double get_duration() {
		if(m_media_position == 0) {
			logger::get_logger()->warn("Invalid call to audio_player::get_duration");
			return 1.0;
		}
		REFTIME dur = 0.0;
		HRESULT hr = m_media_position->get_Duration(&dur);
		if(FAILED(hr)) {
			win_report_error("IMediaPosition::get_Duration()", hr);	
			return 1.0;
		}
		dur=(dur==0.0)?1.0:dur;
		return dur;
	}	
		
	double get_position() {
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

	double get_remaining() {
		return get_duration() - get_position();
	}
		
	void set_position(double pos) {
		if(m_media_position == 0) return;
		HRESULT hr = m_media_position->put_CurrentPosition(REFTIME(pos));
		if(FAILED(hr))
			win_report_error("IMediaPosition::put_CurrentPosition()", hr);	
	}
			
	bool is_playing() {
		if(m_media_event == 0) return false;
		long msTimeout = 0;
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
		
	// -val is the attenuation in decibels 
	// can be 0 to 100
	void set_volume(long val) {
		if(m_basic_audio == 0) return;
		val = (val>=0)?val:0;
		val = (val<=100)?val:100;
		long cdb = -(100-val)*100;
		m_basic_audio->put_Volume(cdb);
	}
		
	// can be -100 to 100
	// 0 sets a neutral balance
	// and 10 sets -10 db to right and -90 db to left
	void set_balance(long val) {
		if(m_basic_audio == 0) return;
		val = (val>=-100)?val:-100;
		val = (val<=100)?val:100;
		long cdb = val*100;
		m_basic_audio->put_Volume(cdb);
	}
	
	const Context* get_context() const { return m_context;}

  private:
	const Context *m_context;
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

