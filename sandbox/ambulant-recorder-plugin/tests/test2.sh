#!/bin/sh
# assume local install of all extra depencies: AmbulantPlayer, SDL2, gstreamer, gstambulantsrc, etc...
export PATH=$HOME/bin:$PATH
export LD_LIBRARY_PATH=$HOME/lib:$LD_LIBRARY_PATH

export BROWSER=firefox

AMBULANT_RECORDER_PIPE="gst-launch ambulantsrc silent=0 ! ffmpegcolorspace ! videoscale ! ximagesink sync=false" make check &
pid=$!
sleep 10
kill -9 $pid
