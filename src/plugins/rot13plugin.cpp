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

#include "ambulant/version.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/lib/logger.h"
#include "ambulant/net/datasource.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

class rot13_filter_finder : public net::raw_filter_finder {
  public:

	net::datasource* new_raw_filter(const net::url& url, net::datasource *src);
};

class rot13_datasource : public net::filter_datasource_impl
{
  public:
	rot13_datasource(net::datasource *src)
	:	net::filter_datasource_impl(src) {}

	size_t _process(char *data, size_t size);
};

net::datasource *
rot13_filter_finder::new_raw_filter(const net::url& url, net::datasource *src)
{
	std::string path = url.get_path();
	size_t dotpos = path.find_last_of(".");
	if (dotpos <= 0) return src;
	std::string ext = path.substr(dotpos);
	if (ext == ".rot13" && dynamic_cast<rot13_datasource*>(src) == NULL) {
		AM_DBG lib::logger::get_logger()->debug("rot13_filter_finder: success for 0x%x, basetype is %s", src, typeid(src).name());
		return new rot13_datasource(src);
	}
	return src;
}

size_t
rot13_datasource::_process(char *data, size_t size)
{
	char *optr = m_databuf.get_write_ptr(size);
	size_t i;
	for(i=0; i<size; i++) {
		char c = data[i];
		if (c >= 'a' && c <= 'z') {
			c += 13;
			if (c > 'z') c -= 26;
		}
		if (c >= 'A' && c <= 'Z') {
			c += 13;
			if (c > 'Z') c -= 26;
		}
		optr[i] = c;
	}
	m_databuf.pushdata(size);
	return size;
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
		lib::logger::get_logger()->warn(gettext("%s: built for plugin-api version %d, current %d. Skipping."),"rot13_plugin", AMBULANT_PLUGIN_API_VERSION, api_version);
		return;
	}
	if ( !ambulant::check_version() ) {
		lib::logger::get_logger()->warn(gettext("%s: built for different Ambulant version (%s)"),"rot13_plugin", AMBULANT_VERSION);
	}
	factory = bug_workaround(factory);
	AM_DBG lib::logger::get_logger()->debug("rot13_plugin: loaded.");
	net::datasource_factory *df = factory->get_datasource_factory();
	if (df) {
		rot13_filter_finder *ff = new rot13_filter_finder();
		df->add_raw_filter(ff);
		AM_DBG lib::logger::get_logger()->trace("rot13_plugin: registered");
	}
}
