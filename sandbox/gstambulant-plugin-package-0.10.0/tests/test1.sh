#!/bin/sh
set -x
GST_PLUGIN_PATH=/usr/local/lib/gstreamer-0.10 # change this appropriately when a --prefix 'configure' option value is used
# TBD get the caps from tests/input and use these in the sink pad of the plugin
echo $srcdir
gst-launch --gst-debug-level=2 ambulantsrc ! video/x-raw-rgb,width=300,height=260,bpp=24,depth=24,framerate=30/1,endianness=4321,pixel-aspect-ratio=5/4,red_mask=16711680,green_mask=65280,blue_mask=255 ! ffmpegcolorspace ! sdlvideosink < $srcdir/input
if  [ $? ]
then   gst-launch --gst-debug-level=2 ambulantsrc ! video/x-raw-rgb,width=300,height=260,bpp=24,depth=24,framerate=30/1,endianness=4321,pixel-aspect-ratio=5/4,red_mask=16711680,green_mask=65280,blue_mask=255 ! ffmpegcolorspace ! autovideosink < $srcdir/input
fi