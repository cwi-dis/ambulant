// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
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

/* 
 * @$Id$ 
 */

#include "ambulant/config/config.h"
#include "ambulant/common/state.h"
#ifdef WITH_SMIL30
//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;
using namespace lib;

// Implementation of the factory

class global_state_component_factory_impl : public global_state_component_factory {
  public:
	virtual ~global_state_component_factory_impl();
	virtual void add_factory(state_component_factory *sf);
    virtual state_component *new_state_component(const char *uri);
  private:
	std::vector<state_component_factory *> m_factories;
};

static global_state_component_factory *s_gscf;

global_state_component_factory_impl::~global_state_component_factory_impl()
{
	s_gscf = NULL;
}

void
global_state_component_factory_impl::add_factory(state_component_factory *sf)
{
	m_factories.push_back(sf);
}

state_component *
global_state_component_factory_impl::new_state_component(const char *uri)
{
	std::vector<state_component_factory *>::iterator i;
	for (i=m_factories.begin(); i != m_factories.end(); i++) {
		state_component *rv = (*i)->new_state_component(uri);
		if (rv) return rv;
	}
	return NULL;
}

global_state_component_factory *
common::get_global_state_component_factory()
{
	
	if (s_gscf == NULL) s_gscf = new global_state_component_factory_impl();
	return s_gscf;
}
#endif // WITH_SMIL30
