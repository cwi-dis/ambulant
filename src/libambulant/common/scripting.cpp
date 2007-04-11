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

/* 
 * @$Id$ 
 */

#include "ambulant/common/scripting.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;
using namespace lib;

// Implementation of the factory

class global_script_component_factory_impl : public global_script_component_factory {
  public:
	virtual ~global_script_component_factory_impl();
	virtual void add_factory(script_component_factory *sf);
    virtual script_component *new_script_component(const char *uri);
  private:
	std::vector<script_component_factory *> m_factories;
};

global_script_component_factory_impl::~global_script_component_factory_impl()
{
}

void
global_script_component_factory_impl::add_factory(script_component_factory *sf)
{
	m_factories.push_back(sf);
}

script_component *
global_script_component_factory_impl::new_script_component(const char *uri)
{
	std::vector<script_component_factory *>::iterator i;
	for (i=m_factories.begin(); i != m_factories.end(); i++) {
		script_component *rv = (*i)->new_script_component(uri);
		if (rv) return rv;
	}
	return NULL;
}

global_script_component_factory *
common::get_global_script_component_factory()
{
	static global_script_component_factory *gscf;
	
	if (gscf == NULL) gscf = new global_script_component_factory_impl();
	return gscf;
}
