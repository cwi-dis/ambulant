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

#ifndef FACTORY_H
#define FACTORY_H

#include "ambulant/lib/parser_factory.h"
#include "ambulant/net/datasource.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/recorder.h"
#include "ambulant/common/state.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/timer_sync.h"

namespace ambulant {

namespace common {

/// Convenience class that stores all per-document factory objects.
/// As of Ambulant 1.8 this class is usually used through gui_player
/// (which extends it).

class AMBULANTAPI factories {
public:
	factories();
	virtual ~factories();
	/// Initialize all factories by calling the other init_xxx() methods.
	virtual void init_factories();
	/// Create and populate the playable factory.
	virtual void init_playable_factory();
	/// Create the window factory.
	virtual void init_window_factory();
	/// Create and populate the datasource factory.
	virtual void init_datasource_factory();
	/// Create and populate the parser factory.
	virtual void init_parser_factory();
	/// Create the node factory.
	virtual void init_node_factory();
	/// Create the state factory.
	virtual void init_state_component_factory();
    /// Create the timer-synchronizer factory.
    virtual void init_timer_sync_factory();
	/// Create the recorder factory.
	virtual void init_recorder_factory();
	/// Return the playable factory.
	virtual global_playable_factory *get_playable_factory() const { return m_playable_factory; }
	/// Return the window factory.
	virtual window_factory *get_window_factory() const { return m_window_factory; }
	/// Return the datasource factory.
	virtual net::datasource_factory *get_datasource_factory() const { return m_datasource_factory; }
	/// Return the parser factory.
	virtual lib::global_parser_factory *get_parser_factory() const { return m_parser_factory; }
	/// Return the node factory.
	virtual lib::node_factory *get_node_factory() const { return m_node_factory; }
	/// Return the state factory.
	virtual global_state_component_factory *get_state_component_factory() const { return m_state_component_factory; }
	/// Return the recorder factory.
	virtual recorder_factory *get_recorder_factory() const { return m_recorder_factory; }
	/// Override the playable factory. Deletes the old one, if needed.
	virtual void set_playable_factory(global_playable_factory *pf) { delete m_playable_factory; m_playable_factory = pf; }
	/// Override the playable factory.
	virtual void set_window_factory(window_factory *wf) { m_window_factory = wf; }
	/// Override the playable factory. Deletes the old one, if needed.
	virtual void set_datasource_factory(net::datasource_factory *df) { delete m_datasource_factory; m_datasource_factory = df; }
	/// Override the playable factory.
	virtual void set_parser_factory(lib::global_parser_factory *pf) { m_parser_factory = pf; }
	/// Override the playable factory.
	virtual void set_node_factory(lib::node_factory *nf) { m_node_factory = nf; }
	/// Override the state factory.
	virtual void set_state_component_factory(global_state_component_factory *sf) { delete m_state_component_factory; m_state_component_factory = sf; }
    /// Return the timer-synchronizer factory.
    virtual lib::timer_sync_factory *get_timer_sync_factory() const { return m_timer_sync_factory; }
    /// Override the timer-synchronizer factory.
    virtual void set_timer_sync_factory(lib::timer_sync_factory *tsf) { /*delete m_timer_sync_factory;*/ m_timer_sync_factory = tsf; }
	/// Override the recorder factory. Deletes the old one, if needed.
	virtual void set_recorder_factory(recorder_factory *rf) { if(m_recorder_factory) delete m_recorder_factory; m_recorder_factory = rf; }
private:
	global_playable_factory *m_playable_factory;
	window_factory *m_window_factory;
	net::datasource_factory *m_datasource_factory;
	lib::global_parser_factory *m_parser_factory;
	lib::node_factory *m_node_factory;
	common::global_state_component_factory *m_state_component_factory;
    lib::timer_sync_factory *m_timer_sync_factory;
	recorder_factory *m_recorder_factory;
};

} // end namespaces
} // end namespaces
#endif /* _FACTORY_H */
