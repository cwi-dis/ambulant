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

#ifndef AMBULANT_COMMON_PLAYER_H
#define AMBULANT_COMMON_PLAYER_H

#include "ambulant/common/factory.h"
#include "ambulant/config/config.h"

namespace ambulant {

namespace lib {
class event_processor;
class document;
}

namespace common {
class window_factory;
class playable_factory;
class embedder;

/// Current state of the player.
enum play_state {ps_idle, ps_playing, ps_pausing, ps_done};

/// Interface for getting feedback from the player.
/// The player will call methods here so a UI can synchronise
/// any external views with what the player is doing.
class player_feedback {
  public:
	virtual void document_started() = 0;
	virtual void document_stopped() = 0;
	virtual void node_started(const lib::node *n) = 0;
	virtual void node_stopped(const lib::node *n) = 0;
};

/// Baseclass for all players.
/// This is the API an embedding program would use to control the
/// player, to implement things like the "Play" command in the GUI.
class player {
  public:
	virtual ~player() {};
	
#ifdef USE_SMIL21
	/// Do any initializations necessary
	virtual void initialize() = 0;
#endif

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

	/// Set the feedback handler.
	virtual void set_feedback(player_feedback *fb) {}
	
	/// Tell the player to start playing a specific node.
	/// Return true if successful. 
	virtual bool goto_node(const lib::node *n) { return false; }
//	void set_speed(double speed);
//	double get_speed() const;
};

// Factory functions - should these be here?

/// Create a player using the old timeline based MMS scheduler.
AMBULANTAPI player *create_mms_player(lib::document *doc, common::factories* factory);

/// Create a player using the full SMIL 2.0 scheduler.
AMBULANTAPI player *create_smil2_player(lib::document *doc, common::factories* factory, common::embedder *sys);

} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_PLAYER_H
