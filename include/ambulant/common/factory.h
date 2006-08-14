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

#ifndef FACTORY_H
#define FACTORY_H

#include "ambulant/lib/parser_factory.h"
#include "ambulant/net/datasource.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/layout.h"
#include "ambulant/lib/node.h"

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
	void init_factories();
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
	
	/// Return the playable factory.
	global_playable_factory *get_playable_factory() const { return m_playable_factory; }
	/// Return the window factory.
	window_factory *get_window_factory() const { return m_window_factory; }
	/// Return the datasource factory.
	net::datasource_factory *get_datasource_factory() const { return m_datasource_factory; }
	/// Return the parser factory.
	lib::global_parser_factory *get_parser_factory() const { return m_parser_factory; }
	/// Return the node factory.
	lib::node_factory *get_node_factory() const { return m_node_factory; }
	
	/// Override the playable factory. Deletes the old one, if needed.
	void set_playable_factory(global_playable_factory *pf) { delete m_playable_factory; m_playable_factory = pf; }
	/// Override the playable factory.
	void set_window_factory(window_factory *wf) { m_window_factory = wf; }
	/// Override the playable factory. Deletes the old one, if needed.
	void set_datasource_factory(net::datasource_factory *df) { delete m_datasource_factory; m_datasource_factory = df; }
	/// Override the playable factory.
	void set_parser_factory(lib::global_parser_factory *pf) { m_parser_factory = pf; }
	/// Override the playable factory.
	void set_node_factory(lib::node_factory *nf) { m_node_factory = nf; }

protected:
	global_playable_factory *m_playable_factory;
	window_factory *m_window_factory;
	net::datasource_factory *m_datasource_factory;
	lib::global_parser_factory *m_parser_factory;
	lib::node_factory *m_node_factory;
};

} // end namespaces
} // end namespaces
#endif /* _FACTORY_H */
