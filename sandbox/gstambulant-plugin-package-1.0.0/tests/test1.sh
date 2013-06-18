#!/bin/sh
# set -x
echo "Using ximagesink. On the commandline you may specify OPTIONS=... for ambulantsrc. "
    
if [ "x$DELAY" = 'x' ] ; then DELAY=0; fi
if [ "x$INPUT" = "x" ] ; then INPUT=$srcdir/input; fi
#VALGRIND=valgrind --leak-check=full

get_input () {
    sleep $DELAY
    cat $INPUT
}
get_input | $VALGRIND gst-launch-1.0 ambulantsrc $OPTIONS ! videoconvert ! videoscale ! ximagesink sync=false


