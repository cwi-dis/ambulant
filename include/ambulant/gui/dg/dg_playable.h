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

#ifndef AMBULANT_GUI_DG_PLAYABLE_H
#define AMBULANT_GUI_DG_PLAYABLE_H

#include "ambulant/config/config.h"
#include "ambulant/common/renderer.h"

namespace ambulant {

namespace gui {

namespace dg {

class dg_transition;

class dg_playables_context {
  public:
	virtual void stopped(common::playable *p) = 0;
	virtual void paused(common::playable *p) = 0;
	virtual void resumed(common::playable *p) = 0;
	virtual void set_intransition(common::playable *p, const lib::transition_info *info) = 0;
	virtual void start_outtransition(common::playable *p, const lib::transition_info *info) = 0; 
	virtual dg_transition *get_transition(common::playable *p) = 0;
};

class dg_renderer_playable : public common::renderer_playable {
  public:
	dg_renderer_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor* evp,
		common::gui_window *window, 
		dg_playables_context *dgplayer) 
	:	common::renderer_playable(context, cookie, node, evp), 
		m_window(window), m_dgplayer(dgplayer), m_transitioning(false) {}
	
	void set_intransition(const lib::transition_info *info) {
		//m_transitioning = true; 
		//m_dgplayer->set_intransition(this, info);
	}
	void start_outtransition(const lib::transition_info *info) {  
		//m_transitioning = true; 
		//m_dgplayer->start_outtransition(this, info);
	}
  protected:
	common::gui_window *m_window;
	dg_playables_context *m_dgplayer;
	bool m_transitioning;
};

} // namespace dg

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DG_PLAYABLE_H
