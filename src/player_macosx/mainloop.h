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

 

#include "ambulant/common/factory.h"

#include "ambulant/version.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/refcount.h"
//#include "ambulant/lib/event_processor.h"
//#include "ambulant/lib/asb.h"
#include "ambulant/common/player.h"
#include "ambulant/common/playable.h"
//#include "ambulant/common/renderer.h"
#include "ambulant/lib/document.h"
#include "ambulant/common/embedder.h"
#include "ambulant/net/url.h"

class mainloop : public ambulant::lib::ref_counted_obj {
  public:
	mainloop(const char *filename, ambulant::common::window_factory *wf,
		bool use_mms, ambulant::common::embedder *app);
	~mainloop();
	
	// The callback member function.
	void player_done_callback() {
		m_running = false;
	}
	
	void play();
	void stop();
	void set_speed(double speed);
	double get_speed() const { return m_speed; }
	bool is_running() const;
	
	int get_cursor() const {return m_player?m_player->get_cursor():0; };
	void set_cursor(int cursor) { if (m_player) m_player->set_cursor(cursor); }
	
	static void set_preferences(std::string &path);

  private:
	ambulant::lib::document *create_document(ambulant::net::url& url);
  	bool m_running;
	double m_speed;
	ambulant::lib::document *m_doc;
	ambulant::common::player *m_player;
  	ambulant::common::factories *m_factory;
	ambulant::common::embedder *m_embedder;
	const ambulant::lib::node *m_goto_node;	// XXX Quick hack
};
