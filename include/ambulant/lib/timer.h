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


/// Client interface to timer objects: allows you to get the
/// current time and the rate at which time passes.
class timer {
  public:
	/// The underline time type used by this timer. 
	/// Assumed to be an integral type.
	typedef unsigned long time_type;
	
	// Allows subclasses to be deleted using base pointers
	virtual ~timer() {}
		
	/// Returns the time elapsed.
	virtual time_type elapsed() const = 0;
	
	/// Gets the realtime speed of this timer as modulated by its parent.
	virtual double get_realtime_speed() const = 0;
};

/// Controller interface to timer objects.
/// Augments the base class
/// with methods to start and stop the timer, and set its speed.

class timer_control : public timer {
  public:	
	
	/// Returns the zero-based elapsed time.
	/// Does not take periodicity into account.
	virtual time_type elapsed() const = 0;

	// Returns the zero-based time elapsed for the provided parent elapsed time.
	virtual time_type elapsed(time_type pt) const = 0;
	
	/// Starts ticking at t (t>=0).
	virtual void start(time_type t = 0) = 0;
	
	/// Stop ticking and reset elapsed time to zero.
	virtual void stop() = 0;
	
	/// Stop ticking but do not reset the elapsed time.
	/// While paused this timer's elapsed() returns the same value. 
	/// Speed remains unchanged and when resumed
	/// will be ticking at that speed.
	virtual void pause() = 0;
	
	/// Resumes ticking.
	virtual void resume() = 0;
	
	/// Sets the speed of this timer.
	/// At any state, paused or running, set_speed() 
	/// may be called to change speed.
	/// When paused, the new speed will be
	/// used when the timer is resumed else
	/// the new speed is applied immediately.
	/// The current elapsed time is not affected. 
	virtual void set_speed(double speed) = 0;
	
	/// Set the current elapsed time.
	virtual void set_time(time_type t) = 0;
	
	// Returns the speed of this timer.
	virtual double get_speed() const = 0;
	
	/// Returns true when this timer is running.
	virtual bool running() const = 0;
	
	/// Returns the realtime speed of this timer 
	/// as modulated by its parent.
	virtual double get_realtime_speed() const = 0;

// Some methods that aren't used, yet:
//
//	/// Return the current elapsed time. 
//	/// If this is a periodic timer this returns the
//	/// elapsed time within the current period.
//	virtual time_type get_time() const = 0;
//	
//	/// Return the period number.
//	/// If this is a non-periodic timer it returns 0.
//	virtual time_type get_repeat() const = 0;
//	
//	/// Set timer to periodic mode, and period duration.
//	virtual void set_period(time_type t) = 0;
//	/// Add timer_events listener.
//	virtual void add_listener(timer_events *listener) = 0;
//	
//	/// Remove timer_events listener.
//	virtual void remove_listener(timer_events *listener) = 0;
};


/// An implementation of timer_control.
class timer_control_impl : public timer_control, public timer_events {
  public:	
	/// Creates a timer.
	/// Pass the parent timer, 
	/// the relative speed and
	/// initial run/pause status. 
	timer_control_impl(timer *parent, double speed = 1.0, bool run = true, bool owned = false);
	
	~timer_control_impl();
	
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
	
	// Returns the speed of this timer.
	double get_speed() const { return m_speed;}
	
	/// Returns true when this timer is running.
	bool running() const { return m_running;}
	
	/// Returns the realtime speed of this timer 
	/// as modulated by its parent.
	double get_realtime_speed() const;
	
	/// Receives timer_events notifications.
	void speed_changed();
	
//	/// Return the current elapsed time. 
//	/// If this is a periodic timer this returns the
//	/// elapsed time within the current period.
//	time_type get_time() const;
//	
//	/// Return the period number.
//	/// If this is a non-periodic timer it returns 0.
//	time_type get_repeat() const;
//	
//	/// Set timer to periodic mode, and period duration.
//	void set_period(time_type t) { m_period = t;}
//	
//	/// Add timer_events listener.
//	void add_listener(timer_events *listener);
//	
//	/// Remove timer_events listener.
//	void remove_listener(timer_events *listener);
	
  private:
	time_type apply_speed_manip(time_type dt) const;
	
	timer *m_parent;
	bool m_parent_owned;
	time_type m_parent_epoch;
	time_type m_local_epoch;
	double m_speed;
	bool m_running;
	long m_period;
	
	// Note: event listeners are not owned by this.
	std::set<timer_events *> *m_listeners;
};

/// Factory function that returns a machine-dependent timer implementation.
AMBULANTAPI timer *realtime_timer_factory();

} // namespace lib
 
} // namespace ambulant
#endif // AMBULANT_LIB_TIMER_H


