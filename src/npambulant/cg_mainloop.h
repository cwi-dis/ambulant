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
#include "ambulant/gui/cg/cg_gui.h"

class cg_mainloop :
	public ambulant::common::gui_player,
	public ambulant::common::player_feedback
{
  public:
	cg_mainloop(const char *filename, void *view,
		bool use_mms, ambulant::common::embedder *app);
	~cg_mainloop();

	void restart(bool reparse=true);
	void init_playable_factory();
	void init_window_factory();
	void init_datasource_factory();
	void init_parser_factory();
	ambulant::common::gui_screen *get_gui_screen();

	void document_loaded(ambulant::lib::document *doc) {};
	void document_started() {};
	void document_stopped() {};
	void node_started(const ambulant::lib::node *n) {};
	void node_stopped(const ambulant::lib::node *n) {};
	void node_focussed(const ambulant::lib::node *n);
	void update(CGContextRef ctx);
	const CGSize get_size_from_doc();
	// dummies, for now
	void node_filled(const ambulant::lib::node*) {}
	void playable_started(const ambulant::common::playable*, const ambulant::lib::node*, const char*) {}
	void playable_stalled(const ambulant::common::playable*, const char*) {}
	void playable_unstalled(const ambulant::common::playable*) {}
	void playable_cached(const ambulant::common::playable*) {}
	void playable_deleted(const ambulant::common::playable*) {}
	void playable_resource(const ambulant::common::playable*, const char*, long int) {}
  private:
	void* m_view;
	ambulant::common::gui_screen *m_gui_screen;
};
