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

#include "ambulant/gui/dg/dg_player.h"
#include "ambulant/gui/dg/dg_viewport.h"
#include "ambulant/gui/dg/dg_window.h"
#include "ambulant/gui/dg/dg_wmuser.h"
#include "ambulant/gui/dg/dg_rgn.h"

#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/logger.h"

// Players
#include "ambulant/smil2/smil_player.h"
#include "ambulant/mms/mms_player.h"
#include "ambulant/smil2/test_attrs.h"

// Renderer playables
#include "ambulant/gui/dg/dg_bgrenderer.h"
#include "ambulant/gui/dg/dg_text.h"
#include "ambulant/gui/dg/dg_img.h"
#include "ambulant/gui/dg/dg_brush.h"

// Playables
#include "ambulant/gui/dg/dg_area.h"

// Layout
#include "ambulant/common/region.h"
#include "ambulant/smil2/smil_layout.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

int gui::dg::dg_gui_region::s_counter = 0;

gui::dg::dg_player::dg_player(const std::string& url) 
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

gui::dg::dg_player::~dg_player() {
	if(m_player) stop();
	delete m_player;
	assert(m_windows.empty());
	if(dg_gui_region::s_counter != 0) 
		m_logger->warn("Undeleted gui regions: %d", dg_gui_region::s_counter);
}

void gui::dg::dg_player::start() {
	if(m_player) m_player->start();
}

void gui::dg::dg_player::stop() {
	if(m_player) {
		m_player->stop();
		on_done();
	}
}

void gui::dg::dg_player::pause() {
	if(m_player) {
		if(m_player->is_playing())
			m_player->pause();
		else if(m_player->is_pausing())
			m_player->resume();
	}
}

void gui::dg::dg_player::resume() {
	if(m_player) m_player->resume();
}

bool gui::dg::dg_player::is_playing() const {
	return m_player && m_player->is_playing();
}

bool gui::dg::dg_player::is_pausing() const {
	return m_player && m_player->is_pausing();
}

bool gui::dg::dg_player::is_done() const {
	return m_player && m_player->is_done();
}

void gui::dg::dg_player::set_preferences(const std::string& url) {
	smil2::test_attrs::load_test_attrs(url);
	if(is_playing()) stop();
	if(m_player) m_player->build_timegraph();
}

void gui::dg::dg_player::on_click(int x, int y, HWND hwnd) {
	if(!m_player || !is_playing()) return;
	lib::point pt(x, y);
	dg_window *dgwin = (dg_window *) get_window(hwnd);
	if(!dgwin) return;
	region *r = dgwin->get_region();
	if(r) r->user_event(pt, common::user_event_click);
}

int gui::dg::dg_player::get_cursor(int x, int y, HWND hwnd) {
	if(!m_player || !is_playing()) return 0;
	lib::point pt(x, y);
	dg_window *dgwin = (dg_window *) get_window(hwnd);
	if(!dgwin) return 0;
	region *r = dgwin->get_region();
	m_player->set_cursor(0);
	if(r) r->user_event(pt, common::user_event_mouse_over);
	return m_player->get_cursor();
}
	
void gui::dg::dg_player::on_char(int ch) {
	if(m_player) m_player->on_char(ch);
}

void gui::dg::dg_player::redraw(HWND hwnd, HDC hdc) {
	wininfo *wi = get_wininfo(hwnd);
	if(wi) wi->v->redraw(hdc);
}

void gui::dg::dg_player::on_done() {
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		(*it).second->v->clear();
		(*it).second->v->redraw();
	}
}

////////////////////
// common::window_factory implementation

common::abstract_window *
gui::dg::dg_player::new_window(const std::string &name, 
	lib::size bounds, common::renderer *renderer) {
	
	AM_DBG lib::logger::get_logger()->trace("dx_window_factory::new_window(%s): %s", 
		name.c_str(), repr(bounds).c_str());
	
	// wininfo struct that will hold the associated objects
	wininfo *winfo = new wininfo;
	
	// Create an os window
	winfo->h = ::new_os_window();
	
	// Create the associated dg viewport
	winfo->v = create_viewport(bounds.w, bounds.h, winfo->h);
	
	// XXX: Wrong arg in new_window() interface!
	region *rgn = (region *) renderer;
	
	// Clear the viewport
	const common::region_info *ri = rgn->get_info();
	winfo->v->set_background(ri?ri->get_bgcolor():CLR_INVALID);
	winfo->v->clear();
	
	// Create a concrete abstract_window
	winfo->w = new dg_window(name, bounds, rgn, this, winfo->v);
	winfo->f = 0;
	
	// Store the wininfo struct
	m_windows[name] = winfo;
	AM_DBG m_logger->trace("windows: %d", m_windows.size());
	
	// Return abstract_window
	return winfo->w;
}

void 
gui::dg::dg_player::window_done(const std::string &name) {
	// called when the window is destructed (wi->w)
	std::map<std::string, wininfo*>::iterator it = m_windows.find(name);
	assert(it != m_windows.end());
	wininfo *wi = (*it).second;
	m_windows.erase(it);
	delete wi->v;
	destroy_os_window(wi->h);
	delete wi;
	AM_DBG m_logger->trace("windows: %d", m_windows.size());
}

common::gui_region*
gui::dg::dg_player::new_mouse_region() {
	return new dg_gui_region();
}

common::renderer*
gui::dg::dg_player::new_background_renderer(const common::region_info *src) {
	return new dg_bgrenderer(src);
}

gui::dg::viewport* gui::dg::dg_player::create_viewport(int w, int h, HWND hwnd) {
	AM_DBG m_logger->trace("dg_player::create_viewport(%d, %d)", w, h);
	PostMessage(hwnd, WM_SET_CLIENT_RECT, w, h);
	viewport *v = new gui::dg::viewport(w, h, hwnd);
	v->redraw();
	return v;
}

gui::dg::dg_player::wininfo*
gui::dg::dg_player::get_wininfo(HWND hwnd) {
	wininfo *winfo = 0;
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		wininfo *wi = (*it).second;
		if(wi->h = hwnd) {winfo = wi;break;}
	}
	return winfo;
}

common::abstract_window *
gui::dg::dg_player::get_window(HWND hwnd) {
	wininfo *wi = get_wininfo(hwnd);
	return wi?wi->w:0;
}

////////////////////
// common::playable_factory implementation

common::playable *
gui::dg::dg_player::new_playable(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp) {
	
	common::abstract_window *window = get_window(node);
	common::playable *p = 0;
	lib::xml_string tag = node->get_qname().second;
	if(tag == "text") {
		p = new dg_text_renderer(context, cookie, node, evp, window);
	} else if(tag == "img") {
		p = new dg_img_renderer(context, cookie, node, evp, window);
	} else if(tag == "audio") {
		p = new dg_area_renderer(context, cookie, node, evp, window);
	} else if(tag == "video") {
		p = new dg_area_renderer(context, cookie, node, evp, window);
	} else if(tag == "area") {
		p = new dg_area_renderer(context, cookie, node, evp, window);
	} else if(tag == "brush") {
		p = new dg_brush(context, cookie, node, evp, window);
	} else {
		p = new dg_area_renderer(context, cookie, node, evp, window);
	}
	return p;
}

////////////////////////
// Layout helpers with a lot of hacks

typedef common::surface_template iregion;
typedef common::passive_region region;

static const region* 
get_top_layout(smil2::smil_layout_manager *layout, const lib::node* n) {
	iregion *ir = layout->get_region(n);
	if(!ir) return 0;
	const region *r = (const region*) ir;
	while(r->get_parent()) r = r->get_parent();
	return r;
}

static const char*
get_top_layout_name(smil2::smil_layout_manager *layout, const lib::node* n) {
	const region* r = get_top_layout(layout, n);
	if(!r) return 0;
	const common::region_info *ri = r->get_info();
	return ri?ri->get_name().c_str():0;
}

common::abstract_window *
gui::dg::dg_player::get_window(const lib::node* n) {
	typedef common::surface_template region;
	smil2::smil_layout_manager *layout = m_player->get_layout();
	const char *tlname = get_top_layout_name(layout, n);
	if(tlname) {
		std::map<std::string, wininfo*>::iterator it;
		it = m_windows.find(tlname);
		if(it != m_windows.end())
			return (*it).second->w;
	}
	std::map<std::string, wininfo*>::iterator it = m_windows.begin();
	assert(it != m_windows.end());
	wininfo* winfo = (*it).second;
	return winfo->w;
}







