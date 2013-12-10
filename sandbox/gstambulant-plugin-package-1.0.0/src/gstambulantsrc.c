/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2013 Kees Blom <Kees.Blom@cwi.nl>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION: ambulantsrc
 *
 * ambulantsrc reads data from stdin, expected to be produced by ambulant_recorder_plugin
 * 
 * The data is assembled into GstBuffers on each call to gst_ambulantsrc_create ()
 * which overrides the default implementation of the 'create()' function of the
 * gst_basesrc parent class
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 ambulantsrc ! videoconvert ! videoscale ! ximagesink < tests/input
 * ]|
 * </refsect2>
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "gstambulantsrc.h"

static gboolean tracing = FALSE; //TRUE: turn on to trace function calls without object (e.g. static)
// Turn on gstreamer debugging output e.g.: GST_DEBUG=ambulantsrc:6 gst-launch-1.0
// We us level 5 (during start/stop) and level 6 (during frame processing and details during start/stop)
GST_DEBUG_CATEGORY_STATIC (gst_ambulantsrc_debug);
#define GST_CAT_DEFAULT gst_ambulantsrc_debug

/* Filter signals and args */
enum
{
	/* FILL ME */
	LAST_SIGNAL
};


#define DEFAULT_MIN_LATENCY	((30 * GST_MSECOND) /GST_USECOND)
#define DEFAULT_MAX_LATENCY	((30 * GST_MSECOND) /GST_USECOND)
#define DEFAULT_WIDTH		0 // undefined
#define DEFAULT_HEIGHT		0 // undefined
enum
{
	PROP_0,
	PROP_MIN_LATENCY,
	PROP_MAX_LATENCY,
	PROP_WIDTH,
	PROP_HEIGHT
};

/* the capabilities of the outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
	GST_PAD_SRC,
	GST_PAD_ALWAYS,
#ifndef WITH_AUDIO
// The caps are compatible with the data produced by anbulant_recorder_plugin.
// 'width' and 'height' are taken the header preceding each frame, or should match,
// if they are specified as properties. In the latter case, all data is read asynchronous.
// When (if ever) the window size may change, possibly the plugin need to be adapted to 
// implement caps renogetiation using the new width and height.
	GST_STATIC_CAPS ("video/x-raw,format=BGRA,width=(int) [ 1, 2147483647 ],height=(int) [ 1, 2147483647 ],bpp=32,depth=32,framerate=30/1,endianness=4321,pixel-aspect-ratio=1/1,green_mask=16711680,red_mask=65280;")
#else// WITH_AUDIO
	GST_STATIC_CAPS ("audio/x-raw,format=S16LE,rate=44100,channels=2,layout=interleaved;")
//	GST_STATIC_CAPS_ANY
#endif
	);

#define gst_ambulantsrc_parent_class parent_class
G_DEFINE_TYPE (GstAmbulantSrc, gst_ambulantsrc, GST_TYPE_BASE_SRC);

/* GObject vmethod implementations */
static void gst_ambulantsrc_set_property (GObject * object, guint prop_id,
					  const GValue * value, GParamSpec * pspec);
static void gst_ambulantsrc_get_property (GObject * object, guint prop_id,
					  GValue * value, GParamSpec * pspec);
static gboolean gst_ambulantsrc_start (GstBaseSrc * basesrc);
static gboolean gst_ambulantsrc_stop (GstBaseSrc * basesrc);
static GstCaps* gst_ambulantsrc_get_caps (GstBaseSrc * bsrc, GstCaps* caps);
//static gboolean gst_ambulantsrc_set_caps (GstBaseSrc * bsrc, GstCaps * caps);
static gboolean gst_ambulantsrc_is_seekable (GstBaseSrc * bsrc);
static gboolean gst_ambulantsrc_query (GstBaseSrc * bsrc, GstQuery * query);
/* GstBaseSrc virtual methods we need to override */

/* ask the subclass to create a buffer with offset and size */
static GstFlowReturn gst_ambulantsrc_create (GstBaseSrc * bsrc, guint64 offset, guint length,
					     GstBuffer ** buffer);
/* given a buffer, return start and stop time when it should be pushed
 * out. The base class will sync on the clock using these times. */
static void gst_ambulantsrc_get_times (GstBaseSrc *src, GstBuffer *buffer,
				       GstClockTime *start, GstClockTime *end);
/* get the total size of the resource in bytes */
static gboolean gst_ambulantsrc_get_size (GstBaseSrc *src, guint64 *size);

/* internal functions forward declarations */
static void gst_ambulantsrc_run(GstAmbulantSrc* asrc);
static void gst_ambulantsrc_init_frame(GstAmbulantSrc* asrc);
static GstAmbulantFrame* gst_ambulantsrc_read_frame(GstAmbulantSrc* asrc);
static void gst_ambulantsrc_get_next_frame (GstAmbulantSrc* asrc);

/* initialize the plugin's class */
static void gst_ambulantsrc_class_init (GstAmbulantSrcClass * klass)
{
	if (tracing) fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

	GObjectClass *gobject_class;
	GstBaseSrcClass *gstbasesrc_class;
	
	gobject_class = (GObjectClass *) klass;
	gstbasesrc_class = (GstBaseSrcClass *) klass;
	
	gobject_class->set_property = gst_ambulantsrc_set_property;
	gobject_class->get_property = gst_ambulantsrc_get_property;
	g_object_class_install_property (gobject_class, PROP_WIDTH,
		g_param_spec_uint ("width", "Frame width", "width of initial frame in pixels",
				   0, G_MAXINT32, DEFAULT_WIDTH, G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_HEIGHT,
		g_param_spec_uint ("height", "Frame height","height of initial frame in pixels",
				   0, G_MAXINT32, DEFAULT_HEIGHT, G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_MIN_LATENCY,
		g_param_spec_int64 ("min-latency", "Minimum latency time", "Minimum latency in microseconds",
				    1, G_MAXINT64, DEFAULT_MIN_LATENCY, G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_MAX_LATENCY,
	g_param_spec_int64 ("max-latency", "Maximum latency time", "Maximum latency in microseconds",
			    1, G_MAXINT64, DEFAULT_MAX_LATENCY, G_PARAM_READWRITE));

	/* register functions pointers we want to override */
	gstbasesrc_class->start = GST_DEBUG_FUNCPTR(gst_ambulantsrc_start);
	gstbasesrc_class->stop = GST_DEBUG_FUNCPTR(gst_ambulantsrc_stop);
	gstbasesrc_class->get_caps = GST_DEBUG_FUNCPTR(gst_ambulantsrc_get_caps);
//	gstbasesrc_class->set_caps = GST_DEBUG_FUNCPTR(gst_ambulantsrc_set_caps); // disabled, SEGV
	gstbasesrc_class->is_seekable = GST_DEBUG_FUNCPTR(gst_ambulantsrc_is_seekable);
	gstbasesrc_class->query = GST_DEBUG_FUNCPTR(gst_ambulantsrc_query);
	gstbasesrc_class->get_size = GST_DEBUG_FUNCPTR(gst_ambulantsrc_get_size);
	gstbasesrc_class->get_times = GST_DEBUG_FUNCPTR(gst_ambulantsrc_get_times);
	gstbasesrc_class->create = GST_DEBUG_FUNCPTR(gst_ambulantsrc_create);
	gst_element_class_set_details_simple((GstElementClass*) gstbasesrc_class,
	"Ambulant Source Element",
	"Source",
	"Read frames produced by 'ambulant-recorder-plugin' from 'stdin' "
	"(format: 80 byte ASCII header, followed by variable number of pixels, BGRA 32bpp)"
	" and push these as buffers in a gstreamer pipeline. "
	"When both properties 'width' and 'height' are set greater than 0, "
	"the element pushes a white frame of that size before reading from 'stdin'.",
	"Kees Blom <<Kees.Blom@cwi.nl>>");
	gst_element_class_add_pad_template ((GstElementClass*) gstbasesrc_class,
					    gst_static_pad_template_get (&src_factory));
	/* register internal functions pointers for debugging */
	GST_DEBUG_REGISTER_FUNCPTR (gst_ambulantsrc_read_frame);
	GST_DEBUG_REGISTER_FUNCPTR (gst_ambulantsrc_get_next_frame);
	GST_DEBUG_REGISTER_FUNCPTR (gst_ambulantsrc_init_frame);
	GST_DEBUG_REGISTER_FUNCPTR (gst_ambulantsrc_init);
	GST_DEBUG_REGISTER_FUNCPTR (gst_ambulantsrc_run);
	GST_DEBUG_REGISTER_FUNCPTR (gst_ambulantsrc_set_property);
	GST_DEBUG_REGISTER_FUNCPTR (gst_ambulantsrc_get_property);
}

void gst_ambulantsrc_delete_frame (GstAmbulantFrame* frame) {
	if (frame != NULL) {
		if(tracing)fprintf(stderr,"%s frame=%p databuffer=%p datapointer=%p\n", __PRETTY_FUNCTION__, frame, frame->databuffer, frame->datapointer);
       		if (frame->databuffer != NULL) {
			// frame->datapointer is destroyed as well
			gst_buffer_unref (frame->databuffer);
		}
		g_free (frame);
	}
}

GstAmbulantFrame* gst_ambulantsrc_new_frame (guint W, guint H, gulong datasize, gulong timestamp, gpointer data)
{
//	if (W == 0 || H == 0 || datasize == 0) {
	if (datasize == 0) {
		return NULL;
	}
	GstAmbulantFrame* frame = (GstAmbulantFrame*) g_malloc (sizeof(GstAmbulantFrame));
	if (frame == NULL) {
		return NULL;
	}
	if (data == NULL) {
		frame->datapointer = g_malloc(datasize);
		if (frame->datapointer == NULL) {
			gst_ambulantsrc_delete_frame (frame);
			return NULL;
		}
	} else {
		frame->datapointer = data;
	}
	frame->databuffer = gst_buffer_new_wrapped (frame->datapointer, datasize);
	if (frame->databuffer == NULL) {
		gst_ambulantsrc_delete_frame (frame);
		return NULL;
	}
	frame->W = W;
	frame->H = H;
	frame->datasize = datasize;
	frame->timestamp = timestamp;
	if(tracing)fprintf(stderr,"%s frame=%p databuffer=%p datapointer=%p\n", __PRETTY_FUNCTION__, frame, frame->databuffer, frame->datapointer);
	return frame;
}

// read a frame from stdin: fixed size (80 bytes) header followed by variable size pixel data
GstAmbulantFrame* gst_ambulantsrc_read_frame(GstAmbulantSrc* asrc)
{
	GstAmbulantFrame* frame = NULL;
	gulong timestamp = 0;
	GST_LOG_OBJECT (asrc, "enter");

	if (asrc == NULL) {
		GST_ERROR_OBJECT(NULL, "asrc==NULL, should not happen");
		return NULL;
	}
	// don' block during read
	gboolean was_locked = asrc->locked;
	if (was_locked) {
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
	}
	if(asrc->eos) {
		GST_DEBUG_OBJECT(asrc, "eos already true, bailing out");
		goto done;
	}
	fd_set readfds;
	int nfds = fileno(stdin)+1;
	struct timeval timeout = { 0, 200000 }; // 200 millisec.
	char buf[81];
	buf[80] = 0;
	guint W,H; 
	gulong datasize;

	// First wait for input available
	gboolean input_available = FALSE;
	while ( ! input_available && ! asrc->exit_requested && ! asrc->eos) {
		FD_ZERO(&readfds);
		FD_SET(fileno(stdin), &readfds);
		int rv = select (nfds, &readfds, NULL, NULL, &timeout);
		if (rv < 0) {
			GST_ERROR_OBJECT (asrc, "select failed, errno- %d  while waiting for frames.", errno);
			asrc->eos = TRUE;
			goto done;
		}
		if (rv != 0) {
			input_available = TRUE;
		}
	}
	if ( asrc->exit_requested || asrc->eos) {
		goto done;
	}
	// Then read the 80-byte header.
	if (fread(buf,1,80,stdin) != 80) {
		GST_DEBUG_OBJECT (asrc, "end-of-file reading frame header fd=%d", fileno(stdin));
		asrc->eos = TRUE;
		goto done;
	}
	char type[5];
	int err = sscanf(buf, "Type: %4s\nTime: %12lu\nSize: %9lu\nW: %5u\nH: %5u\n", type, &timestamp, &datasize, &W, &H);
	if (err != 5) { // sscanf failed, garbage read ?
		GST_ERROR_OBJECT (asrc, "sscanf returned %d  while reading frame header\nInput was:%s", err, buf);
		asrc->eos = TRUE;
		goto done;
	}	
	// first frame: remember width, height if not specified as properties
	if (asrc->width == 0 && asrc->height == 0) {
		asrc->width = W;
		asrc->height = H;
		GST_LOG_OBJECT (asrc, "Input framesize (%" G_GUINT32_FORMAT ",%" G_GUINT32_FORMAT ") registered", W, H);
	} else {
		// Check that frame has same dimensions as specified (or as the first frame read)
		if (asrc->width != W || asrc->height != H) {
			GST_ERROR_OBJECT (asrc, "Input frame size (%" G_GUINT32_FORMAT ",%" G_GUINT32_FORMAT ") differs from properties (%" G_GUINT32_FORMAT ",%" G_GUINT32_FORMAT ")", W, H, asrc->width, asrc->height);
			asrc->eos = TRUE;
			goto done;
		}
	}
	// timestamp correction
	if (asrc->basetime == 0) {
	  // first frame
	  asrc->basetime = timestamp;
	} else {
	  timestamp -= asrc->basetime;
	}
	// Now read the frame data
	frame = gst_ambulantsrc_new_frame (W, H, datasize, timestamp, NULL);
	if (frame == NULL) return frame;
	size_t n_bytes = fread (frame->datapointer,1,frame->datasize,stdin);
	if (n_bytes != frame->datasize) {
		GST_DEBUG_OBJECT (asrc, "return: eos detected: fread returns n_bytes=%" G_GUINT64_FORMAT, n_bytes);
		asrc->eos = TRUE;
	}	
done:
	if (was_locked) {
		GST_OBJECT_LOCK (asrc);
		asrc->locked = TRUE;
	}
	GST_LOG_OBJECT (asrc, "return: eos=%" G_GUINT32_FORMAT ", frame=%p timestamp=%" G_GUINT64_FORMAT, asrc->eos, frame, timestamp);
	return frame;
}

void gst_ambulantsrc_get_next_frame (GstAmbulantSrc* asrc)
{
	if (asrc == NULL) {
		return;
	}
	GST_LOG_OBJECT (asrc, "queue.length=%d", asrc->queue ? g_queue_get_length (asrc->queue):-999);

	gboolean was_locked = asrc->locked;
	if ( ! was_locked) {
		asrc->locked = TRUE;
		GST_OBJECT_LOCK (asrc);
	}
	// while nothing has been read from <stdin>, stick to the initial frame
	if (asrc->initial_frame) {
		if (asrc->queue != NULL && g_queue_get_length (asrc->queue) > 0) {
			asrc->initial_frame = FALSE;
		}
	}
	if ( ! asrc->initial_frame) {
		if (asrc->queue != NULL && g_queue_get_length (asrc->queue) > 0) {
			gst_ambulantsrc_delete_frame (asrc->frame);
			asrc->frame = NULL;
			if (asrc->queue != NULL) {
				asrc->frame = g_queue_pop_tail (asrc->queue);
			}
			if (asrc->frame == NULL) {
				asrc->eos = TRUE;
			}
		}
	}
	if ( ! was_locked) {
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
	}
}

void gst_ambulantsrc_init_frame(GstAmbulantSrc* asrc)
{
	GST_DEBUG_OBJECT (asrc, "enter");

	guint W = asrc->width, H = asrc->height;
	glong datasize = W * H * 4;
	unsigned char* data = (unsigned char*) g_malloc (datasize);
	if (data == NULL) {
		asrc->eos = TRUE;
		return;
	}
	int i = 0;
	while (i < datasize) {
		data[i++] = 0x0FF;
	}
	asrc->frame = gst_ambulantsrc_new_frame (W,H,datasize,0,data);
	asrc->initial_frame = TRUE;
} 

/* initialize the new source element
 * instantiate pads and add them to basesrc
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_ambulantsrc_init (GstAmbulantSrc * asrc)
{
	GST_DEBUG_OBJECT (asrc, "enter");

	GstBaseSrc* bsrc = (GstBaseSrc*) asrc;

	if(tracing)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);
	asrc->eos = FALSE;
	asrc->initial_frame = FALSE;
	asrc->width = 0;
	asrc->height = 0;
	asrc->frame = NULL;
	asrc->min_latency = DEFAULT_MIN_LATENCY;
	asrc->max_latency = DEFAULT_MAX_LATENCY;
	asrc->basetime = 0;
	asrc->locked = FALSE;
	gst_base_src_set_async (bsrc, TRUE);
	gst_base_src_set_live (bsrc, TRUE);
	gst_base_src_set_format (bsrc, GST_FORMAT_TIME);
	asrc->caps = gst_static_pad_template_get_caps(&src_factory);
	asrc->thread = NULL;
	asrc->exit_requested = FALSE;
}

static void
gst_ambulantsrc_run (GstAmbulantSrc * asrc)
{
	if (asrc != NULL) {
		GST_DEBUG_OBJECT (asrc, "enter");

		GST_OBJECT_LOCK (asrc);
		asrc->locked = TRUE;
		while ( ! asrc->eos && ! asrc->exit_requested) {
			GstAmbulantFrame* frame = gst_ambulantsrc_read_frame(asrc);
			if (asrc->exit_requested) {
				gst_ambulantsrc_delete_frame (frame);
			} else {
				g_queue_push_head (asrc->queue, frame);
			}
			struct timespec sleeptime = { 0, 10000000 }, slept;
			nanosleep (&sleeptime, &slept);
		}
		if (asrc->exit_requested && asrc->queue != NULL) {
			g_queue_free_full (asrc->queue, (GDestroyNotify) gst_ambulantsrc_delete_frame);
			asrc->queue = NULL;
		}
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
		GST_DEBUG_OBJECT (asrc, "return: eos=%" G_GUINT32_FORMAT " exit_requested=%d", asrc->eos, asrc->exit_requested);
		g_thread_exit((gpointer) NULL);
	}
}

static void gst_ambulantsrc_set_property (GObject * object, guint prop_id,
					  const GValue * value, GParamSpec * pspec)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (object);
	GST_DEBUG_OBJECT (asrc, "prop_id=%" G_GUINT32_FORMAT " value=%" G_GUINT32_FORMAT, prop_id, g_value_get_uint (value));

	switch (prop_id) {
		case PROP_MIN_LATENCY:
			asrc->min_latency = g_value_get_int64 (value);
			break;
		case PROP_MAX_LATENCY:
			asrc->max_latency = g_value_get_int64 (value);
			break;
		case PROP_WIDTH:
			asrc->width = g_value_get_uint (value);
			break;
		case PROP_HEIGHT:
			asrc->height = g_value_get_uint (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gst_ambulantsrc_get_property (GObject * object, guint prop_id,
			      GValue * value, GParamSpec * pspec)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (object);
	GST_DEBUG_OBJECT (asrc, "enter");

	switch (prop_id) {
		case PROP_MIN_LATENCY:
			g_value_set_int64 (value, asrc->min_latency);
			break;
		case PROP_MAX_LATENCY:
			g_value_set_int64 (value, asrc->max_latency);
			break;
		case PROP_WIDTH:
			g_value_set_uint (value, asrc->width);
			break;
		case PROP_HEIGHT:
			g_value_set_uint (value, asrc->height);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}


/* GstBaseSrc vmethod implementations */

static gboolean
gst_ambulantsrc_start (GstBaseSrc * basesrc)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (basesrc);
	GST_DEBUG_OBJECT (asrc, "enter");

	gboolean rv = TRUE;
	GST_OBJECT_LOCK (asrc);
	asrc->locked = TRUE;

	// Create reader queue
	if (asrc->queue == NULL) {
		asrc->queue = g_queue_new();
		if (asrc->queue == NULL) {
			asrc->eos = TRUE;
			rv = FALSE;
		}
	}
	// Create reader thread
	if (rv == TRUE && asrc->thread == NULL) {
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
		GThread* thread = g_thread_new ("reader", (GThreadFunc) &gst_ambulantsrc_run, asrc);
		GST_OBJECT_LOCK (asrc);
		asrc->locked = TRUE;
		if (thread == NULL) {
			asrc->eos = TRUE;
			rv = FALSE;
		}
		asrc->thread = thread;
			// g_thread_new() aborts on failure
			// XXXXX cond wait here until properly initialized
		
	}
	// Create initial frame, if wanted
	if (rv == TRUE && asrc->width != 0 && asrc->height != 0 && asrc->frame == NULL) {
		if (asrc->frame == NULL) {
			gst_ambulantsrc_init_frame (asrc);
		}
	}
	asrc->locked = FALSE;
	GST_OBJECT_UNLOCK (asrc);
	return rv;
}

static gboolean gst_ambulantsrc_stop (GstBaseSrc * basesrc)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (basesrc);
	GST_DEBUG_OBJECT (asrc, "databuffer=0x%p", asrc->frame == NULL ? NULL : asrc->frame->databuffer);
	GST_OBJECT_LOCK (asrc);
	asrc->locked = TRUE;
	asrc->eos = TRUE;
//	if (asrc->caps != NULL) {
//		gst_caps_unref (asrc->caps);
//		asrc->caps = NULL;
//	}
	if (asrc->thread != NULL) {
		asrc->exit_requested = TRUE;
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
		g_thread_join (asrc->thread);
		GST_OBJECT_LOCK (asrc);
		asrc->locked = TRUE;
	}
/* XXXX cleanup in thread 
	if (asrc->queue != NULL) {
		g_queue_free_full (asrc->queue, (GDestroyNotify) gst_ambulantsrc_delete_frame);
		asrc->queue = NULL;
	}
	gst_ambulantsrc_delete_frame (asrc->frame);
*/
	asrc->locked = FALSE;
	GST_OBJECT_UNLOCK (asrc);

	return TRUE;
}

// The caps are fully known, simplified and fixated during the first call of gst_ambulantsrc_query() after
// gst_ambulantsrc_init() was called, since then the actual With,Height of the video data are read.
static GstCaps* gst_ambulantsrc_get_caps (GstBaseSrc * bsrc, GstCaps* filter) {
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
	gchar* s = filter != NULL ? gst_caps_to_string(filter) : NULL;
	GST_DEBUG_OBJECT (asrc, "caps=%p filter=%s", asrc->caps, s);

	if (s != NULL) g_free(s);
	gst_caps_ref (asrc->caps);
	return asrc->caps;
}
/* this function handles the link with other elements 
static gboolean gst_ambulantsrc_set_caps (GstBaseSrc* bsrc, GstCaps * caps)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
	gchar* s = gst_caps_to_string(caps);

	g_free(s);
	return TRUE;
}
*/

static gboolean gst_ambulantsrc_is_seekable (GstBaseSrc * bsrc)
{
	return FALSE;
}

static gboolean gst_ambulantsrc_query (GstBaseSrc * bsrc, GstQuery * query)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
	gboolean res = FALSE;
	GST_DEBUG_OBJECT (asrc, "%s", gst_query_type_get_name(GST_QUERY_TYPE(query)));

	switch (GST_QUERY_TYPE (query)) {
		case GST_QUERY_LATENCY: {
			gst_query_set_latency (query, TRUE, asrc->min_latency, asrc->max_latency);
			GST_LOG_OBJECT(asrc, "min-latency=%" G_GUINT64_FORMAT " max-latency=%" G_GUINT64_FORMAT,asrc->min_latency,asrc->max_latency);
			res = TRUE;
			break;
		}
		case GST_QUERY_CAPS: {
			if (asrc->width != 0 && asrc->height != 0 && asrc->frame == NULL) {
				gst_ambulantsrc_init_frame(asrc);
			} else {
				if (asrc->frame == NULL) { // W,H not yet known
					break;
				}
			}
			GST_LOG_OBJECT(asrc, "caps=%p frame=%p W=%" G_GUINT32_FORMAT " H=%" G_GUINT32_FORMAT, 
				       asrc->caps, asrc->frame, asrc->frame->W, asrc->frame->H);
			// answer to query: template caps + witdh, height from input data header
			GstCaps* caps_org = asrc->caps;
			GstCaps* caps_new = NULL;
			if (caps_org != NULL && ! gst_caps_is_fixed (caps_org)) {
				// convert original caps to string
				gchar* caps_org_str = gst_caps_to_string (caps_org);
				GST_LOG_OBJECT(asrc, "fixating: caps=%p %s", caps_org, caps_org_str);
				// create new caps including actual width, height
				gchar* caps_new_str = g_strdup_printf ("%s, width=(int)%d, height=(int)%d", 
								       caps_org_str, asrc->frame->W, asrc->frame->H);
				caps_new = gst_caps_from_string (caps_new_str);
				g_free (caps_org_str);
				g_free (caps_new_str);
				if (caps_new != NULL) {
					// simplify (remove multiple width,height) and fixate
					GstCaps* caps_simplified = gst_caps_simplify (caps_new);
					gchar* caps_simplified_str = gst_caps_to_string (caps_simplified);
					caps_simplified = gst_caps_fixate (caps_simplified); //XXXX make depend on debug level
					GST_LOG_OBJECT(asrc, "fixated: caps=%p %s", caps_simplified, caps_simplified_str);
					g_free (caps_simplified_str);
					gst_query_set_caps_result (query, caps_simplified);
					gst_caps_unref (asrc->caps);
					asrc->caps = caps_simplified;
					gst_caps_ref (asrc->caps);
					// now we know the size of each frame
					GstBaseSrc* bsrc = (GstBaseSrc*) asrc;
					gst_base_src_set_blocksize (bsrc, asrc->frame->datasize);
					res = TRUE;
				}
			}
		}
		default:
			res = GST_BASE_SRC_CLASS (gst_ambulantsrc_parent_class)->query (bsrc, query);
			break;
	}
	return res;
}

static GstFlowReturn gst_ambulantsrc_create (GstBaseSrc * bsrc, guint64 offset, guint length, GstBuffer ** buffer)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
	GST_LOG_OBJECT (asrc, "enter");
	
	GST_OBJECT_LOCK (asrc);
	asrc->locked = TRUE;
	
	if (buffer == NULL) {
		GST_LOG_OBJECT(asrc, "OK: buffer=%p",buffer);
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
		return GST_FLOW_OK;
	}
	if (asrc->eos && (asrc->queue == NULL || g_queue_get_length(asrc->queue)) == 0) {
		GST_LOG_OBJECT(asrc, "EOS: asrc->queue=%" G_GUINT32_FORMAT, asrc->queue ? asrc->queue->length : 0);
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
		return GST_FLOW_EOS; // end of stream
	}
	GST_LOG_OBJECT(asrc, "Timestamp=%" G_GUINT64_FORMAT " size=%" G_GUINT64_FORMAT " offset=%" G_GUINT64_FORMAT, asrc->frame->timestamp, asrc->frame->datasize, offset);
	GstBuffer* buf = asrc->frame->databuffer;
	GstClockTime dts = GST_BUFFER_DTS (buf) = asrc->frame->timestamp * 1000000; // millis to nanos
	GST_BUFFER_PTS (buf) = dts;
	GST_BUFFER_DURATION (buf) = GST_CLOCK_TIME_NONE;
	GST_BUFFER_OFFSET (buf) = offset;
	gst_buffer_ref(buf);
	*buffer = buf;

	if (asrc->initial_frame) {
		/* In "sniffing mode" when data buffers are produced with video images of
		   specified  'width' and 'height'; then also monotonically increase 'timestamp'
		   with a 'reasonable' value to mimic a real live source
		*/
		asrc->frame->timestamp += 0;
		GST_OBJECT_UNLOCK (asrc);
		/* Get rid of obnoxious error message
		   GStreamer-CRITICAL **: gst_segment_to_running_time: assertion `segment->format == format' failed
		*/
		gst_base_src_new_seamless_segment ((GstBaseSrc*) asrc, dts, GST_CLOCK_TIME_NONE, dts);
		GST_OBJECT_LOCK (asrc);
	}
	gst_ambulantsrc_get_next_frame(asrc);
	if (asrc->frame == NULL) {
		GST_LOG_OBJECT(asrc, "EOS: asrc->frame=%p asrc->queue=%" G_GUINT32_FORMAT, asrc->queue,  asrc->queue ? asrc->queue->length : 0);
		asrc->eos = TRUE;
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
		return GST_FLOW_EOS; // end of stream
	}
	asrc->locked = FALSE;
	GST_OBJECT_UNLOCK (asrc);
	return GST_FLOW_OK;
}

static void gst_ambulantsrc_get_times (GstBaseSrc *src, GstBuffer *buffer,
					GstClockTime *start, GstClockTime *end)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC(src);
	GST_LOG_OBJECT (asrc, "timestamp=%" G_GUINT64_FORMAT, GST_BUFFER_PTS (buffer));

	if (start != NULL) {
		*start = GST_BUFFER_PTS (buffer);
	}
	if (end != NULL) {
		*end = GST_CLOCK_TIME_NONE;
	}
}

/* get the total size of the resource in bytes */
static gboolean gst_ambulantsrc_get_size (GstBaseSrc *src, guint64 *size)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC(src);
	GST_LOG_OBJECT (asrc, "enter");

	return 0; // size of datafile undetermined
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean ambulantsrc_init (GstPlugin* ambulantsrc)
{
	if(tracing)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

	/* debug category for fltering log messages
	 *
	 * exchange the string 'Template ambulantsrc' with your description
	 */
	GST_DEBUG_CATEGORY_INIT (gst_ambulantsrc_debug, "ambulantsrc",
				 0, "ambulantsrc");

	return gst_element_register (ambulantsrc, "ambulantsrc", GST_RANK_NONE,
				     GST_TYPE_AMBULANTSRC);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_AMBULANTSRC_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstambulantsrc"
#endif

/* gstreamer looks for this structure to register ambulantsrcs
 *
 * exchange the string 'Template ambulantsrc' with your ambulantsrc description
 */
GST_PLUGIN_DEFINE (
	GST_VERSION_MAJOR,
	GST_VERSION_MINOR,
	ambulantsrc,
	"Ambulant source element for gstreamer",
	ambulantsrc_init,
	VERSION,
	"LGPL",
	"GStreamer",
	"http://gstreamer.net/"
)
