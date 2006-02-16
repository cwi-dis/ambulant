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
	
class factories {
public:
	factories();
	virtual ~factories();
	void init_factories();
	virtual void init_playable_factory();
	virtual void init_window_factory();
	virtual void init_datasource_factory();
	virtual void init_parser_factory();
	virtual void init_node_factory();
	
	playable_factory *get_playable_factory() const { return m_playable_factory; }
	window_factory *get_window_factory() const { return m_window_factory; }
	net::datasource_factory *get_datasource_factory() const { return m_datasource_factory; }
	lib::global_parser_factory *get_parser_factory() const { return m_parser_factory; }
	lib::node_factory *get_node_factory() const { return m_node_factory; }
	
	void set_playable_factory(global_playable_factory *pf) { delete m_playable_factory; m_playable_factory = pf; }
	void set_window_factory(window_factory *wf) { m_window_factory = wf; }
	void set_datasource_factory(net::datasource_factory *df) { delete m_datasource_factory; m_datasource_factory = df; }
	void set_parser_factory(lib::global_parser_factory *pf) { m_parser_factory = pf; }
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
