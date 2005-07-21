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
 * $Id$
 */

#ifndef AMBULANT_GUI_QT_QT_RENDERER_H
#define AMBULANT_GUI_QT_QT_RENDERER_H


#include "ambulant/common/factory.h"
#include "ambulant/common/renderer.h"
#include "ambulant/smil2/transition.h"
#include "ambulant/lib/mtsync.h"

namespace ambulant {

using namespace lib;
//JNK using namespace net;
using namespace common;

namespace gui {

namespace qt {

class qt_transition_renderer : public ref_counted_obj {
  public:
	qt_transition_renderer(event_processor *evp)
	:	m_event_processor(evp),
		m_transition_dest(NULL),
		m_view(NULL),
		m_intransition(NULL),
		m_outtransition(NULL),
#ifdef USE_SMIL21
		m_fullscreen(false),
#endif
		m_trans_engine(NULL) {};
	~qt_transition_renderer();

	void set_surface(common::surface *dest);
	void start(double where);
	void stop();
	void redraw_pre(gui_window *window);
	void redraw_post(gui_window *window);
	void set_intransition(const lib::transition_info *info);
	void start_outtransition(const lib::transition_info *info);

  protected:
  private:
	void transition_step();

	event_processor* m_event_processor;
	surface* m_transition_dest;
	void* m_view; //ambulant_qt_window* 
	const lib::transition_info* m_intransition;
	const lib::transition_info* m_outtransition;
#ifdef USE_SMIL21
	bool m_fullscreen;
#endif
	smil2::transition_engine* m_trans_engine;
	critical_section m_lock;
};

template <class RP_Base>
class qt_renderer : public RP_Base {
  public:
	qt_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory)
	:	RP_Base(context, cookie, node, evp, factory),
		m_transition_renderer(new qt_transition_renderer(evp)) {};
	qt_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp)
	:	RP_Base(context, cookie, node, evp),
		m_transition_renderer(new qt_transition_renderer(evp)) {};
	~qt_renderer() {
		m_transition_renderer->release();
	}

	void set_surface(common::surface *dest) {
		RP_Base::set_surface(dest);
		m_transition_renderer->set_surface(dest);
	}

	virtual void start(double where) {
		start_transition(where);
		RP_Base::start(where);
	}
	
 	virtual void stop() {
		stop_transition();
		RP_Base::stop();
	}

	void redraw(const rect &dirty, gui_window *window) {
		m_transition_renderer->redraw_pre(window);
		redraw_body(dirty, window);
		m_transition_renderer->redraw_post(window);
		if (RP_Base::m_erase_never) RP_Base::m_dest->keep_as_background();
	}

	void set_intransition(const lib::transition_info *info) {
		m_transition_renderer->set_intransition(info);
	}
	
	void start_outtransition(const lib::transition_info *info) {
		m_transition_renderer->start_outtransition(info);
	}
  protected:
	void start_transition(double where) {
		m_transition_renderer->start(where);
	}
	void stop_transition() {
		m_transition_renderer->stop();
	}
	virtual void redraw_body(const rect &dirty, gui_window *window) = 0;

  private:
	qt_transition_renderer *m_transition_renderer;
};

} // namespace qt

} // namespace gui

} // namespace ambulant

#endif  /*AMBULANT_GUI_QT_QT_RENDERER_H*/
