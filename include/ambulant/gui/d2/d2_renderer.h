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

#ifndef AMBULANT_GUI_D2_D2_RENDERER_H
#define AMBULANT_GUI_D2_D2_RENDERER_H

#include "ambulant/common/renderer_impl.h"
#include "ambulant/smil2/transition.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/gui/d2/d2_player.h"

namespace ambulant {

using namespace lib;
using namespace common;

namespace gui {

namespace d2 {

class AMBULANTAPI d2_transition_renderer : public ref_counted_obj {
  public:
	d2_transition_renderer(event_processor *evp)
	:	m_event_processor(evp),
		m_d2player(NULL),
		m_transition_dest(NULL),
		m_intransition(NULL),
		m_outtransition(NULL),
		m_transition_rendertarget(NULL),
		m_fullscreen(false),
		m_fullscreen_checked(false),
		m_trans_engine(NULL)
	{}
	~d2_transition_renderer();

	void set_surface(common::surface *dest);
	void start(double where);
	void stop();
	void redraw_pre(gui_window *window);
	void redraw_post(gui_window *window);
	void set_intransition(const lib::transition_info *info);
	void start_outtransition(const lib::transition_info *info);
	ID2D1RenderTarget* get_current_rendertarget();
	ID2D1BitmapRenderTarget* get_transition_rendertarget();
	void check_fullscreen_outtrans(const lib::node* node);

  protected:
	d2_player* m_d2player;
	d2_player* get_d2player ();
	ID2D1BitmapRenderTarget* m_transition_rendertarget;


  private:
	void transition_step();

	event_processor *m_event_processor;
	surface *m_transition_dest;
	const transition_info *m_intransition;
	const transition_info *m_outtransition;
	smil2::transition_engine *m_trans_engine;
	bool m_fullscreen;
	bool m_fullscreen_checked;
	critical_section m_lock;
};

template <class RP_Base>
class d2_renderer : public d2_resources, public RP_Base {
  public:
	d2_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
	:	RP_Base(context, cookie, node, evp, factory, mdp),
		m_d2player(dynamic_cast<d2_player*>(mdp)),
		m_transition_renderer(new d2_transition_renderer(evp))
	{
		assert(m_d2player);
		m_d2player->register_resources(this);
	};
	~d2_renderer() {
		if(m_d2player)
			m_d2player->unregister_resources(this);
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

	virtual bool stop() {
		stop_transition();
		return RP_Base::stop();
	}

	void redraw(const rect &dirty, gui_window *window) {
		m_transition_renderer->check_fullscreen_outtrans(m_node);
		recreate_d2d();
		m_transition_renderer->redraw_pre(window);
		redraw_body(dirty, window, (ID2D1RenderTarget*) m_transition_renderer->get_current_rendertarget());
		m_transition_renderer->redraw_post(window);
		if (RP_Base::m_erase_never) RP_Base::m_dest->keep_as_background();
	}

	void set_intransition(const transition_info *info) {
		m_transition_renderer->set_intransition(info);
	}

	void start_outtransition(const transition_info *info) {
		m_transition_renderer->start_outtransition(info);
	}
  protected:
	void start_transition(double where) {
		m_transition_renderer->start(where);
	}
	void stop_transition() {
		m_transition_renderer->stop();
	}
	virtual void redraw_body(const rect &dirty, gui_window *window, ID2D1RenderTarget* rt) = 0;

    d2_player *m_d2player;
  private:
	d2_transition_renderer *m_transition_renderer;
};

} // namespace d2

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_D2_D2_RENDERER_H
