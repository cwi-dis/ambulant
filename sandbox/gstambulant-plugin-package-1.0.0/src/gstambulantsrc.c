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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>
#include "gstambulantsrc.h"

static gboolean tracing = FALSE; // turn on to trace static function calls (without object)
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
	PROP_HEIGHT,
	PROP_NO_STDIN,
	PROP_SILENT
};

/* the capabilities of the outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
	GST_PAD_SRC,
	GST_PAD_ALWAYS,
// The caps are compatible with the data produced by anbulant_recorder_plugin.
// 'width' and 'height' are taken the header preceding each frame, or should match,
// if they are specified as properties. In the latter case, all data is read asynchronous.
// When (if ever) the window size may change, possibly the plugin need to be adapted to 
// implement caps renogetiation using the new width and height.
	GST_STATIC_CAPS ("video/x-raw,format=BGRA,width=(int) [ 1, 2147483647 ],height=(int) [ 1, 2147483647 ],bpp=32,depth=32,framerate=30/1,endianness=4321,pixel-aspect-ratio=1/1,green_mask=16711680,red_mask=65280;")
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
	
	g_object_class_install_property (gobject_class, PROP_SILENT,
		g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
				      FALSE, G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_WIDTH,
		g_param_spec_uint ("width", "Frame width", "width of initial frame in pixels",
				   0, G_MAXINT32, DEFAULT_WIDTH, G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_HEIGHT,
		g_param_spec_uint ("height", "Frame height","height of initial frame in pixels",
				   0, G_MAXINT32, DEFAULT_HEIGHT, G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_NO_STDIN,
		g_param_spec_boolean ("no-stdin", "No >stdin>", "Do not use stdin, produce only initial frame (valid with width, height only)",
				      FALSE, G_PARAM_READWRITE));
//	g_object_class_install_property (gobject_class, PROP_NO_WAIT,
//		g_param_spec_boolean ("no-wait", "No wait", "Do not wait for input on stdin, always produce a frame",
//				      FALSE, G_PARAM_READWRITE));
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
		if(tracing)fprintf(stderr,"%s databuffer=0x%p datapointer=0x%p\n", __PRETTY_FUNCTION__, frame->databuffer, frame->datapointer);
		if (frame->databuffer != NULL) {
			gst_buffer_unref (frame->databuffer);
		} else {
			if (frame->datapointer != NULL) {
				g_free (frame->datapointer);
			}
		}
	}
}

GstAmbulantFrame* gst_ambulantsrc_new_frame (guint W, guint H, gulong datasize, gulong timestamp, gulong checksum, gpointer data)
{
	if (W == 0 || H == 0 || datasize == 0) {
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
	frame->checksum = checksum;
	if(tracing)fprintf(stderr,"%s databuffer=0x%p datapointer=0x%p\n", __PRETTY_FUNCTION__, frame->databuffer, frame->datapointer);
	return frame;
}

// read a frame from stdin: fixed size (80 bytes) header followed by variable size pixel data
GstAmbulantFrame* gst_ambulantsrc_read_frame(GstAmbulantSrc* asrc)
{
	GstAmbulantFrame* frame = NULL;
	gulong timestamp = 0;
	GST_LOG_OBJECT (asrc, "");

	if (asrc == NULL) {
		GST_LOG_OBJECT(NULL, "asrc==NULL, should not happen");
		return;
	}
	// don' block during read
	GST_LOG_OBJECT (asrc, "input_fd=%" G_GUINT32_FORMAT, asrc->input_fd);
	gboolean was_locked = asrc->locked;
	if (was_locked) {
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
	}
	if(asrc->eos) {
		GST_LOG_OBJECT(asrc, "eos already true, bailing out");
		goto done;
	}
	char buf[81]; 
	buf[80] = 0;
	guint W,H; 
	gulong datasize, checksum;

	// First read the 80-byte header.
	GST_DEBUG_OBJECT(asrc, "xxxjack about to do fread for header fd=%d", fileno(asrc->input_stream));
	if (fread(buf,1,80,asrc->input_stream) != 80) {
		GST_DEBUG_OBJECT (asrc, "end-of-file reading frame header fd=%d", fileno(asrc->input_stream));
		asrc->eos = TRUE;
		goto done;
	}
	GST_DEBUG_OBJECT(asrc, "xxxjack returned from fread for header");
	if (sscanf(buf, "Time: %8lu\nSize: %8lu\nW: %5u\nH: %5u\nChksm: %24lx\n", &timestamp, &datasize, &W, &H, &checksum) != 5) {
		GST_DEBUG_OBJECT (asrc, "scanf failed while reading frame header");
		asrc->eos = TRUE;
		goto done;
	}
	
	// first frame, remember width, height
	if (asrc->width == 0 && asrc->height == 0) {
		asrc->width = W;
		asrc->height = H;
	}

	// Check that frame has same dimensions as specified (or as the first frame read)
	if (asrc->width != W || asrc->height != H) {
		GST_ERROR_OBJECT (asrc, "Input size (%" G_GUINT32_FORMAT ",%" G_GUINT32_FORMAT ") differs from (%" G_GUINT32_FORMAT ",%" G_GUINT32_FORMAT ")", W, H, asrc->width, asrc->height);
		asrc->eos = TRUE;
		goto done;
	}
	
	// Now read the frame data
	frame = gst_ambulantsrc_new_frame (W, H, datasize, timestamp, checksum, NULL);
	GST_DEBUG_OBJECT(asrc, "xxxjack about to do fread for data fd=%d", fileno(asrc->input_stream));
	size_t n_bytes = fread (frame->datapointer,1,frame->datasize,asrc->input_stream);
	GST_DEBUG_OBJECT(asrc, "xxxjack returned from fread for data fd=%d", fileno(asrc->input_stream));
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

gulong gst_ambulantsrc_checksum (void* data, gulong size)
{
	gulong cs = 0;
	guchar* dp = &((guchar*)data)[size];

	while (dp > (guchar* )data) cs += *--dp;

	return cs;
}


void gst_ambulantsrc_get_next_frame (GstAmbulantSrc* asrc)
{
	if (asrc == NULL) {
		return;
	}
	GST_LOG_OBJECT (asrc, "");

//	if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);
	gboolean was_locked = asrc->locked;
	if ( ! was_locked) {
		asrc->locked = TRUE;
		GST_OBJECT_LOCK (asrc);
	}
	// while nothing has been read from <stdin>, stick to the initial frame
	if (asrc->initial_frame) {
		if (asrc->queue == NULL || g_queue_get_length (asrc->queue) > 0) {
			asrc->initial_frame = FALSE;
		}
	}
	if ( ! asrc->initial_frame) {
		if (asrc->queue == NULL || g_queue_get_length (asrc->queue) > 0) {
			gst_ambulantsrc_delete_frame (asrc->frame);
			asrc->frame = NULL;
			if (asrc->queue == NULL) {
				asrc->frame = gst_ambulantsrc_read_frame (asrc);
			} else {
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
	GST_DEBUG_OBJECT (asrc, "");
//	if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);
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
	asrc->frame = gst_ambulantsrc_new_frame (W,H,datasize,0,0,data);
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
	GST_LOG_OBJECT (asrc, "");

	GstBaseSrc* bsrc = (GstBaseSrc*) asrc;
//	if(tracing)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);
	asrc->silent = TRUE;
	asrc->eos = FALSE;
	asrc->initial_frame = FALSE;
	asrc->width = 0;
	asrc->height = 0;
	asrc->frame = NULL;
	asrc->min_latency = DEFAULT_MIN_LATENCY;
	asrc->max_latency = DEFAULT_MAX_LATENCY;
	asrc->locked = FALSE;
	gst_base_src_set_async (bsrc, TRUE);
	gst_base_src_set_live (bsrc, TRUE);
	gst_base_src_set_format (bsrc, GST_FORMAT_TIME);
	asrc->caps = gst_static_pad_template_get_caps(&src_factory);
	asrc->thread = NULL;
	asrc->no_wait = FALSE;
	asrc->exit_requested = FALSE;
	asrc->input_fd = -1;
	asrc->input_stream = NULL;
}

static void
gst_ambulantsrc_run (GstAmbulantSrc * asrc)
{
	if (asrc != NULL) {
		GST_DEBUG_OBJECT (asrc, "");

//		if ( ! asrc->silent) fprintf(stderr,"%s start\n", __PRETTY_FUNCTION__);
		GST_OBJECT_LOCK (asrc);
		asrc->locked = TRUE;
		while ( ! asrc->eos && ! asrc->exit_requested) {
			GstAmbulantFrame* frame = gst_ambulantsrc_read_frame(asrc);
			if (asrc->exit_requested) {
				gst_ambulantsrc_delete_frame (frame);
			} else {
				g_queue_push_head (asrc->queue, frame);
			}
		}
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
		GST_LOG_OBJECT (asrc, "return: eos=%" G_GUINT32_FORMAT " timestamp=%" G_GUINT32_FORMAT, asrc->eos, asrc->exit_requested);
//		if ( ! asrc->silent) fprintf(stderr,"%s stop eos: %d exit_requested: %d\n", __PRETTY_FUNCTION__,asrc->eos,asrc->exit_requested);
		g_thread_exit((gpointer) NULL);
	}
}

static void gst_ambulantsrc_set_property (GObject * object, guint prop_id,
					  const GValue * value, GParamSpec * pspec)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (object);
	GST_DEBUG_OBJECT (asrc, "");
//	if (tracing || ! asrc->silent) fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

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
		case PROP_SILENT:
			asrc->no_stdin = g_value_get_boolean (value);
			break;
		case PROP_NO_STDIN:
			asrc->silent = g_value_get_boolean (value);
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
	GST_DEBUG_OBJECT (asrc, "");
//	if(tracing || !asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

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
		case PROP_NO_STDIN:
			g_value_set_boolean (value, asrc->no_stdin);
			break;
		case PROP_SILENT:
			g_value_set_boolean (value, asrc->silent);
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
	GST_DEBUG_OBJECT (asrc, "");

	gboolean rv = TRUE;
	GST_OBJECT_LOCK (asrc);
	asrc->locked = TRUE;
//	if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);
	int stdin_fd = fileno(stdin);
	asrc->input_fd = dup (stdin_fd);
	asrc->input_stream = fdopen (asrc->input_fd, "r");
	if (stdin_fd == -1 || asrc->input_fd == -1 || asrc->input_stream == NULL) { //XXXX GST_DEBUG
		fprintf (stderr, "Cannot 'dup' <stdin>: stdin_fd=%d, input_fd=%d, input_stream=%p, errno=%d\n", stdin_fd, asrc->input_fd, asrc->input_stream, errno);
		asrc->eos = TRUE;
	}
	if (asrc->width != 0 && asrc->height != 0) {
		asrc->no_wait = TRUE;
	}
	if (asrc->no_wait && asrc->thread == NULL) {
		asrc->queue = g_queue_new();
		if (asrc->queue == NULL) {
			asrc->eos = TRUE;
			rv = FALSE;
		}
		if (rv) {
		gst_ambulantsrc_init_frame (asrc); // XXXX wrong sprops
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
		GThread* thread = g_thread_new ("reader", (GThreadFunc) &gst_ambulantsrc_run, asrc);
		GST_OBJECT_LOCK (asrc);
		asrc->thread = thread;
		asrc->locked = TRUE;
		// g_thread_new() aborts on failure
		// XXXXX cond wait here until properly initialized
		}
	} else if (asrc->frame == NULL) {
		asrc->frame = gst_ambulantsrc_read_frame (asrc);
	}
	asrc->locked = FALSE;
	GST_OBJECT_UNLOCK (asrc);
	return rv;
}

static gboolean gst_ambulantsrc_stop (GstBaseSrc * basesrc)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (basesrc);
	GST_DEBUG_OBJECT (asrc, "databuffer=0x%p", asrc->frame == NULL ? NULL : asrc->frame->databuffer);
//	if(!asrc->silent)fprintf(stderr,"%s databuffer=0x%p\n", __PRETTY_FUNCTION__,
//				asrc-> frame == NULL ? NULL : asrc->frame->databuffer);
	GST_OBJECT_LOCK (asrc);
	asrc->locked = TRUE;
	asrc->eos = TRUE;
//	if (asrc->caps != NULL) {
//		gst_caps_unref (asrc->caps);
//		asrc->caps = NULL;
//	}
	if (asrc->thread != NULL) {
		asrc->exit_requested = TRUE;
	}
	if (asrc->queue != NULL) {
		g_queue_free_full (asrc->queue, (GDestroyNotify) gst_ambulantsrc_delete_frame);
		asrc->queue = NULL;
	}
	gst_ambulantsrc_delete_frame (asrc->frame);
	if (close (asrc->input_fd) != 0) {
		GST_ERROR_OBJECT (asrc, "close() failed%" G_GUINT32_FORMAT, errno);
//		fprintf(stderr,"%s: close() failed%d\n", __PRETTY_FUNCTION__, errno);
	}
	asrc->input_fd = -1;
	asrc->locked = FALSE;
	GST_OBJECT_UNLOCK (asrc);

	return TRUE;
}

// The caps are fully known, simplified and fixated during the first call of gst_ambulantxrsc_query() after
// gst_ambulantsrc_init() was called, since then the actual With,Height of the video data are read.
static GstCaps* gst_ambulantsrc_get_caps (GstBaseSrc * bsrc, GstCaps* filter) {
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
	gchar* s = gst_caps_to_string(filter);
	GST_DEBUG_OBJECT (asrc, "");

//	if(!asrc->silent)fprintf(stderr,"%s=%s\n", __PRETTY_FUNCTION__,s);

	g_free(s);
	return asrc->caps;
}
/* this function handles the link with other elements 
static gboolean gst_ambulantsrc_set_caps (GstBaseSrc* bsrc, GstCaps * caps)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
	gchar* s = gst_caps_to_string(caps);
	if(!asrc->silent)fprintf(stderr,"%s=%s\n", __PRETTY_FUNCTION__,s);

	g_free(s);
	return TRUE;
}
*/

static gboolean gst_ambulantsrc_query (GstBaseSrc * bsrc, GstQuery * query)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
	gboolean res = FALSE;
	GST_DEBUG_OBJECT (asrc, "%s",	gst_query_type_get_name(GST_QUERY_TYPE(query)));

//	if(!asrc->silent) {
//		fprintf(stderr,"%s:%s\n", __PRETTY_FUNCTION__,
//			gst_query_type_get_name(GST_QUERY_TYPE(query)));
//	}
	switch (GST_QUERY_TYPE (query)) {
		case GST_QUERY_LATENCY: {
			gst_query_set_latency (query, TRUE, asrc->min_latency, asrc->max_latency);
			if(!asrc->silent) {
				GST_DEBUG_OBJECT(asrc, "min-latency=%" G_GUINT64_FORMAT " max-latency=%" G_GUINT64_FORMAT,asrc->min_latency,asrc->max_latency);
//	        		fprintf(stderr,"%s min-latency=%" G_GUINT64_FORMAT 
//			        	" max-latency=%" G_GUINT64_FORMAT"\n",
//					__PRETTY_FUNCTION__,asrc->min_latency,asrc->max_latency);
			}
			res = TRUE;
			break;
		}
		case GST_QUERY_CAPS: {
		if (asrc->no_wait) {
				if (asrc->thread) {
					gst_ambulantsrc_init_frame(asrc);
				} else break;
			} else {
				if (asrc->frame == NULL) { // W,H not yet known
					break;
				}
			}
			GST_DEBUG_OBJECT(asrc, "%s caps_query: frame=%p W=%" G_GUINT32_FORMAT " H=%" G_GUINT32_FORMAT, 
					 asrc->frame, asrc->frame->W, asrc->frame->H);
//			if(!asrc->silent) {
//				fprintf(stderr,"%s caps_query: frame=%p W=%d H=%d\n", 
//				__PRETTY_FUNCTION__, asrc->frame, asrc->frame->W, asrc->frame->H);
//			}
			// answer to query: template caps + witdh, height from input data header
			GstCaps* caps_org = asrc->caps;
			GstCaps* caps_new = NULL;
			if (caps_org != NULL && ! gst_caps_is_fixed (caps_org)) {
				// convert original caps to string
				gchar* caps_org_str = gst_caps_to_string (caps_org);
				// create new caps including actual width, height
				gchar* caps_new_str = g_strdup_printf ("%s,width=(int)%d, height=(int)%d", 
								       caps_org_str, asrc->frame->W, asrc->frame->H);
				caps_new = gst_caps_from_string (caps_new_str);
				g_free (caps_org_str);
				g_free (caps_new_str);
				if (caps_new != NULL) {
					// simplify (remove multiple width,height) and fixate
					GstCaps* caps_simplified = gst_caps_simplify (caps_new);
					gchar* caps_simplified_str = gst_caps_to_string (caps_simplified);
					caps_simplified = gst_caps_fixate (caps_simplified); //XXXX make depend on debug level
					GST_DEBUG_OBJECT(asrc, "caps_new_str=%s", caps_simplified_str);
					g_free (caps_simplified_str);
//					if(!asrc->silent) {
//						gchar* caps_simplified_str = gst_caps_to_string (caps_simplified);
//						fprintf(stderr,"%s caps_new_str=%s\n",
//							__PRETTY_FUNCTION__, caps_simplified_str);
//						g_free (caps_simplified_str);
//					}
					gst_query_set_caps_result (query, caps_simplified);
					gst_caps_unref (asrc->caps);
					asrc->caps = caps_simplified;
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
	GST_LOG_OBJECT (asrc, "");
	
	GST_OBJECT_LOCK (asrc);
	asrc->locked = TRUE;
	
	if (buffer == NULL) {
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
		return GST_FLOW_OK;
	}
	if (asrc->eos && (asrc->queue == NULL || g_queue_get_length(asrc->queue)) == 0) {
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
		return GST_FLOW_EOS; // end of stream
	}
	gst_ambulantsrc_get_next_frame(asrc);
	if (asrc->frame == NULL) {
		asrc->eos = TRUE;
		asrc->locked = FALSE;
		GST_OBJECT_UNLOCK (asrc);
		return GST_FLOW_EOS; // end of stream
	}
	if(!asrc->silent) {
		GST_LOG_OBJECT(asrc, "Timestamp=%" G_GUINT64_FORMAT "size=%" G_GUINT64_FORMAT "offset=%" G_GUINT64_FORMAT, asrc->frame->timestamp, asrc->frame->datasize, offset);
//      	fprintf(stderr, "%s: Timestamp=%ld ms size=%ld offset=%ld \n",
//			__PRETTY_FUNCTION__, asrc->frame->timestamp, asrc->frame->datasize, offset);
	}
	GstBuffer* buf = asrc->frame->databuffer;
	GST_BUFFER_DTS (buf) = asrc->frame->timestamp * 1000000; // millis to nanos
	GST_BUFFER_PTS (buf) = GST_BUFFER_DTS (buf);
	GST_BUFFER_DURATION (buf) = GST_CLOCK_TIME_NONE;
	GST_BUFFER_OFFSET (buf) = offset;
	gst_buffer_ref(buf);
	*buffer = buf;
	asrc->locked = FALSE;
	GST_OBJECT_UNLOCK (asrc);
	return GST_FLOW_OK;
}

static void gst_ambulantsrc_get_times (GstBaseSrc *src, GstBuffer *buffer,
					GstClockTime *start, GstClockTime *end)
{
	GstAmbulantSrc *asrc = GST_AMBULANTSRC(src);
	GST_LOG_OBJECT (asrc, "timestamp=%" G_GUINT64_FORMAT, GST_BUFFER_PTS (buffer));

//	if(!asrc->silent) {
//      	fprintf(stderr,"%s timestamp=%lu\n", __PRETTY_FUNCTION__, GST_BUFFER_PTS (buffer));
//	}
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
	GST_LOG_OBJECT (asrc, "");

//	if(asrc && asrc->frame && !asrc->silent) fprintf(stderr,"%s=%ld\n", __PRETTY_FUNCTION__, asrc->frame->datasize);

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
