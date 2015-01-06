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

#ifndef AMBULANT_COMMON_TRACE_PLAYABLE_H
#define AMBULANT_COMMON_TRACE_PLAYABLE_H

#include "ambulant/config/config.h"
#include "ambulant/common/playable.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/logger.h"

#include <string>
#include <utility>

// A non-gui playable for testing scheduler features.

namespace ambulant {

namespace common {

class trace_playable : virtual public playable {

  public:

	trace_playable(const lib::node* n, cookie_type c)
	:	m_node(n),
		m_cookie(c)
	{
		m_logger = lib::logger::get_logger();
		const char *pid = m_node->get_attribute("id");
		m_id = (pid?pid:"no-id");
		m_tag = m_node->get_local_name();
		trace_call("ctr");
	}

	~trace_playable() { trace_call("dtr");}


	void start(double t)  { trace_call("start", t);}

	//void stop()  { trace_call("stop");}
	bool stop()  { trace_call("stop"); 	return true;}
	void post_stop() {}
	void init_with_node(const lib::node *n) {};

	void pause(pause_display d=display_show){ trace_call("pause");}

	void resume() { trace_call("resume");}

	void seek(double t) { trace_call("seek", t);}

	void wantclicks(bool want) { trace_call("wantclicks");}

	void preroll(double when, double where, double how_much) {
		m_logger->trace("PLAYABLE: %s[%s].preroll(%.3f, %.3f, %.3f)", m_tag.c_str(),
			m_id.c_str(), when, where, how_much);
	}


	duration get_dur() {
		trace_call("get_dur");
		return duration(false, 0);
	}

	cookie_type get_cookie() const { return m_cookie; }

  private:
	const lib::node *m_node;
	cookie_type m_cookie;
	std::string m_tag;
	std::string m_id;
	lib::logger *m_logger;

	void trace_call(const char *mfn) {
		m_logger->trace("PLAYABLE: %s[%s].%s()", m_tag.c_str(), m_id.c_str(), mfn);
	}

	void trace_call(const char *mfn, double v) {
		m_logger->trace("PLAYABLE: %s[%s].%s(%.3f)", m_tag.c_str(), m_id.c_str(), mfn, v);
	}
};

} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_COUT_PLAYABLE_H


