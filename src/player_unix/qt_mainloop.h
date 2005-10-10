/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
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
#include "ambulant/common/factory.h"
#include "ambulant/common/player.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/gui/qt/qt_renderer.h"
#include "ambulant/smil2/smil_player.h"
#include "qt_gui.h"

using namespace ambulant;
using namespace common;
using namespace lib;
using namespace gui;
using namespace qt;

void open_web_browser(const std::string &href);

class qt_mainloop_callback_arg {
};
class qt_gui;

class qt_mainloop : public ambulant::common::embedder, public ambulant::lib::ref_counted {
  //  static bool m_done;
  public:
        qt_mainloop(qt_gui* parent);
	~qt_mainloop();
	
	// The callback member function.
	void player_done_callback() {
		m_running = false;
	}
	
	void play();
	void stop();
	void set_speed(double speed);
	double get_speed() const { return m_speed; }
	bool is_running() const;
	bool is_open() const;
	
	void show_file(const ambulant::net::url&);
	void close(common::player *p);
	void done(common::player *p);
	void open(net::url newdoc, bool start, common::player *old=NULL);
	bool player_done();
	void player_start(QString document_name, bool start, bool old);
	
	long add_ref() {
		return ++m_refcount;
	}
	
	long release() {
		if(--m_refcount == 0) {
			delete this;
			return 0;
		}
		return m_refcount;
	}
	
	long get_ref_count() const {
		return m_refcount;
	}
	int get_cursor() const { 
		return m_player?m_player->get_cursor():0;
	};

	void set_cursor(int cursor) { 
		if (m_player)
		  m_player->set_cursor(cursor); 
	}	
 private: 
	// from dx_player
	// The frames stack
	struct frame {
		qt_gui* windows; 
	  	ambulant::common::player* player;
	};
	std::stack<frame*> m_frames;

	ambulant::lib::document *create_document(const char *filename);
	ambulant::common::player* create_player(const char* filename);
	// sorted alphabetically on member name
	document*				m_doc;
 	common::factories*			m_factory;
	qt_gui*					m_gui;
	lib::logger*				m_logger;
	player*					m_player;
	basic_atomic_count<critical_section>	m_refcount;
	
 	bool					m_running;
	double					m_speed;
	window_factory* 			m_wf;
};
#endif/*__QT_MAINLOOP_H__*/
