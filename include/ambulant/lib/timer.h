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

#ifndef AMBULANT_LIB_TIMER_H
#define AMBULANT_LIB_TIMER_H

#include "ambulant/config/config.h"

#include <set>

namespace ambulant {

namespace lib {

/// Notification interface for timer events such as speed change.
class timer_events {
  public:
	virtual ~timer_events() {}
	
	/// Called when a timer has changed speed.
	virtual void speed_changed() = 0;
};


/// Interface to timer objects.
class abstract_timer {
  public:
	/// The underline time type used by this timer. 
	/// Assumed to be an integral type.
	typedef unsigned long time_type;
	
	// Allows subclasses to be deleted using base pointers
	virtual ~abstract_timer() {}
		
	/// Returns the time elapsed.
	virtual time_type elapsed() const = 0;
	
	/// Gets the realtime speed of this timer as modulated by its parent.
	virtual double get_realtime_speed() const = 0;
};


/// A timer class able to fulfill SMIL 2.0 timing requirements.
/// The timer can be 
class timer : public abstract_timer, public timer_events {
  public:	
	/// Creates a timer with the provided parent, 
	/// ticking at the speed specified and
	/// initially running or paused as specified. 
	timer(abstract_timer *parent, double speed = 1.0, bool run = true);
	
	~timer();
	
	/// Returns the zero-based elapsed time.
	/// Does not take periodicity into account.
	time_type elapsed() const;
	
	// Returns the zero-based time elapsed for the provided parent elapsed time.
	time_type elapsed(time_type pt) const;
	
	/// Starts ticking at t (t>=0).
	void start(time_type t = 0);
	
	/// Stop ticking and reset elapsed time to zero.
	void stop();
	
	/// Stop ticking but do not reset the elapsed time.
	/// While paused this timer's elapsed() returns the same value. 
	/// Speed remains unchanged and when resumed
	/// will be ticking at that speed.
	void pause();
	
	/// Resumes ticking.
	void resume();
	
	/// Sets the speed of this timer.
	/// At any state, paused or running, set_speed() 
	/// may be called to change speed.
	/// When paused, the new speed will be
	/// used when the timer is resumed else
	/// the new speed is applied immediately.
	/// The current elapsed time is not affected. 
	void set_speed(double speed);
	
	/// Set the current elapsed time.
	void set_time(time_type t);
	
	/// Return the current elapsed time. 
	/// If this is a periodic timer this returns the
	/// elapsed time within the current period.
	time_type get_time() const;
	
	/// Return the period number.
	/// If this is a non-periodic timer it returns 0.
	time_type get_repeat() const;
	
	/// Set timer to periodic mode, and period duration.
	void set_period(time_type t) { m_period = t;}
	
	// Returns the speed of this timer.
	double get_speed() const { return m_speed;}
	
	/// Returns true when this timer is running.
	bool running() const { return m_running;}
	
	/// Returns the realtime speed of this timer 
	/// as modulated by its parent.
	double get_realtime_speed() const;
	
	/// Receives timer_events notifications.
	void speed_changed();
	
	/// Add timer_events listener.
	void add_listener(timer_events *listener);
	
	/// Remove timer_events listener.
	void remove_listener(timer_events *listener);
	
  private:
	time_type apply_speed_manip(time_type dt) const;
	
	abstract_timer *m_parent;
	time_type m_parent_epoch;
	time_type m_local_epoch;
	double m_speed;
	bool m_running;
	long m_period;
	
	// Note: event listeners are not owned by this.
	std::set<timer_events *> *m_listeners;
};

// A (machine-dependent) routine to create a timer object
AMBULANTAPI abstract_timer *realtime_timer_factory();

} // namespace lib
 
} // namespace ambulant
#endif // AMBULANT_LIB_TIMER_H


