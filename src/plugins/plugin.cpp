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

#include "ambulant/common/renderer.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/unix/unix_mtsync.h"



#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

class basic_plugin_factory : public common::playable_factory {
  public:

	basic_plugin_factory(net::datasource_factory *df)
	:   m_datasource_factory(df) {}
	~basic_plugin_factory() {};
		
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);
  private:
	net::datasource_factory *m_datasource_factory;
	
};

class basic_plugin : public common::playable_imp 
{
  public:
  basic_plugin(
    common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	net::datasource_factory *df);

  	~basic_plugin() {};
	
    void data_avail();
	void start(double where);
    void stop();
    void pause();
    void resume();
};



common::playable* 
basic_plugin_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
{
	common::playable *rv;
	
	lib::xml_string tag = node->get_qname().second;
    AM_DBG lib::logger::get_logger()->trace("sdl_renderer_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "audio") /*or any other tag ofcourse */ {
		rv = new basic_plugin(context, cookie, node, evp, m_datasource_factory);
		//rv = NULL;
		AM_DBG lib::logger::get_logger()->trace("basic_plugin_factory: node 0x%x: returning basic_plugin 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->trace("basic_plugin_factory : plugin does not support \"%s\"", tag.c_str());
        return NULL;
	}
	return rv;
}

basic_plugin::basic_plugin(
	common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	net::datasource_factory *df) 
:	common::playable_imp(context, cookie, node, evp)
{

}

void
basic_plugin::data_avail()
{

}

void
basic_plugin::start(double t)
{
}


void 
basic_plugin::stop()
{
}

void 
basic_plugin::pause()
{
}

void 
basic_plugin::resume()
{
}


extern "C" void initialize(ambulant::common::global_playable_factory* rf, ambulant::net::datasource_factory* df)
{	
	rf->add_factory(new basic_plugin_factory(df));
}
