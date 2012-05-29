/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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
#include "ambulant/common/gui_player.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/url.h"
#include "ambulant/gui/dx/html_bridge.h"
#include "ambulant/gui/dx/dx_playable.h"

namespace ambulant {

// classes used by dx_player

namespace lib {
	class event_processor;
	class logger;
	class transition_info;
	class event;
}

namespace smil2 {
	class smil_player;
}

namespace gui {

namespace dx {

class viewport;
class dx_window;
class dx_transition;


class dx_player_callbacks : public html_browser_factory {
  public:
	virtual HWND new_os_window() = 0;
	virtual void destroy_os_window(HWND hwnd) = 0;
	virtual SIZE get_default_size() = 0;
};

class AMBULANTAPI dx_player :
	public common::gui_player,
	public common::window_factory,
	public dx_playables_context,
	public common::embedder,
	public lib::event_processor_observer
{

  public:
	dx_player(dx_player_callbacks &hoster, common::focus_feedback *feedback, const net::url& u);
	~dx_player();

	/// Call on application termination
	static void cleanup();

	////////////////////
	// common::gui_player implementation
	void init_playable_factory();
	void init_window_factory();
	void init_datasource_factory();
	void init_parser_factory();

	void play();
	void stop();
	void pause();

	void restart(bool reparse=true);

	void set_preferences(const std::string& url);

	////////////////////
	// common::window_factory implementation

	common::gui_window *new_window(const std::string& name,
		lib::size bounds, common::gui_events *src);

	common::bgrenderer *new_background_renderer(const common::region_info *src);

	void window_done(const std::string& name);

	lib::size get_default_size();

	////////////////////
	// common::embedder implementation
	void show_file(const net::url& href);
	void close(common::player *p);
	void open(net::url newdoc, bool start, common::player *old=NULL);
	void done(common::player *p);
	html_browser_factory *get_html_browser_factory() { return &m_hoster; }

	////////////////////
	// Implementation specific artifacts

	void on_char(int ch);
	void on_click(int x, int y, HWND hwnd);
	int get_cursor(int x, int y, HWND hwnd);
	std::string get_pointed_node_str();
//	const net::url& get_url() const { return m_url;}
	void on_done();
	void on_zoom(double factor, HWND hwnd) {}; // Not implemented for dx

	common::window_factory *get_window_factory() { return this;}
	viewport* create_viewport(int w, int h, HWND hwnd);
	void redraw(HWND hwnd, HDC hdc, RECT *dirty);

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
	common::gui_window* get_window(HWND hwnd);
	HWND get_main_window();
	void lock_redraw();
	void unlock_redraw();

	// The hosting application
	dx_player_callbacks &m_hoster;
	// The current document URL
//	net::url m_url;
	// The current view
	struct wininfo {HWND h; viewport *v; dx_window *w; long f;};
	wininfo* get_wininfo(HWND hwnd);

	lib::event *m_update_event;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

	// The frames stack
	struct frame {
		std::map<std::string, wininfo*> windows;
		common::player* player;
		lib::document* doc;
	};
	std::map<std::string, wininfo*> m_windows;
	std::stack<frame*> m_frames;

	typedef std::map<common::playable *, dx_transition*> trmap_t;
	trmap_t m_trmap;
	lib::critical_section m_trmap_cs;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	lib::logger *m_logger;
};

} // namespace dx

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_DX_PLAYER_H
