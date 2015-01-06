/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
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

#ifndef AMBULANT_COMMON_PLAYER_H
#define AMBULANT_COMMON_PLAYER_H

#include "ambulant/common/factory.h"
#include "ambulant/config/config.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/common/state.h"

namespace ambulant {

namespace lib {
class event_processor;
class document;
}

namespace common {
class window_factory;
class playable_factory;
class embedder;
class state_component;

/// Current state of the player.
enum play_state {
	ps_idle,	///< The player has not started yet
	ps_playing, ///< The player is playing
	ps_pausing, ///< The player is paused
	ps_done	 ///< The player has finished playing the document
};

/// This class may be implemented by gui players, to show mouseover.
class focus_feedback {
  public:
	/// Called by the player to signal the given node received focus.
	/// This can happen either through a mouseover event or a tabindex event.
	virtual void node_focussed(const lib::node *n) = 0;
};	

/// Interface for getting feedback from the player.
/// The player will call methods here so a UI can synchronise
/// any external views with what the player is doing.
class player_feedback {
  public:
	virtual ~player_feedback(){}

	/// Called by the player when the document is loaded.
	/// Called after parsing, but before the
	/// timegraph is created. At this time the embedding application can
	/// modify the document, if it needs to.
	virtual void document_loaded(lib::document *doc) = 0;

	/// Called by the player when the document starts playing
	virtual void document_started() = 0;

	/// Called by the player when the document stopped playing
	virtual void document_stopped() = 0;

	/// Called by the player to signal that the given node starts playing
	virtual void node_started(const lib::node *n) = 0;

    /// Called by the player when a node goes into fill mode
    virtual void node_filled(const lib::node *n) = 0;
    
	/// Called by the player to signal the given node stopped playing
	virtual void node_stopped(const lib::node *n) = 0;

	/// Called by the player when a new renderer is assigned to a node.
	virtual void playable_started(const playable *p, const lib::node *n, const char *comment) = 0;
	
	/// Called by the playable when it stalls.
	virtual void playable_stalled(const playable *p, const char *reason) = 0;
	
	/// Called by the playable when the stall is over.
	virtual void playable_unstalled(const playable *p) = 0;
	
	/// Called when a playable is entered into the cache.
	virtual void playable_cached(const playable *p) = 0;
	
	/// Called when a playable is deleted.
	virtual void playable_deleted(const playable *p) = 0;
	
	/// Called when a playable consumes a resource.
	virtual void playable_resource(const playable *p, const char *resource, long amount) = 0;
};

/// Baseclass for all players.
/// This is the API an embedding program could use to control the
/// player, to implement things like the "Play" command in the GUI.
/// As of AmbulantPlayer 1.8, however, most embedding applications will
/// use the gui_player class which contains aditional functionality such
/// as storing the document and all factories, and which contains
/// convenience methods to call most common player methods. But some
/// methods, such as set_feedback(), are not available in gui_player.
class player : public state_change_callback, virtual public lib::ref_counted
{
  public:
	virtual ~player() {};

	/// Do any initializations necessary.
	virtual void initialize() = 0;

	/// Call this early during termination, before things like the
	/// lib::document and DOM tree become invalid.
	virtual void terminate() = 0;

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
	virtual int after_mousemove() { return 0; }

	/// Set desired cursor.
	virtual void before_mousemove(int cursor) {}

	/// Call this when the user has pressed a key.
	virtual void on_char(int ch) {}

	/// Call this when a state variable has changed.
	virtual void on_state_change(const char *ref) {}

	/// Returns the SMIL State handler for the current document, or NULL.
	virtual state_component* get_state_engine() { return NULL; }

	/// Call this to advance the focus.
	virtual void on_focus_advance() {}

	/// Call this to activate/select the current focus.
	virtual void on_focus_activate() {}

	/// Set the focus feedback handler.
	virtual void set_focus_feedback(focus_feedback *fb) {}

	/// Set the feedback handler.
	virtual void set_feedback(player_feedback *fb) {}
    
    /// Get the feedback handler.
    virtual player_feedback* get_feedback() { return NULL; }

	/// Tell the player to start playing a specific node.
	/// Return true if successful.
	virtual bool goto_node(const lib::node *n) { return false; }

	/// Highlight a specific node, if visible, and return true if it happened.
	virtual bool highlight(const lib::node *n, bool on=true) { return false; }

    virtual void clicked_external(lib::node *n, lib::timer::time_type t) {}

    virtual bool uses_external_sync() const { return false; }
//	void set_speed(double speed);
//	double get_speed() const;
};

// Factory functions - should these be here?

/// Create a player using the full SMIL 2.0 scheduler.
AMBULANTAPI player *create_smil2_player(lib::document *doc, common::factories* factory, common::embedder *sys);

} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_PLAYER_H
