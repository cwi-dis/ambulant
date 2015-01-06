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
#include "wkdomplugin.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

static void *extra_data;
struct ambulant::common::plugin_extra_data plugin_extra_data = {
	"webkit_extra_data",
	&extra_data
};


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
		lib::logger::get_logger()->warn(gettext("%s: built for plugin-api version %d, current %d. Skipping."),"wkdom_plugin", AMBULANT_PLUGIN_API_VERSION, api_version);
		return;
	}
	if ( !ambulant::check_version() ) {
		lib::logger::get_logger()->warn(gettext("%s: built for different Ambulant version (%s)"),"wkdom_plugin", AMBULANT_VERSION);
	}
	factory = bug_workaround(factory);
	lib::logger::get_logger()->debug("wkdom_plugin: loaded.");
	if (extra_data) {
		lib::node_factory *nf = create_wkdom_node_factory(extra_data);
		if (nf == NULL) {
			lib::logger::get_logger()->trace("wkdom_plugin: cannot create wknode_factory");
		} else {
			factory->set_node_factory(nf);
			lib::logger::get_logger()->trace("wkdom_plugin: registered wkdom node factory");
		}
	} else {
		lib::logger::get_logger()->trace("wkdom_plugin: skipped: WebView not available (webkit_extra_data not set)");
	}
}
