#!/bin/sh
# set -x
#export GST_PLUGIN_PATH=../src/.libs
echo GST_PLUGIN_PATH=$GST_PLUGIN_PATH # change this appropriately when a --prefix 'configure' option value is use
echo "Using ximagesink"
gst-launch-1.0 --gst-debug-level=1 ambulantsrc silent=0 ! videoconvert ! videoscale ! ximagesink < $srcdir/input&
PID=$?
#sleep 5
#kill -9 $PID

