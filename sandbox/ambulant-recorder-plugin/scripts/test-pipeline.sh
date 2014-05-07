set -x
if [ "$GST_RTSP_EXAMPLES" == "" ]
then
	echo GST_RTSP_EXAMPLES not set. It should point to \'gstreamer/gst-rtsp/examples\' after building.
	exit -1
fi
if [ ! -e /dev/shm/pipe ] ; then mkfifo /dev/shm/pipe; fi
killall -9 lt-test-launch 2>>/dev/null # hangs sometimes, blocking port 8554
killall -9 vlc cvlc mplayer AmbulantPlayer_sdl 2>>/dev/null
sleep 1
cvlc v4l2:///dev/video0 --sout "#transcode{vcodec=h264,venc=x264{preset=veryfast,tune=zerolatency,no-cabac,bframes=0,fps=30,vbv-maxrate=5000,vbv-bufsize=200,slice-maxsize=1500},vb=0,scale=0,acodec=none}:rtp{sdp=rtsp://:5544/webcam},rtp-sap=no,standard-sap=no,ttl=1,keep" &
# cvlc v4l2:///dev/video1 :sout="#transcode{vcodec=h264,vb=0,scale=0,acodec=none}:rtp{sdp=rtsp://:5544/webcam}" :no-sout-rtp-sap :no-sout-standard-sap :ttl=1 :sout-keep &
sleep 1
# cvlc rtsp://localhost:5544/webcam &
AMBULANT_RECORDER_PIPE="$GST_RTSP_EXAMPLES/test-launch --gst-debug-level=0 '(ambulantsrc silent=true max-latency=500000  min-latency=500000 ! video/x-raw-rgb,width=640,height=480,bpp=24,depth=24,framerate=25/1,endianness=4321,pixel-aspect-ratio=1/1,red_mask=16711680,green_mask=65280,blue_mask=255 ! ffmpegcolorspace ! x264enc tune=zerolatency  cabac=false ! rtph264pay name=pay0 pt=96 )' " AmbulantPlayer_sdl  ./test2.smil &
sleep 2
# mplayer rtsp://localhost:5544/webcam -fps 30 -nosound &
#cvlc rtsp://localhost:8554/test &
mplayer rtsp://localhost:8554/test -fps 30 -nosound &
#AmbulantPlayer test3.smil  &


