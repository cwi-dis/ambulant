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

/* from sanbox/Nokia770/AudioPlayer/mp3player.c */

static gboolean
gstbus_callback (GstBus *bus, GstMessage *msg, gpointer data);

int
gst_mp3_player(const char* uri, GstElement** gst_player_p, gstreamer_player* gstreamer_player, GMainLoop** mainloop_p)
{
  GMainLoop *loop;
  GstElement *source=NULL,*sink=NULL, *pipeline=NULL;
  char **files = NULL;
  const char* id = "gst_mp3_player";
  void gstreamer_audio_renderer_pipeline_store(void* player, GstElement* p);
  loop = g_main_loop_new (NULL, FALSE);
  if (mainloop_p) *mainloop_p = loop;
  AM_DBG g_print ("%s: %s=0x%x, %s=0x%x\n", id, "starting, gst_player_p", gst_player_p,"loop=",loop);
#ifdef  WITH_NOKIA770
  if (pthread_mutex_lock(&s_main_nokia770_mutex) < 0) {
    lib::logger::get_logger()->fatal("gst_mp3_player:: pthread_mutex_lock(s_main_nokia770_mutex) failed: %s", strerror(errno));
    abort();
  }
#endif//WITH_NOKIA770
  /* create elements */
  pipeline = (GstElement*)gst_pipeline_new ("mp3-player");
#ifdef  WITH_NOKIA770
  AM_DBG g_print ("%s: %s\n", id, "gst_element_factory_make()");
  source   = gst_element_factory_make ("gnomevfssrc", "source"); 
  sink     = gst_element_factory_make ("dspmp3sink", "sink"); 
#else //WITH_NOKIA770
  source   = gst_element_factory_make ("playbin", "playbin"); 
#endif//WITH_NOKIA770
  if (gst_player_p) *gst_player_p = pipeline;

  if ( !( pipeline && source
#ifdef  WITH_NOKIA770
	  && sink
#endif//WITH_NOKIA770
	  ) ) {
    g_print ("%s:", "gst_mp3_player");
    if ( ! pipeline) g_print (" %s() failed", "get_pipeline_new");
#ifdef  WITH_NOKIA770
    if ( ! source)   g_print (" %s=%s(%s) failed", "source", 
			      "gst_element_factory_make", "gnomevfssrc");
    if ( ! sink)   g_print (" %s=%s(%s) failed", "sink", 
			      "gst_element_factory_make", "dspmp3sink");
#else //WITH_NOKIA770
    if ( ! source)   g_print (" %s=%s(%s) failed", "source", 
			      "gst_element_factory_make", "playbin");
#endif//WITH_NOKIA770
    g_print ("\n");
    return -1;
  }
  AM_DBG g_print ("%s: %s\n", id, "set the source audio file");
  /* set the source audio file */
#ifdef  WITH_NOKIA770
  g_object_set (G_OBJECT(source), "location", uri, NULL);
#else //WITH_NOKIA770
  g_object_set (G_OBJECT(source), "uri", uri, NULL);
#endif//WITH_NOKIA770

  /* put all elements  to the main pipeline */
  gst_bin_add_many (GST_BIN(pipeline), source,
#ifdef  WITH_NOKIA770
		    sink,
#endif//WITH_NOKIA770
		    NULL);

#ifdef  WITH_NOKIA770
  /* link the elements */
  gst_element_link (source, sink);
#endif//WITH_NOKIA770


  /* wait for start */
  gst_element_set_state (pipeline, GST_STATE_PAUSED);
//gst_element_set_state (GST_ELEMENT(pipeline), GST_STATE_PAUSED);

  AM_DBG g_print ("%s: %s\n", id, "add call-back message handler for eos");
  /* add call-back message handler to check for eos and errors */
  gst_bus_add_watch (gst_pipeline_get_bus (GST_PIPELINE (pipeline)),
		     gstbus_callback, loop);

  gstreamer_audio_renderer_pipeline_store(gstreamer_player, pipeline);

 /* start playback */
  AM_DBG g_print ("%s: %s\n", id, "start play");
  gst_element_set_state (GST_ELEMENT(pipeline), GST_STATE_PLAYING);

  AM_DBG g_print ("%s: %s\n", id, "iterate");
   /* iterate */
  AM_DBG g_print("Now playing %s ...", uri);
  g_main_loop_run (loop);
//KB if (mainloop_p) *mainloop_p = NULL;
//KB if (gst_player_p) *gst_player_p = NULL;
  AM_DBG g_print ("done !\n");

  /* stop the pipeline */
  gst_element_set_state (GST_ELEMENT(pipeline), GST_STATE_NULL);

  /* cleanup */
  gst_object_unref (GST_OBJECT(pipeline));

#ifdef  WITH_NOKIA770
  if (pthread_mutex_unlock(&s_main_nokia770_mutex) < 0) {
    lib::logger::get_logger()->fatal("gst_mp3_player:: pthread_mutex_unlock(s_main_nokia770_mutex) failed: %s", strerror(errno));
    abort();
  }
#endif//WITH_NOKIA770

  return 0;
}
static gboolean
gstbus_callback (GstBus* bus, GstMessage *msg, gpointer data)
{
  GMainLoop* loop = (GMainLoop*) data;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      AM_DBG g_print ("End-of-stream\n");
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_ERROR: {
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
      break;
  }
  return TRUE;
}
} /* extern "C" */

//************************* gstreamer_player ***************************
void
gstreamer_player_initialize(int* argcp, char*** argvp) {
	gst_init(argcp, argvp);
}

void
gstreamer_player_finalize() {
	gst_deinit(); // not avail in gstreamer 0.8
}

gstreamer_player::gstreamer_player(const char* uri, gstreamer_audio_renderer* rend)
  : m_gst_player(NULL),
    m_gst_mainloop(NULL),
    m_audio_renderer(NULL),
    m_uri(NULL) {
	m_uri = strdup(uri);
	m_audio_renderer = rend;
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player(0x%x) uri=%s", (void*)this, uri);
}

gstreamer_player::~gstreamer_player() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::~gstreamer_player()(0x%x) m_uri=%s", (void*)this, m_uri);
	stop_player();
	if (m_uri) free(m_uri);
	m_uri = NULL;
	// static mutex needs to exists during the lifetime of the program
	//  if (pthread_mutex_destroy(&s_mutex) < 0) {
	//  	    lib::logger::get_logger()->fatal("gstreamer_player: pthread_mutex_destroy failed: %s", strerror(errno));
        // }
}

GstElement*
gstreamer_player::gst_player() {
	return m_gst_player;
}

unsigned long
gstreamer_player::run() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::run(0x%x)m_uri=%s", (void*)this, m_uri);
	gst_mp3_player (m_uri, &m_gst_player, this, &m_gst_mainloop);
	m_gst_player = NULL;
	m_gst_mainloop = NULL;
	// inform the scheduler that the gstreamer player has terminated
	if (m_audio_renderer)
		m_audio_renderer->stop();
}

unsigned long
gstreamer_player::init() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::init(0x%x)m_uri=%s", (void*)this, m_uri);
	start();	 // starts run() in separate thread
	return 0;
}

void
gstreamer_player::stop_player() {
  AM_DBG lib::logger::get_logger()->debug("gstreamer_player::stop_player(0x%x)m_uri=%s, m_gst_mainloop=0x%x", (void*)this, m_uri, m_gst_mainloop);
	if (m_gst_player) {
		gst_element_set_state (m_gst_player, GST_STATE_NULL);
		m_gst_player = NULL;
	}
	if (m_gst_mainloop) {
		g_main_loop_quit (m_gst_mainloop);
//KN		g_main_context_wakeup(g_main_loop_get_context(m_gst_mainloop));
		m_gst_mainloop = NULL;
	}
	if (m_audio_renderer) {
		gstreamer_audio_renderer* audio_renderer = m_audio_renderer;
		m_audio_renderer = NULL;
		audio_renderer->stop();
	}
}

void
gstreamer_player::pause() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::pause(0x%x)m_uri=%s", (void*)this, m_uri);
	if (m_gst_player)
		gst_element_set_state (m_gst_player, GST_STATE_PAUSED);
}

void
gstreamer_player::play() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::play(0x%x)m_uri=%s", (void*)this, m_uri);
	if (m_gst_player)
		gst_element_set_state (m_gst_player, GST_STATE_PLAYING);
}
