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

#include "ambulant/lib/xerces_parser.h"
#include "ambulant/lib/sax_handler.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/plugin_engine.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/version.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif
using namespace ambulant;

#ifdef WITH_XERCES

class xerces_plugin_factory : public lib::parser_factory {
  public:

	xerces_plugin_factory(common::factories* factory)
	:	m_factory(factory) {}
	~xerces_plugin_factory() {};

	lib::xml_parser* new_parser(
		lib::sax_content_handler* content_handler,
		lib::sax_error_handler* error_handler);

	std::string get_parser_name();

  private:
	common::factories* m_factory;

};


lib::xml_parser*
xerces_plugin_factory::new_parser(
		lib::sax_content_handler* content_handler,
		lib::sax_error_handler* error_handler)
{
	AM_DBG lib::logger::get_logger()->debug("xerces_factory::new_parser(): xerces parser returned");
	//return NULL;
	return new lib::xerces_sax_parser(content_handler, error_handler);
}


std::string
xerces_plugin_factory::get_parser_name()
{
	AM_DBG lib::logger::get_logger()->debug("xerces_factory::get_parser_name(): xerces parser");
	return "xerces";
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
		lib::logger::get_logger()->warn(gettext("%s: built for plugin-api version %d, current %d. Skipping."),"xerces_plugin", AMBULANT_PLUGIN_API_VERSION, api_version);
		return;
	}
	if ( !ambulant::check_version() ) {
		lib::logger::get_logger()->warn(gettext("%s: built for different Ambulant version (%s)"),"xerces_plugin", AMBULANT_VERSION);
	}
	AM_DBG lib::logger::get_logger()->debug("xerces_plugin::initialize registering factory function");
	lib::global_parser_factory *pf = factory->get_parser_factory();
	if (pf) {
		pf->add_factory(new xerces_plugin_factory(factory));
		lib::logger::get_logger()->trace("xerces_plugin: registered");
	}
}

#endif
