#!/bin/sh
# set -x
if  xvinfo|grep "no adaptors"  ; then echo "\n*** No XVideo available, nothing to see. ***\n"; fi
echo GST_PLUGIN_PATH=$GST_PLUGIN_PATH # change this appropriately when a --prefix 'configure' option value is use
# TBD get the caps from tests/input and use these in the sink pad of the plugin
echo $srcdir
gst-launch-1.0 --gst-debug-level=2 ambulantsrc ! video/x-raw,format=RGB,width=300,height=260,bpp=24,depth=24,framerate=30/1,endianness=4321,pixel-aspect-ratio=5/4,red_mask=16711680,green_mask=65280,blue_mask=255 ! videoconvert ! sdlvideosink < $srcdir/input
if  [ $? ]
then   gst-launch-1.0 --gst-debug-level=2 ambulantsrc ! video/x-raw,format=RGB,width=300,height=260,bpp=24,depth=24,framerate=30/1,endianness=4321,pixel-aspect-ratio=5/4,red_mask=16711680,green_mask=65280,blue_mask=255 ! videoconvert ! autovideosink < $srcdir/input
fi
