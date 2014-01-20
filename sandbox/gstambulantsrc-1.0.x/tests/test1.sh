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
if [ x$OPTIONS == x ] ; then OPTIONS="is-live=false" ; fi
if [ x$DISPLAY == x ] ; then GSTSINK="fakesink" ; else GSTSINK="ximagesink"; OPTIONS="$OPTIONS width=200 height=200"; fi
if [[ \""$CFLAGS"\" != *WITH_AUDIO* ]] ;
then
    echo "Showing video"
    if [ "x$INPUT" = "x" ] ; then INPUT=$srcdir/Welcome-video-raw; fi
    get_input | $VALGRIND gst-launch-1.0 $GSTOPTIONS ambulantsrc $OPTIONS ! videoconvert ! videoscale ! $GSTSINK sync=false
else
    echo "Re-playing audio"
if [ "x$INPUT" = "x" ] ; then INPUT=$srcdir/Welcome-audio-raw; fi
    get_input | $VALGRIND gst-launch-1.0 $GSTOPTIONS ambulantsrc $OPTIONS ! audiorate ! audioconvert  ! audioresample ! alsasink sync=false 
fi
