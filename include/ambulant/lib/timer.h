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

#ifndef AMBULANT_LIB_TIMER_H
#define AMBULANT_LIB_TIMER_H

#include "ambulant/config/config.h"
#include "ambulant/lib/mtsync.h"
#include <set>

namespace ambulant {

namespace lib {

/// An interface that can be used to receive notification when
/// interesting things happen to a timer.
class timer_observer {
  public:
	virtual void started() = 0;
	virtual void stopped() = 0;
	virtual void paused() = 0;
	virtual void resumed() = 0;
};

/// Client interface to timer objects: allows you to get the
/// current time and the rate at which time passes.
class timer {
  public:
	/// The underline time type used by this timer.
	/// Assumed to be an integral type.
	typedef unsigned long time_type;

	/// A type that can also hold nagative time values (for delta-t purposes).
	typedef long signed_time_type;

	// Allows subclasses to be deleted using base pointers
	virtual ~timer() {}

	/// Returns the time elapsed.
	virtual time_type elapsed() const = 0;

	/// Gets the realtime speed of this timer as modulated by its parent.
	virtual double get_realtime_speed() const = 0;

	/// Signals that some realtime renderer has detected a clock drift.
	/// Positive values means the clock has to speed up, negative numbers that the clock has to slow down.
	/// Returns the amount of drift that the clock will _not_ fix, in other words: the amount of drift
	/// the renderer has to fix itself.
	virtual signed_time_type set_drift(signed_time_type drift) = 0;

	/// Returns the currently recorded drift.
	virtual signed_time_type get_drift() const = 0;

	/// Skew the clock.
	virtual void skew(signed_time_type skew) = 0;
	
	/// Return true if the clock is running.
	virtual bool running() const { return true; }
	
	/// Return true if the clock is slaved to another one.
	virtual bool is_slaved() const { return false; }
};

/// Controller interface to timer objects.
/// Augments the base class
/// with methods to start and stop the timer, and set its speed.

class timer_control : public timer {
  public:

	/// Returns the zero-based elapsed time.
	/// Does not take periodicity into account.
	virtual time_type elapsed() const = 0;

	/// Returns the zero-based time elapsed for the provided parent elapsed time.
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

	/// Returns the speed of this timer, relative to its parent timer.
	virtual double get_speed() const = 0;

	/// Returns true when this timer is running.
	virtual bool running() const = 0;

	/// Returns the realtime speed of this timer relative to the wallclock timer.
	virtual double get_realtime_speed() const = 0;

	/// Signals that some realtime renderer has detected a clock drift.
	/// Positive values means the clock has to speed up, negative numbers that the clock has to slow down.
	/// Returns the amount of drift that the clock will _not_ fix, in other words: the amount of drift
	/// the renderer has to fix itself.
	virtual signed_time_type set_drift(signed_time_type drift) = 0;

	/// Returns the currently recorded drift.
	virtual signed_time_type get_drift() const = 0;

	/// Skew the clock.
	virtual void skew(signed_time_type skew) = 0;
	
	/// Set the observer.
	virtual void set_observer(timer_observer *obs) = 0;
	
	/// Indicate that this clock is slaved to another one, i.e. it may be adjusted by an
	/// externap agent. This affects clock resyncing by continuous mediia items.
	virtual void set_slaved(bool slaved) = 0;
	
	/// Returns true if this clock is slaved to another one.
	virtual bool is_slaved() const = 0;

};


/// An implementation of timer_control.
class timer_control_impl : public timer_control {
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
	time_type elapsed(time_type pet) const;

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
	bool running() const { return m_running && m_parent->running();}

	/// Returns the realtime speed of this timer
	/// as modulated by its parent.
	double get_realtime_speed() const ;

	/// Signals that some realtime renderer has detected a clock drift.
	/// Positive values means the clock has to speed up, negative numbers that the clock has to slow down.
	signed_time_type set_drift(signed_time_type drift) {
		m_lock.enter();
		m_drift = drift;
		m_lock.leave();
		return 0;
	};

	/// Returns the currently recorded drift.
		signed_time_type get_drift() const {
		const_cast<timer_control_impl*>(this)->m_lock.enter();
		signed_time_type rv = m_drift;
		const_cast<timer_control_impl*>(this)->m_lock.leave();
		return rv;
	};

	/// Skew the clock.
	void skew(signed_time_type skew_);

	/// Set the observer.
	void set_observer(timer_observer *obs);
	
	/// Indicate that this clock is slaved to another one, i.e. it may be adjusted by an
	/// externap agent. This affects clock resyncing by continuous mediia items.
	void set_slaved(bool slaved) { m_slaved = true; }
	
	/// Returns true if this clock is slaved to another one.
	bool is_slaved() const { return m_slaved || m_parent->is_slaved(); }

  private:
	void _start(time_type t = 0);
	void _stop();
	void _pause(bool tell_observer);
	void _resume(bool tell_observer);
	time_type _elapsed() const;
	time_type _elapsed(time_type pt) const;
	time_type _apply_speed_manip(time_type dt) const;

	timer *m_parent;
	bool m_parent_owned;
	time_type m_parent_epoch;
	time_type m_local_epoch;
	double m_speed;
	bool m_running;
	signed_time_type m_drift;
	timer_observer *m_observer;
	bool m_slaved;
	critical_section m_lock;
};

/// Factory function that returns a machine-dependent timer implementation.
AMBULANTAPI timer *realtime_timer_factory();

} // namespace lib

} // namespace ambulant
#endif // AMBULANT_LIB_TIMER_H


