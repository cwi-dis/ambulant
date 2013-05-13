#!/bin/sh
# set -x
echo "Using ximagesink"
gst-launch-1.0 ambulantsrc silent=1 ! videoconvert ! videoscale ! ximagesink < $srcdir/input


