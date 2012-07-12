// Derived from: http://stackoverflow.com/questions/8746719/gstreamer-appsrc-test-application, 1st reply
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/video/video.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

GST_DEBUG_CATEGORY (appsrc_pipeline_debug);
#define GST_CAT_DEFAULT appsrc_pipeline_debug

typedef struct _App App;

struct _App
{
  GstElement *pipeline;
  GstElement *appsrc;

  GMainLoop *loop;
  guint sourceid;

  GTimer *timer;

  unsigned int W, H;
  unsigned int datasize;
  unsigned long int timestamp;
  void* databuffer;
};

App s_app;

void read_header(App* app)
{
  if (app != NULL) {
    fscanf(stdin, "Time: %8lu\nSize: %8u\nW: %5u\nH: %5u\n", &app->timestamp, &app->datasize, &app->W, &app->H);
  }
}

void read_buffer(App* app)
{
  if (app != NULL && app->datasize != 0) {
    if (app->databuffer == NULL) {
      app->databuffer = malloc(app->datasize);
    } else {
      app->databuffer = realloc(app->databuffer, app->datasize);
    }
    fread (app->databuffer,1,app->datasize,stdin);
  }
}

static gboolean
read_data (App * app)
{
    guint len;
    GstFlowReturn ret;
    gdouble ms;

    ms = g_timer_elapsed(app->timer, NULL);
    if (ms > 1.0/20.0) {
        static unsigned int color = 0xffffffff;
        GstBuffer *buffer;
        gboolean ok = TRUE;

        buffer = gst_buffer_new();
        GST_BUFFER_DATA (buffer) = app->databuffer;
        GST_BUFFER_SIZE (buffer) = app->datasize;
        GST_DEBUG ("feed buffer");
        g_signal_emit_by_name (app->appsrc, "push-buffer", buffer, &ret);
        gst_buffer_unref (buffer);

        if (ret != GST_FLOW_OK) {
            /* some error, stop sending data */
            GST_DEBUG ("some error");
            ok = FALSE;
        }
	read_header(app);
	read_buffer(app);
        g_timer_start(app->timer);

        return ok;
    }

    // g_signal_emit_by_name (app->appsrc, "end-of-stream", &ret);
    return TRUE;
}

/* This signal callback is called when appsrc needs data, we add an idle handler
* to the mainloop to start pushing data into the appsrc */
static void
start_feed (GstElement * pipeline, guint size, App * app)
{
  if (app->sourceid == 0) {
    GST_DEBUG ("start feeding");
    app->sourceid = g_idle_add ((GSourceFunc) read_data, app);
  }
}

/* This callback is called when appsrc has enough data and we can stop sending.
* We remove the idle handler from the mainloop */
static void
stop_feed (GstElement * pipeline, App * app)
{
  if (app->sourceid != 0) {
    GST_DEBUG ("stop feeding");
    g_source_remove (app->sourceid);
    app->sourceid = 0;
  }
}

static gboolean
bus_message (GstBus * bus, GstMessage * message, App * app)
{
  GST_DEBUG ("got message %s",
      gst_message_type_get_name (GST_MESSAGE_TYPE (message)));

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR: {
        GError *err = NULL;
        gchar *dbg_info = NULL;

        gst_message_parse_error (message, &err, &dbg_info);
        g_printerr ("ERROR from element %s: %s\n",
            GST_OBJECT_NAME (message->src), err->message);
        g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
        g_error_free (err);
        g_free (dbg_info);
        g_main_loop_quit (app->loop);
        break;
    }
    case GST_MESSAGE_EOS:
      g_main_loop_quit (app->loop);
      break;
    default:
      break;
  }
  return TRUE;
}

int
main (int argc, char *argv[])
{
  App *app = &s_app;
  GError *error = NULL;
  GstBus *bus;
  GstCaps *caps;

  gst_init (&argc, &argv);

  read_header(app);
  read_buffer(app);

  GST_DEBUG_CATEGORY_INIT (appsrc_pipeline_debug, "appsrc-pipeline", 0,
      "appsrc pipeline example");

  /* create a mainloop to get messages and to handle the idle handler that will
* feed data to appsrc. */
  app->loop = g_main_loop_new (NULL, TRUE);
  app->timer = g_timer_new();

  // Option 1: Display on screen via autovideosink
  char pipeline[256];
  sprintf (pipeline, "appsrc name=mysource ! video/x-raw-rgb,width=%u,height=%u ! ffmpegcolorspace ! videoscale method=1 ! autovideosink", app->W, app->H);
  fprintf(stderr,"pipeline=%s",pipeline);
  app->pipeline = gst_parse_launch(pipeline, NULL);

  // Option 2: Encode using Theora and stream through UDP
  // NOTE: first launch receiver by executing:
  //       gst-launch udpsrc port=5000 ! theoradec ! ffmpegcolorspace ! autovideosink
  //app->pipeline = gst_parse_launch("appsrc name=mysource ! videorate ! ffmpegcolorspace ! videoscale method=1 ! video/x-raw-yuv,width=640,height=480,framerate=\(fraction\)15/1 ! theoraenc bitrate=700 ! udpsink host=127.0.0.1 port=5000", NULL);

  g_assert (app->pipeline);

  bus = gst_pipeline_get_bus (GST_PIPELINE (app->pipeline));
  g_assert(bus);

  /* add watch for messages */
  gst_bus_add_watch (bus, (GstBusFunc) bus_message, app);

  /* get the appsrc */
    app->appsrc = gst_bin_get_by_name (GST_BIN(app->pipeline), "mysource");
    g_assert(app->appsrc);
    g_assert(GST_IS_APP_SRC(app->appsrc));
    g_signal_connect (app->appsrc, "need-data", G_CALLBACK (start_feed), app);
    g_signal_connect (app->appsrc, "enough-data", G_CALLBACK (stop_feed), app);

  /* set the caps on the source */
  caps = gst_video_format_new_caps(GST_VIDEO_FORMAT_RGB, app->W, app->H, 0, 1, app->W, app->H);
  gst_app_src_set_caps(GST_APP_SRC(app->appsrc), caps);


  /* go to playing and wait in a mainloop. */
  gst_element_set_state (app->pipeline, GST_STATE_PLAYING);

  /* this mainloop is stopped when we receive an error or EOS */
  g_main_loop_run (app->loop);

  GST_DEBUG ("stopping");

  gst_element_set_state (app->pipeline, GST_STATE_NULL);

  gst_object_unref (bus);
  g_main_loop_unref (app->loop);

  if (app->databuffer != NULL) {
    free(app->databuffer);
  }

  return 0;
}
