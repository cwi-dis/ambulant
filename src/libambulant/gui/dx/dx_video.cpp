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

#include "ambulant/gui/dx/dx_video.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_video_player.h"
#include "ambulant/gui/dx/dx_transition.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_asb.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
extern const char dx_video_playable_tag[] = "video";
extern const char dx_video_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirectX");
extern const char dx_video_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererDirectXVideo");
extern const char dx_video_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererVideo");

common::playable_factory *
gui::dx::create_dx_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectX"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectXVideo"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererVideo"), true);
	return new common::single_playable_factory<
		gui::dx::dx_video_renderer,
		dx_video_playable_tag,
		dx_video_playable_renderer_uri,
		dx_video_playable_renderer_uri2,
		dx_video_playable_renderer_uri3 >(factory, mdp);
}

gui::dx::dx_video_renderer::dx_video_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories *fp,
	common::playable_factory_machdep *dxplayer)
:	dx_renderer_playable(context, cookie, node, evp, fp, dynamic_cast<dx_playables_context*>(dxplayer)),
	m_player(0),
	m_update_event(0),
	m_frametime(50)
{
	AM_DBG lib::logger::get_logger()->debug("dx_video_renderer(0x%x)", this);
}

gui::dx::dx_video_renderer::~dx_video_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~dx_video_renderer(0x%x)", this);
	if(m_player) stop();
}

void gui::dx::dx_video_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("start: %s", m_node->get_xpath().c_str());
	common::surface *surf = get_surface();

	dx_window *dxwindow = static_cast<dx_window*>(surf->get_gui_window());
	viewport *v = dxwindow->get_viewport();
	net::url url = m_node->get_url("src");
	_init_clip_begin_end();
	if(url.is_local_file() || lib::win32::file_exists(url.get_file())) {
		m_player = new gui::dx::video_player(url.get_file(), v->get_direct_draw());
	} else if(url.is_absolute()) {
		m_player = new gui::dx::video_player(url.get_url(), v->get_direct_draw());
	} else {
		lib::logger::get_logger()->show("The location specified for the data source does not exist. [%s]",
			url.get_url().c_str());
	}
	if(!m_player) {
		// Not created or stopped (gone)

		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}

	// Does it have all the resources to play?
	if(!m_player->can_play()) {
		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}

	// Has this been activated
	if(m_activated) {
		// repeat
		m_player->start(t + (m_clip_begin / 1000000.0));
		m_player->update();
		m_dest->need_redraw();
		schedule_update();
		return;
	}

	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;

	// Start the underlying player
	m_player->start(t + (m_clip_begin / 1000000.0));
	m_frametime = m_player->ms_per_frame();
	AM_DBG lib::logger::get_logger()->debug("dx_video: %d ms/frame", m_frametime);
	m_player->update();

	// Request a redraw
	m_dest->need_redraw();

	// Notify the scheduler; may take benefit
	m_context->started(m_cookie);

	// Schedule a self-update
	schedule_update();
}

void gui::dx::dx_video_renderer::seek(double t) {
	if (m_player) m_player->seek(t + (m_clip_begin / 1000000.0));
	// ?? if(!m_update_event) schedule_update();
	// ?? m_dest->need_redraw();
}
std::pair<bool, double> gui::dx::dx_video_renderer::get_dur() {
	if(m_player) {
		std::pair<bool, double> durp = m_player->get_dur();
		if (!durp.first) return durp;
		double dur = durp.second;
		if (m_clip_end > 0 && dur > m_clip_end / 1000000.0)
			dur = m_clip_end / 1000000.0;
		dur -= (m_clip_begin / 1000000.0);
		return std::pair<bool, double>(true, dur);
	}
	return std::pair<bool, double>(false, 0.0);
}

bool gui::dx::dx_video_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("stop: %s", m_node->get_xpath().c_str());
	if(!m_player) return true;
	m_cs.enter();
	m_update_event = 0;
	video_player *p = m_player;
	m_player = 0;
	p->stop();
	delete p;
	m_cs.leave();
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	m_dxplayer->stopped(this);
	m_context->stopped(m_cookie);
	return false;
}

void gui::dx::dx_video_renderer::pause(common::pause_display d) {
	AM_DBG lib::logger::get_logger()->debug("dx_video_renderer.pause(0x%x)", this);
	m_update_event = 0;
	if(m_player) m_player->pause();
}

void gui::dx::dx_video_renderer::resume() {
	AM_DBG lib::logger::get_logger()->debug("dx_video_renderer.resume(0x%x)", this);
	if(m_player) m_player->resume();
	if(!m_update_event) schedule_update();
	m_dest->need_redraw();
}

bool gui::dx::dx_video_renderer::user_event(const lib::point& pt, int what) {
	if (!user_event_sensitive(pt)) return false;
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	return true;
}

void gui::dx::dx_video_renderer::redraw(const lib::rect &dirty, common::gui_window *window) {
	if(!m_player || !m_player->can_play() || !m_update_event) {
		// No bits available
		return;
	}

	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) return;

	// Update our bits.
	if(!m_player->update()) {
		// next time please...
		return;
	}

	// Get fit rectangles
	lib::rect vid_rect1;
	lib::rect vid_reg_rc = m_dest->get_fit_rect(m_player->get_size(), &vid_rect1, m_alignment);

	// Use one type of rect to do op
	lib::rect vid_rect(vid_rect1);

	// A complete repaint would be:
	// vid_rect -> vid_reg_rc

	// We have to paint only the intersection.
	// Otherwise we will override upper layers
	lib::rect vid_reg_rc_dirty = vid_reg_rc & dirty;
	if(vid_reg_rc_dirty.empty()) {
		// this renderer has no pixels for the dirty rect
		return;
	}

	// Find the part of the image that is mapped to img_reg_rc_dirty
	lib::rect vid_rect_dirty = reverse_transform(&vid_reg_rc_dirty,
		&vid_rect, &vid_reg_rc);


	// Translate vid_reg_rc_dirty to viewport coordinates
	lib::point pt = m_dest->get_global_topleft();
	vid_reg_rc_dirty.translate(pt);

	dx_transition *tr = get_transition();
	if (tr && tr->is_fullscreen()) {
		v->set_fullscreen_transition(tr);
		tr = NULL;
	}

	// Finally blit img_rect_dirty to img_reg_rc_dirty
	//AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::redraw %0x %s", m_dest, m_node->get_url("src").c_str());
	v->draw(m_player->get_ddsurf(), vid_rect_dirty, vid_reg_rc_dirty, false, tr);

}

void gui::dx::dx_video_renderer::update_callback() {
	// Schedule a redraw callback
	m_cs.enter();
	if(!m_update_event || !m_player) {
		m_cs.leave();
		return;
	}
	m_dest->need_redraw();
	bool need_callback = m_player->is_playing();
	if (need_callback && m_clip_end > 0) {
		// Also check that we haven't gone past clipEnd yet
		double mediatime = m_player->get_position();
		if (mediatime > (m_clip_end / 1000000.0)) {
			m_player->stop();
			need_callback = false;
		}
	}
	m_cs.leave();

	if( need_callback ) {
		schedule_update();
	} else {
		m_update_event = 0;
		m_context->stopped(m_cookie);
	}
}

void gui::dx::dx_video_renderer::schedule_update() {
	m_update_event = new lib::no_arg_callback<dx_video_renderer>(this,
		&dx_video_renderer::update_callback);
	m_event_processor->add_event(m_update_event, m_frametime/2, lib::ep_high);
}
