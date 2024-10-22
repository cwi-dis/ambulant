/* GStreamer
 * Copyright (C) 2008 Wim Taymans <wim.taymans at gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gst/gst.h>
#include <string.h>
#include <stdio.h>

#include <gst/rtsp-server/rtsp-server.h>

int
main (int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMediaMapping *mapping;
  GstRTSPMediaFactory *factory;
  char pipeline[512];
  char* webcam = "/dev/video0";
  char* port = "5544"; // /etc/services: RTSP Alternate (see port 554)
  char* url = "/webcam";
  const char* format =  "( v4l2src device=%s queue-size=2 ! video/x-raw-yuv,width=640,height=480,framerate=30/1 ! x264enc tune=zerolatency cabac=false bframes=0 ip-factor=2 ! rtph264pay name=pay0 pt=96 )";
  if (argc > 1) {
	webcam = argv[1];
	if (argc > 2) {
	  port = argv[2];
	}
	if (argc > 3) {
	  url = argv[3];
	}
  }
  strcpy(pipeline, format);
  sprintf(pipeline, format, webcam);

  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* create a server instance */
  server = gst_rtsp_server_new ();
  gst_rtsp_server_set_service (server, port);

  /* get the mapping for this server, every server has a default mapper object
   * that be used to map uri mount points to media factories */
  mapping = gst_rtsp_server_get_media_mapping (server);

  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines. 
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */
  factory = gst_rtsp_media_factory_new ();
  gst_rtsp_media_factory_set_shared(factory,TRUE);
  
  gst_rtsp_media_factory_set_launch (factory, pipeline);

  gst_rtsp_media_factory_set_shared (factory, TRUE);

  /* attach the test factory to the /test url */
  gst_rtsp_media_mapping_add_factory (mapping, url, factory);

  /* don't need the ref to the mapper anymore */
  g_object_unref (mapping);

  /* attach the server to the default maincontext */
  gst_rtsp_server_attach (server, NULL);

  /* start serving */
  g_main_loop_run (loop);

  return 0;
}
