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

#ifndef AMBULANT_SMIL2_TIME_NCTX_H
#define AMBULANT_SMIL2_TIME_NCTX_H

#include "ambulant/config/config.h"
#include "ambulant/lib/node.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/player.h"
#include "ambulant/smil2/smil_time.h"

#include <string>

namespace ambulant {

namespace lib {
	class timer;
}

namespace smil2 {

class animation_engine;

enum src_playstate { src_play, src_pause, src_replace };
enum dst_playstate { dst_play, dst_pause, dst_external };

// Time nodes context requirements
// This interface is used by time nodes to communicate with their environment
// This interface is used by the timegraph builder
class time_node_context : public common::player_feedback {
  public:
	// Services
	virtual time_traits::value_type elapsed() const = 0;
	virtual lib::timer* get_timer() = 0;
	virtual void show_link(const lib::node *n, const net::url& href,
		src_playstate srcstate, dst_playstate dststate, const char * target) = 0;
	virtual animation_engine* get_animation_engine() = 0;
	virtual bool wait_for_eom() const = 0;
	virtual void set_wait_for_eom(bool b) = 0;

	// Playable commands
	virtual common::playable *create_playable(const lib::node *n) = 0;
	virtual void start_playable(const lib::node *n, double t, const lib::transition_info *trans = 0) = 0;
	virtual void stop_playable(const lib::node *n) = 0;
	virtual void pause_playable(const lib::node *n, common::pause_display d = common::display_show) = 0;
	virtual void resume_playable(const lib::node *n) = 0;
	virtual void seek_playable(const lib::node *n, double t) = 0;
	virtual void wantclicks_playable(const lib::node *n, bool want) = 0;
	virtual void start_transition(const lib::node *n, const lib::transition_info *trans, bool in) = 0;

	// Playable queries
	virtual common::duration get_dur(const lib::node *n) = 0;

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
		src_playstate srcstate, dst_playstate dststate, const char * target) {}
	virtual smil2::animation_engine* get_animation_engine() { return 0;}
	virtual bool wait_for_eom() const { return false;}
	virtual void set_wait_for_eom(bool b) {}

	// Playable commands
	virtual common::playable *create_playable(const lib::node *n) { return 0;}
	virtual void start_playable(const lib::node *n, double t, const lib::transition_info *trans = 0) {}
	virtual void stop_playable(const lib::node *n) {}
	virtual void pause_playable(const lib::node *n, common::pause_display d = common::display_show) {}
	virtual void resume_playable(const lib::node *n) {}
	virtual void seek_playable(const lib::node *n, double t) {}
	virtual void wantclicks_playable(const lib::node *n, bool want) {}
	virtual void start_transition(const lib::node *n, const lib::transition_info *trans, bool in) {}

	// Playable queries
	virtual common::duration get_dur(const lib::node *n) {
		return common::duration(true, 1.0);} // allow for fast forward

	// Notifications
	virtual void started_playback() {}
	virtual void done_playback(){}
	virtual void document_loaded(lib::document *doc) {}
	virtual void document_started() {}
	virtual void document_stopped() {}
	virtual void node_started(const lib::node *n) {}
	virtual void node_filled(const lib::node *n) {}
	virtual void node_stopped(const lib::node *n) {}
	virtual void playable_started(const playable *p, const lib::node *n, const char *comment) {};
	virtual void playable_stalled(const playable *p, const char *reason) {};
	virtual void playable_unstalled(const playable *p) {};
	virtual void playable_cached(const playable *p) {};
	virtual void playable_deleted(const playable *p) {};
	virtual void playable_resource(const playable *p, const char *resource, long amount) {};
};

} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_TIME_NCTX_H
