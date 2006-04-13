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

// forward decl. of some internal functions
static void
mutex_initialize(void);
static void
mutex_acquire(void* player);
static void
mutex_release(void* player);

extern "C" {

/* from sanbox/Nokia770/AudioPlayer/mp3player.c */

static void
mp3player_eos_cb (GstElement * bin, gpointer user_data);
static void
mp3player_error_cb (GstElement * bin, GstElement * error_element,
		    GError * error,  const gchar * debug_msg,
		    gpointer user_data);

int
gst_mp3_player(const char* uri, GstElement** gst_player_p, gstreamer_player* gstreamer_player, gboolean* player_done_p)
{
  GMainLoop *loop;
  GstElement *source=NULL,*sink=NULL, *pipeline=NULL;
  char **files = NULL;
  const char* id = "gst_mp3_player";
  void gstreamer_audio_renderer_pipeline_store(void* player, GstElement* p);
  AM_DBG g_print ("%s: %s\n", id, "starting");

  /* create elements */
  pipeline = (GstElement*)gst_pipeline_new ("mp3-player");
#ifdef  WITH_NOKIA770
  source   = gst_element_factory_make ("gnomevfssrc", "source"); 
  sink     = gst_element_factory_make ("dspmp3sink", "sink"); 
#else //WITH_NOKIA770
  source   = gst_element_factory_make ("playbin", "playbin"); 
  if (gst_player_p) *gst_player_p = pipeline;
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
  /* add call-back message handlers to check for eos and errors */
  g_signal_connect (GST_BIN(pipeline), "eos",
		    G_CALLBACK (mp3player_eos_cb), player_done_p);
  AM_DBG g_print ("%s: %s\n", id, "add call-back message handler for error");
  g_signal_connect (GST_BIN(pipeline), "error",
		    G_CALLBACK (mp3player_error_cb), player_done_p);

  AM_DBG g_print ("%s: %s\n", id, "wait for start");

  gstreamer_audio_renderer_pipeline_store(gstreamer_player, pipeline);
  mutex_release(gstreamer_player);

  AM_DBG g_print ("%s: %s\n", id, "iterate");
   /* iterate */
  AM_DBG if ( ! *player_done_p) g_print ("Now playing %s ...", uri);
  while ( ! *player_done_p) {
    mutex_acquire(gstreamer_player); 
    gst_bin_iterate (GST_BIN(pipeline));
    mutex_release(gstreamer_player);
  }
  AM_DBG if (player_done_p) g_print (" done !\n"); 

  mutex_acquire(gstreamer_player); // to be released by the caller

  /* stop the pipeline */
  gst_element_set_state (GST_ELEMENT(pipeline), GST_STATE_NULL);

  /* cleanup */
  gst_object_unref (GST_OBJECT(pipeline));

  return 0;
}

static void
mp3player_eos_cb (GstElement *pipeline,gpointer user_data)
{
  const char* id = "mp3player_eos_cb";
  gboolean *p_player_done = (gboolean *) user_data;

  AM_DBG g_print ("%s: %s\n", id, "called");
  *p_player_done = TRUE;
}

static void
mp3player_error_cb (GstElement *pipeline, GstElement *error_element,
		    GError *error, const gchar *debug_msg,
gpointer user_data)
{
  const char* id = "mp3player_error_cb";
  gboolean *p_player_done = (gboolean *) user_data;

  AM_DBG g_print ("%s: %s\n", id, "called");
  if (error)
    g_printerr ("Error: %s\n", error->message);

  *p_player_done = TRUE;
}
}

//************************* gstreamer_player ***************************
void gstreamer_player_initialize(int* argcp, char*** argvp) {
  	mutex_initialize(); // the sooner the better
	gst_init(argcp, argvp);
}

gstreamer_player::gstreamer_player(const char* uri, gstreamer_audio_renderer* rend)
  : m_gst_player(NULL),
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
	gst_mp3_player (m_uri, &m_gst_player, this, &m_player_done);
	m_gst_player = NULL;
	mutex_release(); // lock was acquired by gst_mp3_player()
	// inform the scheduler that the gstreamer player has terminated
	m_audio_renderer->stop();
}

unsigned long
gstreamer_player::init() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::init(0x%x)m_uri=%s", (void*)this, m_uri);
	mutex_acquire(); // lock will be released by gst_mp3_player()
	m_player_done = FALSE;
	start();	 // starts run() in separate thread
	mutex_acquire(); // wait until gst_mp3_player() has initialized
	mutex_release();
	return 0;
}

void
gstreamer_player::stop_player() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::stop_player(0x%x)m_uri=%s", (void*)this, m_uri);
	mutex_acquire(); 
	if (m_gst_player)
		mp3player_eos_cb(m_gst_player, &m_player_done);
	mutex_release(); 
}

void
gstreamer_player::pause() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::pause(0x%x)m_uri=%s", (void*)this, m_uri);
	mutex_acquire(); 
	if (m_gst_player)
		gst_element_set_state (m_gst_player, GST_STATE_PAUSED);
	mutex_release(); 
}

void
gstreamer_player::play() {
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::play(0x%x)m_uri=%s", (void*)this, m_uri);
	mutex_acquire(); 
	if (m_gst_player)
		gst_element_set_state (m_gst_player, GST_STATE_PLAYING);
	mutex_release(); 
}

static void
mutex_initialize(void) {
	// next test is not completely safe unless there are no other competing threads
	if ( ! s_initialized) {
		s_initialized = true;
		if (pthread_mutex_init(&s_mutex, NULL) < 0) {
                	printf("gstreamer_player:::mutex_acquire() pthread_mutex_init failed: %s\n", strerror(errno));
			abort();
		}
	}
}

void
gstreamer_player::mutex_acquire() {
	if ( ! s_initialized)
		mutex_initialize();
	if (pthread_mutex_lock(&s_mutex) < 0) {
                lib::logger::get_logger()->fatal("gstreamer_player::mutex_acquire(): pthread_mutex_lock failed: %s", strerror(errno));
	}
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::mutex_acquire(0x%x): pthread_mutex_lock called.", this);
}

void
gstreamer_player::mutex_release() {
	if ( ! s_initialized) {
                lib::logger::get_logger()->fatal("gstreamer_player::mutex_release() called while not initialized");
	}
	AM_DBG lib::logger::get_logger()->debug("gstreamer_player::mutex_release(0x%x): pthread_mutex_unlock called.", this);
	if (pthread_mutex_unlock(&s_mutex) < 0) {
             	lib::logger::get_logger()->fatal("gstreamer_player::mutex_release() pthread_mutex_unlock failed: %s", strerror(errno));
	}
}


static void
mutex_acquire(void* obj) {
        gstreamer_player* player = (gstreamer_player*) obj;
        if (player) {
	        player->mutex_acquire();
        }
}

static void
mutex_release(void* obj) {
        gstreamer_player* player = (gstreamer_player*) obj;
        if (player) {
	        player->mutex_release();
        }
}
