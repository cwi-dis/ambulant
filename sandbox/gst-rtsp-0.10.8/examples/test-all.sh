#!/bin/sh
ARCH=`unmae -m`
BINDIR=../build-$ARCH/examples
$BINDIR/test-webcam /dev/video1 8553 & sleep 2;AMBULANT_RECORDER_PIPE="$BINDIR/test-launch 'ambulantsrc ! video/x-raw-rgb,width=640,height=480,bpp=24,depth=24,framerate=30/1,endianness=4321,pixel-aspect-ratio=4/3,red_mask=16711680,green_mask=65280,blue_mask=255 ! ffmpegcolorspace ! x264enc tune=zerolatency ! rtph264pay name=pay0 pt=96'" AmbulantPlayer ./test-webcam.smil & sleep 4; mplayer -noframedrop -fps 30 rtsp://localhost:8554/test
