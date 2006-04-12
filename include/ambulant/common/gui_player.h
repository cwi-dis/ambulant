/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
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

#ifndef GUI_PLAYER_H
#define GUI_PLAYER_H

#include "ambulant/config/config.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/player.h"

namespace ambulant {

namespace common {

/// This class allows plugins access to the screen.
class gui_screen {
  public:
	virtual ~gui_screen() {};
	
	virtual void get_size(int *width, int *height) = 0;
	virtual bool get_screenshot(const char *type, char **out_data, size_t *out_size) = 0;
	virtual bool set_overlay(const char *type, const char *data, size_t size) = 0;
	virtual bool clear_overlay() = 0;
};

class AMBULANTAPI gui_player : public factories {
  public:
	gui_player()
	:	factories(),
		m_doc(NULL),
		m_embedder(NULL),
		m_player(NULL),
		m_goto_node(NULL) {}
	virtual ~gui_player();
	
	virtual void init_playable_factory() { factories::init_playable_factory(); }
	virtual void init_window_factory() { factories::init_window_factory(); }
	virtual void init_datasource_factory() { factories::init_datasource_factory(); }
	virtual void init_parser_factory() { factories::init_parser_factory(); }
	virtual void init_plugins();
	
	virtual void play();
	virtual void stop();
	virtual void pause();
	
	virtual void restart(bool reparse=true);
	
	virtual void goto_node(const lib::node *n);

//	virtual void set_speed(double speed) = 0;
//	virtual double get_speed() const = 0;
	
	virtual bool is_play_enabled() const;
	virtual bool is_stop_enabled() const;
	virtual bool is_pause_enabled() const;
	virtual bool is_play_active() const;
	virtual bool is_stop_active() const;
	virtual bool is_pause_active() const;
	
	virtual int after_mousemove();
	virtual void before_mousemove(int cursor);
	
	virtual void on_char(int c);
	virtual void on_focus_advance();
	virtual void on_focus_activate();
	
	virtual lib::document *get_document() const { return m_doc; }
	virtual void set_document(lib::document *doc) { m_doc = doc; }
	
	virtual embedder *get_embedder() const { return m_embedder; }
	virtual void set_embedder(embedder *em) { m_embedder = em; }
	
	virtual player *get_player() const { return m_player; }
	virtual void set_player(player *pl) { m_player = pl; }

	virtual net::url get_url() const { return m_url; }
	
	virtual gui_screen *get_gui_screen() { return NULL; }
	
	static void load_test_attrs(std::string& filename);
  protected:
	lib::document *create_document(const net::url& url);
	net::url m_url;
	lib::document *m_doc;
	embedder *m_embedder;
	player *m_player;
	const lib::node *m_goto_node;
	lib::critical_section m_lock;
};

} // end namespaces
} // end namespaces
#endif /* GUI_PLAYER_H */
