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

#include "ambulant/version.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/embedder.h"
#include "ambulant/net/url.h"

class mainloop :
	public ambulant::common::gui_player,
	public ambulant::common::focus_feedback
{
  public:
	mainloop(const char *filename, void *view, ambulant::common::embedder *app);
	~mainloop();

	void restart(bool reparse=true);
	void init_playable_factory();
	void init_window_factory();
	void init_datasource_factory();
	void init_parser_factory();
	ambulant::common::gui_screen *get_gui_screen();

	void node_focussed(const ambulant::lib::node *n);
  private:
	void *m_view;
	ambulant::common::gui_screen *m_gui_screen;
};
