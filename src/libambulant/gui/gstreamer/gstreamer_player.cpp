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

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/gstreamer/gstreamer_audio_renderer.h"
#include "ambulant/gui/gstreamer/gstreamer_player.h"
#include "ambulant/net/posix_datasource.h"
#include "ambulant/common/region_info.h"

#include <stdlib.h>

using namespace ambulant;
using namespace gui::gstreamer;

extern "C" {

static gboolean
gst_bus_callback (GstBus* bus, GstMessage *msg, gpointer data)
{
	GMainLoop* main_loop = (GMainLoop*)data;

	switch (GST_MESSAGE_TYPE (msg)) {

	case GST_MESSAGE_EOS:
		AM_DBG g_print ("End-of-stream\n");
		g_main_loop_quit (main_loop);
		break;

	case GST_MESSAGE_ERROR:
	{
		gchar *debug;
		GError *err;

		gst_message_parse_error (msg, &err, &debug);
		g_free (debug);

		g_print ("Error: %s\n", err->message);
		g_error_free (err);

		g_main_loop_quit (main_loop);
		break;
	}
	default:
		AM_DBG g_print("Unhandled Message type=%s received\n",GST_MESSAGE_TYPE_NAME(msg));
		break;
	}
	return TRUE;
}
} /* extern "C" */

//************************* gstreamer_player ***************************

// Global initialize/finilize, to be called from main thread
void
gstreamer_player_initialize(int* argcp, char*** argvp) {
	gst_init(argcp, argvp);
}

void
gstreamer_player_finalize() {
	gst_deinit(); // not avail in gstreamer 0.8
}

gstreamer_player::gstreamer_player(const char* uri, gstreamer_audio_renderer* rend)
:	m_gst_player(NULL),
	m_gst_mainloop(NULL),
	m_audio_renderer(NULL),
	m_uri(NULL)
{
	pthread_mutex_init(&m_gst_player_mutex, NULL);
	// mutex will be unlocked by gst_player after pipeline initialize is complete
	pthread_mutex_lock(&m_gst_player_mutex);
	m_uri = strdup(uri);
	m_audio_renderer = rend;
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player(0x%x) uri=%s", (void*)this, uri);
	start();  //  lib::unix::thread::start() starts run() in separate thread
}

gstreamer_player::~gstreamer_player() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::~gstreamer_player()(0x%x) m_uri=%s", (void*)this, m_uri);
	pthread_mutex_lock(&m_gst_player_mutex);
	m_audio_renderer = NULL;
	if (m_uri) free(m_uri);
	m_uri = NULL;
	pthread_mutex_unlock(&m_gst_player_mutex);
	pthread_mutex_destroy(&m_gst_player_mutex);
}

unsigned long
gstreamer_player::run() {
  GstElement *source=NULL,*sink=NULL;
	GstStateChangeReturn gst_state_changed;
	const char* id = "gsteamer_player::run()";

	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::run(0x%x)m_uri=%s", (void*)this, m_uri);

	m_gst_mainloop = g_main_loop_new (NULL, FALSE);
	AM_DBG g_print ("%s: %s=0x%x, %s=0x%x\n", id, "starting, m_gst_player", (void*) m_gst_player,"m_gst_mainloop=", (void*) m_gst_mainloop);
	m_gst_player = (GstElement*)gst_pipeline_new ("mp3-player");
	/* create elements */
	AM_DBG g_print ("%s: %s\n", id, "gst_element_factory_make()");
	source   = gst_element_factory_make ("playbin", "playbin");
	if ( !( m_gst_player && source)) {
		g_print ("%s:", "gstreamer_player::run()");
		if ( ! m_gst_player) g_print (" %s() failed", "get_pipeline_new");
		if ( ! source) g_print (" %s=%s(%s) failed", "source", "gst_element_factory_make", "playbin");
		g_print ("\n");
		abort();
	}
	AM_DBG g_print ("%s: %s\n", id, "set the source audio file");
	g_object_set (G_OBJECT(source), "uri", m_uri, NULL);
	/* put all elements  to the main pipeline */
	gst_bin_add_many (GST_BIN(m_gst_player), source, sink, NULL);

	/* wait for start */
	gst_state_changed = gst_element_set_state (m_gst_player, GST_STATE_READY);
	if (gst_state_changed != GST_STATE_CHANGE_SUCCESS) {
		//g_print("gst_element_set_state(..%s) returned %d\n", "GST_STATE_READY", gst_state_changed);
	}

	AM_DBG g_print ("%s: %s\n", id, "add call-back message handler for eos");
	/* add call-back message handler to check for eos and errors */
	gst_bus_add_watch (gst_pipeline_get_bus (GST_PIPELINE (m_gst_player)), gst_bus_callback, m_gst_mainloop);

	gst_state_changed = gst_element_set_state (m_gst_player, GST_STATE_PAUSED);
	if (gst_state_changed == GST_STATE_CHANGE_ASYNC) {
		gst_state_changed = gst_element_get_state (m_gst_player, NULL, NULL, GST_CLOCK_TIME_NONE);
	}
	if (m_audio_renderer) m_audio_renderer->m_pipeline = m_gst_player;

	/* allow start playback*/
	pthread_mutex_unlock(&m_gst_player_mutex);
	AM_DBG g_print ("%s: %s\n", id, "iterate");

	/* iterate */
	AM_DBG g_print("Now playing %s ...", m_uri);
	g_main_loop_run (m_gst_mainloop);
	AM_DBG g_print ("done !\n");

	/* lock for cleanup */
	pthread_mutex_lock(&m_gst_player_mutex);
	m_gst_mainloop = NULL;

	/* stop the pipeline */
	if (gst_state_changed != GST_STATE_CHANGE_SUCCESS) {
		if (gst_state_changed == GST_STATE_CHANGE_ASYNC) {
			gst_state_changed = gst_element_get_state (GST_ELEMENT(m_gst_player), NULL, NULL, GST_CLOCK_TIME_NONE);
		}
		// g_print("gst_element_set_state(..%s) returned %d\n", "GST_STATE_PAUSED", gst_state_changed);
	}
	gst_state_changed = gst_element_set_state (m_gst_player, GST_STATE_READY);
	if (gst_state_changed != GST_STATE_CHANGE_SUCCESS) {
		//g_print("gst_element_set_state(..%s) returned %d\n", "GST_STATE_REA", gst_state_changed);
	}
	gst_state_changed = gst_element_set_state (m_gst_player, GST_STATE_NULL);
	if (gst_state_changed != GST_STATE_CHANGE_SUCCESS) {
		//g_print("gst_element_set_state(..%s) returned %d\n", "GST_STATE_NULL", gst_state_changed);
	}
	// inform the scheduler that the gstreamer player has terminated
	if (m_audio_renderer) {
		m_audio_renderer->stopped();
		m_audio_renderer = NULL;
	}
	/* cleanup */
	gst_object_unref (GST_OBJECT(m_gst_player));
	m_gst_player = NULL;
	pthread_mutex_unlock(&m_gst_player_mutex);

	return 0;
}

void
gstreamer_player::stop_player() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::stop_player(0x%x)m_uri=%s, m_gst_mainloop=0x%x", (void*)this, m_uri, m_gst_mainloop);
	pthread_mutex_lock(&m_gst_player_mutex);
	if (m_gst_mainloop) {
		g_main_loop_quit (m_gst_mainloop);
		m_gst_mainloop = NULL;
	}
	m_audio_renderer = NULL;
	pthread_mutex_unlock(&m_gst_player_mutex);
}

void
gstreamer_player::pause() {
	GstStateChangeReturn gst_state_changed;
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::pause(0x%x)m_uri=%s", (void*)this, m_uri);
	pthread_mutex_lock(&m_gst_player_mutex);
	if (m_gst_player) {
		gst_state_changed = gst_element_set_state (m_gst_player, GST_STATE_PAUSED);
		if (gst_state_changed == GST_STATE_CHANGE_ASYNC) {
			gst_state_changed = gst_element_get_state (m_gst_player, NULL, NULL, GST_CLOCK_TIME_NONE);
		}
	}
	pthread_mutex_unlock(&m_gst_player_mutex);
}

void
gstreamer_player::play() {
	GstStateChangeReturn gst_state_changed;
	pthread_mutex_lock(&m_gst_player_mutex);
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::play(0x%x)m_uri=%s", (void*)this, m_uri);
	if (m_gst_player) {
		gst_state_changed = gst_element_set_state (m_gst_player, GST_STATE_PLAYING);
		if (gst_state_changed == GST_STATE_CHANGE_ASYNC) {
			gst_state_changed = gst_element_get_state (m_gst_player, NULL, NULL, GST_CLOCK_TIME_NONE);
		}
	}
	pthread_mutex_unlock(&m_gst_player_mutex);
}

void
gstreamer_player::seek(double where) {
	guint64 where_guint64;
	where_guint64 = llrint(where)* GST_SECOND;
	// gst_element_seek hangs inside gstmp3sink, therefore it is disabled.
	if (1) return;
	pthread_mutex_lock(&m_gst_player_mutex);
	lib::logger::get_logger()->trace("gstreamer_player: seek() where=%f, where_guint64=%lu", where, where_guint64);
	if (m_gst_player) {
		if ( ! gst_element_seek(m_gst_player, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, where_guint64, GST_SEEK_TYPE_NONE, 0)) {
			lib::logger::get_logger()->trace("gstreamer_player: seek() failed.");
		}
	}
	pthread_mutex_unlock(&m_gst_player_mutex);
}

double
gstreamer_player::get_dur() {
	gint64 length = -1;
	GstFormat fmtTime = GST_FORMAT_TIME;
	double dur = 0.0, nanosec = 1e9;

	pthread_mutex_lock(&m_gst_player_mutex);
	if (m_gst_player)
		gst_element_query_duration(m_gst_player, &fmtTime, &length);

	pthread_mutex_unlock(&m_gst_player_mutex);
	if (length != -1)
		dur  = double(length) / nanosec;
	return dur;
}
