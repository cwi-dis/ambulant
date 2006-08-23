// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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
	GMainLoop* loop = (GMainLoop*) data;

	switch (GST_MESSAGE_TYPE (msg)) {

	case GST_MESSAGE_EOS:
		AM_DBG g_print ("End-of-stream\n");
		g_main_loop_quit (loop);
		break;

	case GST_MESSAGE_ERROR: 
	{
		gchar *debug;
		GError *err;

		gst_message_parse_error (msg, &err, &debug);
		g_free (debug);

		g_print ("Error: %s\n", err->message);
		g_error_free (err);

		g_main_loop_quit (loop);
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
  : 	m_gst_player(NULL),
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
	stop_player();
	if (m_uri) free(m_uri);
	m_uri = NULL;
}

GstElement*
gstreamer_player::gst_player() {
	return m_gst_player;
}

unsigned long
gstreamer_player::run() {
	GstElement *source=NULL,*sink=NULL, *pipeline=NULL;
	GstStateChangeReturn gst_state_changed;
	char **files = NULL;
	const char* id = "gst_mp3_player";

	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::run(0x%x)m_uri=%s", (void*)this, m_uri);

	m_gst_mainloop = g_main_loop_new (NULL, FALSE);
	AM_DBG g_print ("%s: %s=0x%x, %s=0x%x\n", id, "starting, m_gst_player", m_gst_player,"m_gst_mainloop=",m_gst_mainloop);
#ifdef  WITH_NOKIA770
	/* On Nokia770 we use a dedicated gstreamer module "dspmp3sink" which most
	   efficiently playes mp3 clips using the DSP signal co-processor
	   It only plays one clip at any time.
	*/
	if (pthread_mutex_lock(&s_main_nokia770_mutex) < 0) {
		lib::logger::get_logger()->fatal("gst_mp3_player:: pthread_mutex_lock(s_main_nokia770_mutex) failed: %s", strerror(errno));
		abort();
	}
	m_gst_player = (GstElement*)gst_pipeline_new ("mp3-player");
	/* create elements */
	AM_DBG g_print ("%s: %s\n", id, "gst_element_factory_make()");
	source   = gst_element_factory_make ("gnomevfssrc", "source"); 
	sink     = gst_element_factory_make ("dspmp3sink", "sink"); 
	if ( !( m_gst_player && source && sink)) {
		g_print ("%s:", "gst_mp3_player");
		if ( ! m_gst_player) g_print (" %s() failed", "get_pipeline_new");
		if ( ! source) g_print (" %s=%s(%s) failed", "source", 
					  "gst_element_factory_make", "gnomevfssrc");
		if ( ! sink) g_print (" %s=%s(%s) failed", "sink", "gst_element_factory_make", "dspmp3sink");
		g_print ("\n");
		abort();
	}
 	AM_DBG g_print ("%s: %s\n", id, "set the source audio file");
	g_object_set (G_OBJECT(source), "location", m_uri, NULL);
	/* put all elements  to the main pipeline */
	gst_bin_add_many (GST_BIN(m_gst_player), source, sink, NULL);
	/* link the elements */
	if ( ! gst_element_link (source, sink)) {
		g_print ("gst_element_link (source=%s, sink%s) failed\n", source, sink);
		abort();
	}
#else //WITH_NOKIA770
	m_gst_player = (GstElement*)gst_pipeline_new ("mp3-player");
	/* create elements */
	AM_DBG g_print ("%s: %s\n", id, "gst_element_factory_make()");
	source   = gst_element_factory_make ("playbin", "playbin"); 
	if ( !( m_gst_player && source)) {
		g_print ("%s:", "gst_mp3_player");
		if ( ! m_gst_player) g_print (" %s() failed", "get_pipeline_new");
		if ( ! m_gst_player) g_print (" %s() failed", "get_pipeline_new");
		if ( ! source) g_print (" %s=%s(%s) failed", "source", "gst_element_factory_make", "playbin");
		g_print ("\n");
		abort();
	}
 	AM_DBG g_print ("%s: %s\n", id, "set the source audio file");
	g_object_set (G_OBJECT(source), "uri", m_uri, NULL);
	/* put all elements  to the main pipeline */
	gst_bin_add_many (GST_BIN(m_gst_player), source, sink, NULL);
#endif//WITH_NOKIA770

	/* wait for start */
	gst_state_changed = gst_element_set_state (m_gst_player, GST_STATE_READY);
	if (gst_state_changed != GST_STATE_CHANGE_SUCCESS) {
	  //g_print("gst_element_set_state(..%s) returned %d\n", "GST_STATE_READY", gst_state_changed);
	}

	AM_DBG g_print ("%s: %s\n", id, "add call-back message handler for eos");
	/* add call-back message handler to check for eos and errors */
	gst_bus_add_watch (gst_pipeline_get_bus (GST_PIPELINE (m_gst_player)),
			   gst_bus_callback, m_gst_mainloop);

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
	gst_state_changed = gst_element_set_state (m_gst_player, GST_STATE_PAUSED);
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

	/* cleanup */
	gst_object_unref (GST_OBJECT(m_gst_player));
	m_gst_player = NULL;
	pthread_mutex_unlock(&m_gst_player_mutex);
	pthread_mutex_destroy(&m_gst_player_mutex);
  
#ifdef  WITH_NOKIA770
	//KB experimental 10 sec delay
	//usleep(10000000);
	if (pthread_mutex_unlock(&s_main_nokia770_mutex) < 0) {
		lib::logger::get_logger()->fatal("gst_mp3_player:: pthread_mutex_unlock(s_main_nokia770_mutex) failed: %s", strerror(errno));
		abort();
	}
#endif//WITH_NOKIA770

	// inform the scheduler that the gstreamer player has terminated
	if (m_audio_renderer)
		m_audio_renderer->stop();
}

void
gstreamer_player::stop_player() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::stop_player(0x%x)m_uri=%s, m_gst_mainloop=0x%x", (void*)this, m_uri, m_gst_mainloop);
	pthread_mutex_lock(&m_gst_player_mutex);
	if (m_gst_mainloop) {
		g_main_loop_quit (m_gst_mainloop);
		m_gst_mainloop = NULL;
	}
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
