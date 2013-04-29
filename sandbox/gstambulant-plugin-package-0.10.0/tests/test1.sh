#!/bin/sh
# set -x
GST_PLUGIN_PATH=/usr/local/lib/gstreamer-0.10 # change this appropriately when a --prefix 'configure' option value is used
# TBD get the caps from tests/input and use these in the sink pad of the plugin
echo $srcdir
gst-launch --gst-debug-level=2 ambulantsrc ! video/x-raw-rgb,width=300,height=260,bpp=32,depth=32,framerate=30/1,endianness=4321,pixel-aspect-ratio=1/1,green_mask=16711680,red_mask=65280 ! ffmpegcolorspace ! sdlvideosink < $srcdir/input
if  [ ! $? ]
then   gst-launch --gst-debug-level=2 ambulantsrc ! video/x-raw-rgb,width=300,height=260,bpp=32,depth=32,framerate=30/1,endianness=4321,pixel-aspect-ratio=1/1,green_mask=16711680,red_mask=65280 ! ffmpegcolorspace ! autovideosink < $srcdir/input
fi
if  [ ! $? ]
then gst-launch --gst-debug-level=2 ambulantsrc ! video/x-raw-rgb,width=300,height=260,bpp=32,depth=32,framerate=30/1,endianness=4321,pixel-aspect-ratio=1/1,green_mask=16711680,red_mask=65280 ! ffmpegcolorspace ! videoscale ! ximagesink sync=false < $srcdir/input
fi