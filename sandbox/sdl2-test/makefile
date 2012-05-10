

#all: tutorial01 tutorial02 tutorial02 tutorial03 tutorial04 tutorial05 tutorial06 tutorial07 tutorial08 tutorial02-SDL2
all: tutorial02 tutorial02-SDL2

tutorial01: tutorial01.c
	cc -g -O0 -I. -I/usr/local/include/libavcodec -I/usr/local/include/libavformat -I/usr/local/include/libavutil -I/usr/local/libswscale -I/usr/include/SDL  -DCODEC_TYPE_VIDEO=AVMEDIA_TYPE_VIDEO tutorial01.c -Davcodec_decode_video=avcodec_decode_video2 -lavformat -lavcodec -lavutil -lswscale -lSDL -o tutorial01

tutorial02: tutorial02.c
	cc -g -O0 -I. -I/usr/local/include/libavcodec -I/usr/local/include/libavformat -I/usr/local/include/libavutil -I/usr/local/include/libswscale -I/usr/include/SDL  -DCODEC_TYPE_VIDEO=AVMEDIA_TYPE_VIDEO tutorial02.c -Davcodec_decode_video=avcodec_decode_video2 -lavformat -lavcodec -lavutil -lswscale -lSDL -o tutorial02

tutorial03: tutorial03.c
	cc -g -O0 -I. -I/usr/local/include/libavcodec -I/usr/local/include/libavformat -I/usr/local/include/libavutil -I/usr/local/include/libswscale -I/usr/include/SDL  -DCODEC_TYPE_VIDEO=AVMEDIA_TYPE_VIDEO tutorial03.c -Davcodec_decode_video=avcodec_decode_video2 -lavformat -lavcodec -lavutil -lswscale -lSDL -o tutorial03

tutorial04: tutorial04.c
	cc -g -O0 -I. -I/usr/local/include/libavcodec -I/usr/local/include/libavformat -I/usr/local/include/libavutil -I/usr/local/include/libswscale -I/usr/include/SDL  -DCODEC_TYPE_VIDEO=AVMEDIA_TYPE_VIDEO tutorial04.c -Davcodec_decode_video=avcodec_decode_video2 -lavformat -lavcodec -lavutil -lswscale -lSDL -o tutorial04

tutorial05: tutorial05.c
	cc -g -O0 -I. -I/usr/local/include/libavcodec -I/usr/local/include/libavformat -I/usr/local/include/libavutil -I/usr/local/include/libswscale -I/usr/include/SDL  -DCODEC_TYPE_VIDEO=AVMEDIA_TYPE_VIDEO tutorial05.c -Davcodec_decode_video=avcodec_decode_video2 -lavformat -lavcodec -lavutil -lswscale -lSDL -o tutorial05

tutorial06: tutorial06.c
	cc -g -O0 -I. -I/usr/local/include/libavcodec -I/usr/local/include/libavformat -I/usr/local/include/libavutil -I/usr/local/include/libswscale -I/usr/include/SDL  -DCODEC_TYPE_VIDEO=AVMEDIA_TYPE_VIDEO tutorial06.c -Davcodec_decode_video=avcodec_decode_video2 -lavformat -lavcodec -lavutil -lswscale -lSDL -o tutorial06

tutorial07: tutorial07.c
	cc -g -O0 -I. -I/usr/local/include/libavcodec -I/usr/local/include/libavformat -I/usr/local/include/libavutil -I/usr/local/include/libswscale -I/usr/include/SDL  -DCODEC_TYPE_VIDEO=AVMEDIA_TYPE_VIDEO tutorial07.c -Davcodec_decode_video=avcodec_decode_video2 -lavformat -lavcodec -lavutil -lswscale -lSDL -o tutorial07

tutorial08: tutorial08.c
	cc -g -O0 -I. -I/usr/local/include/libavcodec -I/usr/local/include/libavformat -I/usr/local/include/libavutil -I/usr/local/include/libswscale -I/usr/include/SDL  -DCODEC_TYPE_VIDEO=AVMEDIA_TYPE_VIDEO tutorial08.c -Davcodec_decode_video=avcodec_decode_video2 -lavformat -lavcodec -lavutil -lswscale -lSDL -o tutorial08

tutorial02-SDL2: tutorial02-SDL2.c
	cc -g -O0 -I. -I/usr/local/include/libavcodec -I/usr/local/include/libavformat -I/usr/local/include/libavutil -I/usr/local/include/libswscale -I /usr/local/include/SDL2/ -DCODEC_TYPE_VIDEO=AVMEDIA_TYPE_VIDEO tutorial02-SDL2.c -Davcodec_decode_video=avcodec_decode_video2 -L/usr/local/lib -lavformat -lavcodec -lavutil -lswscale -lSDL2 -o tutorial02-SDL2

clean:
	rm -f  tutorial01 tutorial02 tutorial02 tutorial03 tutorial04 tutorial05 tutorial06 tutorial07 tutorial08 tutorial02-SDL2
