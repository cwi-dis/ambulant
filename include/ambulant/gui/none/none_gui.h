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

#ifndef AMBULANT_GUI_NONE_NONE_GUI_H
#define AMBULANT_GUI_NONE_NONE_GUI_H

#include "ambulant/config/config.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"

namespace ambulant {

namespace gui {

namespace none {

/// Dummy implementation of gui_window.
class none_window : public common::gui_window {
  public:
	/// Constructor.
	none_window(const std::string &name, lib::size bounds, common::gui_events *handler)
	:	common::gui_window(handler)
	{};

	void need_redraw(const lib::rect &r) { m_handler->redraw(r, this); };
	void need_events(bool want) {};
	void redraw_now() {};
};

/// Implementation of window_factory that returns none_window objects.
class AMBULANTAPI none_window_factory : public common::window_factory {
  public:
	none_window_factory() {}

	common::gui_window *new_window(const std::string &name, lib::size bounds, common::gui_events *handler);
	common::bgrenderer *new_background_renderer(const common::region_info *src);
};

/// Dummy implementation of playable.
class none_playable : public common::playable_imp {
  public:
	/// Constructor.
	none_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		common::factories *fp,
		common::playable_factory_machdep *mdp);

	void start(double where);
//	void stop();
	bool stop();
	void seek(double where) {}
};

/// Dummy implementation of background_renderer.
class none_background_renderer : public common::background_renderer {
  public:
	/// Constructor.
	none_background_renderer(const common::region_info *src)
	:   background_renderer(src) {}
	~none_background_renderer() {}
	void redraw(const lib::rect &dirty, common::gui_window *window);
	void keep_as_background();
	void highlight(common::gui_window *window);
};

/// Implementation of playable_factory that returns none_playable objects.
class none_playable_factory : public common::playable_factory {
  public:
	/// Constructor.
	none_playable_factory() {}

	/// The none_renderer supports any node, so always return true.
	bool supports(common::renderer_select *)
	{
		return true;
	}

	/// Create a new playable.
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);

	/// Create a new auxiliary (to a video playable) audio playable.
	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src);
};

/// Create a singleton none_window_factory.
AMBULANTAPI common::window_factory *create_none_window_factory();

} // namespace none

} // namespace gui

} // namespace ambulant

#endif // AMBULANT_GUI_NONE_NONE_GUI_H
