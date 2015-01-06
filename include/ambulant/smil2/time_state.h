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

#ifndef AMBULANT_SMIL2_TIME_STATE_H
#define AMBULANT_SMIL2_TIME_STATE_H

#include "ambulant/config/config.h"

#include "ambulant/smil2/smil_time.h"
#include "ambulant/smil2/time_attrs.h"

namespace ambulant {

namespace smil2 {

enum time_state_type {
	ts_reset,
	ts_proactive,
	ts_active,
	ts_postactive,
	ts_dead
};

const char* time_state_str(time_state_type state);

class time_node;

class time_state : public time_traits {
  public:
	time_state(time_node *tn);
	virtual ~time_state(){}

	virtual void enter(qtime_type timestamp) {}
	virtual void sync_update(qtime_type timestamp);
	virtual void kill(qtime_type timestamp, time_node *oproot);
	virtual void reset(qtime_type timestamp, time_node *oproot);
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() = 0;
	virtual const char* name() const = 0;
	virtual char sig() const = 0;

  protected:
	void report_state(qtime_type timestamp);
	void report_state();
	time_node *m_self;

	// store refs to simplify code layout
	// these are shared variables between the states
	// and the time_node.
	interval_type& m_interval;
	bool& m_active;
	bool& m_needs_remove;
	time_type& m_last_cdur;
	time_type& m_rad;
	time_type& m_pad;
	long& m_precounter;
	time_type& m_impldur;
	const time_attrs& m_attrs;
};

class reset_state : public time_state {
  public:
	reset_state(time_node *tn)
	:	time_state(tn) {}
	virtual ~reset_state(){}

	virtual void enter(qtime_type timestamp);
	virtual void sync_update(qtime_type timestamp);
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() { return ts_reset;}
	virtual const char* name() const { return "reset";}
	virtual char sig() const { return 'r';}
};

class proactive_state : public time_state {
  public:
	proactive_state(time_node *tn)
	:	time_state(tn) {}
	virtual ~proactive_state(){}

	virtual void enter(qtime_type timestamp);
	virtual void sync_update(qtime_type timestamp);
	virtual void kill(qtime_type timestamp, time_node *oproot);
	virtual void reset(qtime_type timestamp, time_node *oproot);
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() { return ts_proactive;}
	virtual const char* name() const { return "proactive";}
	virtual char sig() const { return 'e';}
  private:
	void on_sync_update(qtime_type timestamp);
};

class active_state : public time_state {
  public:
	active_state(time_node *tn)
	:	time_state(tn) {}
	virtual ~active_state(){}

	virtual void enter(qtime_type timestamp);
	virtual void sync_update(qtime_type timestamp);
	virtual void kill(qtime_type timestamp, time_node *oproot);
	virtual void reset(qtime_type timestamp, time_node *oproot);
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() { return ts_active;}
	virtual const char* name() const { return "active";}
	virtual char sig() const { return 'a';}
};

class postactive_state : public time_state {
  public:
	postactive_state(time_node *tn)
	:	time_state(tn) {}
	virtual ~postactive_state(){}

	virtual void enter(qtime_type timestamp);
	virtual void sync_update(qtime_type timestamp);
	virtual void kill(qtime_type timestamp, time_node *oproot);
	virtual void reset(qtime_type timestamp, time_node *oproot);
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() { return ts_postactive;}
	virtual const char* name() const { return "postactive";}
	virtual char sig() const { return 'c';}
};

class dead_state : public time_state {
  public:
	dead_state(time_node *tn)
	:	time_state(tn) {}
	virtual ~dead_state(){}

	virtual void enter(qtime_type timestamp);
	virtual void kill(qtime_type timestamp, time_node *oproot);
	virtual void reset(qtime_type timestamp, time_node *oproot);
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() { return ts_dead;}
	virtual const char* name() const { return "dead";}
	virtual char sig() const { return 'd';}
};

} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_TIME_STATE_H
