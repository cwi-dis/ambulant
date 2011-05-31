/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __QT_MAINLOOP_H__
#define __QT_MAINLOOP_H__

// Environment for testing design classes

#include <iostream>
#include <string>
#include <map>
#include <stack>


#include "ambulant/version.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/common/embedder.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/common/player.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/gui/qt/qt_factory.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/smil2/smil_player.h"
#include "ambulant/net/url.h"
#include "qt_gui.h"

using namespace ambulant;
using namespace common;
using namespace lib;
using namespace gui;
using namespace qt;

void open_web_browser(const std::string &href);

class qt_gui;

class qt_mainloop :
	public common::gui_player,
	public ambulant::common::embedder
//	public ambulant::lib::ref_counted
{
  //  static bool m_done;
  public:
	qt_mainloop(qt_gui* parent, int mbheight);
	~qt_mainloop();

	void init_playable_factory();
	void init_datasource_factory();
	void init_parser_factory();

	bool is_open() const { return m_player != NULL; }

	void show_file(const ambulant::net::url&);
	void close(common::player *p);
	void done(common::player *p);
	void open(net::url newdoc, bool start, common::player *old=NULL);
	bool player_done();
	void player_start(QString document_name, bool start, bool old);

  private:
#if 0
	// from dx_player
	// The frames stack
	struct frame {
		qt_gui* windows;
		ambulant::common::player* player;
	};
	std::stack<frame*> m_frames;
#endif
	ambulant::common::player* create_player(const char* filename);
	// sorted alphabetically on member name
	qt_gui*					m_gui;
	lib::logger*				m_logger;
};
#endif/*__QT_MAINLOOP_H__*/
