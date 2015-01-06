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

#ifndef AMBULANT_GUI_GSTREAMER_GSTREAMER_PLAYER_H
#define AMBULANT_GUI_GSTREAMER_GSTREAMER_PLAYER_H

#include <iostream>
#include <gst/gst.h>

#include "ambulant/common/factory.h"
#include "ambulant/lib/mtsync.h"

#include "ambulant/common/factory.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/lib/logger.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/smil2/transition.h"
#include "ambulant/lib/transition_info.h"
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

// gstreamer_player: interface to low-level gstreamer code

class gstreamer_player :  public lib::unix::thread {

  public:
	gstreamer_player(const char* uri,  gstreamer_audio_renderer* rend);
	~gstreamer_player();

	void pause();
	void play();
	void stop_player();
	void seek(double where);
	double get_dur();
	void mutex_acquire(const char* id);
	void mutex_release(const char* id);

  protected:
	unsigned long run();

  private:
	char* m_uri;
	gstreamer_audio_renderer* m_audio_renderer;
	GstElement* m_gst_player;
	GMainLoop* m_gst_mainloop;

	// m_gst_player_mutex will be locked in the constructor and unlocked by the
	// run() function in another thread after pipeline initialization is complete
	// when the pipeline terminates, the run() function locks again for cleanup,
	// and destoying the mutex.
	// all other access to the gstreamer playerd is done using this mutex protection
	pthread_mutex_t m_gst_player_mutex;
};

} // end namespace gstreamer
} // end namespace gui
} // end namespace ambulant

#endif // AMBULANT_GUI_GSTREAMER_GSTREAMER_PLAYER_H
