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

#include "ambulant/common/factory.h"
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
class embedder;

/// Current state of the player.
enum play_state {ps_idle, ps_playing, ps_pausing, ps_done};

/// Baseclass for all players.
/// This is the API an embedding program would use to control the
/// player, to implement things like the "Play" command in the GUI.
class player {
  public:
	virtual ~player() {};
	
	/// Return the timer this player uses.
	virtual lib::timer* get_timer() = 0;
	
	/// Return the event_processor this player uses.
	virtual lib::event_processor* get_evp() = 0;

	/// Start playback.
	virtual void start() = 0;
	
	/// Stop playback.
	virtual void stop() = 0;
	
	/// Pause playback.
	virtual void pause() = 0;
	
	/// Undo the effect of pause.
	virtual void resume() = 0;
	
	/// Return true if player is playing.
	virtual bool is_playing() const { return false;}
	
	/// Retirn true if player is paused.
	virtual bool is_pausing() const { return false;}
	
	/// Return true if player has finished.
	virtual bool is_done() const { return false;}
	
	/// Return index of desired cursor (arrow or hand).
	virtual int get_cursor() const { return 0; }
	
	/// Set desired cursor.
	virtual void set_cursor(int cursor) {}
		
//	void set_speed(double speed);
//	double get_speed() const;
};

// Factory functions - should these be here?

/// Create a player using the old timeline based MMS scheduler.
player *create_mms_player(lib::document *doc, common::factories* factory);

/// Create a player using the full SMIL 2.0 scheduler.
player *create_smil2_player(lib::document *doc, common::factories* factory, common::embedder *sys);

} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_PLAYER_H
