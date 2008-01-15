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

#ifndef AM_DBG
#define AM_DBG if(0)
#endif
 
#include "ambulant/gui/arts/arts.h"
#include "ambulant/gui/arts/arts_audio.h"


using namespace ambulant;
using namespace gui::arts;

common::playable_factory *
ambulant::gui::arts::create_arts_playable_factory(common::factories *f)
{
    return new arts_renderer_factory(f);
}

common::playable *
gui::arts::arts_renderer_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp)
{
	common::playable *rv;
	lib::xml_string tag = node->get_qname().second;
     AM_DBG lib::logger::get_logger()->debug("arts_renderer_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "audio") {
		rv = new gui::arts::arts_active_audio_renderer(context, cookie, node, evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("arts_renderer_factory: node 0x%x: returning arts_active_audio_renderer 0x%x", (void *)node, (void *)rv);
	} else {
	AM_DBG lib::logger::get_logger()->error("arts_renderer_factory: no aRts renderer for tag \"%s\"", tag.c_str());
                return NULL;
	}
	return rv;
}

common::playable *
gui::arts::arts_renderer_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *const evp,
		net::audio_datasource *ds)
{
	common::playable *rv;
	lib::xml_string tag = node->get_qname().second;
     AM_DBG lib::logger::get_logger()->debug("arts_renderer_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	//if ( tag == "audio") {
	assert(ds);
		rv = new gui::arts::arts_active_audio_renderer(context, cookie, node, evp, m_factory, ds);
		AM_DBG lib::logger::get_logger()->debug("arts_renderer_factory: node 0x%x: returning arts_active_audio_renderer 0x%x", (void *)node, (void *)rv);
	//} else {
	//AM_DBG lib::logger::get_logger()->error("arts_renderer_factory: no aRts renderer for tag \"%s\"", tag.c_str());
     //           return NULL;
	//}
	return rv;
}
