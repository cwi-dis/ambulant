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
 
#include "ambulant/gui/dx/dx_video.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_video_player.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_asb.h"
#include "ambulant/lib/textptr.h"

#include "ambulant/common/region_info.h"

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dx::dx_video_renderer::dx_video_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::gui_window *window,
	dx_playables_context *dxplayer)
:   dx_renderer_playable(context, cookie, node, evp, window, dxplayer),
	m_player(0), 
	m_update_event(0) {
	AM_DBG lib::logger::get_logger()->trace("dx_video_renderer(0x%x)", this);
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();	
	net::url url = m_node->get_url("src");
	if(url.is_local_file() || lib::win32::file_exists(url.get_file())) {
		m_player = new gui::dx::video_player(url.get_file(), v->get_direct_draw());
	} else if(url.is_absolute()) {
		m_player = new gui::dx::video_player(url.get_url(), v->get_direct_draw());
	} else {
		lib::logger::get_logger()->show("The location specified for the data source does not exist. [%s]",
			url.get_url().c_str());
	}
}

gui::dx::dx_video_renderer::~dx_video_renderer() {
	AM_DBG lib::logger::get_logger()->trace("~dx_video_renderer(0x%x)", this);
	if(m_player) stop();
}

void gui::dx::dx_video_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->trace("start: %s", m_node->get_path_display_desc().c_str()); 
	
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
		m_player->start(t);
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
	m_player->start(t);
	m_player->update();
		
	// Request a redraw
	m_dest->need_redraw();
		
	// Notify the scheduler; may take benefit
	m_context->started(m_cookie);
		
	// Schedule a self-update
	schedule_update();
}

std::pair<bool, double> gui::dx::dx_video_renderer::get_dur() {
	if(m_player) return m_player->get_dur();
	return std::pair<bool, double>(false, 0.0);
}

void gui::dx::dx_video_renderer::stop() {
	AM_DBG lib::logger::get_logger()->trace("stop: %s", m_node->get_path_display_desc().c_str()); 
	if(!m_player) return;
	m_cs.enter();
	m_update_event = 0;
	video_player *p = m_player;
	m_player = 0;
	p->stop();
	delete p;
	m_cs.leave();
	m_dest->renderer_done(this);
	m_activated = false;
	m_dxplayer->stopped(this);
}

void gui::dx::dx_video_renderer::pause() {
	AM_DBG lib::logger::get_logger()->trace("dx_video_renderer.pause(0x%x)", this);
	m_update_event = 0;
	if(m_player) m_player->pause();
}

void gui::dx::dx_video_renderer::resume() {
	AM_DBG lib::logger::get_logger()->trace("dx_video_renderer.resume(0x%x)", this);
	if(m_player) m_player->resume();
	if(!m_update_event) schedule_update();
	m_dest->need_redraw();
}

void gui::dx::dx_video_renderer::user_event(const lib::point& pt, int what) {
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
}

void gui::dx::dx_video_renderer::redraw(const lib::screen_rect<int> &dirty, common::gui_window *window) {
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
	lib::screen_rect<int> vid_reg_rc = m_dest->get_fit_rect(m_player->get_size(), &vid_rect1, m_alignment);
	
	// Use one type of rect to do op
	lib::screen_rect<int> vid_rect(vid_rect1);
	
	// A complete repaint would be:  
	// vid_rect -> vid_reg_rc
	
	// We have to paint only the intersection.
	// Otherwise we will override upper layers 
	lib::screen_rect<int> vid_reg_rc_dirty = vid_reg_rc & dirty;
	if(vid_reg_rc_dirty.empty()) {
		// this renderer has no pixels for the dirty rect
		return;
	}	
		
	// Find the part of the image that is mapped to img_reg_rc_dirty
	lib::screen_rect<int> vid_rect_dirty = reverse_transform(&vid_reg_rc_dirty, 
		&vid_rect, &vid_reg_rc);
		
	
	// Translate vid_reg_rc_dirty to viewport coordinates 
	lib::point pt = m_dest->get_global_topleft();
	vid_reg_rc_dirty.translate(pt);
	
	// keep debug message area
	m_msg_rect |= vid_reg_rc_dirty;
	
	dx_transition *tr = 0;
	if(m_transitioning) {
		tr = m_dxplayer->get_transition(this);
		m_transitioning = tr?true:false;
	}
	
	// Finally blit img_rect_dirty to img_reg_rc_dirty
	//AM_DBG lib::logger::get_logger()->trace("dx_img_renderer::redraw %0x %s", m_dest, m_node->get_url("src").c_str());
	v->draw(m_player->get_ddsurf(), vid_rect_dirty, vid_reg_rc_dirty, false, tr);
		
	AM_DBG 	{
		std::basic_string<text_char> msg = m_node->get_path_display_desc();
		v->draw(msg, vid_reg_rc_dirty, lib::to_color("orange"));
	}
}

void gui::dx::dx_video_renderer::update_callback() {
	// Schedule a redraw callback 
	if(!m_update_event || !m_player) {
		return;
	}
	m_cs.enter();
	m_dest->need_redraw();
	m_cs.leave();
	
	if(m_player->is_playing()) {
		schedule_update();
	} else {
		m_update_event = 0;
		m_context->stopped(m_cookie);
	}
}

void gui::dx::dx_video_renderer::schedule_update() {
	m_update_event = new lib::no_arg_callback<dx_video_renderer>(this, 
		&dx_video_renderer::update_callback);
	m_event_processor->add_event(m_update_event, 50);
}
