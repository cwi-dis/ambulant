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

#include "ambulant/config/config.h"
#include "ambulant/gui/dx/dx_player.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_wmuser.h"
#include "ambulant/gui/dx/dx_rgn.h"
#include "ambulant/gui/dx/dx_transition.h"

#include "ambulant/lib/event.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/transition_info.h"

//#include "vld.h" // Enable to use Visual Leak Detector... uhm... leak detection?

#include "ambulant/common/plugin_engine.h"

#include "ambulant/smil2/transition.h"

// Players
#include "ambulant/smil2/smil_player.h"
#include "ambulant/smil2/test_attrs.h"

// Renderer playables
#include "ambulant/gui/dx/html_bridge.h"
#include "ambulant/gui/dx/dx_bgrenderer.h"
#include "ambulant/gui/dx/dx_text.h"
#include "ambulant/gui/dx/dx_smiltext.h"
#include "ambulant/gui/dx/dx_html_renderer.h"
#include "ambulant/gui/dx/dx_img.h"
#include "ambulant/gui/dx/dx_brush.h"


#if 1 // Kees: this causes multiple definitions on my Windows 7
#if _MSC_VER == 1500
// This is a workaround for a bug in VS2008/MSSDK, where installation
// order can mess up standard include files.
// See <http://social.msdn.microsoft.com/Forums/en-US/vcgeneral/thread/4bc93a16-4ad5-496c-954c-45efbe4b180b>
// for details.
namespace std {
// TEMPLATE FUNCTION _Swap_adl
template<class _Ty> inline void _Swap_adl(_Ty& _Left, _Ty& _Right) {	// exchange values stored at _Left and _Right, using ADL
	swap(_Left, _Right);
}
}
#endif 
#endif//0

// Select audio renderer to use.
// Multiple selections are possible.
#ifdef WITH_FFMPEG
// Use the datasource-based SDL audio renderer
#define USE_SDL_AUDIO
#endif
// Use the DirectX audio renderer
#define USE_DX_AUDIO

#ifdef USE_SDL_AUDIO
#include "ambulant/gui/SDL/sdl_audio.h"
#endif
#ifdef USE_DX_AUDIO
#include "ambulant/gui/dx/dx_audio.h"
#endif/*WITH_FFMPEG*/

// Select video renderer to use.
// Multiple selections are possible.
#ifdef WITH_FFMPEG
// Define this one to use the datasource-based video renderer
#define USE_DS_VIDEO
#endif
// Define this one to use the minimal DirectX video renderer
// #define USE_BASIC_VIDEO
// Define this one to use the more full-featured DirectX video renderer
#define USE_DX_VIDEO

#ifdef USE_DS_VIDEO
#include "ambulant/gui/dx/dx_dsvideo.h"
#endif
#ifdef USE_BASIC_VIDEO
#include "ambulant/gui/dx/dx_basicvideo.h"
#endif
#ifdef USE_DX_VIDEO
#include "ambulant/gui/dx/dx_video.h"
#endif

// "Renderer" playables
#include "ambulant/gui/dx/dx_audio.h"

// Playables
#include "ambulant/gui/dx/dx_area.h"

// Layout
#include "ambulant/common/region.h"
#include "ambulant/smil2/smil_layout.h"

// Xerces parser
#ifdef WITH_XERCES_BUILTIN
#include "ambulant/lib/xerces_parser.h"
#endif

// Datasources
#include "ambulant/net/datasource.h"
#include "ambulant/net/win32_datasource.h"
#ifdef WITH_FFMPEG
#include "ambulant/net/ffmpeg_factory.h"
#include "ambulant/net/rtsp_factory.h"
#endif
//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

int gui::dx::dx_gui_region::s_counter = 0;

gui::dx::dx_player::dx_player(dx_player_callbacks &hoster, common::focus_feedback *feedback, const net::url& u)
:	m_hoster(hoster),
	m_update_event(0),
	m_logger(lib::logger::get_logger())
{
	set_embedder(this);
	// Fill the factory objects
	init_factories();
	init_plugins();

	// Order the factories according to the preferences
	common::preferences *prefs = common::preferences::get_preferences();
	m_logger->debug("constructing dx player with %s", prefs->repr().c_str());
	if (prefs->m_prefer_ffmpeg)
		get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererOpen"));
	else
		get_playable_factory()->preferred_renderer(AM_SYSTEM_COMPONENT("RendererDirectX"));

	// Parse the provided URL.
	AM_DBG m_logger->debug("Parsing: %s", u.get_url().c_str());
	m_doc = create_document(u);

	if(!m_doc) {
		// message already logged
		return;
	}

	// If there's a fragment ID remember the node it points to,
	// and when we first start playback we'll go there.
	const std::string& idd = u.get_ref();
	if (idd != "") {
		const lib::node *node = m_doc->get_node(idd);
		if (node) {
			goto_node(node);
		} else {
			m_logger->warn(gettext("%s: node ID not found"), idd.c_str());
		}
	}

	// Create a player instance
	AM_DBG m_logger->debug("Creating player instance for: %s", u.get_url().c_str());
	m_player = smil2::create_smil2_player(m_doc, this, m_embedder);

	if (feedback) m_player->set_focus_feedback(feedback);
	m_player->initialize();
	lib::event_processor *evp = m_player->get_evp();
	assert(evp);
	evp->set_observer(this);

	// Create a worker processor instance
}

gui::dx::dx_player::~dx_player() {
	lib::event_processor *evp = NULL;
	if(m_player) stop();
	if (m_player) {
		evp = m_player->get_evp();
		if (evp) evp->set_observer(NULL);
		m_player->terminate();
		m_player->release();
		m_player = NULL;
	}
	while(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_windows = pf->windows;
		m_player = pf->player;
		m_doc = pf->doc;
		delete pf;
		stop();
		if (m_player) {
			evp = m_player->get_evp();
			if (evp) evp->set_observer(NULL);
			m_player->terminate();
			m_player->release();
			m_player = NULL;
		}
		delete m_doc;
	}
	delete m_doc;
	m_player = NULL;
	assert(m_windows.empty());
	if(dx_gui_region::s_counter != 0)
		m_logger->warn("Undeleted gui regions: %d", dx_gui_region::s_counter);
}

void
gui::dx::dx_player::cleanup()
{
	lib::nscontext::cleanup();
	common::global_playable_factory *pf = common::get_global_playable_factory();
	delete pf;
	lib::global_parser_factory *prf = lib::global_parser_factory::get_parser_factory();
	delete prf;
	common::global_state_component_factory *scf = common::get_global_state_component_factory();
	delete scf;
}

void
gui::dx::dx_player::init_playable_factory()
{
	common::global_playable_factory *pf = common::get_global_playable_factory();
	set_playable_factory(pf);
	// Add the playable factory
	pf->add_factory(create_dx_area_playable_factory(this, this));
#ifdef USE_DX_AUDIO
	pf->add_factory(create_dx_audio_playable_factory(this, this));
#endif
#ifdef USE_SDL_AUDIO
	pf->add_factory(gui::sdl::create_sdl_playable_factory(this));
#endif
	pf->add_factory(create_dx_brush_playable_factory(this, this));
	pf->add_factory(create_dx_image_playable_factory(this, this));
	pf->add_factory(create_dx_smiltext_playable_factory(this, this));
	pf->add_factory(create_dx_text_playable_factory(this, this));
#ifdef WITH_HTML_WIDGET
	pf->add_factory(create_dx_html_playable_factory(this, this));
#endif
#ifdef USE_BASIC_VIDEO
	pf->add_factory(create_dx_basicvideo_playable_factory(this, this));
#endif
#ifdef USE_DS_VIDEO
	pf->add_factory(create_dx_dsvideo_playable_factory(this, this));
#endif
#ifdef USE_DX_VIDEO
	pf->add_factory(create_dx_video_playable_factory(this, this));
#endif
}

void
gui::dx::dx_player::init_window_factory()
{
		set_window_factory(this);
}

void
gui::dx::dx_player::init_datasource_factory()
{
	net::datasource_factory *df = new net::datasource_factory();
	set_datasource_factory(df);
#ifdef WITH_LIVE
	AM_DBG m_logger->debug("dx_player: add live_audio_datasource_factory");
	df->add_video_factory(net::create_live_video_datasource_factory());
	df->add_audio_factory(net::create_live_audio_datasource_factory());
#endif
#ifdef WITH_FFMPEG
	AM_DBG m_logger->debug("dx_player: add ffmpeg_audio_datasource_factory");
	df->add_audio_factory(net::get_ffmpeg_audio_datasource_factory());
	AM_DBG m_logger->debug("dx_player: add ffmpeg_audio_decoder_finder");
	df->add_audio_decoder_finder(net::get_ffmpeg_audio_decoder_finder());
	AM_DBG m_logger->debug("dx_player: add ffmpeg_audio_filter_finder");
	df->add_audio_filter_finder(net::get_ffmpeg_audio_filter_finder());
	AM_DBG m_logger->debug("dx_player: add ffmpeg_video_datasource_factory");
	df->add_video_factory(net::get_ffmpeg_video_datasource_factory());
	AM_DBG m_logger->debug("dx_player: add ffmpeg_raw_datasource_factory");
	df->add_raw_factory(net::get_ffmpeg_raw_datasource_factory());
#endif
	df->add_raw_factory(net::get_win32_datasource_factory());

}

void
gui::dx::dx_player::init_parser_factory()
{
	lib::global_parser_factory *pf = lib::global_parser_factory::get_parser_factory();
	set_parser_factory(pf);
	// Add the xerces parser, if available
#ifdef WITH_XERCES_BUILTIN
	pf->add_factory(new lib::xerces_factory());
#endif
}


void gui::dx::dx_player::play() {
	if(m_player) {
		lock_redraw();
		std::map<std::string, wininfo*>::iterator it;
		for(it=m_windows.begin();it!=m_windows.end();it++) {
			dx_window *dxwin = (dx_window *)(*it).second->w;
			dxwin->need_redraw();
		}
		common::gui_player::play();
		unlock_redraw();
	}
}

void gui::dx::dx_player::stop() {
	if(m_player) {
		m_update_event = 0;
		clear_transitions();
		common::gui_player::stop();
	}
}

void gui::dx::dx_player::pause() {
	if(m_player) {
		lock_redraw();
		common::gui_player::pause();
		unlock_redraw();
	}
}

void gui::dx::dx_player::restart(bool reparse) {
	bool playing = is_play_active();
	stop();
	lib::event_processor *evp = m_player->get_evp();
	if (evp) evp->set_observer(NULL);

	assert(m_player);
	m_player->terminate();
	m_player->release();
	m_player = NULL;
	while(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_windows = pf->windows;
		evp = m_player->get_evp();
		if (evp) evp->set_observer(NULL);
		m_player = pf->player;
		assert(m_player);
		m_doc = pf->doc;
		delete pf;
		stop();
		m_player->terminate();
		m_player->release();
		m_player = NULL;
	}
	m_player = 0;
	if (reparse) {
		m_doc = create_document(m_url);
		if(!m_doc) {
			m_logger->show("Failed to parse document %s", m_url.get_url().c_str());
			return;
		}
	}
	AM_DBG m_logger->debug("Creating player instance for: %s", m_url.get_url().c_str());
	m_player = smil2::create_smil2_player(m_doc, this, m_embedder);
	m_player->initialize();
	evp = m_player->get_evp();
	if (evp) evp->set_observer(this);
	if(playing) play();
}

void gui::dx::dx_player::on_click(int x, int y, HWND hwnd) {
	if(!m_player) return;
	lib::point pt(x, y);
	dx_window *dxwin = (dx_window *) get_window(hwnd);
	if(!dxwin) return;
	common::gui_events *r = dxwin->get_gui_events();
	if(r)
		r->user_event(pt, common::user_event_click);
}

int gui::dx::dx_player::get_cursor(int x, int y, HWND hwnd) {
	if(!m_player) return 0;
	lib::point pt(x, y);
	dx_window *dxwin = (dx_window *) get_window(hwnd);
	if(!dxwin) return 0;
	common::gui_events *r = dxwin->get_gui_events();
	m_player->before_mousemove(0);
	if(r) r->user_event(pt, common::user_event_mouse_over);
	return m_player->after_mousemove();
}

std::string gui::dx::dx_player::get_pointed_node_str() {
#if 1
	return "";
#else
	if(!m_player || !is_playing()) return "";
	return m_player->get_pointed_node_str();
#endif
}

void gui::dx::dx_player::on_char(int ch) {
	if(m_player) m_player->on_char(ch);
}

void gui::dx::dx_player::redraw(HWND hwnd, HDC hdc, RECT *dirty) {
	wininfo *wi = get_wininfo(hwnd);
	if(wi) wi->v->redraw(hdc);
}

void gui::dx::dx_player::on_done() {
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		(*it).second->v->clear();
		(*it).second->v->redraw();
	}
}

void gui::dx::dx_player::lock_redraw() {
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		(*it).second->w->lock_redraw();
	}
}

void gui::dx::dx_player::unlock_redraw() {
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		(*it).second->w->unlock_redraw();
	}
}

////////////////////
// common::window_factory implementation

common::gui_window *
gui::dx::dx_player::new_window(const std::string &name,
	lib::size bounds, common::gui_events *src) {

	AM_DBG lib::logger::get_logger()->debug("dx_window_factory::new_window(%s): %s",
		name.c_str(), ::repr(bounds).c_str());

	// wininfo struct that will hold the associated objects
	wininfo *winfo = new wininfo;

	// Create an os window
	winfo->h = m_hoster.new_os_window();

	// Create the associated dx viewport
	winfo->v = create_viewport(bounds.w, bounds.h, winfo->h);

	// Clear the viewport
	winfo->v->set_background(dxparams::I()->invalid_color());
	winfo->v->clear();

	// Create a concrete gui_window
	winfo->w = new dx_window(name, bounds, src, this, winfo->v);
	winfo->f = 0;

	// Store the wininfo struct
	m_windows[name] = winfo;
	AM_DBG m_logger->debug("windows: %d", m_windows.size());

	// Return gui_window
	return winfo->w;
}

void
gui::dx::dx_player::window_done(const std::string &name) {
	// called when the window is destructed (wi->w)
	std::map<std::string, wininfo*>::iterator it = m_windows.find(name);
	assert(it != m_windows.end());
	wininfo *wi = (*it).second;
	m_windows.erase(it);
	wi->v->clear();
	wi->v->redraw();
	delete wi->v;
	m_hoster.destroy_os_window(wi->h);
	delete wi;
	AM_DBG m_logger->debug("windows: %d", m_windows.size());
}

lib::size
gui::dx::dx_player::get_default_size() {
	SIZE sz = m_hoster.get_default_size();
	if (sz.cx != 0 && sz.cy != 0)
		return lib::size(sz.cx, sz.cy);
	return lib::size(common::default_layout_width, common::default_layout_height);
}

common::bgrenderer*
gui::dx::dx_player::new_background_renderer(const common::region_info *src) {
	return new dx_bgrenderer(src);
}

gui::dx::viewport* gui::dx::dx_player::create_viewport(int w, int h, HWND hwnd) {
	AM_DBG m_logger->debug("dx_player::create_viewport(%d, %d)", w, h);
	PostMessage(hwnd, WM_SET_CLIENT_RECT, w, h);
	viewport *v = new gui::dx::viewport(w, h, hwnd);
	v->redraw();
	return v;
}

gui::dx::dx_player::wininfo*
gui::dx::dx_player::get_wininfo(HWND hwnd) {
	wininfo *winfo = 0;
	std::map<std::string, wininfo*>::iterator it;
	for(it=m_windows.begin();it!=m_windows.end();it++) {
		wininfo *wi = (*it).second;
		if(wi->h = hwnd) {winfo = wi;break;}
	}
	return winfo;
}

common::gui_window *
gui::dx::dx_player::get_window(HWND hwnd) {
	wininfo *wi = get_wininfo(hwnd);
	return wi?wi->w:0;
}

HWND
gui::dx::dx_player::get_main_window() {
	// XXXX Unsure that this is correct: we just return any window
	std::map<std::string, wininfo*>::iterator it = m_windows.begin();
	if (it == m_windows.end()) return NULL;
	return (*it).second->h;
}

void gui::dx::dx_player::set_intransition(common::playable *p, const lib::transition_info *info) {
	AM_DBG lib::logger::get_logger()->debug("set_intransition : %s", repr(info->m_type).c_str());
	dx_transition *tr = set_transition(p, info, false);
	// XXXX Note by Jack: the next two steps really shouldn't be done for
	// intransitions, they should be done later (when playback starts).
	tr->first_step();
	if(!m_update_event) schedule_update();
}

void gui::dx::dx_player::start_outtransition(common::playable *p, const lib::transition_info *info) {
	lib::logger::get_logger()->debug("start_outtransition : %s", repr(info->m_type).c_str());
	dx_transition *tr = set_transition(p, info, true);
	// XXXX Note by Jack: the next two steps really shouldn't be done for
	// intransitions, they should be done later (when playback starts).
	tr->first_step();
	if(!m_update_event) schedule_update();
}

gui::dx::dx_transition *
gui::dx::dx_player::set_transition(common::playable *p,
	const lib::transition_info *info,
	bool is_outtransition)
{
	assert(m_player);
	lib::timer_control *timer = new lib::timer_control_impl(m_player->get_timer(), 1.0, false);
	dx_transition *tr = make_transition(info->m_type, p, timer);
	m_trmap[p] = tr;
	common::surface *surf = p->get_renderer()->get_surface();
	if (info->m_scope == lib::scope_screen) surf = surf->get_top_surface();
	tr->init(surf, is_outtransition, info);
	return tr;
}

bool gui::dx::dx_player::has_transitions() const {
	return !m_trmap.empty();
}

void gui::dx::dx_player::update_transitions() {
	m_trmap_cs.enter();
	assert(m_player);
	lib::timer::time_type pt = m_player->get_timer()->elapsed();
	//lock_redraw();
	AM_DBG lib::logger::get_logger()->debug("update_transitions: updating %d transitions", m_trmap.size());
	// First make all transitions do a step.
	std::vector<common::playable*> finished_transitions;
	for(trmap_t::iterator it=m_trmap.begin();it!=m_trmap.end();it++) {
		if(!(*it).second->next_step(pt)) {
			finished_transitions.push_back((*it).first);
		}
	}
	// Next clean up all finished transitions
	std::vector<common::playable*>::iterator pit;
	for (pit=finished_transitions.begin(); pit!= finished_transitions.end(); pit++) {
		// Find the transition map entry and clear it
		trmap_t::iterator it=m_trmap.find(*pit);
		assert(it != m_trmap.end());
		if (it == m_trmap.end()) continue;
		delete (*it).second;
		it = m_trmap.erase(it);
		// Find the surface and tell it a transition has finished.
		common::renderer *r = (*pit)->get_renderer();
		if (r) {
			common::surface *surf = r->get_surface();
			if (surf) surf->transition_done();
		}
	}
	//unlock_redraw();
	m_trmap_cs.leave();
}

void gui::dx::dx_player::clear_transitions() {
	m_trmap_cs.enter();
	for(trmap_t::iterator it=m_trmap.begin();it!=m_trmap.end();it++)
		delete (*it).second;
	m_trmap.clear();
	m_trmap_cs.leave();
}

gui::dx::dx_transition *gui::dx::dx_player::get_transition(common::playable *p) {
	trmap_t::iterator it = m_trmap.find(p);
	return (it != m_trmap.end())?(*it).second:0;
}

void gui::dx::dx_player::stopped(common::playable *p) {
	m_trmap_cs.enter();
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		delete (*it).second;
		common::playable *p = (*it).first;
		it = m_trmap.erase(it);
		common::renderer *r = p->get_renderer();
		if (r) {
			common::surface *surf = r->get_surface();
			if (surf) surf->transition_done();
		}
	}
	m_trmap_cs.leave();
}

void gui::dx::dx_player::paused(common::playable *p) {
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		(*it).second->pause();
	}
}

void gui::dx::dx_player::resumed(common::playable *p) {
	trmap_t::iterator it = m_trmap.find(p);
	if(it != m_trmap.end()) {
		(*it).second->resume();
	}
}

void gui::dx::dx_player::update_callback() {
	if(!m_update_event) return;
	if(has_transitions()) {
		update_transitions();
		schedule_update();
	} else {
		m_update_event = 0;
	}
}

void gui::dx::dx_player::schedule_update() {
	if(!m_player) return;
	lib::event_processor *evp = m_player->get_evp();
	m_update_event = new lib::no_arg_callback_event<dx_player>(this,
		&dx_player::update_callback);
	evp->add_event(m_update_event, 50, lib::ep_high);
}

void gui::dx::dx_player::show_file(const net::url& href) {
	ShellExecute(GetDesktopWindow(), text_str("open"), lib::textptr(href.get_url().c_str()), NULL, NULL, SW_SHOWNORMAL);
}

void gui::dx::dx_player::done(common::player *p) {
	m_update_event = 0;
	clear_transitions();
	if(!m_frames.empty()) {
		frame *pf = m_frames.top();
		m_frames.pop();
		m_windows = pf->windows;
		m_player = pf->player;
		m_doc = pf->doc;
		delete pf;
		assert(0); // resume();
		std::map<std::string, wininfo*>::iterator it;
		for(it=m_windows.begin();it!=m_windows.end();it++) {
			dx_window *dxwin = (dx_window *)(*it).second->w;
			dxwin->need_redraw();
		}
	}
}

void gui::dx::dx_player::close(common::player *p) {
	PostMessage(get_main_window(), WM_CLOSE, 0, 0);
}

void gui::dx::dx_player::open(net::url newdoc, bool startnewdoc, common::player *old) {
	std::string urlstr = newdoc.get_url();
	if(old) {
		// Replace the current document
		PostMessage(get_main_window(), WM_REPLACE_DOC,
			startnewdoc?1:0, LPARAM(new std::string(urlstr)));
		return;
	}
	if(!lib::ends_with(urlstr, ".smil") && !lib::ends_with(urlstr, ".smi") &&
		!lib::ends_with(urlstr, ".grins")) {
		show_file(newdoc);
		return;
	}

	// Parse the document.
	m_doc = create_document(newdoc);
	if (!m_doc) return;

	// Push the old frame on the stack
	if(m_player) {
		pause();
		frame *pf = new frame();
		pf->windows = m_windows;
		pf->player = m_player;
		pf->doc = m_doc;
		m_windows.clear();
		m_player = 0;
		m_frames.push(pf);
	}

	// Create a player instance
	AM_DBG m_logger->debug("Creating player instance for: %s", newdoc.get_url().c_str());
	m_logger->debug("creating smil2 player with %s", ambulant::common::preferences::get_preferences()->repr());
	m_player = smil2::create_smil2_player(m_doc, this, m_embedder);
	m_player->initialize();
	lib::event_processor *evp = m_player->get_evp();
	assert(evp);
	evp->set_observer(this);
	if(startnewdoc) play();
}

