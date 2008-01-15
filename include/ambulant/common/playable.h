/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
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

#ifndef AMBULANT_COMMON_PLAYABLE_H
#define AMBULANT_COMMON_PLAYABLE_H

#include <utility>
#include "ambulant/lib/logger.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/node.h"

namespace ambulant {

namespace lib {
class event_processor;
} // namespace lib

namespace net {
class datasource_factory;
class audio_datasource;
} // namespace net

namespace common {

class renderer;

/// Display mode when the playable is paused.
enum pause_display {
    display_disable,    ///< ???
    display_hide,       ///< Do not show media while paused
    display_show        ///< Continue showing media while paused
};

/// Duration of a node or media item.
/// The first item is true for
/// known, false for unknown; the second item is the duration in
/// seconds (if known).
typedef std::pair<bool, double> duration;

/// Interface the scheduler uses to control playback.
/// The playable interface is used by the scheduler to control
/// the objects being scheduled (renderers, animations, timelines,
/// maybe transitions, maybe other schedulers.
/// There is a corresponding interface playable_notification that
/// implementations of playable will use to communicate back: things
/// like media end reached, user clicked the mouse, etc.
///
/// Playable requirements as imposed by the scheduler design:
///
/// For the following description assume the playable as a video player.
///
/// 1) We must be able start playing at media time t independent of the 
/// previous state (not playing, playing or paused) 
/// e.g. without having to remember its previous state. 
/// Internally the playable may keep whatever is needed. 
/// Two consecutive start(t) can be issued and start(t) 
/// can be issued while the playable is paused.
///
/// 2) We must be able to reset the player as if it has never been called 
/// e.g remove any effects from the screen. 
///
/// 3) We must be able to pause the player at any moment. 
/// Pause should be idempotent.
///
/// 4) On media end the playable should raise an endMediaEvent and 
/// keep the last frame
/// e.g. as if pause and then seek to end has been called.
///
/// 5) We must be able to seek into the media without 
/// changing playable state (playing/paused).
///
/// The playable interface specifies time as double.
/// This may change in future versions of the interface.

class AMBULANTAPI playable : public lib::ref_counted_obj {

  public:
  
    /// States a playable can be in.
	enum playable_state {
        ps_not_playing, ///< Not playing.
        ps_playing,     ///< Playing in its SMIL2 active duration.
        ps_frozen       ///< Frozen (in its SMIL2 fill period).
	};
	
	/// An id identifying this playable to the client code.
	typedef int cookie_type;
	
	virtual ~playable() {}
	
	/// Start playback.
	/// Starts playing at media time t independent 
	/// of the previous state (not playing, playing, paused). 
	virtual void start(double t) = 0;
	
	/// Stop playback.
	/// Stops playing and removes any effects from the screen. 
	/// Resets playable to its initial state.
	/// The playable may be invoked again later
	/// and therefore may keep its data cashed.
	virtual void stop() = 0;
	
	/// Pauses playback, keeping the last frame. 
	/// On media end the playable should be in a state 
	/// as if pause and then seek to end has been called.
	/// While paused a start(t) may be called.
	/// Pause is equivalent to freeze.
	virtual void pause(pause_display d=display_show) = 0;
	
	/// Resume playing from the paused state. 
	virtual void resume() = 0;
	
	/// Seek to media time t without changing state (playing/paused). 
	/// Note: we may need a function specifying offsets 
	/// from media begin or media end.
	virtual void seek(double t) = 0;

	/// Specifies whether this playable should send notifications for clicks.
	virtual void wantclicks(bool want) = 0;
	
	/// Start preloading data.
	/// when: the estimated time when this playable start() will be called
	/// where: where playing will start in media time
	/// how_much: the duration of the media that will be played
	virtual void preroll(double when, double where, double how_much) = 0;
	
	/// Get duration of media item.
	/// Returns a pair of values:
	/// The first value is a boolean indicating whether the dur of this 
	/// playable is known (true) or not (false).
	/// When the first value is true the second value contains the
	/// dur of this playable in secs otherwise it is ignored.
	/// This function may be called more than once and therefore
	/// the playable may cash the value. Also, it may return 
	/// std::pair<false, any> originally and later, when the dur becomes 
	/// known std::pair<true, dur>.
	virtual duration get_dur() = 0;
	
	
	/// Returns the cookie identifying this playable to the client code.
	/// The cookie is usually provided to this playable when it was constructed.
	virtual cookie_type get_cookie() const = 0;
	
	/// Return the renderer interface of this playable, or NULL.
	virtual renderer *get_renderer() { return (renderer *)NULL; }
};


/// API for playable objects to do callbacks.
/// The playable_notification interface is (probably) implemented by the scheduler
/// itself, and passed to the playable constructor so it can do callbacks. The
/// node argument that is passed to the constructor is also passed when doing
/// the callback.
///	
/// The started and stopped callbacks are always done, the clicked
/// callback (and the associated GUI feedback) only when wantclicks(true)
/// has been called on the playable.

class playable_notification {
  public:
	typedef playable::cookie_type cookie_type;
	
	// Allows subclasses to be deleted using base pointers
	virtual ~playable_notification() {}
	
	/// Playable corresponding to cookie n has started. 
	virtual void started(cookie_type n, double t = 0) = 0;

	/// Playable corresponding to cookie n has finished. 
	virtual void stopped(cookie_type n, double t = 0) = 0;

	/// Playable corresponding to cookie n is stalled. 
	virtual void stalled(cookie_type n, double t = 0) = 0;

	/// Playable corresponding to cookie n is no longer stalled. 
	virtual void unstalled(cookie_type n, double t = 0) = 0;

	/// Playable corresponding to cookie n received a mouse click. 
	virtual void clicked(cookie_type n, double t = 0) = 0;

	/// Playable corresponding to cookie n received a mouse-over. 
	virtual void pointed(cookie_type n, double t = 0) = 0;

	/// Playable corresponding to cookie n has finished a transition. 
	virtual void transitioned(cookie_type n, double t = 0) = 0;

	/// Playable corresponding to cookie n has seen a marker. 
	virtual void marker_seen(cookie_type n, const char *name, double t = 0) = 0;
};

/// Factory for playable objects.
class playable_factory {
  public:
	virtual ~playable_factory() {};
	
	/// Create a playable for a given node.
	virtual playable *new_playable(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp) = 0;
		
	/// Create a playable for a given audio stream, to be used to play the audio track
	/// accompanying a video stream.
	virtual playable *new_aux_audio_playable(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src) = 0;	
};

/// Provider interface to playable_factory.
/// Extends the base class with a method that can be used to register new
/// factories.
class global_playable_factory : public playable_factory {
  public:
    virtual ~global_playable_factory() {}
    
	/// Add a factory.
    virtual void add_factory(playable_factory *rf) = 0;
};

/// Factory function to get a (singleton?) global_playable_factory object.
AMBULANTAPI global_playable_factory *get_global_playable_factory();


} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_PLAYABLE_H
