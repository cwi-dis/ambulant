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

#include "ambulant/common/factory.h"
#include "ambulant/common/renderer.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/unix/unix_mtsync.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/gui/arts/arts_audio.h"

#include <artsc.h>


//#define AM_DBG
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
	AM_DBG lib::logger::get_logger()->debug("arts_plugin::initialize registering factory function");
	if (factory->rf) {
		factory->rf->add_factory(new arts_plugin_factory(factory));
		lib::logger::get_logger()->trace("arts_plugin: registered");
	}
}
