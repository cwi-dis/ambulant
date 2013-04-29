#!/bin/sh
# set -x
echo GST_PLUGIN_PATH=$GST_PLUGIN_PATH # change this appropriately when a --prefix 'configure' option value is use
echo "Using ximagesink"
gst-launch-1.0 --gst-debug-level=2 ambulantsrc ! video/x-raw,format=BGRA,width=300,height=260,bpp=32,depth=32,framerate=30/1,endianness=4321,pixel-aspect-ratio=1/1,green_mask=16711680,red_mask=65280 ! videoconvert ! videoscale ! ximagesink < $srcdir/input

