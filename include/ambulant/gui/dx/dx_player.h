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

#ifndef AMBULANT_GUI_DX_PLAYER_H
#define AMBULANT_GUI_DX_PLAYER_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/common/player.h"

#include <string>


namespace ambulant {

// classes used by dx_player
namespace lib {
	class event_processor;
	class logger;
}

namespace common {
	class window_factory;
	class playable_factory;
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

class dx_player {
  public:
	dx_player(const std::string& url, HWND hwnd);
	~dx_player();
	
	void start();
	void stop();
	void pause();
	void resume();
	void on_click(int x, int y);
	void on_char(int ch);
	int get_cursor(int x, int y);
	
	bool is_playing() const;
	bool is_pausing() const;
	bool is_done() const;

	void set_preferences(const std::string& url);
	
	// Implementation specific
	common::window_factory *get_window_factory() { return m_wf;}
	common::playable_factory *get_playable_factory() {return m_pf;}
	viewport* create_viewport(int w, int h);
	void redraw();
	void on_done();
	
  private:
	std::string m_url;
	HWND m_hwnd;
	viewport* m_viewport;
	common::window_factory *m_wf;
	common::playable_factory *m_pf;
	smil2::smil_player *m_player;
	lib::event_processor *m_processor;
	lib::logger *m_logger;
	
};

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_PLAYER_H
