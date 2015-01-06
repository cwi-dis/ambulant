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
#include "iOSpreferences.h"

class mainloop :
	public ambulant::common::gui_player,
	public ambulant::common::player_feedback,
	public ambulant::common::focus_feedback
{
  public:
	mainloop(const char *filename, void *view, ambulant::common::embedder *app);
	~mainloop();
	
	void restart(bool reparse=true);
	void no_stopped_callbacks() { m_no_stopped_callbacks = true; }
	void init_factories();
	void init_playable_factory();
	void init_window_factory();
	void init_datasource_factory();
	void init_parser_factory();
	void* get_view() { return m_view; }
	ambulant::common::gui_screen *get_gui_screen();
	
	void document_loaded(ambulant::lib::document *doc) {};
	void document_started() {};
	void document_stopped();
	void node_started(const ambulant::lib::node *n);
	void node_stopped(const ambulant::lib::node *n);
	void node_focussed(const ambulant::lib::node *n);
	void node_filled(const ambulant::lib::node*) {};
	void playable_started(const ambulant::common::playable*, const ambulant::lib::node*, const char*) {};
	void playable_stalled(const ambulant::common::playable*, const char*) {};
	void playable_unstalled(const ambulant::common::playable*) {};
	void playable_cached(const ambulant::common::playable*) {};
	void playable_deleted(const ambulant::common::playable*) {};
	void playable_resource(const ambulant::common::playable*, const char*, long int) {};	
	void goto_node_repr(const std::string node_repr);
	PlaylistItem* get_current_item();

  private:
	void* m_view;
	NSURL* m_nsurl;
	PlaylistItem* m_current_item;
	ambulant::common::gui_screen *m_gui_screen;
    const ambulant::lib::node *m_last_node_started;
	void print_nodes();
	bool m_no_stopped_callbacks; // during the 'restart' operation 'document_stopped' is disabled
	NSString* get_meta_content(const char* name);
};
