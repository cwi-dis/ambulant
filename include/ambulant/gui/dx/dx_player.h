/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_DX_PLAYER_H
#define AMBULANT_GUI_DX_PLAYER_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"

#include <string>
#include <map>
#include <stack>

// The interfaces implemented by dx_player
#include "ambulant/common/player.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/embedder.h"
#include "ambulant/common/factory.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/url.h"
#include "ambulant/gui/dx/dx_playable.h"

namespace ambulant {

// classes used by dx_player

namespace lib {
	class event_processor;
	class logger;
	class transition_info;
	class event;
}

namespace mms {
	class mms_player;
}

namespace smil2 {
	class smil_player;
}

namespace gui {

namespace dx {

class viewport;
class dx_window;
class dx_transition;

class dx_player_callbacks {
  public:
	virtual HWND new_os_window() = 0;
	virtual void destroy_os_window(HWND hwnd) = 0;
};

class AMBULANTAPI dx_player : 
	//public common::player, 
	public common::window_factory, 
	public common::playable_factory,
	public dx_playables_context,
	public common::embedder {
	
  public:
	  dx_player(dx_player_callbacks &hoster, common::player_feedback *feedback, const net::url& u);
	~dx_player();
	
	////////////////////
	// common::player implementation
	
	void start();
	void stop();
	void pause();
	void resume();
	void restart();

	bool is_playing() const;
	bool is_pausing() const;
	bool is_done() const;

	void set_preferences(const std::string& url);
	
	// should these be part of the player interface?
	lib::timer* get_timer() { return 0;}
	lib::event_processor* get_evp() { return 0;}
	
	
	////////////////////
	// common::window_factory implementation
	
	common::gui_window *new_window(const std::string& name, 
		lib::size bounds, common::gui_events *src);
		
	common::bgrenderer *new_background_renderer(const common::region_info *src);
	
	void window_done(const std::string& name);
	
	////////////////////
	// common::playable_factory implementation
	
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const ambulant::lib::node *node,
		lib::event_processor * evp);

	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src);
	
	////////////////////
	// common::embedder implementation
	void show_file(const net::url& href);
	void close(common::player *p);
	void open(net::url newdoc, bool start, common::player *old=NULL);
	void done(common::player *p);
	
	////////////////////
	// Implementation specific artifacts
	
	void on_char(int ch);
	void on_click(int x, int y, HWND hwnd);
	int get_cursor(int x, int y, HWND hwnd);
	std::string get_pointed_node_str();
	const net::url& get_url() const { return m_url;}
	void on_done();
	
	common::window_factory *get_window_factory() { return this;}
	common::playable_factory *get_playable_factory() {return this;}
	viewport* create_viewport(int w, int h, HWND hwnd);
	void redraw(HWND hwnd, HDC hdc);
	
	///////////////////
	// Timeslices services and transitions
	void update_callback();
	void schedule_update();
	void update_transitions();
	void clear_transitions();
	bool has_transitions() const;
	void stopped(common::playable *p);
	void paused(common::playable *p);
	void resumed(common::playable *p);
	void set_intransition(common::playable *p, const lib::transition_info *info);
	void start_outtransition(common::playable *p, const lib::transition_info *info);
	dx_transition *get_transition(common::playable *p);
	
  private:
	dx_transition *set_transition(common::playable *p, const lib::transition_info *info, bool is_outtransition);
	common::gui_window* get_window(const lib::node* n);
	common::gui_window* get_window(HWND hwnd);
	HWND get_main_window();
	void lock_redraw();
	void unlock_redraw();
	
	// The hosting application
	dx_player_callbacks &m_hoster;
	// The current document URL
	net::url m_url;
	
	// The node we want to start playback at.
	const lib::node *m_goto_node;

	// The current SMIL2 player
	smil2::smil_player *m_player;
	
	// The current view	
	struct wininfo {HWND h; viewport *v; dx_window *w; long f;};
	std::map<std::string, wininfo*> m_windows;	
	wininfo* get_wininfo(HWND hwnd);
	
	// The frames stack
	struct frame {std::map<std::string, wininfo*> windows; smil2::smil_player* player;};
	std::stack<frame*> m_frames;
	
	// The secondary timer and processor
	lib::timer_control *m_timer;
	lib::event_processor *m_worker_processor;	
		
	lib::event *m_update_event;
	typedef std::map<common::playable *, dx_transition*> trmap_t;
	trmap_t m_trmap;
	lib::critical_section m_trmap_cs;
	
	lib::logger *m_logger;
	common::factories m_factory;
};

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_PLAYER_H
