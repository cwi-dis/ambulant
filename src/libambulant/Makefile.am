# See which toolkit-dependent libraries we want to make
if WITH_EXPAT
expat_parser_source = lib/expat_parser.cpp

else
expat_parser_source = 

endif

if WITH_XERCES_BUILTIN
xerces_parser_source = lib/xerces_parser.cpp
xerces_includes = $(XERCES_INCLUDES)
else
xerces_parser_source = 
xerces_includes =
endif

if WITH_COCOA
toolkit_cocoa_libraries = libambulant_cocoa.la
libambulant_cocoa_la_SOURCES = \
	gui/cocoa/cocoa_fill.m \
	gui/cocoa/cocoa_gui.m \
	gui/cocoa/cocoa_image.m \
	gui/cocoa/cocoa_text.m \
	gui/cocoa/cocoa_renderer.m \
	gui/cocoa/cocoa_transition.m \
	gui/cocoa/cocoa_audio.m \
	gui/cocoa/cocoa_video.m \
	gui/cocoa/cocoa_dsvideo.m
else
toolkit_cocoa_libraries =
endif

if WITH_QT
toolkit_qt_libraries = libambulant_qt.la
qtincludes = -I$(QT_PREFIX)/include

libambulant_qt_la_SOURCES = 		\
	gui/qt/qt_renderer.cpp		\
	gui/qt/qt_fill.cpp		\
	gui/qt/qt_image_renderer.cpp	\
	gui/qt/qt_text_renderer.cpp 	\
	gui/qt/qt_transition.cpp 	\
	gui/qt/qt_video_renderer.cpp	\
	gui/qt/qt_factory.cpp

else
qtincludes=
toolkit_qt_libraries =
endif

if WITH_ARTS
toolkit_arts_libraries = libambulant_arts.la

libambulant_arts_la_LDFLAGS = `artsc-config --libs`
artsincludes = `artsc-config --cflags`
libambulant_arts_la_SOURCES =             \
	gui/arts/arts.cpp \
	gui/arts/arts_audio.cpp
else
artsincludes=
toolkit_arts_libraries =
endif

if WITH_SDL
toolkit_sdl_libraries = libambulant_sdl.la
libambulant_sdl_la_LDFLAGS =  -static ` sdl-config --libs`
sdlincludes = `sdl-config --cflags`
libambulant_sdl_la_SOURCES =             \
        gui/SDL/sdl.cpp \
        gui/SDL/sdl_audio.cpp
else
sdlincludes=
toolkit_sdl_libraries =
endif

if WITH_FFMPEG
toolkit_ffmpeg_libraries = libambulant_ffmpeg.la
#
# Using the dynamic ffmpeg libraries will fail on RedHat 9, which has
# incompatible ffmpeg libraries installed into /usr/lib, which will be
# preferred over our libraries because of linker flags provided by sdl_config:-(
#
##libambulant_ffmpeg_la_LDFLAGS =  \
##	-L$(FFMPEG_CODEC_LIB)  -lavcodec \
##	-L$(FFMPEG_FORMAT_LIB) -lavformat
libambulant_ffmpeg_la_LDFLAGS =  \
	$(FFMPEG_FORMAT_LIB)/libavformat.a \
	$(FFMPEG_CODEC_LIB)/libavcodec.a

ffmpegincludes = -I$(FFMPEG_CODEC_INCLUDE) -DWITH_FFMPEG \
	-I$(FFMPEG_FORMAT_INCLUDE) -DWITH_FFMPEG_AVFORMAT

libambulant_ffmpeg_la_SOURCES = 	\
		net/ffmpeg_datasource.cpp \
		net/ffmpeg_rawdatasource.cpp
else
ffmpegincludes=
toolkit_ffmpeg_libraries=
endif

INCLUDES = \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	$(EXPAT_INCLUDES) \
	$(xerces_includes) \
	$(qtincludes) \
	$(artsincludes) \
	$(sdlincludes) \
	$(ffmpegincludes) \
	$(INCLTDL)

lib_LTLIBRARIES = \
	libambulant.la \
	$(toolkit_cocoa_libraries) \
	$(toolkit_qt_libraries) \
	$(toolkit_arts_libraries) \
	$(toolkit_sdl_libraries) \
	$(toolkit_ffmpeg_libraries)

# AM_CPPFLAGS=-rdynamic
AM_LDFLAGS=-export-dynamic $(LIBLTDL)

libambulant_la_SOURCES = \
	get_version.cpp \
	\
	$(expat_parser_source) \
	$(xerces_parser_source) \
	\
	lib/colors.cpp \
	lib/delta_timer.cpp \
	lib/document.cpp \
	lib/event_processor.cpp \
	lib/gpaths.cpp \
	lib/logger.cpp \
	lib/nfa.cpp \
	lib/node.cpp \
	lib/parselets.cpp \
	lib/string_util.cpp \
	lib/timer.cpp \
	lib/transition_info.cpp \
	lib/tree_builder.cpp \
	\
	lib/parser_factory.cpp \
	lib/unix/unix_event_processor.cpp \
	lib/unix/unix_mtsync.cpp \
	lib/unix/unix_thread.cpp \
	lib/unix/unix_timer.cpp \
	\
	net/posix_datasource.cpp \
	net/stdio_datasource.cpp \
	net/raw_video_datasource.cpp \
	net/datasource.cpp \
	net/databuffer.cpp \
	net/url.cpp \
	\
	common/preferences.cpp \
	common/region.cpp \
	common/renderer.cpp \
	common/schema.cpp \
	common/smil_alignment.cpp \
	common/smil_handler.cpp \
	common/plugin_engine.cpp \
	\
	smil2/animate_a.cpp \
	smil2/animate_e.cpp \
	smil2/animate_n.cpp \
	smil2/params.cpp \
	smil2/region_node.cpp \
	smil2/smil_layout.cpp \
	smil2/smil_player.cpp \
	smil2/smil_time.cpp \
	smil2/sync_rule.cpp \
	smil2/test_attrs.cpp \
	smil2/time_attrs.cpp \
	smil2/time_calc.cpp \
	smil2/time_node.cpp \
	smil2/time_sched.cpp \
	smil2/time_state.cpp \
	smil2/timegraph.cpp \
	smil2/trace_player.cpp \
	smil2/transition.cpp \
	\
	mms/mms_player.cpp \
	mms/timeline_builder.cpp \
	mms/timelines.cpp \
	\
	gui/none/none_area.cpp \
	gui/none/none_gui.cpp \
	gui/none/none_video_renderer.cpp \
	gui/none/none_factory.cpp

## EXTRA_DIST= \
## 	gui/cocoa/cocoa_fill.m gui/cocoa/cocoa_fill.mm \
## 	gui/cocoa/cocoa_gui.m gui/cocoa/cocoa_gui.mm \
## 	gui/cocoa/cocoa_image.m gui/cocoa/cocoa_image.mm \
## 	gui/cocoa/cocoa_mouse.m gui/cocoa/cocoa_mouse.mm \
## 	gui/cocoa/cocoa_text.m gui/cocoa/cocoa_text.mm \
## 	gui/cocoa/cocoa_audio.m gui/cocoa/cocoa_audio.mm \
## 	gui/cocoa/cocoa_audio.m gui/cocoa/cocoa_video.mm
EXTRA_DIST= \
	libambulant_win32.dsp \
	libambulant_win32.vcproj \
	libambulant_win32-noxerces.vcproj \
	libambulant_wince.vcp \
	gui/arts \
	gui/cocoa \
	gui/dg \
	gui/dx \
	gui/qt \
	lib/win32
