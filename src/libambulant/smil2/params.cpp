/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#include "ambulant/lib/logger.h"
#include "ambulant/lib/node.h"
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
	bool has_params = n->get_first_child("param");
#ifdef SMIL_21
	const char *indir_params = n->get_attribute("param");
	if (indir_params) abort();
#endif
	if (has_params) return new params(n);
	return NULL;
}

params::params(const lib::node *n)
{
#ifdef SMIL_21
	const char *indir_params = n->get_attribute("param");
	if (indir_params) abort();
#endif
	const lib::node *pnode = n->get_first_child("param");
	while (pnode && pnode->get_local_name() == "param") {
		const char *cname = pnode->get_attribute("name");
		const char *cvalue = pnode->get_attribute("value");
		
		if (cname == NULL) {
			lib::logger::get_logger()->trace("<param> missed \"name\" attribute");
		} else if (cvalue == NULL) {
			lib::logger::get_logger()->trace("<param> missed \"value\" attribute");
		} else {
			std::string name(cname);
			m_params[name] = cvalue;
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
	char *s_color = get_str(paramname);
	if (s_color == NULL) return dft;
	return lib::to_color(s_color);
}


