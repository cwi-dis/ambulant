#!/bin/bash
#set -x
#echo "Using ximagesink. On the commandline you may specify OPTIONS=... for ambulantsrc. "
    
if [ "x$DELAY" = 'x' ] ; then DELAY=0; fi
export GST_PLUGIN_PATH=$PWD/../src/.libs

#VALGRIND=valgrind --leak-check=full

get_input () {
    sleep $DELAY
    cat $INPUT
}
echo CFLAGS=$CFLAGS
if [[ \""$CFLAGS"\" != *WITH_AUDIO* ]] ;
then
    echo "Showing video"
    if [ "x$INPUT" = "x" ] ; then INPUT=$srcdir/Welcome-video-raw; fi
    get_input | $VALGRIND gst-launch-1.0 $GSTOPTIONS ambulantsrc is-live=false $OPTIONS ! videoconvert ! videoscale ! ximagesink sync=false
else
    echo "Re-playing audio"
if [ "x$INPUT" = "x" ] ; then INPUT=$srcdir/Welcome-audio-raw; fi
    get_input | $VALGRIND gst-launch-1.0 $GSTOPTIONS ambulantsrc is-live=false $OPTIONS ! audiorate ! audioconvert  ! audioresample ! alsasink sync=false 
fi