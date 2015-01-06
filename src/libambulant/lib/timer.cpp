// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/lib/timer.h"

#	if __GNUC__ == 2 && __GNUC_MINOR__ <= 97
#include "ambulant/compat/limits"
#else
#include <limits>
#endif

#include "ambulant/lib/logger.h"
#include <cmath>
#include <cassert>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

static long infinite = std::numeric_limits<long>::max();

lib::timer_control_impl::timer_control_impl(lib::timer* parent, double speed /* = 1.0 */,
	bool run /* = true */, bool owned /* = false */)
:	m_parent(parent),
	m_parent_owned(owned),
	m_parent_epoch(parent->elapsed()),
	m_local_epoch(0),
	m_speed(speed),
	m_running(run),
	m_drift(0),
    m_observer(NULL),
	m_slaved(false)
{
	AM_DBG lib::logger::get_logger()->debug("lib::timer_control_impl(0x%x), parent=0x%x", this, parent);
}

lib::timer_control_impl::~timer_control_impl()
{
	// This class does not own event listeners.
	// Therefore, deleting the container is enough
	if (m_parent_owned) delete m_parent; // Is this correct?
	AM_DBG lib::logger::get_logger()->debug("~lib::timer_control_impl()");
}

lib::timer_control_impl::time_type
lib::timer_control_impl::elapsed() const
{
	lib::timer_control_impl::time_type rv;
	const_cast<lib::timer_control_impl*>(this)->m_lock.enter();
	rv = _elapsed();
	const_cast<lib::timer_control_impl*>(this)->m_lock.leave();
	return rv;
}

lib::timer_control_impl::time_type
lib::timer_control_impl::_elapsed() const
{
	if(!m_running) return m_local_epoch;
	lib::timer_control_impl::time_type pt = m_parent->elapsed();
	if (pt < m_parent_epoch) return m_local_epoch;
	return m_local_epoch + _apply_speed_manip(pt - m_parent_epoch);
}

lib::timer_control_impl::time_type
lib::timer_control_impl::elapsed(time_type pet) const
{
	lib::timer_control_impl::time_type rv;
	const_cast<lib::timer_control_impl*>(this)->m_lock.enter();
	rv = _elapsed(pet);
	const_cast<lib::timer_control_impl*>(this)->m_lock.leave();
	return rv;
}

lib::timer_control_impl::time_type
lib::timer_control_impl::_elapsed(time_type pt) const
{
	if(!m_running) return m_local_epoch;
	if (pt < m_parent_epoch) return m_local_epoch;
	return m_local_epoch + _apply_speed_manip(pt - m_parent_epoch);
}

void
lib::timer_control_impl::start(time_type t /* = 0 */) {
	m_lock.enter();
	_start(t);
	m_lock.leave();
}

void
lib::timer_control_impl::_start(time_type t) {
	m_parent_epoch = m_parent->elapsed();
	m_local_epoch = t;
	m_running = true;
#ifdef WITH_REMOTE_SYNC
		if (m_observer) {
			// Bah. Observer may want to access the timer. Need to unlock.
			// Should refactor this code some time...
			m_lock.leave();
			m_observer->started();
			m_lock.enter();
		}
#endif

}

void
lib::timer_control_impl::stop() {
	m_lock.enter();
	_stop();
	m_lock.leave();
}

void
lib::timer_control_impl::_stop() {
	m_local_epoch = 0;
	m_running = false;
#ifdef WITH_REMOTE_SYNC
		if (m_observer) {
			// Bah. Observer may want to access the timer. Need to unlock.
			// Should refactor this code some time...
			m_lock.leave();
			m_observer->stopped();
			m_lock.enter();
		}
#endif
}

void
lib::timer_control_impl::pause() {
	m_lock.enter();
	_pause(true);
	m_lock.leave();
}

void
lib::timer_control_impl::_pause(bool tell_observer) {
	if(m_running) {
		m_local_epoch += _apply_speed_manip(m_parent->elapsed() - m_parent_epoch);
		m_running = false;
#ifdef WITH_REMOTE_SYNC
		if (tell_observer && m_observer) {
			// Bah. Observer may want to access the timer. Need to unlock.
			// Should refactor this code some time...
			m_lock.leave();
			m_observer->paused();
			m_lock.enter();
		}
#endif
	}
}

void
lib::timer_control_impl::resume() {
	m_lock.enter();
	_resume(true);
	m_lock.leave();
}

void
lib::timer_control_impl::_resume(bool tell_observer) {
	if(!m_running) {
		m_parent_epoch = m_parent->elapsed();
		m_running = true;
#ifdef WITH_REMOTE_SYNC
		if (tell_observer && m_observer) {
			// Bah. Observer may want to access the timer. Need to unlock.
			// Should refactor this code some time...
			m_lock.leave();
			m_observer->resumed();
			m_lock.enter();
		}
#endif
	}
}

void
lib::timer_control_impl::set_time(time_type t) {

	m_lock.enter();
	if(!m_running) {
		if (t < m_local_epoch) {
			AM_DBG lib::logger::get_logger()->debug("timer: setting paused timer 0x%x from %d to %d", this, m_local_epoch, t);
		}
		m_local_epoch = t;
	} else {
		_pause(false);
		// XXXJACK: Hard-setting a running clock is a bad idea: it makes things like animations and
		// transitions "stutter". One possible solution would be to skew the clock if
		if (t < m_local_epoch) {
			AM_DBG lib::logger::get_logger()->debug("timer: setting running timer 0x%x from %d to %d", this, m_local_epoch, t);
		}
		m_local_epoch = t;
		_resume(false);
	}
	m_lock.leave();
}

void
lib::timer_control_impl::set_speed(double speed)
{
	m_lock.enter();
	if(!m_running) {
		m_speed = speed;
	} else {
		_pause(false);
		m_speed = speed;
		_resume(false);
	}
	m_lock.leave();
}

double
lib::timer_control_impl::get_realtime_speed() const
{
	double rv;
	const_cast<lib::timer_control_impl*>(this)->m_lock.enter();
	rv = m_speed * m_parent->get_realtime_speed();
	const_cast<lib::timer_control_impl*>(this)->m_lock.leave();
	return rv;
}

lib::timer_control_impl::time_type
lib::timer_control_impl::_apply_speed_manip(lib::timer::time_type dt) const
{
	if(m_speed == 1.0) return dt;
	else if(m_speed == 0.0) return 0;
	return time_type(::floor(m_speed*dt + 0.5));
}

void
lib::timer_control_impl::skew(signed_time_type skew_) {
	m_lock.enter();
	m_drift -= skew_;
	if (skew_ >= 0) {
		m_local_epoch += skew_;
	} else {
		// We don't want the clock to run backward. So, we fiddle the epochs.
		// We hold the lock, so we don't have to care about the order in which we do things.

		time_type parent_time = m_parent->elapsed();
		if (parent_time < m_parent_epoch) {
			// If we are already skewing we don't re-skew, because we don't know whether this
			// is a new request or a re-issue of the old one (because the perceived clock is
			// still at the old value).
			AM_DBG lib::logger::get_logger()->debug("skew: already skewing");
		} else {
			m_local_epoch += _apply_speed_manip(m_parent->elapsed() - m_parent_epoch);
			AM_DBG lib::logger::get_logger()->debug("skew: actually skewing %dms from %d", skew_, parent_time);
			m_parent_epoch = parent_time - skew_;
		}
	}
	m_lock.leave();
}

void
lib::timer_control_impl::set_observer(timer_observer *obs) {
#ifdef WITH_REMOTE_SYNC
	if (obs) {
		assert(m_observer == NULL);
	}
	m_observer = obs;
    
#endif
}
