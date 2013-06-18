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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
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
 * gst-launch-1.0  ambulantsrc ! videoconvert ! videoscale ! ximagesink < tests/input
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gstambulantsrc.h"
#include <stdio.h>

#ifdef  FRAME_DELAY_DEBUG  
#include <sys/time.h>
#endif//FRAME_DELAY_DEBUG  

static gboolean tracing = FALSE; // turn on to trace static function calls (without object)
GST_DEBUG_CATEGORY_STATIC (gst_ambulantsrc_debug);
#define GST_CAT_DEFAULT gst_ambulantsrc_debug

/* Filter signals and args */
enum
{
    /* FILL ME */
    LAST_SIGNAL
};


#define DEFAULT_MIN_LATENCY ((30 * GST_MSECOND) /GST_USECOND)
#define DEFAULT_MAX_LATENCY ((30 * GST_MSECOND) /GST_USECOND)

enum
{
    PROP_0,
    PROP_MIN_LATENCY,
    PROP_MAX_LATENCY,
    PROP_NO_WAIT,
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
// 'width' and 'height' are taken the header preceding each frame.
// When (if ever) the window size may change,  possibly the plugin need to be adapted to 
// implement caps renogetiation using the new width and height.  
    GST_STATIC_CAPS ("video/x-raw,format=BGRA,width=(int) [ 1, 2147483647 ],height=(int) [ 1, 2147483647 ],bpp=32,depth=32,framerate=30/1,endianness=4321,pixel-aspect-ratio=1/1,green_mask=16711680,red_mask=65280;")
    );

#define gst_ambulantsrc_parent_class parent_class
G_DEFINE_TYPE (GstAmbulantSrc, gst_ambulantsrc, GST_TYPE_BASE_SRC);

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

/* GObject vmethod implementations */

/* initialize the plugin's class */
static void
gst_ambulantsrc_class_init (GstAmbulantSrcClass * klass)
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
    g_object_class_install_property (gobject_class, PROP_NO_WAIT,
        g_param_spec_boolean ("no-wait", "No wait", "Do not wait for input on stdin, always produce a frame",
			      FALSE, G_PARAM_READWRITE));
    g_object_class_install_property (gobject_class, PROP_MIN_LATENCY,
        g_param_spec_int64 ("min-latency", "Minimum latency time", "Minimum latency in microseconds",
			    1, G_MAXINT64, DEFAULT_MIN_LATENCY, G_PARAM_READWRITE));
    g_object_class_install_property (gobject_class, PROP_MAX_LATENCY,
    g_param_spec_int64 ("max-latency", "Maximum latency time", "Maximum latency in microseconds",
			1, G_MAXINT64, DEFAULT_MAX_LATENCY, G_PARAM_READWRITE));

    gstbasesrc_class->start = gst_ambulantsrc_start;
    gstbasesrc_class->stop = gst_ambulantsrc_stop;
    gstbasesrc_class->get_caps = gst_ambulantsrc_get_caps;
//  gstbasesrc_class->set_caps = gst_ambulantsrc_set_caps; // disabled, SEGV
    gstbasesrc_class->query = gst_ambulantsrc_query;
    gstbasesrc_class->get_size = gst_ambulantsrc_get_size;
    gstbasesrc_class->get_times = gst_ambulantsrc_get_times;
    gstbasesrc_class->create = gst_ambulantsrc_create;
    gst_element_class_set_details_simple((GstElementClass*) gstbasesrc_class,
        "Ambulant Source Element",
        "Source",
        "Read frames produced by 'ambulant-recorder-plugin' from 'stdin'"
        "(format: 80 byte ASCII header, follewed by variable number of pixels, BGRA 32bpp)"
        " and push these as buffers in a gstreamer pipeline",
        "Kees Blom <<Kees.Blom@cwi.nl>>");
    gst_element_class_add_pad_template ((GstElementClass*) gstbasesrc_class,
					gst_static_pad_template_get (&src_factory));
}

void delete_frame (GstAmbulantFrame* frame) {
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

GstAmbulantFrame* new_frame (guint W, guint H, gulong datasize, gulong timestamp,  gulong checksum, gpointer data)
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
            delete_frame (frame);
	    return NULL;
	}
    } else {
        frame->datapointer = data;
    }
    frame->databuffer = gst_buffer_new_wrapped (frame->datapointer, datasize);
    if (frame->databuffer == NULL) {
        delete_frame (frame);
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
GstAmbulantFrame* read_frame(GstAmbulantSrc* asrc)
{
    GstAmbulantFrame* frame = NULL;

    if (asrc != NULL) {
        // don' block during read
        if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);
	gboolean was_locked = asrc->locked;
	if (was_locked) {
	    asrc->locked = FALSE;
	    GST_OBJECT_UNLOCK (asrc);
	}
	clearerr(stdin);

	char buf[81]; buf[80] = 0;
	guint W,H; gulong timestamp, datasize, checksum;
	if (fread(buf,1,80,stdin) != 80 
	    || sscanf(buf, "Time: %8lu\nSize: %8lu\nW: %5u\nH: %5u\nChksm: %24lx\n",
		      &timestamp, &datasize, &W, &H, &checksum) != 5) {
            asrc->eos = TRUE;
	}
	if ( ! asrc->eos) {
	    frame = new_frame (W, H, datasize, timestamp, checksum, NULL);
	    size_t n_bytes = fread (frame->datapointer,1,frame->datasize,stdin);
	    if (n_bytes != frame->datasize) {
                asrc->eos = TRUE;
	    } else {
//              gulong cs = checksum (asrc->databuffer,asrc->datasize);
//              if (cs != asrc->checksum) {
//                  fprintf (stderr, "checksum failed:  cs=%lx, asrc->checksum=%lx\n", cs, asrc->checksum);
//              }
	    }
	}
	if (was_locked) {
	    GST_OBJECT_LOCK (asrc);
	    asrc->locked = TRUE;
	}
    }
    return frame;
}

gulong checksum (void* data, gulong size)
{
    gulong cs = 0;
    guchar* dp = &((guchar*)data)[size];

    while (dp > (guchar* )data) cs += *--dp;

    return cs;
}


void get_next_frame (GstAmbulantSrc* asrc)
{
    if (asrc == NULL) {
        return;
    }
    if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);
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
        delete_frame (asrc->frame);
	asrc->frame = NULL;
	if (asrc->queue != NULL) {
	    if (g_queue_get_length (asrc->queue) > 0) {
                asrc->frame = g_queue_pop_tail (asrc->queue);
	    }
	} else {
	  asrc->frame = read_frame (asrc);
	}
	if (asrc->frame == NULL) {
            asrc->eos = TRUE;
	}
    }
    if ( ! was_locked) {
        asrc->locked = FALSE;
	GST_OBJECT_UNLOCK (asrc);
    }
}

void init_frame(GstAmbulantSrc* asrc)
{
    if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);
    
    guint W=300, H=240;
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
    asrc->frame = new_frame (300,240,datasize,0,0,data);
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
    GstBaseSrc* bsrc = (GstBaseSrc*) asrc;
    if(tracing || !asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);
    asrc->silent = TRUE;
    asrc->eos = FALSE;
    asrc->initial_frame = FALSE;
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
}

static void
gst_ambulantsrc_run (GstAmbulantSrc * asrc)
{
    if (asrc != NULL) {
        GST_OBJECT_LOCK (asrc);
	asrc->locked = TRUE;
	while ( ! asrc->eos && ! asrc->exit_requested) {
	    GstAmbulantFrame* frame = read_frame(asrc);
	    if (asrc->exit_requested) {
	        delete_frame (frame);
	    } else {
	        g_queue_push_head (asrc->queue, frame);
	    }
	}
	asrc->locked = FALSE;
	GST_OBJECT_UNLOCK (asrc);
	g_thread_exit((gpointer) NULL);
    }
}  

static void
gst_ambulantsrc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstAmbulantSrc *asrc = GST_AMBULANTSRC (object);
    if (tracing || ! asrc->silent) fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

    switch (prop_id) {
        case PROP_MIN_LATENCY:
            asrc->min_latency = g_value_get_int64 (value);
	    break;
        case PROP_MAX_LATENCY:
	    asrc->max_latency = g_value_get_int64 (value);
	    break;
        case PROP_NO_WAIT:
            asrc->no_wait = g_value_get_boolean (value);
	    break;
        case PROP_SILENT:
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
    if(tracing || !asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);

    switch (prop_id) {
        case PROP_MIN_LATENCY:
            g_value_set_int64 (value, asrc->min_latency);
	    break;
        case PROP_MAX_LATENCY:
            g_value_set_int64 (value, asrc->max_latency);
	    break;
        case PROP_NO_WAIT:
            g_value_set_boolean (value, asrc->no_wait);
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
    gboolean rv = TRUE;
    GST_OBJECT_LOCK (asrc);
    asrc->locked = TRUE;
    if(!asrc->silent)fprintf(stderr,"%s\n", __PRETTY_FUNCTION__);
    if (asrc->no_wait && asrc->thread == NULL) {
	asrc->queue = g_queue_new();
	if (asrc->queue == NULL) {
	    asrc->eos = TRUE;
	    rv = FALSE;
	}
	if (rv) {
	    init_frame (asrc); // XXXX wrong sprops
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
        asrc->frame = read_frame (asrc);
        asrc->initial_frame = TRUE;
    }
    asrc->locked = FALSE;
    GST_OBJECT_UNLOCK (asrc);
    return  rv;
}

static gboolean
gst_ambulantsrc_stop (GstBaseSrc * basesrc)
{
    GstAmbulantSrc *asrc = GST_AMBULANTSRC (basesrc);
    if(!asrc->silent)fprintf(stderr,"%s databuffer=0x%p\n", __PRETTY_FUNCTION__,
			     asrc-> frame == NULL ? "<no frame>" : asrc->frame->databuffer);

    GST_OBJECT_LOCK (asrc);
    asrc->locked = TRUE;

    asrc->eos = TRUE;
//    if (asrc->caps != NULL) {
//      gst_caps_unref (asrc->caps);
//	asrc->caps = NULL;
//    }
    if (asrc->thread != NULL) {
        asrc->exit_requested = TRUE;
    }
    if (asrc->queue != NULL) {
        g_queue_free_full (asrc->queue, (GDestroyNotify) delete_frame);
	asrc->queue = NULL;
    }
    delete_frame (asrc->frame);
    asrc->locked = FALSE;
    GST_OBJECT_UNLOCK (asrc);

    return TRUE;
}

// The caps are fully known, simplified and fixated during the first call of gst_ambulantxrsc_query() after
// gst_ambulantsrc_init() was called, since then the actual With,Height of the video data are read.
static GstCaps* gst_ambulantsrc_get_caps (GstBaseSrc * bsrc, GstCaps* filter) {
    GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
    gchar* s = gst_caps_to_string(filter);
    if(!asrc->silent)fprintf(stderr,"%s=%s\n", __PRETTY_FUNCTION__,s);

    g_free(s);
    return asrc->caps;
}
/* this function handles the link with other elements
   static gboolean 
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

    if(!asrc->silent) {
        fprintf(stderr,"%s:%s\n", __PRETTY_FUNCTION__,
		gst_query_type_get_name(GST_QUERY_TYPE(query)));
    }
    switch (GST_QUERY_TYPE (query)) {
        case GST_QUERY_LATENCY:
            {
	        gst_query_set_latency (query, TRUE, asrc->min_latency, asrc->max_latency);
		if(!asrc->silent) {
	            fprintf(stderr,"%s min-latency=%" G_GUINT64_FORMAT 
			             " max-latency=%" G_GUINT64_FORMAT"\n",
			    __PRETTY_FUNCTION__,asrc->min_latency,asrc->max_latency);
		}
		res = TRUE;
		break;
	    }
        case GST_QUERY_CAPS:
	    {
	        if (asrc->no_wait) {
		    if (asrc->thread) {
		        init_frame(asrc);
		    } else break;
		} else {
	            if (asrc->frame == NULL) { // W,H not yet known
		        break;
		    }
		}
		if(!asrc->silent) {
		    fprintf(stderr,"%s caps_query: frame=%p W=%d H=%d\n", 
			    __PRETTY_FUNCTION__, asrc->frame, asrc->frame->W, asrc->frame->H);
		}
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
			caps_simplified = gst_caps_fixate (caps_simplified);
			if(!asrc->silent) {
			    gchar* caps_simplified_str = gst_caps_to_string (caps_simplified);
			    fprintf(stderr,"%s caps_new_str=%s\n",
				    __PRETTY_FUNCTION__, caps_simplified_str);
			    g_free (caps_simplified_str);
			}
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
	    res =  GST_BASE_SRC_CLASS (gst_ambulantsrc_parent_class)->query (bsrc, query);
	    break;
    }
    return res;
}

static GstFlowReturn gst_ambulantsrc_create (GstBaseSrc * bsrc, guint64 offset, guint length, GstBuffer ** buffer)
{
    GstAmbulantSrc *asrc = GST_AMBULANTSRC (bsrc);
    GST_OBJECT_LOCK (asrc);
    asrc->locked = TRUE;

#ifdef FRAME_DELAY_DEBUG  
    time_t now  = time(NULL);
    struct tm *lt = localtime(&now);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    fprintf(stderr,"%02d:%02d:%02d.%06ld %s(bsrc=%p,offset=%lu,length=%u,buffer=%p) timestamp=%ld datapointer=0x%lx\n", 
	    lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec, __PRETTY_FUNCTION__,bsrc, 
	    offset, length, buffer, asrc->timestamp, (long unsigned int) asrc->datapointer);  // enable for frame delay debugging
#else
    if(!asrc->silent) {
        fprintf(stderr, "%s: Timestamp=%ld ms size=%ld offset=%ld \n",
		__PRETTY_FUNCTION__, asrc->frame->timestamp, asrc->frame->datasize, offset);
    }
#endif//FRAME_DELAY_DEBUG  

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
    get_next_frame(asrc);
    if (asrc->frame == NULL) {
        asrc->eos = TRUE;
	asrc->locked = FALSE;
	GST_OBJECT_UNLOCK (asrc);
	return GST_FLOW_EOS; // end of stream
    }
    GstBuffer* buf = asrc->frame->databuffer;
    GST_BUFFER_DTS (buf) =  asrc->frame->timestamp * 1000000; // millis to nanos
    GST_BUFFER_PTS (buf) =  GST_BUFFER_DTS (buf);
    GST_BUFFER_DURATION (buf) = GST_CLOCK_TIME_NONE;
    GST_BUFFER_OFFSET (buf) = offset;
    gst_buffer_ref(buf);
    *buffer = buf;
#ifdef FRAME_DELAY_DEBUG
    now  = time(NULL);
    lt   = localtime(&now);
    gettimeofday(&tv, NULL);
    fprintf(stderr,"%02d:%02d:%02d.%06ld %s(bsrc=%p,offset=%lu,length=%u,buffer=%p) timestamp=%ld data=0x%x\n",
	    lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec, __PRETTY_FUNCTION__,bsrc, offset, length, buffer, asrc->timestamp, *(unsigned int*) asrc->datapointer);  // enable for frame delay debugging
#endif//FRAME_DELAY_DEBUG  
    asrc->locked = FALSE;
    GST_OBJECT_UNLOCK (asrc);
    return GST_FLOW_OK;
}

static void gst_ambulantsrc_get_times (GstBaseSrc *src, GstBuffer *buffer,
					GstClockTime *start, GstClockTime *end)
{
    GstAmbulantSrc *asrc = GST_AMBULANTSRC(src);
    if(!asrc->silent) {
        fprintf(stderr,"%s timestamp=%lu\n", __PRETTY_FUNCTION__, GST_BUFFER_PTS (buffer));
    }
    if (start != NULL) {
        *start = GST_BUFFER_PTS (buffer);
    }
    if (end != NULL) {
        *end =  GST_CLOCK_TIME_NONE;
    }
}

/* get the total size of the resource in bytes */
static gboolean gst_ambulantsrc_get_size (GstBaseSrc *src, guint64 *size)
{
    GstAmbulantSrc *asrc = GST_AMBULANTSRC(src);
    if(asrc && asrc->frame && !asrc->silent) fprintf(stderr,"%s=%ld\n", __PRETTY_FUNCTION__, asrc->frame->datasize);

    return 0; // size of datafile undetermined
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
ambulantsrc_init (GstPlugin* ambulantsrc)
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
