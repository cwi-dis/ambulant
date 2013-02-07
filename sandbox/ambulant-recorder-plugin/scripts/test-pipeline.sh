if [ "$GST_RTSP_EXAMPLES" == "" ]
then
	echo GST_RTSP_EXAMPLES not set. It should point to \'gstreamer/gst-rtsp/examples\' after building.
	exit -1
fi
killall -9 lt-test-launch 2>>/dev/null # hangs sometimes, blocking port 8554
AMBULANT_RECORDER_PIPE="$GST_RTSP_EXAMPLES/test-launch --gst-debug-level=2 '(ambulantsrc silent=true ! video/x-raw-rgb,width=640,height=480,bpp=24,depth=24,framerate=25/1,endianness=4321,pixel-aspect-ratio=1/1,red_mask=16711680,green_mask=65280,blue_mask=255 ! ffmpegcolorspace ! x264enc tune=zerolatency  cabac=false ! rtph264pay name=pay0 pt=96 )'" AmbulantPlayer  ./test2.smil
