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

#ifndef AMBULANT_COMMON_PLAYER_H
#define AMBULANT_COMMON_PLAYER_H

#include "ambulant/config/config.h"

namespace ambulant {

namespace lib {
class timer;
class event_processor;
class document;
}

namespace common {
class window_factory;
class playable_factory;

class abstract_player {
  public:
	virtual ~abstract_player() {};
	
	virtual lib::timer* get_timer() = 0;
	virtual lib::event_processor* get_evp() = 0;

	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual bool is_done() const = 0;
//	void set_speed(double speed);
//	double get_speed() const;
};

// Factory functions - should these be here?
abstract_player *create_mms_player(lib::document *doc, window_factory *wf, playable_factory *rf);
abstract_player *create_smil2_player(lib::document *doc, window_factory *wf, playable_factory *rf);


// XXX: The following interface is incomplete.
// a) should handle multiple windows
// b) other events
// c) partial redraws
// ...

// A player_control interface is implemented by a toolkit
// providing a player implementation.
// The GUI uses this interface to wrap and interact with the
// player provided by the toolkit.

class player_control {
  public:
	// Instruct the compiler to destroy objects 
	// implementing this using the virtual table.
	virtual ~player_control() {}
	
	// A gui call to set the document to play.
	// Opens and parses the document.
	// Creates the underlying player.
	// Probably shows the layout.
	virtual void set_document(const char *url) = 0;
	
	// A gui call to set the preferences for this player
	virtual void set_preferences(const char *url) = 0;
	
	// A gui call requesting from this player to start playing.
	virtual void start() = 0;
	
	// A gui call requesting from this player to stop playing.
	virtual void stop() = 0;
	
	// A gui call requesting from this player to pause playing.
	virtual void pause() = 0;
	
	// Tells if this is in state that can start. 
	// Depending on the answer the GUI should
	// enable or disable the start button. 
	virtual bool can_start() = 0;
	
	// Tells if this is in state that can stop. 
	// Depending on the answer the GUI should
	// enable or disable the stop button. 
	virtual bool can_stop() = 0;
	
	// Tells if this is in state that can pause. 
	// Depending on the answer the GUI should
	// enable or disable the pause button. 
	virtual bool can_pause() = 0;
	
	// A gui call requesting a redraw of the current scene.
	virtual void redraw() = 0;
	
	// A gui originating notification for a click event at x, y
	// x, y is in pixels in viewport 'screen' coordinates
	virtual void on_click(int x, int y) = 0;
	
	// A gui originating notification for a char event. 
	virtual void on_char(int ch) = 0;
	
	// Tells what cursor the gui should use when 
	// the mouse is at point x, y. 
	// x, y is in pixels in viewport 'screen' coordinates
	// Returns 0 for the default.
	virtual int get_cursor(int x, int y) = 0;
};

} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_PLAYER_H
