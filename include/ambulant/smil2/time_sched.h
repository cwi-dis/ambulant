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

#ifndef AMBULANT_SMIL2_TIME_SCHED_H
#define AMBULANT_SMIL2_TIME_SCHED_H

#include "ambulant/config/config.h"

#include "ambulant/smil2/time_node.h"
#include "ambulant/lib/timer.h"
#include <map>
#include <list>

namespace ambulant {

namespace smil2 {

class time_node;

class scheduler {
  public:
	typedef lib::timer::time_type time_type;
	
	scheduler(time_node *root, lib::timer *timer);
	~scheduler();
	
	time_type exec();
	time_type exec(time_type now);
	void reset_document();
	void start(time_node *tn);
	
	static void reset(time_node *tn);
	static void set_context(time_node *tn, time_node_context *ctx);
	static bool has_resolved_end(time_node *tn);
	static std::string get_state_sig(time_node *tn);
	
  private:
	void get_pending_events();
	void goto_next(time_node *tn);
	void goto_previous(time_node *tn);
	void restart(time_node *tn);
	void activate_node(time_node *tn);
	void activate_seq_child(time_node *parent, time_node *child);
	void activate_par_child(time_node *parent, time_node *child);
	void activate_excl_child(time_node *parent, time_node *child);
	void activate_media_child(time_node *parent, time_node *child);
	void set_ffwd_mode(time_node *tn, bool b);
	
	time_node *m_root;
	lib::timer *m_timer;
	time_type m_horizon;
	
	bool m_locked;
	void lock() { m_locked = true;}
	void unlock() { m_locked = false;}
	bool locked() const { return m_locked;}
	
	typedef std::map<time_node::time_type, std::list<time_node*> > event_map_t;
	event_map_t m_events;
	enum { idle_resolution = 100};
};


} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_TIME_SCHED_H
