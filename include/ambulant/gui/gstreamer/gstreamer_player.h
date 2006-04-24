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

#ifndef __GSTREAMER_PLAYER_h
#define __GSTREAMER_PLAYER_h

#include <gst/gst.h>
#include <iostream>

#include "ambulant/common/factory.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/lib/logger.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/lib/event_processor.h"
#ifdef USE_SMIL21
#include "ambulant/smil2/transition.h"
#include "ambulant/lib/transition_info.h"
#endif
#include "ambulant/lib/asb.h"
#include "ambulant/lib/unix/unix_thread.h"


#define GSTREAMER_BUFFER_MAX_BYTES 819200
#define GSTREAMER_BUFFER_MIN_BYTES 20480

#define AMBULANT_MAX_CHANNELS 2
 
void gstreamer_player_initialize(int* argcp, char*** argvp);
void gstreamer_player_finalize();

namespace ambulant {
namespace gui {
namespace gstreamer {	
  
class gstreamer_audio_renderer;

static pthread_mutex_t s_mutex;
static bool s_initialized = false;

// gstreamer_player: interface to low-level gstreamer code

class gstreamer_player :  public lib::unix::thread {

 public:
	gstreamer_player(const char* uri,  gstreamer_audio_renderer* rend); 
	~gstreamer_player(); 
	GstElement* gst_player();
 	unsigned long init();
	void pause();
	void play();
	void stop_player();
	void mutex_acquire();
	void mutex_release();

  protected:
	unsigned long run();

  private:
	char* m_uri;
	gstreamer_audio_renderer* m_audio_renderer;
	GstElement* m_gst_player;
	gboolean m_player_done;

  };

} // end namespace gstreamer
} // end namespace gui
} // end namespace ambulant

#endif // __GSTREAMER_PLAYER_h
