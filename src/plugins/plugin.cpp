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
#include "ambulant/version.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

class basic_plugin_factory : public common::playable_factory {
  public:

	basic_plugin_factory(common::factories* factory)
	:   m_factory(factory) {}
	~basic_plugin_factory() {};
		
	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);
  private:
	common::factories *m_factory;
	
};

class basic_plugin : public common::playable_imp 
{
  public:
  basic_plugin(
    common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	common::factories *factory);

  	~basic_plugin() {};
	
    void data_avail();
	void start(double where);
	void seek(double where) {};
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
    AM_DBG lib::logger::get_logger()->debug("sdl_renderer_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "plugindatatype") /*or any other tag ofcourse */ {
		rv = new basic_plugin(context, cookie, node, evp, m_factory);
		//rv = NULL;
		AM_DBG lib::logger::get_logger()->debug("basic_plugin_factory: node 0x%x: returning basic_plugin 0x%x", (void *)node, (void *)rv);
	} else {
		AM_DBG lib::logger::get_logger()->debug("basic_plugin_factory : plugin does not support \"%s\"", tag.c_str());
        return NULL;
	}
	return rv;
}

basic_plugin::basic_plugin(
	common::playable_notification *context,
    common::playable_notification::cookie_type cookie,
    const lib::node *node,
    lib::event_processor *evp,
	common::factories* factory) 
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


static ambulant::common::factories * 
bug_workaround(ambulant::common::factories* factory)
{
	return factory;
}

extern "C" void initialize(ambulant::common::factories* factory)
{
    if ( !ambulant::check_version() )
        lib::logger::get_logger()->warn("basic_plugin: built for different Ambulant version (%s)", AMBULANT_VERSION);
	factory = bug_workaround(factory);
    AM_DBG lib::logger::get_logger()->debug("basic_plugin: loaded.");
    if (factory->rf) {
    	basic_plugin_factory *bpf = new basic_plugin_factory(factory);
		factory->rf->add_factory(bpf);
    	AM_DBG lib::logger::get_logger()->trace("basic_plugin: registered");
    }
}
