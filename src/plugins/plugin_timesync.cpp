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
#include "ambulant/lib/timer_sync.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

#ifdef WITH_REMOTE_SYNC
class timesync_plugin_factory : public lib::timer_sync_factory {
  public:

	timesync_plugin_factory(common::factories* factory, common::playable_factory_machdep *mdp)
	:	m_factory(factory),
		m_mdp(mdp)
	{}
	timesync_plugin_factory() {};

    lib::timer_sync *new_timer_sync(lib::document *doc);
    
  private:
	common::factories *m_factory;
	common::playable_factory_machdep *m_mdp;
};

using namespace ambulant;


class timer_sync_impl : public lib::timer_sync {
  public:
	timer_sync_impl()
	:	m_timer(NULL)
	{
	}
	
	virtual ~timer_sync_impl() {
        AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): deleted", (void*)this);
		if (m_timer) {
			m_timer->set_observer(NULL);
		}
	}
	
  
	void initialize(lib::timer_control *timer) {
		m_timer = timer;
		AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): initialize(0x%x)", (void*)this, (void*)m_timer);
        m_timer->set_observer(this);
	}
	
	void started() {
		AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): timer 0x%x: started", (void*)this, (void*)m_timer);
	}
	
	void stopped() {
		AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): timer 0x%x: stopped", (void*)this, (void*)m_timer);
	}
	
	void paused() {
		AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): timer 0x%x: paused", (void*)this, (void*)m_timer);
	}
	
	void resumed() {
		AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): timer 0x%x: resumed", (void*)this, (void*)m_timer);
	}
	
	void clicked(const lib::node *n, lib::timer::time_type t) {
		AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): timer 0x%x: clicked(%s, %ld)", (void*)this, (void*)m_timer, n->get_sig().c_str(), (long)t);
	}
	
  private:
	lib::timer_control* m_timer;
};

lib::timer_sync *
timesync_plugin_factory::new_timer_sync(lib::document *doc)
{
	lib::timer_sync *rv = new timer_sync_impl();
	AM_DBG lib::logger::get_logger()->debug("timer_sync_factory(0x%x) -> 0x%x", doc, rv);
	return rv;
}

static ambulant::common::factories *
bug_workaround(ambulant::common::factories* factory)
{
	return factory;
}

#endif // WITH_REMOTE_SYNC

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
#ifdef WITH_REMOTE_SYNC
	factory = bug_workaround(factory);
	lib::logger::get_logger()->debug("timesync_plugin: loaded.");
    if (factory->get_timer_sync_factory()) {
        lib::logger::get_logger()->trace("warning: overriding earlier timer_sync_factory");
    }
    factory->set_timer_sync_factory(new timesync_plugin_factory());
	lib::logger::get_logger()->trace("timesync_plugin_factory: registered");
#else
    lib::logger::get_logger()->debug("timesync_plugin: ambulant player without timer_sync support");
#endif // WITH_REMOTE_SYNC
}
