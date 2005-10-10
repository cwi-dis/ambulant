// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */

#include "ambulant/lib/timer.h"

#   if __GNUC__ == 2 && __GNUC_MINOR__ <= 97
#include "ambulant/compat/limits"
#else
#include <limits>
#endif

#include "ambulant/lib/logger.h"
#include <cmath>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

static long infinite = std::numeric_limits<long>::max();

lib::timer_control_impl::timer_control_impl(lib::timer* parent, double speed /* = 1.0 */, bool run /* = true */)
:   m_parent(parent),
	m_parent_epoch(parent->elapsed()),
	m_local_epoch(0),
	m_speed(speed),
	m_running(run),
	m_period(infinite),
	m_listeners(0)
{	
	AM_DBG lib::logger::get_logger()->debug("lib::timer_control_impl()");
}

lib::timer_control_impl::~timer_control_impl()
{
	// This class does not own event listeners.
	// Therefore, deleting the container is enough
	delete m_listeners;
	AM_DBG lib::logger::get_logger()->debug("~lib::timer_control_impl()");
}

lib::timer_control_impl::time_type
lib::timer_control_impl::elapsed() const
{
	if(!m_running) return m_local_epoch;
	return m_local_epoch + apply_speed_manip(m_parent->elapsed() - m_parent_epoch);
}

lib::timer_control_impl::time_type
lib::timer_control_impl::elapsed(time_type pe) const
{
	if(!m_running) return m_local_epoch;
	return m_local_epoch + apply_speed_manip(pe - m_parent_epoch);
}

void lib::timer_control_impl::start(time_type t /* = 0 */) {
	m_parent_epoch = m_parent->elapsed();
	m_local_epoch = t;
	m_running = true;
}

void lib::timer_control_impl::stop() {
	m_local_epoch = 0;
	m_running = false;
}
	
void lib::timer_control_impl::pause() {
	if(m_running) {
		m_local_epoch += apply_speed_manip(m_parent->elapsed() - m_parent_epoch);
		m_running = false;
	}
}
	
void lib::timer_control_impl::resume() {
	if(!m_running) {
		m_parent_epoch = m_parent->elapsed();
		m_running = true;
	}
}

void lib::timer_control_impl::set_time(time_type t) {
	if(!m_running) {
		m_local_epoch = t;
	} else {
		pause();
		m_local_epoch = t;
		resume();
	}
}	

#if 0
lib::timer_control_impl::time_type 
lib::timer_control_impl::get_time() const {
	return (m_period == infinite)?elapsed():(elapsed() % m_period);
}

lib::timer_control_impl::time_type 
lib::timer_control_impl::get_repeat() const {
	return (m_period == infinite)?0:(elapsed() / m_period);
}
#endif

void lib::timer_control_impl::set_speed(double speed)
{
	if(!m_running) {
		m_speed = speed;
	} else {
		pause();
		m_speed = speed;
		resume();
	}
	speed_changed();
}

double
lib::timer_control_impl::get_realtime_speed() const
{
	return m_speed * m_parent->get_realtime_speed();
}

lib::timer_control_impl::time_type 
lib::timer_control_impl::apply_speed_manip(lib::timer::time_type dt) const 
{
	if(m_speed == 1.0) return dt;
	else if(m_speed == 0.0) return 0;
	return time_type(::floor(m_speed*dt + 0.5));
}

#if 0
void 
lib::timer_control_impl::add_listener(lib::timer_events *listener) 
{
	if(!m_listeners) m_listeners = new std::set<lib::timer_events*>();
	typedef std::set<lib::timer_events*>::iterator iterator;
	std::pair<iterator, bool> rv = m_listeners->insert(listener);
	if(!rv.second)
		lib::logger::get_logger()->debug("abstract_timer_client::add_listener: listener already added");
}

void 
lib::timer_control_impl::remove_listener(lib::timer_events *listener)
{
	if(!m_listeners || !m_listeners->erase(listener))
		lib::logger::get_logger()->debug("abstract_timer_client::remove_listener: listener not present");
}
#endif

void
lib::timer_control_impl::speed_changed()
{
	if(!m_listeners) return;
	std::set<lib::timer_events*>::iterator it;
	for(it = m_listeners->begin();it!=m_listeners->end();it++)
		(*it)->speed_changed();
}
