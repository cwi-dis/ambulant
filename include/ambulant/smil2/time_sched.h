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

	scheduler(time_node *root, lib::timer_control *timer);
	~scheduler();

	time_type exec();
	void reset_document();
	void start(time_node *tn);
	void update_horizon(time_type t);

	static void reset(time_node *tn);
	static void set_context(time_node *tn, time_node_context *ctx);
	static bool has_resolved_end(time_node *tn);
	static std::string get_state_sig(time_node *tn);

  private:
	void _reset_document();
	time_type _exec();
	time_type _exec(time_type now);

	void goto_next(time_node *tn);
	void goto_previous(time_node *tn);
	void restart(time_node *tn);
	void activate_node(time_node *tn);
	void activate_seq_child(time_node *parent, time_node *child);
	void activate_par_child(time_node *parent, time_node *child);
	void activate_excl_child(time_node *parent, time_node *child);
	void activate_media_child(time_node *parent, time_node *child);
	void set_ffwd_mode(time_node *tn, bool b);
	void sync_playable_clocks(time_node *tnroot, time_node *tntarget);

	time_node *m_root;
	lib::timer_control *m_timer;
	time_type m_horizon;

	bool m_locked;
	lib::critical_section m_lock;
	//xxxbo 2012-02-15
	// reduce the idle_resolution from 100 milliseconds to 10 milliseconds to 
	// speed up the seamless playback performance for MyVideos
#if 0
	enum { idle_resolution = 100};
#else
	enum { idle_resolution = 10};
#endif //xxxbo 2012-02-15 end 
  public:
	void lock();
	void unlock();
	bool locked() const { return m_locked;}
};


} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_TIME_SCHED_H
