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

#ifndef AMBULANT_LIB_PLAYABLE_H
#define AMBULANT_LIB_PLAYABLE_H

#include <utility>

namespace ambulant {

namespace lib {

// Display mode when the playable is paused.
enum pause_display {display_disable, display_hide, display_show};

// The playable interface is used by the scheduler to control
// the objects being scheduled (renderers, animations, timelines,
// maybe transitions, maybe other schedulers.
//
//
// Playable requirements as imposed by the scheduler design:
//
// For the foolowing description assume the playable as a video player.
//
// 1) We must be able start playing at media time t independent of the 
// previous state (not playing, playing or paused) 
// e.g. without having to remember its previous state. 
// Internally the playable may keep whatever is needed. 
// Two consecutive start(t) can be issued and start(t) 
// can be issued while the playable is paused.
//
// 2) We must be able to reset the player as if it has never been called 
// e.g remove any effects from the screen. 
//
// 3) We must be able to pause the player at any moment. 
// Pause should be idempotent.
//
// 4) On media end the playable should raise an endMediaEvent and 
// keep the last frame
// e.g. as if pause and then seek to end has been called.
//
// 5) We must be able to seek into the media without 
// changing playable state (playing/paused).

// The playable interface specifies time as double.
// This may change in future versions of the interface.

class playable {

  public:
  
	enum playable_state {ps_not_playing, ps_playing, ps_frozen};
	
	// An id identifying this playable to the client code 
	typedef int cookie_type;
	
	// Allows subclasses to be deleted using base pointers
	virtual ~playable() {}
	
	// Starts playing at media time t independent 
	// of the previous state (not playing, playing, paused). 
	virtual void start(double t) = 0;
	
	// Stops playing and removes any effects from the screen. 
	// Resets playable to its initial state.
	// The playable may be invoked again later
	// and therefore may keep its data cashed.
	virtual void stop() = 0;
	
	// Pauses playing keeping the last frame. 
	// On media end the playable should be in a state 
	// as if pause and then seek to end has been called.
	// While paused a start(t) may be called.
	// Pause is equivalent to freeze.
	virtual void pause() = 0;
	
	// Resume playing from the paused state. 
	virtual void resume() = 0;
	
	// Seek to media time t without changing state (playing/paused). 
	// Note: we may need a function specifying offsets 
	// from media begin or media end
	virtual void seek(double t) = 0;

	// Specifies whether this playable should send notifications for clicks.
	virtual void wantclicks(bool want) = 0;
	
	// Starts prerolling adjusting if possible the process to the provided hint.
	// when: the estimated time when this playable start() will be called
	// where: where playing will start in media time
	// how_much: the duration of the media that will be played
	virtual void preroll(double when, double where, double how_much) = 0;
	
	// Returns a pair of values.
	// The first value is a boolean indicating whether the dur of this 
	// playbale is known (true) or not (false).
	// When the first value is true the second value contains the
	// dur of this playbale in secs otherwise it is ignored.
	// This function may be called more than once and therefore
	// the playable may cash the value. Also, it may return 
	// std::pair<false, any> originally and later, when the dur becomes 
	// known std::pair<true, dur>.
	virtual std::pair<bool, double> get_dur() = 0;
	
	// Returns the cookie identifying this playable to the client code.
	// The cookie was provided to this playable when it was constructed.
	virtual const cookie_type& get_cookie() const = 0;
};


//
// The playable_events interface is (probably) implemented by the scheduler
// itself, and passed to the playable constructor so it can do callbacks. The
// node argument that is passed to the constructor is also passed when doing
// the callback.
//	
// The started and stopped callbacks are always done, the clicked
// callback (and the associated GUI feedback) only when wantclicks(true)
// has been called on the playable.

class playable_events {
  public:
	typedef playable::cookie_type cookie_type;
	
	// Allows subclasses to be deleted using base pointers
	virtual ~playable_events() {}
	
	// Playables nodifications 
	virtual void started(cookie_type n, double t) = 0;
	virtual void stopped(cookie_type n, double t) = 0;
	virtual void clicked(cookie_type n, double t) = 0;
};

class abstract_playable : public playable {
  public:
    abstract_playable(playable_events *context, const cookie_type cookie)
    :   m_context(context),
        m_cookie(cookie) {};
    virtual ~abstract_playable() {};

	virtual void seek(double t) {};
	virtual void preroll(double when, double where, double how_much) {};
	virtual std::pair<bool, double> get_dur() { return std::pair<bool, double>(false, 0); };
	const cookie_type& get_cookie() const { return m_cookie;}
	
  protected:
    void started_callback() const { m_context->started(m_cookie, 0); };
    void stopped_callback() const { m_context->stopped(m_cookie, 0); };
    void clicked_callback() const { m_context->clicked(m_cookie, 0); };
    
    playable_events *const m_context;
    cookie_type m_cookie;
};

typedef abstract_playable active_playable;
typedef playable_events active_playable_events;

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_PLAYABLE_H


