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

#ifndef AMBULANT_SMIL2_TIME_NCTX_H
#define AMBULANT_SMIL2_TIME_NCTX_H

#include "ambulant/config/config.h"
#include "ambulant/common/playable.h"
#include "ambulant/smil2/smil_time.h"

#include <string>

namespace ambulant {

namespace lib {
	class node;
	class timer;
}

namespace smil2 {

class animation_engine;

enum src_playstate { src_play, src_pause, src_replace };
enum dst_playstate { dst_play, dst_pause, dst_external };

// Time nodes context requirements
// This interface is used by time nodes to communicate with their environment
// This interface is used by the timegraph builder 
class time_node_context {
  public:
	// Services
	virtual time_traits::value_type elapsed() const = 0;
	virtual lib::timer* get_timer() = 0;
	virtual void show_link(const lib::node *n, const net::url& href, 
		src_playstate srcstate=src_replace, dst_playstate dststate=dst_play) = 0;
	virtual animation_engine* get_animation_engine() = 0;
	virtual bool wait_for_eom() const = 0;
	virtual void set_wait_for_eom(bool b) = 0;
	
	// Playable commands
	virtual common::playable *create_playable(const lib::node *n) = 0;
	virtual void start_playable(const lib::node *n, double t, const lib::node *trans = 0) = 0;
	virtual void stop_playable(const lib::node *n) = 0;
	virtual void pause_playable(const lib::node *n, common::pause_display d = common::display_show) = 0;
	virtual void resume_playable(const lib::node *n) = 0;
	virtual void seek_playable(const lib::node *n, double t) = 0;
	virtual void wantclicks_playable(const lib::node *n, bool want) = 0;
	virtual void start_transition(const lib::node *n, const lib::node *trans, bool in) = 0;
	
	// Playable queries
	virtual std::pair<bool, double> get_dur(const lib::node *n) = 0;
		
	// Notifications
	virtual void started_playback() = 0;
	virtual void done_playback() = 0;
};

// A dummy implementation to be used by algorithms
//
//  For example:
//	time_node_context *algo_player = new dummy_time_node_context();
//	timegraph tg(algo_player, doc, common::schema::get_instance());
//	time_node* timeroot = tg.detach_root();
//  timeroot->start();
//  // apply fast forward algorithm for example
//  // count document states
//  // check if the doc will play from start to end without req events
//	// dislay states, etc
// 
class dummy_time_node_context : public time_node_context {
  public:
	dummy_time_node_context() {}
	virtual ~dummy_time_node_context() {}
	
	// Services
	virtual smil2::time_traits::value_type elapsed() const {return 0;}
	
	virtual lib::timer* get_timer() {return 0;}
	virtual void show_link(const lib::node *n, const net::url& href, 
		src_playstate srcstate=src_replace, dst_playstate dststate=dst_play) {}
	virtual smil2::animation_engine* get_animation_engine() { return 0;}
	virtual bool wait_for_eom() const { return false;}
	virtual void set_wait_for_eom(bool b) {}
	
	// Playable commands
	virtual common::playable *create_playable(const lib::node *n) { return 0;}
	virtual void start_playable(const lib::node *n, double t, const lib::node *trans = 0) {}
	virtual void stop_playable(const lib::node *n) {}
	virtual void pause_playable(const lib::node *n, common::pause_display d = common::display_show) {}
	virtual void resume_playable(const lib::node *n) {}
	virtual void seek_playable(const lib::node *n, double t) {}
	virtual void wantclicks_playable(const lib::node *n, bool want) {}
	virtual void start_transition(const lib::node *n, const lib::node *trans, bool in) {}
	
	// Playable queries
	virtual std::pair<bool, double> get_dur(const lib::node *n) { 
		return std::pair<bool, double>(true, 1.0);} // allow for fast forward
		
	// Notifications
	virtual void started_playback() {}
	virtual void done_playback(){}
};

} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_TIME_NCTX_H
