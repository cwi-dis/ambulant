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

#include "ambulant/gui/dx/dx_player.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_wmuser.h"

#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/document.h"

#include "ambulant/common/region.h"
#include "ambulant/smil2/smil_layout.h"
#include "ambulant/smil2/region_node.h"

// Players
#include "ambulant/smil2/smil_player.h"
#include "ambulant/mms/mms_player.h"
#include "ambulant/smil2/test_attrs.h"

// Renderer playables
#include "ambulant/gui/dx/dx_bgrenderer.h"
#include "ambulant/gui/dx/dx_text.h"
#include "ambulant/gui/dx/dx_img.h"
#include "ambulant/gui/dx/dx_audio.h"
#include "ambulant/gui/dx/dx_video.h"
#include "ambulant/gui/dx/dx_brush.h"

// "Renderer" playables
#include "ambulant/gui/dx/dx_audio.h"

// Playables
#include "ambulant/gui/dx/dx_area.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;


gui::dx::dx_player::dx_player(const std::string& url) 
:	m_url(url),
	m_player(0),
	m_logger(lib::logger::get_logger()) {
	
	// Parse the provided URL. 
	AM_DBG m_logger->trace("Parsing: %s", m_url.c_str());	
	lib::document *doc = lib::document::create_from_file(m_url);
	if(!doc) {
		m_logger->show("Failed to parse document %s", m_url.c_str());
		return;
	}
	
	// Create a player instance
	AM_DBG m_logger->trace("Creating player instance for: %s", m_url.c_str());	
	m_player = new smil2::smil_player(doc, this, this);	
}

gui::dx::dx_player::~dx_player() {
	if(m_player) stop();
	delete m_player;
}

void gui::dx::dx_player::start() {
	if(m_player) m_player->start();
}

void gui::dx::dx_player::stop() {
	if(m_player) {
		m_player->stop();
		on_done();
	}
}

void gui::dx::dx_player::pause() {
	if(m_player) {
		if(m_player->is_playing())
			m_player->pause();
		else if(m_player->is_pausing())
			m_player->resume();
	}
}

void gui::dx::dx_player::resume() {
	m_logger->trace("Attempting to resume: %s", m_url.c_str());
	if(m_player) m_player->resume();
}

bool gui::dx::dx_player::is_playing() const {
	return m_player && m_player->is_playing();
}

bool gui::dx::dx_player::is_pausing() const {
	return m_player && m_player->is_pausing();
}

bool gui::dx::dx_player::is_done() const {
	return m_player && m_player->is_done();
}

void gui::dx::dx_player::set_preferences(const std::string& url) {
	smil2::test_attrs::load_test_attrs(url);
	m_player->build_timegraph();
}

void gui::dx::dx_player::on_click(int x, int y, HWND hwnd) {
	// locate common::abstract_window from hwnd
	POINT pt = {0, 0}; // margins
	if(m_player) m_player->on_click(x-pt.x, y-pt.y, 0);
}

int gui::dx::dx_player::get_cursor(int x, int y, HWND hwnd) {
	// locate common::abstract_window from hwnd
	if(!m_player || !is_playing()) return 0;
	return m_player->get_cursor(x, y, 0);
}
	
void gui::dx::dx_player::on_char(int ch) {
	if(m_player) m_player->on_char(ch);
}


void gui::dx::dx_player::redraw() {
	//if(m_viewport) m_viewport->redraw();
}

void gui::dx::dx_player::on_done() {
	/*
	if(m_viewport) {
		m_viewport->clear();
		m_viewport->redraw();
	}*/
}

////////////////////
// common::window_factory implementation

common::abstract_window *
gui::dx::dx_player::new_window(const std::string &name, 
	lib::size bounds, common::renderer *region) {
	
	AM_DBG lib::logger::get_logger()->trace("dx_window_factory::new_window(%s): %s", 
		name.c_str(), repr(bounds).c_str());
	
	// wininfo struct that will hold the associated objects
	wininfo *winfo = new wininfo;
	
	// Create an os window
	winfo->h = ::new_os_window();
	
	// Create the associated dx viewport
	winfo->v = create_viewport(bounds.w, bounds.h, winfo->h);
	
	// XXX: Wrong arg in new_window() interface!
	common::passive_region *rl = (common::passive_region *) region;
	
	// Clear the viewport
	const common::region_info *ri = rl->get_info();
	winfo->v->set_background(ri?ri->get_bgcolor():CLR_INVALID);
	winfo->v->clear();
	
	// Create a concrete abstract_window
	winfo->w = new dx_window(name, bounds, region, this, winfo->v);
	winfo->f = 0;
	
	// Store the wininfo struct
	m_windows[winfo->w] = winfo;
	
	// Return abstract_window
	return winfo->w;
}

void 
gui::dx::dx_player::window_done(common::abstract_window *w) {
	// called when the window is destructed
}

common::gui_region*
gui::dx::dx_player::new_mouse_region() {
	return 0;
}

common::renderer*
gui::dx::dx_player::new_background_renderer(const common::region_info *src) {
	return new dx_bgrenderer(src);
}

gui::dx::viewport* gui::dx::dx_player::create_viewport(int w, int h, HWND hwnd) {
	AM_DBG m_logger->trace("dx_player::create_viewport(%d, %d)", w, h);
	PostMessage(hwnd, WM_SET_CLIENT_RECT, w, h);
	viewport *v = new gui::dx::viewport(w, h, hwnd);
	v->redraw();
	return v;
}

////////////////////
// common::playable_factory implementation

common::playable *
gui::dx::dx_player::new_playable(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp) {
	
	AM_DBG lib::logger::get_logger()->trace_stream() 
		<< "dx_playable_factory::new_renderer "  
		<< node->get_qname().second << lib::endl;
	
	common::abstract_window *window = get_window(node);
	
	common::playable *p = 0;
	lib::xml_string tag = node->get_qname().second;
	if(tag == "text") {
		p = new dx_text_renderer(context, cookie, node, evp, window);
	} else if(tag == "img") {
		p = new dx_img_renderer(context, cookie, node, evp, window);
	} else if(tag == "audio") {
		p = new dx_audio_renderer(context, cookie, node, evp, window);
	} else if(tag == "video") {
		p = new dx_video_renderer(context, cookie, node, evp, window);
	} else if(tag == "area") {
		p = new dx_area_renderer(context, cookie, node, evp, window);
	} else if(tag == "brush") {
		p = new dx_brush(context, cookie, node, evp, window);
	} else {
		p = new dx_area_renderer(context, cookie, node, evp, window);
	}
	return p;
}

////////////////////////
// Helper function we need and is not to be available by the current interface

// The region stuff is too complex and convoluted to get this implemented without hacks
common::abstract_window *
gui::dx::dx_player::get_window(const lib::node* n) {
	std::map<dx_window *, wininfo*>::iterator it = m_windows.begin();
	assert(it != m_windows.end());
	wininfo* winfo = (*it).second;
	return winfo->w;
}


