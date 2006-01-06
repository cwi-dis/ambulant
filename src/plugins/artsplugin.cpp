// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

#include "ambulant/common/factory.h"
#include "ambulant/common/renderer.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/unix/unix_mtsync.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/gui/arts/arts_audio.h"
#include "ambulant/version.h"

#include <artsc.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;




class arts_plugin_factory : public common::playable_factory {
  public:

	arts_plugin_factory(common::factories* factory)
	:   m_factory(factory) {}
	~arts_plugin_factory() {};
		
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);
  private:
	common::factories* m_factory;
	
};

typedef lib::no_arg_callback<ambulant::gui::arts::arts_active_audio_renderer> readdone_callback;
net::audio_format ambulant::gui::arts::arts_active_audio_renderer::m_ambulant_format = net::audio_format(44100, 2, 16);

bool ambulant::gui::arts::arts_active_audio_renderer::m_arts_init = false;


common::playable* 
arts_plugin_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
{
	common::playable *rv;
	
	lib::xml_string tag = node->get_qname().second;
    AM_DBG lib::logger::get_logger()->debug("arts_plugin_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "audio") /*or any other tag ofcourse */ {
		rv = new ambulant::gui::arts::arts_active_audio_renderer(context, cookie, node, evp, m_factory);
		//rv = NULL;
		AM_DBG lib::logger::get_logger()->debug("arts_plugin_factory: node 0x%x: returning basic_plugin 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->debug("arts_plugin_factory: plugin does not support \"%s\"", tag.c_str());
        return NULL;
	}
	return rv;
}





extern "C" void initialize(ambulant::common::factories* factory)
{	
    if ( !ambulant::check_version() )
        lib::logger::get_logger()->warn("arts_plugin: built for different Ambulant version (%s)", AMBULANT_VERSION);
	AM_DBG lib::logger::get_logger()->debug("arts_plugin::initialize registering factory function");
	if (factory->rf) {
		factory->rf->add_factory(new arts_plugin_factory(factory));
		AM_DBG lib::logger::get_logger()->trace("arts_plugin: registered");
	}
}
