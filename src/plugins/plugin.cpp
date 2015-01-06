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
#include "ambulant/version.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

class basic_plugin_factory : public common::playable_factory {
  public:

	basic_plugin_factory(common::factories* factory, common::playable_factory_machdep *mdp)
	:	m_factory(factory),
		m_mdp(mdp)
	{}
	~basic_plugin_factory() {};

	bool supports(common::renderer_select *rs);

	common::playable *new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp);

	common::playable *new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src) { return NULL; }
  private:
	common::factories *m_factory;
	common::playable_factory_machdep *m_mdp;
};

class basic_plugin : public common::playable_imp
{
  public:
  basic_plugin(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp);

	~basic_plugin() {};

	void data_avail();
	void start(double where);
	void seek(double where) {};
	//void stop();
	bool stop();
	void pause(common::pause_display d=common::display_show);
	void resume();
};


bool
basic_plugin_factory::supports(common::renderer_select *rs)
{
	const char *renderer_uri = rs->get_renderer_uri();
	// Only use this plugin if explicitly requested. The tag does not matter.
	if (renderer_uri != NULL && strcmp(renderer_uri, AM_SYSTEM_COMPONENT("RendererDummyPlugin")) == 0)
		return true;
	return false;
}

common::playable*
basic_plugin_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
{
	common::playable *rv;

	lib::xml_string tag = node->get_qname().second;
	AM_DBG lib::logger::get_logger()->debug("basic_plugin_factory: node 0x%x:	inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "plugindatatype") /*or any other tag ofcourse */ {
		rv = new basic_plugin(context, cookie, node, evp, m_factory, m_mdp);
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
	common::factories* factory,
	common::playable_factory_machdep *mdp)
:	common::playable_imp(context, cookie, node, evp, factory, mdp)
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


bool
basic_plugin::stop()
{
	return true;
}

void
basic_plugin::pause(common::pause_display)
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

extern "C"
#ifdef AMBULANT_PLATFORM_WIN32
__declspec(dllexport)
#endif
void initialize(
	int api_version,
	ambulant::common::factories* factory,
	ambulant::common::gui_player *player)
{
	if ( api_version != AMBULANT_PLUGIN_API_VERSION ) {
		lib::logger::get_logger()->warn(gettext("%s: built for plugin-api version %d, current %d. Skipping."),"basic_plugin", AMBULANT_PLUGIN_API_VERSION, api_version);
		return;
	}
	if ( !ambulant::check_version() ) {
		lib::logger::get_logger()->warn(gettext("%s: built for different Ambulant version (%s)"),"basic_plugin", AMBULANT_VERSION);
	}
	factory = bug_workaround(factory);
	lib::logger::get_logger()->debug("basic_plugin: loaded.");
	common::global_playable_factory *pf = factory->get_playable_factory();
	if (pf) {
		basic_plugin_factory *bpf = new basic_plugin_factory(factory, NULL);
	pf->add_factory(bpf);
		lib::logger::get_logger()->trace("basic_plugin: registered");
	}
}
