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

 #ifndef __ARTS_AUDIO__
#define __ARTS_AUDIO__


#include <artsc.h>
#include <iostream>


#include "ambulant/common/factory.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/common/region.h"
#include "ambulant/common/renderer.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/lib/mtsync.h"



namespace ambulant {
namespace gui {
namespace arts {

//bool m_arts_init = false;

class arts_active_audio_renderer : public common::playable_imp {
  public:
	arts_active_audio_renderer(
      common::playable_notification *context,
      common::playable_notification::cookie_type cookie,
      const lib::node *node,
      lib::event_processor *const evp,
      common::factories *factory);

  	arts_active_audio_renderer(
      common::playable_notification *context,
      common::playable_notification::cookie_type cookie,
      const lib::node *node,
      lib::event_processor *const evp,
  	  common::factories *factory,
      net::audio_datasource *ds);
    ~arts_active_audio_renderer();

    int init();
    void start(double where);
    void stop();
    void pause() ;
    void resume() ;
	void seek(double t) {};
    void speed_changed() {};
    void data_avail();
    void redraw(const lib::screen_rect<int> &dirty, common::gui_window *window) {};
	std::pair<bool, double> get_dur();	
		
  private:
    int arts_setup(int rate, int bits, int channels, char *name);
    int arts_play(char *data, int size);
    bool restart_audio_input();
    int m_rate;
    int m_channels;
    int m_bits;
  	arts_stream_t m_stream;
  	net::audio_datasource *m_audio_src;
  	bool m_is_paused;
  	bool m_is_playing;
    char *m_name;
	static bool m_arts_init;
    lib::event *m_playdone;
  	lib::critical_section m_lock;
  	static net::audio_format m_ambulant_format;

};

}
}
}
#endif
