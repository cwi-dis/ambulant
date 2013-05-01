#!/bin/sh
# set -x
echo GST_PLUGIN_PATH=$GST_PLUGIN_PATH # change this appropriately when a --prefix 'configure' option value is use
echo "Using ximagesink"
gst-launch-1.0 --gst-debug-level=1 ambulantsrc silent=0 ! video/x-raw,width=300,height=240, framerate=30/1 ! videoconvert ! videoscale ! ximagesink < $srcdir/input

