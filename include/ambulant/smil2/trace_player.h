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

#ifndef AMBULANT_SMIL2_TRACE_PLAYER_H
#define AMBULANT_SMIL2_TRACE_PLAYER_H

#include "ambulant/lib/timer.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/event.h"
#include "ambulant/lib/node.h"
#include "ambulant/smil2/time_node.h"
#include "ambulant/common/playable.h"
#include "ambulant/lib/logger.h"

#include <map>

/*
	A non-gui player for testing scheduler features.

USAGE:
	#include "ambulant/common/trace_player.h"
	#include "ambulant/lib/document.h"
	#include "ambulant/lib/asb.h"
	#include "ambulant/lib/logger.h"

	int main(int argc, char **argv) {
		lib::logger *logger = lib::logger::get_logger();

		std::string filename = "xxxx.smil";

		lib::document *doc = lib::document::create_from_file(filename);
		if(!doc) {
			logger->error("Failed to parse document %s", filename.c_str());
			return 1;
		}
		lib::trace_player *player = new lib::trace_player(doc);
		logger->trace("Start playing : %s", filename.c_str());
		player->start();
		lib::sleep(1);
		while(!player->is_done()) {
			lib::sleep(1);
		}
		logger->trace("Done playing : %s", filename.c_str());
		player->stop();
		delete player;
	}
*/

namespace ambulant {

namespace smil2 {

class trace_player : public time_node_context,
	public common::playable_notification {
  public:
	typedef time_traits::value_type time_value_type;

	trace_player(lib::document *doc);
	~trace_player();

	lib::timer_control* get_timer() { return m_timer;}
	lib::event_processor* get_evp() { return m_event_processor;}

	///////////////////
	// UI commands

	void start();
	void stop();
	void pause();
	void resume();
	bool is_done();

	///////////////////
	// time_node_context

	// Services
	virtual time_traits::value_type elapsed() const { return m_timer->elapsed();}
	virtual void schedule_event(lib::event *ev, lib::timer::time_type t, lib::event_priority ep = ep_low);
	virtual void cancel_event(lib::event *ev, lib::event_priority ep = ep_low)
		{ m_event_processor->cancel_event(ev, ep);}
	virtual void cancel_all_events()
		{ m_event_processor->cancel_all_events();}

	// Playable commands
	virtual void start_playable(const lib::node *n, double t, const lib::transition_info *trans = 0);
	virtual void stop_playable(const lib::node *n);
	virtual void pause_playable(const lib::node *n, common::pause_display d = common::display_show);
	virtual void resume_playable(const lib::node *n);

	// Playable queries
	virtual common::duration get_dur(const lib::node *n);

	// Notifications
	// none

	///////////////////
	// playable_notification interface

	virtual void started(int, double t){}
	virtual void stopped(int, double t) {}
	virtual void clicked(int, double t){}

  private:
	common::playable *get_playable(const node *n);
	lib::document *m_doc;
	time_node* m_root;
	lib::timer_control *m_timer;
	lib::event_processor *m_event_processor;
	std::map<const node*, playable *> m_playables;

	lib::logger *m_logger;

	void trace_call(const char *mfn) {
		m_logger->trace("%s()", mfn);
	}

	void trace_call(const char *mfn, double v) {
		m_logger->trace("%s(%.3f)", mfn, v);
	}

};

} // namespace common

} // namespace ambulant

#endif // AMBULANT_SMIL2_TRACE_PLAYER_H
