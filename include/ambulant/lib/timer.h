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

#include <set>

namespace ambulant {

namespace lib {

class abstract_timer_client {
  public:
	// this timer time type (assumed in msecs)
	typedef std::set<abstract_timer_client *> client_set;
	typedef client_set::iterator client_index;
	
	virtual ~abstract_timer_client() {}

	virtual void speed_changed();
	// A child clock can call add_dependent to have calls that change
	// timing (such as set_speed) forwarded to it
	client_index add_dependent(abstract_timer_client *child);
	void remove_dependent(client_index pos);
	void remove_dependent(abstract_timer_client *child);
	
  protected:
    client_set m_dependents;
};

class abstract_timer : public abstract_timer_client {
  public:
	// this timer time type (assumed in msecs)
	typedef unsigned long time_type;
	
	virtual ~abstract_timer() {}
		
	// returns the time elapsed
	// e.g. return (time_now>ref_time)?time_now - ref_time:0;
	virtual time_type elapsed() const = 0;
	virtual void set_speed(double speed) = 0;
	virtual double get_realtime_speed() const = 0;
};

// Note: timer objects are not refcounted, because it is assumed that
// a parent timer will always automatically live longer than a child
// timer. If this does not hold in future then we should add refcounting.
class timer : public abstract_timer {
  public:
	timer(abstract_timer *parent, double speed);
	timer(abstract_timer *parent);
	~timer();
//	void add_dependent(timer& child);
		
	// returns the time elapsed
	// e.g. return (time_now>ref_time)?time_now - ref_time:0;
	time_type elapsed() const;
	void set_speed(double speed);
	double lib::timer::get_realtime_speed() const;
  private:
	void re_epoch();
	
	abstract_timer *m_parent;
//	std::vector<timer&> m_children;
	time_type m_parent_epoch;
	time_type m_local_epoch;
	double m_speed;
};

// A (machine-dependent) routine to create a timer object
abstract_timer *realtime_timer_factory();

} // namespace lib
 
} // namespace ambulant
#endif // AMBULANT_LIB_TIMER_H


