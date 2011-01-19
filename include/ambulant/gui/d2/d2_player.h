/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2010 Stichting CWI,
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

/*
 * @$Id$
 */

#ifndef AMBULANT_GUI_D2_PLAYER_H
#define AMBULANT_GUI_D2_PLAYER_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"

#include <string>
#include <map>
#include <stack>

// The interfaces implemented by d2_player
#include "ambulant/common/player.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/embedder.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/lib/timer.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/url.h"
#include "ambulant/gui/dx/html_bridge.h"
//#include "ambulant/gui/d2/d2_playable.h"

#if 0
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

interface ID2D1Factory;
interface ID2D1HwndRenderTarget;
interface ID2D1RenderTarget;
interface ID2D1Bitmap;
interface IWICBitmap;

namespace ambulant {

// classes used by d2_player

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

namespace d2 {


class d2_window;
class d2_transition;

/// Abstract class to be implemented by renderers that allocate
/// Direct2D resources.
/// After registering the class with the d2_player, the discard_d2d
/// method will be called when Direct2D signals that cached resources
/// are no longer valid.
class AMBULANTAPI d2_resources {
public:
	virtual void recreate_d2d() = 0;
	virtual void discard_d2d() = 0;
};

/// This is the callback interface to obtain (partial) screenshots
/// from the rendered content.
class AMBULANTAPI d2_capture_callback {
public:
	virtual void captured(IWICBitmap *bitmap) = 0;
};

class d2_player_callbacks : public html_browser_factory {
  public:
	virtual HWND new_os_window() = 0;
	virtual void destroy_os_window(HWND hwnd) = 0;
	virtual SIZE get_default_size() = 0;
};

class AMBULANTAPI d2_player :
	public common::gui_player,
	public common::window_factory,
	public common::playable_factory_machdep,
	public common::embedder,
	public lib::event_processor_observer,
	public d2_capture_callback
{

  public:
	d2_player(d2_player_callbacks &hoster, common::player_feedback *feedback, const net::url& u);
	~d2_player();

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

	common::window_factory *get_window_factory() { return this;}

	void redraw(HWND hwnd, HDC hdc, RECT *dirty=NULL);


	///////////////////
	// Timeslices services and transitions
	void stopped(common::playable *p);
	void paused(common::playable *p);
	void resumed(common::playable *p);
	void set_intransition(common::playable *p, const lib::transition_info *info);
	void start_outtransition(common::playable *p, const lib::transition_info *info);

	void lock_redraw();
	void unlock_redraw();

	// Direct2D resource management
	void register_resources(d2_resources *resource) {
		m_resources.insert(resource);
	}
	void unregister_resources(d2_resources *resource) {
		m_resources.erase(resource);
	}

	// Schedule a capture of the output are
	void schedule_capture(lib::rect area, d2_capture_callback *cb) {
		m_captures.push_back(std::pair<lib::rect, d2_capture_callback *>(area, cb));
	}

	// Global capture-callback: saves snapshots, keeps bitmap for transitions, etc.
	void captured(IWICBitmap *bitmap);

	// Get current rendertarget, only valid while redrawing
	ID2D1HwndRenderTarget *get_rendertarget() {
		return m_cur_wininfo?m_cur_wininfo->m_rendertarget:NULL;
	}
	// Get current hwnd, only valid while redrawing
	HWND get_hwnd() {
		return m_cur_wininfo?m_cur_wininfo->m_hwnd:_get_main_window();
	}
  private:
	bool _calc_fit(const RECT& dstrect, const lib::size& srcsize, float& xoff, float& yoff, float& fac);

	// Structure to keep hwnd/window and rendertarget together
	struct wininfo {
		HWND m_hwnd;
		RECT m_rect;
		ID2D1HwndRenderTarget *m_rendertarget;
		d2_window *m_window;
	};
	// Valid only during redraw():
	wininfo* m_cur_wininfo;
	wininfo* _get_wininfo(HWND hwnd);
	wininfo* _get_wininfo(d2_window *window);
	common::gui_window* _get_window(HWND hwnd);
	HWND _get_main_window();

    // Our Direct2D glue
    ID2D1Factory *m_d2d;
	void _recreate_d2d(wininfo *wi);
	void _discard_d2d();

	// Transition handling
	lib::event *m_update_event;
	void _update_callback();
	void _schedule_update();
	void _update_transitions();
	void _clear_transitions();
	bool _has_transitions() const;
	d2_transition *_get_transition(common::playable *p);
	d2_transition *_set_transition(common::playable *p, const lib::transition_info *info, bool is_outtransition);

	// Capturing screen output
	ID2D1Bitmap *_capture_bitmap(lib::rect r, ID2D1RenderTarget *src_rt, ID2D1RenderTarget *dst_rt);
	IWICBitmap *_capture_wic(lib::rect r, ID2D1RenderTarget *src_rt);
	std::list<std::pair<lib::rect, d2_capture_callback *> > m_captures;

	// The hosting application
	d2_player_callbacks &m_hoster;

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

	typedef std::map<common::playable *, d2_transition*> trmap_t;
	trmap_t m_trmap;
	lib::critical_section m_trmap_cs;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	// Direct2D resource management
	std::set<d2_resources*> m_resources;

	// The logger
	lib::logger *m_logger;
};

} // namespace d2

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_D2_PLAYER_H
