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
