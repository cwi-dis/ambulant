/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_MMS_MMS_PLAYER_H
#define AMBULANT_MMS_MMS_PLAYER_H

#include "ambulant/config/config.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/player.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/layout.h"
#include "ambulant/mms/timelines.h"
#include "ambulant/common/playable.h"

namespace ambulant {

namespace mms {

class lib::document;

class mms_player : public common::player, public lib::ref_counted_obj {
  public:
	mms_player(lib::document *doc, common::factories* factory);
	void initialize() {}
	~mms_player();

	virtual lib::timer* get_timer() { return m_event_processor->get_timer(); }
	virtual lib::event_processor* get_evp() { return m_event_processor; }
	void start();
	void stop();
	void set_speed(double speed);
	
	void pause();
	void resume();
	bool is_done() const {return m_done;}
	double get_speed() const;
	
  private:
	inline void timeline_done_callback() {
		m_done = true;
	}
	
  	passive_timeline *build_timeline();
	
	lib::document *m_doc;
	lib::node *m_tree;
	lib::timer_control *m_timer;
  	double m_pause_speed;
	lib::event_processor *m_event_processor;
	bool m_playing;
	std::vector<active_timeline *> m_active_timelines;
	bool m_done;
	common::factories* m_factory;
	};

} // namespace mms
 
} // namespace ambulant

#endif // AMBULANT_MMS_MMS_PLAYER_H
