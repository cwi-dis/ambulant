// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/config/config.h"
#include "ambulant/common/factory.h"

using namespace ambulant;
using namespace common;

factories::factories()
:	m_playable_factory(NULL),
	m_window_factory(NULL),
	m_datasource_factory(NULL),
	m_parser_factory(NULL),
	m_node_factory(NULL)
#ifdef WITH_SMIL30
	,
	m_script_component_factory(NULL)
#endif // WITH_SMIL30
{
}

factories::~factories()
{
	delete m_playable_factory;
	// delete m_window_factory; Owned by parent
	delete m_datasource_factory;
	// delete m_parser_factory; singleton
	// delete m_node_factory; singleton
	// delete m_script_component_factory; singleton
}

void
factories::init_factories()
{
	init_playable_factory();
	init_window_factory();
	init_datasource_factory();
	init_parser_factory();
	init_node_factory();
#ifdef WITH_SMIL30
	init_script_component_factory();
#endif
}

void
factories::init_playable_factory()
{
}

void
factories::init_window_factory()
{
}

void
factories::init_datasource_factory()
{
}

void
factories::init_parser_factory()
{
}

void
factories::init_node_factory()
{
	m_node_factory = lib::get_builtin_node_factory();
}
#ifdef WITH_SMIL30
void
factories::init_script_component_factory()
{
	m_script_component_factory = get_global_script_component_factory();
}
#endif // WITH_SMIL30

