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

#ifndef GUI_PLAYER_H
#define GUI_PLAYER_H

#include "ambulant/config/config.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/node.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/player.h"

namespace ambulant {

namespace common {

/// This class allows plugins access to the screen (window, actually).
class gui_screen {
  public:
	virtual ~gui_screen() {};

	/// Return size of the ambulant output window.
	virtual void get_size(int *width, int *height) = 0;

	/// Obtain a screenshot.
	/// Type is a string such as "png" or "jpeg". The caller is responsible
	/// for freeing out_data when done.
	virtual bool get_screenshot(const char *type, char **out_data, size_t *out_size) = 0;

};

/// Complete Ambulant Player, base class for GUI.
/// Contains all factories, loads plugins, parses and plays document,
/// handles feedback from Ambulant Player to user interface code.
/// Some methods are expected to be overridden, some are not, some can
/// optionally be overridden.
class AMBULANTAPI gui_player : public factories {
  public:
	gui_player()
	:	factories(),
		m_doc(NULL),
		m_embedder(NULL),
		m_player(NULL),
		m_goto_node(NULL) {}
	virtual ~gui_player();

	/// Initialize playable factory.
	/// Usually overridden to supply platform- or gui-toolkit-dependent
	/// renderers.
	virtual void init_playable_factory() { factories::init_playable_factory(); }
	/// Initialize window factory.
	/// Always overridden to allow the player to actually create windows.
	virtual void init_window_factory() { factories::init_window_factory(); }
	/// Initialize datasource factory.
	/// Override when the embedder supplies non-standard datasources.
	virtual void init_datasource_factory() { factories::init_datasource_factory(); }
	/// Initialize parser factory.
	/// Override when the embedder supplies additional XML parsers.
	virtual void init_parser_factory() { factories::init_parser_factory(); }
	/// Load and initialize plugins.
	/// Called at end of initialization, to load and initialize plugins
	/// (which will in turn extend the various factories).
	/// Usually not overridden.
	virtual void init_plugins();

	/// Start document playback. Usually not overridden.
	virtual void play();
	/// Stop document playback. Usually not overridden.
	virtual void stop();
	/// Pause document playback. Usually not overridden.
	virtual void pause();

	/// Restart document playback.
	/// If reparse is true the document is re-read, otherwise the in-core
	/// tree is restarted from the beginning. Usually called when the embedder
	/// has modified the document (either the external representation or
	/// the incore tree).
	virtual void restart(bool reparse=true);

	/// Set initial playback node.
	/// Embedder can call this before start() to make the document
	/// start at this node (in stead of at the root).
	virtual void goto_node(const lib::node *n);

//	virtual void set_speed(double speed) = 0;
//	virtual double get_speed() const = 0;

	/// Call this to check whether "play" menu entry should be enabled.
	virtual bool is_play_enabled() const;
	/// Call this to check whether "stop" menu entry should be enabled.
	virtual bool is_stop_enabled() const;
	/// Call this to check whether "pause" menu entry should be enabled.
	virtual bool is_pause_enabled() const;
	/// Call this to check whether "play" menu entry should be flagged.
	virtual bool is_play_active() const;
	/// Call this to check whether "stop" menu entry should be flagged.
	virtual bool is_stop_active() const;
	/// Call this to check whether "pause" menu entry should be flagged.
	virtual bool is_pause_active() const;

	/// Signal to player that a mousemove event is about to be communicated.
	/// The embedder MUST call this method before communicating the mouse move
	/// the the ambulant player core through the gui_event.
	virtual void before_mousemove(int cursor);
	/// Signal to player that mousemove event handling is done.
	/// The embedder MUST call this method after communicating the mouse move
	/// the the ambulant player core through the gui_event.
	virtual int after_mousemove();

	/// Embedder calls this to communicate keypress to the player.
	virtual void on_char(int c);
	/// Embedder calls this to communicate focus advance to the player.
	virtual void on_focus_advance();
	/// Embedder calls this to communicate focus activate to the player.
	virtual void on_focus_activate();

	/// Obtain the current document from the player.
	virtual lib::document *get_document() const { return m_doc; }
	/// Set the document to play.
	virtual void set_document(lib::document *doc) { m_doc = doc; }

	/// Get the current embedder object.
	/// The embedding application can use this if it wants to extend
	/// the embedder functionality while keeping the original embedder
	/// object.
	virtual embedder *get_embedder() const { return m_embedder; }
	/// Set the embedder object.
	virtual void set_embedder(embedder *em) { m_embedder = em; }

	/// Get the player object.
	/// The embedding application can use this if it wants to extend the
	/// player object while using the original player to implement most
	/// functionality.
	virtual player *get_player() const { return m_player; }
	/// Set the player object.
	virtual void set_player(player *pl) { if (m_player) m_player->release(); m_player = pl; }

	/// Get URL of current document.
	virtual net::url get_url() const { return m_url; }

	/// Embedder should implement this if it implements the gui_screen interface.
	virtual gui_screen *get_gui_screen() { return NULL; }

	/// ???
	static void load_test_attrs(std::string& filename);

	/// Simulate a click (activate) on a node, from an external agent.
    virtual void clicked_external(lib::node *n, lib::timer::time_type t) { if (m_player) m_player->clicked_external(n, t); }

    bool uses_external_sync() const { if (m_player) return m_player->uses_external_sync(); return false; }

protected:
	/// Convenience method to parse XML.
	lib::document *create_document(const net::url& url);
	/// The URL for the current document.
	net::url m_url;
	/// The current document.
	lib::document *m_doc;
	/// The object embedding this gui_player, which receives notifications.
	embedder *m_embedder;
	/// The player for the current document.
	player *m_player;
	/// Hack: the node at which playback should start. See the goto_node() code.
	const lib::node *m_goto_node;
	/// Critical section.
	lib::critical_section m_lock;
};

} // end namespaces
} // end namespaces
#endif /* GUI_PLAYER_H */
