# See which toolkit-dependent libraries we want to make
if WITH_COCOA
toolkit_cocoa_libraries = libambulant_cocoa.la
libambulant_cocoa_la_SOURCES = \
	gui/cocoa/cocoa_fill.m \
	gui/cocoa/cocoa_gui.m \
	gui/cocoa/cocoa_image.m \
	gui/cocoa/cocoa_text.m \
	gui/cocoa/cocoa_audio.m
else
toolkit_cocoa_libraries =
endif
if WITH_QT
toolkit_qt_libraries = libambulant_qt.la
qtincludes = -I$(QT_PREFIX)/include

libambulant_qt_la_SOURCES = 		\
	gui/qt/qt_renderer.cpp		\
	gui/qt/qt_image_renderer.cpp	\
	gui/qt/qt_text_renderer.cpp 

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
libambulant_sdl_la_LDFLAGS =  ` sdl-config --libs` -lSDL_mixer
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
libambulant_ffmpeg_la_LDFLAGS =  -L/usr/lib -lavcodec
ffmpegincludes = -I/usr/include/ffmpeg

libambulant_ffmpeg_la_SOURCES = 	\
		net/ffmpeg_datasource.cpp
else
ffmpegincludes=
toolkit_ffmpeg_libraries=
endif

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include \
	-I$(EXPAT_PREFIX)/include $(qtincludes) $(artsincludes)\
	  $(sdlincludes) $(ffmpegincludes)

lib_LTLIBRARIES = libambulant.la $(toolkit_cocoa_libraries)\
$(toolkit_qt_libraries) $(toolkit_arts_libraries) $(toolkit_sdl_libraries) $(toolkit_ffmpeg_libraries)


libambulant_la_SOURCES = \
	get_version.cpp \
	\
	lib/colors.cpp \
	lib/delta_timer.cpp \
	lib/expat_parser.cpp \
	lib/logger.cpp \
	lib/nfa.cpp \
	lib/node.cpp \
	lib/parselets.cpp \
	lib/schema.cpp \
	lib/string_util.cpp \
	lib/smil_time.cpp \
	lib/sync_rule.cpp \
	lib/time_attrs.cpp \
	lib/time_node.cpp \
	lib/time_state.cpp \
	lib/timegraph.cpp \
	lib/timer.cpp \
	lib/trace_player.cpp \
	lib/tree_builder.cpp \
	\
	lib/unix/unix_event_processor.cpp \
	lib/unix/unix_mtsync.cpp \
	lib/unix/unix_thread.cpp \
	lib/unix/unix_timer.cpp \
	\
	net/datasource.cpp \
	net/url.cpp \
	\
	common/mms_player.cpp \
	common/region.cpp \
	common/region_node.cpp \
	common/renderer.cpp \
	common/skeleton.cpp \
	common/smil_handler.cpp \
	common/smil_layout.cpp \
	common/smil_player.cpp \
	common/timeline_builder.cpp \
	common/timelines.cpp \
	\
	gui/none/none_gui.cpp

EXTRA_DIST= \
	gui/cocoa/cocoa_gui.m gui/cocoa/cocoa_gui.mm \
	gui/cocoa/cocoa_image.m gui/cocoa/cocoa_image.mm \
	gui/cocoa/cocoa_text.m gui/cocoa/cocoa_text.mm \
	gui/cocoa/cocoa_audio.m gui/cocoa/cocoa_audio.mm
