#/bin/sh
# start a RTSP server to stream a webcam
usage()
{
# echo "Usage: $0 [display] [name=<stream_name> ($NAME)] [port=port_number ($PORT)] [webcam=<webcam_device> ($WEBCAM)]";
 echo "Usage: $0 [name=<stream_name> ($NAME)] [port=port_number ($PORT)] [webcam=<webcam_device> ($WEBCAM)]";
 echo "Serves webcam device $WEBCAM as RTSP stream at: \"rtsp:localhost:$PORT/$NAME\"" 
}
# set -x
DEBUG=""
DISPLAY="none"
NAME="webcam"
PORT=5544
WEBCAM="/dev/video0"
for i in $@
do
	echo $i
	case $i in
	#	display)  DISPLAY="display" ;; does not wokr
		debug)    DEBUG="-v"	 ;; 
		name=*)	  NAME=${i##*\=} ;;
		port=*)   PORT=${i##*\=} ;;
		webcam=*) WEBCAM=${i##*\=} ;;
		*)		  usage; exit -1 ;;
	esac
done
cvlc $DEBUG v4l2://$WEBCAM :sout=#transcode\{vcodec=h264,vb=0,scale=0,acodec=none\}:duplicate\{dst=rtp\{sdp=rtsp://:$PORT/$NAME\},dst=$DISPLAY\} :no-sout-rtp-sap :no-sout-standard-sap :ttl=1 :sout-keep
