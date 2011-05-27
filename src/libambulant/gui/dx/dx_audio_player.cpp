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

/*
 * @$Id$
 */

#include "ambulant/gui/dx/dx_audio_player.h"

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

#ifdef WITH_TPB_AUDIO_SPEEDUP
bool speedup_filter_available;
bool speedup_filter_available_valid;
#endif

gui::dx::audio_player::audio_player(const std::string& url)
:	m_url(url),
	m_graph_builder(0),
	m_media_control(0),
	m_media_position(0),
	m_media_event(0),
#ifdef WITH_TPB_AUDIO_SPEEDUP
	m_audio_speedup(0),
#endif
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


bool gui::dx::audio_player::stop() {
	//if(m_media_control == 0) return;
	if(m_media_control == 0) return true;
	HRESULT hr = m_media_control->Stop();
	if(FAILED(hr)) {
		win_report_error("IMediaControl::stop()", hr);
	}
	release_player();
	return false;
}


void gui::dx::audio_player::pause(common::pause_display d) {
	if(m_media_control == 0) return;
	HRESULT hr = m_media_control->Pause();
	if(FAILED(hr)) {
		win_report_error("IMediaControl::pause()", hr);
	}
}

void gui::dx::audio_player::resume() {
	if(m_media_control == 0) {
		logger::get_logger()->debug("Invalid call to audio_player::run");
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

void gui::dx::audio_player::endseek(double t) {
	if(m_media_position == 0) return;
	HRESULT hr = m_media_position->put_StopTime(REFTIME(t));
	if(FAILED(hr))
		win_report_error("IMediaPosition::put_StopTime()", hr);
}

std::pair<bool, double> gui::dx::audio_player::get_dur() {
	if(m_media_position == 0) {
		logger::get_logger()->debug("Invalid call to audio_player::get_duration");
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
		if (hr == 0x800c000d)  // XXX This value experimentally determined:-)
			logger::get_logger()->error("%s: Unsupported URL protocol", url.c_str());
		else if (hr == VFW_E_CANNOT_CONNECT)
			logger::get_logger()->error("%s: Unsupported video format", url.c_str());
		else if (hr == VFW_E_NO_AUDIO_HARDWARE) {
			static bool error_shown;
			if (!error_shown) {
				logger::get_logger()->error("%s: No audio hardware on this system", url.c_str());
				error_shown = true;
			} else {
				logger::get_logger()->trace("%s: No audio hardware on this system", url.c_str());
			}
		} else {
			logger::get_logger()->error("%s: DirectX error 0x%x", url.c_str(), hr);
		}
		return false;
	}
#ifdef WITH_TPB_AUDIO_SPEEDUP
	initialize_speedup_filter();
#endif

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

void gui::dx::audio_player::release_player() {
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
#ifdef WITH_TPB_AUDIO_SPEEDUP
		if(m_audio_speedup) {
			m_audio_speedup->Release();
			m_audio_speedup = 0;
		}
		unregister_player(this);
#endif
		m_graph_builder->Release();
		m_graph_builder = 0;
	}
}

#ifdef WITH_TPB_AUDIO_SPEEDUP
void gui::dx::audio_player::initialize_speedup_filter() {
	if (speedup_filter_available_valid && !speedup_filter_available) {
		// We don't seem to have the filter. Too bad.
		return;
	}
	// Either the filter exists or we haven't tried yet. Let's try to create
	// it and remember whether it worked.
	IBaseFilter *pNewFilter = NULL;
	HRESULT res;
	res = CoCreateInstance(CLSID_TPBVupp10, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&pNewFilter);

	if (res != S_OK) {
		lib::logger::get_logger()->trace("dx_audio_player: Speedup filter not available, error 0x%x", res);
		speedup_filter_available = false;
		speedup_filter_available_valid = true;
		return;
	}
	res = m_graph_builder->AddFilter(pNewFilter, NULL);
	if (res != S_OK) {
		lib::logger::get_logger()->trace("dx_audio_player: AddFilter(Speedup filter): error 0x%x", res);
		pNewFilter->Release();
		return;
	}
	speedup_filter_available = true;
	speedup_filter_available_valid = true;
	AM_DBG lib::logger::get_logger()->debug("dx_audio_player: added speedup filter to graph");

	// Next step: find out where we want to add the filter to the graph.
	// We iterate over the filter graph, then for each item in the graph
	// we iterate over the connected output pins util we find one we like.
	IPin *pOutputPin = NULL;
	IPin *pInputPin = NULL;
	IEnumFilters *pEnumFilters = NULL;
	res = m_graph_builder->EnumFilters(&pEnumFilters);
	if (res != S_OK) {
		lib::logger::get_logger()->trace("dx_audio_filter: EnumFilters: error 0x%x", res);
		return;
	}

	IBaseFilter *pCurFilter;
	while (pOutputPin == NULL && (res=pEnumFilters->Next(1, &pCurFilter, NULL)) == S_OK) {
		AM_DBG {
			FILTER_INFO info;
			LPWSTR vendorInfo;
			res = pCurFilter->QueryFilterInfo(&info);
			if (res != S_OK) info.achName[0] = 0;
			res = pCurFilter->QueryVendorInfo(&vendorInfo);
			if (res != S_OK) vendorInfo = L"";
			lib::textptr tInfo(info.achName);
			lib::textptr tVendorInfo(vendorInfo);
			lib::logger::get_logger()->debug("dx_audio_filter: filter found: '%s' vendor '%s'",
				tInfo.c_str(), tVendorInfo.c_str());
		}
		IEnumPins *pEnumPins;
		res = pCurFilter->EnumPins(&pEnumPins);
		IPin *pCurPin;
		while (pOutputPin == NULL && (res=pEnumPins->Next(1, &pCurPin, NULL)) == S_OK) {
			AM_MEDIA_TYPE mediaType;
			PIN_DIRECTION curPinDir;
			res = pCurPin->QueryDirection(&curPinDir);
			HRESULT res2 = pCurPin->ConnectionMediaType(&mediaType);
			if (res == S_OK &&
					res2 == S_OK &&
					curPinDir == PINDIR_OUTPUT &&
					mediaType.majortype == MEDIATYPE_Audio&&
					mediaType.subtype == MEDIASUBTYPE_PCM){
				pOutputPin = pCurPin;
				res = pOutputPin->ConnectedTo(&pInputPin);
				if (res != S_OK) {
					// This output pin was the correct type, but not connected.
					// So it cannot be the one we're looking for.
					pOutputPin = pInputPin = NULL;
				} else {
					// Found it!
					pOutputPin->AddRef();
					pInputPin->AddRef();
				}
			}
			if (res2 == S_OK) {
				if (mediaType.cbFormat != 0) {
					CoTaskMemFree((PVOID)mediaType.pbFormat);
				}
			}
			pCurPin->Release();
		}
		if (res != S_FALSE && res != S_OK)
			lib::logger::get_logger()->trace("dx_audio_filter: enumerating pins: error 0x%x", res);
		pEnumPins->Release();
		pCurFilter->Release();
	}
	if (res != S_FALSE && res != S_OK)
		lib::logger::get_logger()->trace("dx_audio_filter: enumerating filters: error 0x%x", res);

	pEnumFilters->Release();
	// We have the correct pins now.
	if (pOutputPin) {
		lib::logger::get_logger()->trace("dx_audio_filter: found the right pins!");
	} else {
		lib::logger::get_logger()->trace("dx_audio_filter: could not find a good pin");
		pOutputPin->Release();
		pInputPin->Release();
		return;
	}
	// Now we need to find the pins on our speedup filter.
	IPin *pFilterInputPin = NULL;
	IPin *pFilterOutputPin = NULL;
	IEnumPins *pEnumPins;
	res = pNewFilter->EnumPins(&pEnumPins);
	IPin *pCurPin;
	while (res=pEnumPins->Next(1, &pCurPin, NULL) == S_OK) {
		PIN_DIRECTION pinDir;
		res = pCurPin->QueryDirection(&pinDir);
		assert(res == S_OK);
		if (pinDir == PINDIR_INPUT) {
			if (pFilterInputPin) {
				lib::logger::get_logger()->trace("dx_audio_filter: multiple input pins on filter");
				goto bad;
			}
			pFilterInputPin = pCurPin;
			pFilterInputPin->AddRef();
		} else {
			if (pFilterOutputPin) {
				lib::logger::get_logger()->trace("dx_audio_filter: multiple output pins on filter");
				goto bad;
			}
			pFilterOutputPin = pCurPin;
			pFilterOutputPin->AddRef();
		}
	}
	if (!pFilterInputPin) {
		lib::logger::get_logger()->trace("dx_audio_filter: no input pin on filter");
		goto bad;
	}
	if (!pFilterOutputPin) {
		lib::logger::get_logger()->trace("dx_audio_filter: no output pin on filter");
		goto bad;
	}
	// We have everything. Sever the old connection and insert the filter.
	res = m_graph_builder->Disconnect(pOutputPin);
	if (res) {
		lib::logger::get_logger()->trace("dx_audio_filter: Severing old connection: error 0x%x", res);
		goto bad;
	}
	res = m_graph_builder->Disconnect(pInputPin);
	if (res) {
		lib::logger::get_logger()->trace("dx_audio_filter: Severing old connection: error 0x%x", res);
		goto bad;
	}
	res = m_graph_builder->Connect(pOutputPin, pFilterInputPin);
	if (res) {
		lib::logger::get_logger()->trace("dx_audio_filter: Creating filter input connection: error 0x%x", res);
		goto bad;
	}
	res = m_graph_builder->Connect(pFilterOutputPin, pInputPin);
	if (res) {
		lib::logger::get_logger()->trace("dx_audio_filter: Creating filter output connection: error 0x%x", res);
		goto bad;
	}
	// Finally remember the interface to set speedup/slowdown, and register ourselves
	// in the global pool (so Amis can change our speed).
	res = pNewFilter->QueryInterface(IID_IVuppInterface, (void**) &m_audio_speedup);
	if (res != S_OK) {
		lib::logger::get_logger()->trace("dx_audio_filter: filter does not provide IVuppInterface");
		goto bad;
	}
	set_rate(s_current_playback_rate);
	register_player(this);
bad:
	if (pOutputPin) pOutputPin->Release();
	if (pInputPin) pInputPin->Release();
	if (pFilterOutputPin) pFilterOutputPin->Release();
	if (pFilterInputPin) pFilterInputPin->Release();
	return;

}

std::set<gui::dx::audio_player *> gui::dx::audio_player::s_active_players;
double gui::dx::audio_player::s_current_playback_rate = 1.0;

void gui::dx::audio_player::register_player(gui::dx::audio_player *cur) {
	s_active_players.insert(cur);
}

void gui::dx::audio_player::unregister_player(audio_player *cur) {
	s_active_players.erase(cur);
}

void gui::dx::audio_player::set_rate(double rate) {
	if (m_audio_speedup) {
		m_audio_speedup->setCycleSpeed((short)(rate*100));
	}
}

void gui::dx::audio_player::set_global_rate(double rate) {
	s_current_playback_rate = rate;
	std::set<gui::dx::audio_player *>::iterator i;

	for(i=s_active_players.begin(); i!=s_active_players.end(); i++)
		(*i)->set_rate(rate);
}

double gui::dx::audio_player::change_global_rate(double adjustment) {
	if (adjustment != 0.0)
		set_global_rate(s_current_playback_rate+adjustment);
	return s_current_playback_rate;
}

#endif

// -val is the attenuation in decibels
// can be 0 to 100
void gui::dx::audio_player::set_volume(long val) {
	if(m_basic_audio == 0) return;
	if (val < 0) val = 0;
	if (val > 100) val = 100;
	long cdb = (long)(20.0*log10((double)val/100.0)*100);
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
	m_basic_audio->put_Balance(cdb);
}

