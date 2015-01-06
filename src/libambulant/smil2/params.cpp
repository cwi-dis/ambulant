// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#include "ambulant/lib/logger.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include "ambulant/smil2/params.h"
#include <math.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

params *
params::for_node(const lib::node *n)
{
	const lib::node *paramchild = n->get_first_child("param");
	const lib::node *paramcontainerchild = NULL;
	const char *pname = n->get_attribute("paramGroup");
	if (pname) {
		const lib::node_context *ctx = n->get_context();
		const lib::node *pcontainer = ctx->get_node(pname);
		if (pcontainer) {
			paramcontainerchild = pcontainer->down();
		} else {
			lib::logger::get_logger()->trace("param=\"%s\": ID not found", pname);
		}
	}
	if (paramchild || paramcontainerchild) {
		params *rv = new params();
		if (paramcontainerchild) rv->addparamnodes(paramcontainerchild);
		if (paramchild) rv->addparamnodes(paramchild);
		return rv;
	}
	return NULL;
}

void
params::addparamnodes(const lib::node *pnode)
{
	while (pnode) {
		if (pnode->get_local_name() == "param") {
			const char *cname = pnode->get_attribute("name");
			const char *cvalue = pnode->get_attribute("value");

			if (cname == NULL) {
				lib::logger::get_logger()->trace("%s: missing \"name\" attribute", pnode->get_sig().c_str());
			} else if (cvalue == NULL) {
				lib::logger::get_logger()->trace("%s: missing \"value\" attribute", pnode->get_sig().c_str());
			} else {
				std::string name(cname);
				m_params[name] = cvalue;
			}
		}
		pnode = pnode->next();
	}
}

const char *
params::get_str(const char *paramname)
{
	std::string pname(paramname);
	return get_str(pname);
}

const char *
params::get_str(const std::string &paramname)
{
	std::map<std::string, const char *>::const_iterator i = m_params.find(paramname);
	if (i == m_params.end()) return NULL;
	return (*i).second;
}

lib::color_t
params::get_color(const char *paramname, lib::color_t dft)
{
	std::string pname(paramname);
	return get_color(pname, dft);
}

lib::color_t
params::get_color(const std::string &paramname, lib::color_t dft)
{
	const char *s_color = get_str(paramname);
	if (s_color == NULL) return dft;
	return lib::to_color(s_color);
}

float
params::get_float(const char *paramname, float dft)
{
	std::string pname(paramname);
	return get_float(pname, dft);
}

float
params::get_float(const std::string &paramname, float dft)
{
	const char *s_float = get_str(paramname);
	if (s_float == NULL) return dft;
	return (float)atof(s_float);
}

long
params::get_long(const char *paramname, long dft)
{
	std::string pname(paramname);
	return get_long(pname, dft);
}

long
params::get_long(const std::string &paramname, long dft)
{
	const char *s_long = get_str(paramname);
	if (s_long == NULL) return dft;
	return (long)atol(s_long);
}


