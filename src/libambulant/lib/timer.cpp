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

lib::timer::timer(lib::abstract_timer* parent, double speed /* = 1.0 */, bool running /* = true */)
:   m_parent(parent),
	m_parent_epoch(parent->elapsed()),
	m_local_epoch(0),
	m_speed(speed),
	m_running(running),
	m_period(infinite),
	m_listeners(0)
{	
	AM_DBG lib::logger::get_logger()->debug("lib::timer()");
}

lib::timer::~timer()
{
	// This class does not own event listeners.
	// Therefore, deleting the container is enough
	delete m_listeners;
	AM_DBG lib::logger::get_logger()->debug("~lib::timer()");
}

lib::timer::time_type
lib::timer::elapsed() const
{
	if(!m_running) return m_local_epoch;
	return m_local_epoch + apply_speed_manip(m_parent->elapsed() - m_parent_epoch);
}

lib::timer::time_type
lib::timer::elapsed(time_type pe) const
{
	if(!m_running) return m_local_epoch;
	return m_local_epoch + apply_speed_manip(pe - m_parent_epoch);
}

void lib::timer::start(time_type t /* = 0 */) {
	m_parent_epoch = m_parent->elapsed();
	m_local_epoch = t;
	m_running = true;
}

void lib::timer::stop() {
	m_local_epoch = 0;
	m_running = false;
}
	
void lib::timer::pause() {
	if(m_running) {
		m_local_epoch += apply_speed_manip(m_parent->elapsed() - m_parent_epoch);
		m_running = false;
	}
}
	
void lib::timer::resume() {
	if(!m_running) {
		m_parent_epoch = m_parent->elapsed();
		m_running = true;
	}
}

void lib::timer::set_time(time_type t) {
	if(!m_running) {
		m_local_epoch = t;
	} else {
		pause();
		m_local_epoch = t;
		resume();
	}
}	

lib::timer::time_type 
lib::timer::get_time() const {
	return (m_period == infinite)?elapsed():(elapsed() % m_period);
}

lib::timer::time_type 
lib::timer::get_repeat() const {
	return (m_period == infinite)?0:(elapsed() / m_period);
}

void lib::timer::set_speed(double speed)
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
lib::timer::get_realtime_speed() const
{
	return m_speed * m_parent->get_realtime_speed();
}

lib::timer::time_type 
lib::timer::apply_speed_manip(lib::timer::time_type dt) const 
{
	if(m_speed == 1.0) return dt;
	else if(m_speed == 0.0) return 0;
	return time_type(::floor(m_speed*dt + 0.5));
}

void 
lib::timer::add_listener(lib::timer_events *listener) 
{
	if(!m_listeners) m_listeners = new std::set<lib::timer_events*>();
	typedef std::set<lib::timer_events*>::iterator iterator;
	std::pair<iterator, bool> rv = m_listeners->insert(listener);
	if(!rv.second)
		lib::logger::get_logger()->warn("abstract_timer_client::add_listener: listener already added");
}

void 
lib::timer::remove_listener(lib::timer_events *listener)
{
	if(!m_listeners || !m_listeners->erase(listener))
		lib::logger::get_logger()->warn("abstract_timer_client::remove_listener: listener not present");
}

void
lib::timer::speed_changed()
{
	if(!m_listeners) return;
	std::set<lib::timer_events*>::iterator it;
	for(it = m_listeners->begin();it!=m_listeners->end();it++)
		(*it)->speed_changed();
}
