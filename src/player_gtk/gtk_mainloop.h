/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
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

#ifndef __GTK_MAINLOOP_H__
#define __GTK_MAINLOOP_H__

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
#include "ambulant/common/factory.h"
#include "ambulant/common/player.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/gui/gtk/gtk_renderer.h"
#include "ambulant/gui/gtk/gtk_factory.h"
#include "ambulant/smil2/smil_player.h"
#include "gtk_gui.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

using namespace ambulant;
using namespace common;
using namespace lib;
using namespace gui;
using namespace gtk;

void open_web_browser(const std::string &href);

class gtk_mainloop_callback_arg {
};
class gtk_gui;

class gtk_mainloop :
	public common::gui_player,
	public ambulant::common::embedder
{
  public:
	gtk_mainloop(gtk_gui* parent);
	~gtk_mainloop();

	void init_window_factory();
	void init_playable_factory();
	void init_datasource_factory();
	void init_parser_factory();

	bool is_open() const { return m_player != NULL; }

	void show_file(const ambulant::net::url&);
	void close(common::player *p);
	void done(common::player *p);
	void open(net::url newdoc, bool start, common::player *old=NULL);
	bool player_done();
	void player_start(gchar* document_name, bool start, bool old);
	ambulant::common::gui_screen *get_gui_screen();
//	char* convert_data_to_image(const guchar* m_data, gsize size);
  private:
#if 0
	// from dx_player
	// The frames stack
	struct frame {
		gtk_gui* windows;
		ambulant::common::player* player;
	};
	std::stack<frame*> m_frames;
#endif

	ambulant::common::player* create_player(const char* filename);

	gtk_gui *m_gui;
	lib::logger *m_logger;
	bool m_running;
	ambulant::gui::gtk::gtk_ambulant_widget *m_gtk_widget;
};
#endif/*__GTK_MAINLOOP_H__*/
