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

#ifndef AMBULANT_SMIL2_TIME_STATE_H
#define AMBULANT_SMIL2_TIME_STATE_H

#include "ambulant/config/config.h"

#include "ambulant/smil2/smil_time.h"
#include "ambulant/smil2/time_attrs.h"

namespace ambulant {

namespace lib {

enum time_state_type { 
	ts_reset,
	ts_proactive, 
	ts_active, 
	ts_postactive, 
	ts_dead
};

const char* time_state_str(lib::time_state_type state);

class time_node;

class time_state : public time_traits {
  public:
	time_state(time_node *tn);
	virtual void enter(qtime_type timestamp) {}
	virtual void sync_update(qtime_type timestamp);
	virtual void kill(qtime_type timestamp, time_node *oproot);
	virtual void reset(qtime_type timestamp, time_node *oproot);
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() = 0;
	virtual const char* name() const = 0;

  protected:
	void report_state(qtime_type timestamp);
	
	time_node *m_self;
	
	// store refs to simplify code layout
	// these are shared variables between the states
	// and the time_node. 
	interval_type& m_interval;
	long& m_picounter;
	bool& m_active;
	bool& m_needs_remove;
	time_type& m_last_dur;
	value_type& m_rad;
	long& m_precounter;
	time_type& m_impldur;
	const time_attrs& m_attrs;
};

class reset_state : public time_state {
  public:
	reset_state(time_node *tn)
	:	time_state(tn) {}
	virtual void enter(qtime_type timestamp);
	virtual void sync_update(qtime_type timestamp);
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() { return ts_reset;}
	virtual const char* name() const { return "reset";}
};

class proactive_state : public time_state {
  public:
	proactive_state(time_node *tn)
	:	time_state(tn) {}
	virtual void enter(qtime_type timestamp);
	virtual void sync_update(qtime_type timestamp);
	virtual void kill(qtime_type timestamp, time_node *oproot);
	virtual void reset(qtime_type timestamp, time_node *oproot);	 
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() { return ts_proactive;}
	virtual const char* name() const { return "proactive";}
  private:
	void on_sync_update(qtime_type timestamp);  
};

class active_state : public time_state {
  public:
	active_state(time_node *tn)
	:	time_state(tn) {}
	virtual void enter(qtime_type timestamp);
	virtual void sync_update(qtime_type timestamp);
	virtual void kill(qtime_type timestamp, time_node *oproot);
	virtual void reset(qtime_type timestamp, time_node *oproot);	 
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() { return ts_active;}
	virtual const char* name() const { return "active";}
};

class postactive_state : public time_state {
  public:
	postactive_state(time_node *tn) 
	:	time_state(tn) {}
	virtual void enter(qtime_type timestamp);
	virtual void sync_update(qtime_type timestamp); 
	virtual void kill(qtime_type timestamp, time_node *oproot);
	virtual void reset(qtime_type timestamp, time_node *oproot);	 
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() { return ts_postactive;}
	virtual const char* name() const { return "postactive";}
  private:
	interval_type m_played;
};

class dead_state : public time_state {
  public:
	dead_state(time_node *tn)
	:	time_state(tn) {}
	virtual void enter(qtime_type timestamp);
	virtual void exit(qtime_type timestamp, time_node *oproot);
	virtual time_state_type ident() { return ts_dead;}
	virtual const char* name() const { return "dead";}
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_TIME_STATE_H
