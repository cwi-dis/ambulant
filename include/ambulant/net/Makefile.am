library_includedir=$(includedir)/ambulant/net

if WITH_FFMPEG
FFMPEG_FILES=ffmpeg_common.h ffmpeg_audio.h ffmpeg_video.h ffmpeg_factory.h ffmpeg_raw.h
else
FFMPEG_FILES=
endif	

library_include_HEADERS= \
	connection.h \
	datasource.h \
	databuffer.h \
	demux_datasource.h \
	posix_datasource.h \
	socket.h \
	stdio_datasource.h \
	url.h \
	raw_video_datasource.h \
	rtsp_datasource.h \
	rtsp_factory.h \
	$(FFMPEG_FILES)
