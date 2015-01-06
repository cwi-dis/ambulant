// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
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

#include "ambulant/gui/d2/d2_d2video.h"
#include "ambulant/gui/d2/d2_dshowsink.h"
#include "ambulant/gui/d2/d2_transition.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_asb.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/test_attrs.h"

#include <control.h>
#include <strmif.h>
#include <uuids.h>
#include <vfwmsgs.h>
#include <d2d1.h>
#include <d2d1helper.h>

#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"amstrmid.lib")
#pragma comment (lib,"strmiids.lib")
#pragma comment (lib,"uuid.lib")

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

inline D2D1_RECT_F d2_rectf(lib::rect r) {
	return D2D1::RectF((float) r.left(), (float) r.top(), (float) r.right(), (float) r.bottom());
}

extern const char d2_d2video_playable_tag[] = "video";
extern const char d2_d2video_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirectXVideo");
extern const char d2_d2video_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererVideo");

common::playable_factory *
gui::d2::create_d2_d2video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectXVideo"), true);
	return new common::single_playable_factory<
		gui::d2::d2_d2video_renderer,
		d2_d2video_playable_tag,
		d2_d2video_playable_renderer_uri,
		d2_d2video_playable_renderer_uri2,
		d2_d2video_playable_renderer_uri >(factory, mdp);
}

gui::d2::d2_d2video_renderer::d2_d2video_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories *fp,
	common::playable_factory_machdep *mdp)
:	d2_renderer<common::renderer_playable>(context, cookie, node, evp, fp, mdp),
	m_media_event(NULL),
	m_media_position(NULL),
	m_media_control(NULL),
	m_basic_audio(NULL),
	m_graph_builder(NULL),
	m_video_sink(NULL),
	m_update_event(0),
	m_d2player(dynamic_cast<d2_player*>(mdp))
{
	AM_DBG lib::logger::get_logger()->debug("d2_d2video_renderer(0x%x)", this);
}

gui::d2::d2_d2video_renderer::~d2_d2video_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~d2_d2video_renderer(0x%x)", this);
	_stop();
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
	if (m_video_sink) {
		m_video_sink->SetRenderTarget(NULL);
		m_video_sink->SetCallback(NULL);
#if 0
		// For reasons unknown, we should not release the video
		// sink. Maybe the AddRef() is missing?
		m_video_sink->Release();
#endif
		m_video_sink = 0;
	}
	if(m_graph_builder) {
		m_graph_builder->Release();
		m_graph_builder = 0;
	}

}

void gui::d2::d2_d2video_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("start: %s", m_node->get_sig().c_str());
	common::surface *surf = get_surface();

	net::url url = m_node->get_url("src");
	_init_clip_begin_end();
	if(url.is_local_file() || lib::win32::file_exists(url.get_file())) {
		_open(url.get_file());
	} else if(url.is_absolute()) {
		_open(url.get_url());
	} else {
		lib::logger::get_logger()->show("The location specified for the data source does not exist. [%s]",
			url.get_url().c_str());
	}
	if(!_can_play()) {
		// Not created or stopped (gone)

		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}

	ID2D1RenderTarget *rt = m_d2player->get_rendertarget();
	if (rt) {
		m_video_sink->SetRenderTarget(rt);
	}
	m_video_sink->SetCallback(this);

	lib::rect r = surf->get_rect();
	r.translate(surf->get_global_topleft());
	// Has this been activated
	if(m_activated) {
		_start(t + (m_clip_begin / 1000000.0));
		m_dest->need_redraw();
		_schedule_update();
		return;
	}

	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;

	// Start the underlying player
	_start(t + (m_clip_begin / 1000000.0));

	// Request a redraw
	m_dest->need_redraw();

	// Notify the scheduler; may take benefit
	m_context->started(m_cookie);

	// Schedule a self-update
	_schedule_update();
}

bool gui::d2::d2_d2video_renderer::_open(const std::string& url) {
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph,0,CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder,(void**)&m_graph_builder);
	if(FAILED(hr)) {
		lib::win32::win_report_error("CoCreateInstance(CLSID_FilterGraph, ...)", hr);
		return false;
	}
	// We now optionally add a specific output handler.
	m_video_sink = new CVideoD2DBitmapRenderer(NULL, &hr);
	if(FAILED(hr)) {
		lib::win32::win_report_error("CVideoD2DBitmapRenderer()", hr);
		return false;
	}

	m_graph_builder->AddFilter(m_video_sink, L"Direct2D Bitmap Renderer");

	WCHAR wsz[MAX_PATH];
	MultiByteToWideChar(CP_ACP,0, url.c_str(), -1, wsz, MAX_PATH);
	hr = m_graph_builder->RenderFile(wsz, 0);
	if(FAILED(hr)){
		if (hr == 0x800c000d)  // XXX This value experimentally determined:-)
			lib::logger::get_logger()->error("%s: Unsupported URL protocol", url.c_str());
		else if (hr == VFW_E_CANNOT_CONNECT)
			lib::logger::get_logger()->error("%s: Unsupported video format", url.c_str());
		else
			lib::logger::get_logger()->error("%s: DirectX error 0x%x", url.c_str(), hr);
		return false;
	}

	hr = m_graph_builder->QueryInterface(IID_IMediaControl, (void **) &m_media_control);
	if(FAILED(hr)) {
		lib::win32::win_report_error("QueryInterface(IID_IMediaControl, ...)", hr);
		return false;
	}
	hr = m_graph_builder->QueryInterface(IID_IMediaPosition, (void **) &m_media_position);
	if(FAILED(hr)) {
		lib::win32::win_report_error("QueryInterface(IID_IMediaPosition, ...)", hr);
		return false;
	}
	hr = m_graph_builder->QueryInterface(IID_IMediaEvent, (void **) &m_media_event);
	if(FAILED(hr)) {
		lib::win32::win_report_error("QueryInterface(IID_IMediaEvent, ...)", hr);
		return false;
	}

	hr = m_graph_builder->QueryInterface(IID_IBasicAudio, (void **) &m_basic_audio);
	if(FAILED(hr)) {
		lib::win32::win_report_error("QueryInterface(IID_IBasicAudio, ...)", hr);
	}

	return true;
}

bool gui::d2::d2_d2video_renderer::_can_play() {
	return m_graph_builder &&
		m_media_event &&
		m_media_position &&
		m_media_control &&
		m_media_event;
}
void gui::d2::d2_d2video_renderer::_start(double t) {
	if(_is_playing()) pause();
	seek(t);
	resume();
}

bool gui::d2::d2_d2video_renderer::_stop() {
	if(m_media_control == 0) return true;
	HRESULT hr = m_media_control->Stop();
	if(FAILED(hr)) {
		lib::win32::win_report_error("IMediaControl::stop()", hr);
	}
	return false;
}

void gui::d2::d2_d2video_renderer::_pause(common::pause_display d) {
	if(m_media_control == 0) return;
	HRESULT hr = m_media_control->Pause();
	if(FAILED(hr)) {
		lib::win32::win_report_error("IMediaControl::pause()", hr);
	}
}

void gui::d2::d2_d2video_renderer::_resume() {
	if(m_media_control == 0) {
		lib::logger::get_logger()->debug("Invalid call to basicvideo_player::run");
		return;
	}
	HRESULT hr = m_media_control->Run();
	if(FAILED(hr)) {
		lib::win32::win_report_error("IMediaControl::run()", hr);
	}
}

void gui::d2::d2_d2video_renderer::_seek(double t) {
	if(m_media_position == 0) return;
	HRESULT hr = m_media_position->put_CurrentPosition(REFTIME(t));
	if(FAILED(hr))
		lib::win32::win_report_error("IMediaPosition::put_CurrentPosition()", hr);
}

bool gui::d2::d2_d2video_renderer::_is_playing() {
	if(m_media_event == 0) return false;
	long msTimeout = 0;
	long evCode = 0;
	HRESULT hr = m_media_event->WaitForCompletion(msTimeout, &evCode);
	if(hr == E_ABORT) return true;
	else if(hr == S_OK) return false;
	else if(FAILED(hr)) {
		// XXXJack: this error occurs all the time...
		if (hr == 0x80040227) return false;
		lib::win32::win_trace_error("IMediaEvent::WaitForCompletion()", hr);
		return false;
	}
	return evCode == 0;
}

void gui::d2::d2_d2video_renderer::seek(double t) {
	assert( t >= 0);
	_seek(t + (m_clip_begin / 1000000.0));
}

std::pair<bool, double> gui::d2::d2_d2video_renderer::get_dur() {
	if(m_media_position == 0) {
		lib::logger::get_logger()->debug("Invalid call to basicvideo_player::get_duration");
		return std::pair<bool, double>(false, 0);
	}
	REFTIME dur = 0.0;
	HRESULT hr = m_media_position->get_Duration(&dur);
	if(FAILED(hr)) {
		lib::win32::win_report_error("IMediaPosition::get_Duration()", hr);
		return std::pair<bool, double>(false, 0);
	}
	if (m_clip_end > 0 && dur > m_clip_end / 1000000)
		dur = (REFTIME) m_clip_end / 1000000;
	dur -= (REFTIME) (m_clip_begin / 1000000);
	return std::pair<bool, double>(true, dur);
}

bool gui::d2::d2_d2video_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("stop: %s", m_node->get_sig().c_str());
	m_cs.enter();
	m_update_event = 0;
	_stop();
	surface *dest = m_dest;
	m_dest = NULL;
	if (m_video_sink) {
		m_video_sink->SetRenderTarget(NULL);
		m_video_sink->SetCallback(NULL);
	}
	m_cs.leave();
	if (dest) dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	m_context->stopped(m_cookie);
	return false;
}

void gui::d2::d2_d2video_renderer::pause(common::pause_display d) {
	AM_DBG lib::logger::get_logger()->debug("d2_d2video_renderer.pause(0x%x)", this);
	m_update_event = 0;
	_pause(d);
}

void gui::d2::d2_d2video_renderer::resume() {
	AM_DBG lib::logger::get_logger()->debug("d2_d2video_renderer.resume(0x%x)", this);
	_resume();
	if(!m_update_event) _schedule_update();
	m_dest->need_redraw();
}

bool gui::d2::d2_d2video_renderer::user_event(const lib::point& pt, int what) {
	if (!user_event_sensitive(pt)) return false;
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	return true;
}

void gui::d2::d2_d2video_renderer::redraw_body(const lib::rect &dirty, common::gui_window *window, ID2D1RenderTarget* rt)
{
	recreate_d2d();
	if (m_video_sink == NULL) return;
	if (rt == NULL) return;
	m_video_sink->SetRenderTarget(rt);
	ID2D1Bitmap *bitmap = m_video_sink->LockBitmap();
	if (bitmap == NULL) return;

	lib::rect img_rect1;
	lib::rect img_reg_rc;
	D2D1_SIZE_U bmsize = bitmap->GetPixelSize();
	lib::size srcsize(bmsize.width, bmsize.height);

	lib::rect croprect = m_dest->get_crop_rect(srcsize);
	AM_DBG lib::logger::get_logger()->debug("get_crop_rect(%d,%d) -> (%d, %d, %d, %d)", srcsize.w, srcsize.h, croprect.left(), croprect.top(), croprect.width(), croprect.height());
	img_reg_rc = m_dest->get_fit_rect(croprect, srcsize, &img_rect1, m_alignment);
	double alpha_media = 1.0, alpha_media_bg = 1.0, alpha_chroma = 1.0;
	lib::color_t chroma_low = lib::color_t(0x000000), chroma_high = lib::color_t(0xFFFFFF);
	const common::region_info *ri = m_dest->get_info();
	if (ri) {
		alpha_media = ri->get_mediaopacity();
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			lib::compute_chroma_range(chromakey, chromakeytolerance, &chroma_low, &chroma_high);
		} else alpha_chroma = alpha_media;
	}
	img_reg_rc.translate(m_dest->get_global_topleft());

	lib::rect img_rect(img_rect1);
	D2D1_RECT_F d2rect = d2_rectf(img_rect);
	// Video images are upside-down, so we modify the matrix and
	// re-set it after the bitblit.
	D2D1_MATRIX_3X2_F oldmatrix;
	rt->GetTransform(&oldmatrix);
	D2D1_MATRIX_3X2_F newmatrix =
		D2D1::Matrix3x2F::Scale(1.0f, -1.0f, D2D1::Point2F((d2rect.left+d2rect.right)/2, (d2rect.top+d2rect.bottom)/2));
	rt->SetTransform(&newmatrix);
	rt->DrawBitmap(
		bitmap,
		d2_rectf(img_reg_rc),
		alpha_media,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		d2rect);
	rt->SetTransform(&oldmatrix);

	m_video_sink->UnlockBitmap(bitmap);
}

void gui::d2::d2_d2video_renderer::BitmapAvailable(CVideoD2DBitmapRenderer *caller)
{
	if (m_dest) m_dest->need_redraw();
}

void gui::d2::d2_d2video_renderer::recreate_d2d()
{
}

void gui::d2::d2_d2video_renderer::discard_d2d()
{
	if (m_video_sink) m_video_sink->DestroyBitmap();
}

void gui::d2::d2_d2video_renderer::_update_callback() {
	// Schedule a redraw callback
	m_cs.enter();
	if(!m_update_event || !_can_play()) {
		m_cs.leave();
		return;
	}
	m_dest->need_redraw();
	bool need_callback = _is_playing();

	m_cs.leave();

	if( need_callback ) {
		_schedule_update();
	} else {
		m_update_event = 0;
		m_context->stopped(m_cookie);
	}
}

void gui::d2::d2_d2video_renderer::_schedule_update() {
	m_update_event = new lib::no_arg_callback<d2_d2video_renderer>(this,
		&d2_d2video_renderer::_update_callback);
	m_event_processor->add_event(m_update_event, 50, lib::ep_med);
}

