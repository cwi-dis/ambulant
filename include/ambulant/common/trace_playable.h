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

using namespace lib;

class trace_playable : public playable {

  public:
  	
	trace_playable(const node* n, cookie_type c) 
	:	m_node(n),
		m_cookie(c) {
		m_logger = logger::get_logger();
		const char *pid = m_node->get_attribute("id");
		m_id = (pid?pid:"no-id");
		m_tag = m_node->get_local_name();
		trace_call("ctr");
	}
	
	~trace_playable() { trace_call("dtr");}
	
	
	void start(double t)  { trace_call("start", t);}
	
	void stop()  { trace_call("stop");}
	
	void pause(){ trace_call("pause");}
	
	void resume() { trace_call("resume");}
	
	void seek(double t) { trace_call("seek", t);}

	void wantclicks(bool want) { trace_call("wantclicks");}
	
	void preroll(double when, double where, double how_much) {
		m_logger->trace("PLAYABLE: %s[%s].preroll(%.3f, %.3f, %.3f)", m_tag.c_str(), 
			m_id.c_str(), when, where, how_much);	
	}
	
	std::pair<bool, double> get_dur() {
		trace_call("get_dur");
		return std::pair<bool, double>(false, 0);
	}

	const cookie_type& get_cookie() const { return m_cookie; }

  private:
	const node *m_node;
	const cookie_type m_cookie;
	std::string m_tag;
	std::string m_id;
	logger *m_logger;
		
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


