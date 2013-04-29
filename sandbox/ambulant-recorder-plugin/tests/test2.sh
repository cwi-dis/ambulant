#!/bin/sh
# assume local install of all extra depencies: AmbulantPlayer, SDL2, gstreamer, gstambulantsrc, etc...
export PATH=$HOME/bin:$PATH
export LD_LIBRARY_PATH=$HOME/lib:$LD_LIBRARY_PATH

export BROWSER=firefox

AMBULANT_RECORDER_PIPE="gst-launch  ambulantsrc ! video/x-raw-rgb,width=640,height=480,framerate=30/1,bpp=32,depth=32,endianness=4321,green_mask=16711680,red_mask=65280,pixel-aspect-ratio=1/1 ! ffmpegcolorspace ! videoscale ! ximagesink sync=false" make check &
pid=$!
sleep 10
kill -9 $pid

