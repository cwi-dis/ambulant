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
#include "ambulant/lib/logger.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::abstract_timer_client::client_index
lib::abstract_timer_client::add_dependent(abstract_timer_client *child)
{
	std::pair<client_index, bool> rv = m_dependents.insert(child);
	if (!rv.second)
		lib::logger::get_logger()->fatal("abstract_timer_client::add_dependent: child already added");
	return rv.first;
}

void
lib::abstract_timer_client::remove_dependent(client_index pos)
{
	m_dependents.erase(pos);
}

void
lib::abstract_timer_client::remove_dependent(abstract_timer_client *child)
{
	if (!m_dependents.erase(child))
		lib::logger::get_logger()->fatal("abstract_timer_client::remove_dependent: child not present");
}

void
lib::abstract_timer_client::speed_changed()
{
	client_index i;
	for (i=m_dependents.begin(); i != m_dependents.end(); i++)
		(*i)->speed_changed();
}

lib::timer::timer(lib::abstract_timer* parent, double speed)
:   m_parent(parent),
	m_parent_epoch(parent->elapsed()),
	m_local_epoch(0),
	m_speed(speed)
{
}

lib::timer::timer(lib::abstract_timer* parent)
:   m_parent(parent),
	m_parent_epoch(parent->elapsed()),
	m_local_epoch(0),
	m_speed(1.0)
{
}

lib::timer::~timer()
{
}

lib::timer::time_type
lib::timer::elapsed() const
{
	return (lib::timer::time_type)(m_local_epoch + m_speed*(m_parent->elapsed()-m_parent_epoch));
}

void
lib::timer::set_speed(double speed)
{
	re_epoch();
	m_speed = speed;
	speed_changed();
}

double
lib::timer::get_realtime_speed() const
{
	return m_speed * m_parent->get_realtime_speed();
}

void
lib::timer::re_epoch()
{
	// Could be off by a little, but too lazy to do it better right now:-)
	m_local_epoch = elapsed();
	m_parent_epoch = m_parent->elapsed();
}
