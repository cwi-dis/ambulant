CXXFLAGS=-g -O0 -fno-inline $(PTHREAD_CFLAGS)
expat_dir = $(EXPAT_PREFIX)

# See which toolkit-dependent libraries we want to make
if WITH_COCOA
toolkit_cocoa_libraries = libambulant_cocoa.la
libambulant_cocoa_la_SOURCES = \
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

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include \
	-I$(expat_dir)/include $(qtincludes) $(artsincludes)

lib_LTLIBRARIES = libambulant.la $(toolkit_cocoa_libraries) $(toolkit_qt_libraries) $(toolkit_arts_libraries)

libambulant_la_SOURCES = \
	get_version.cpp \
	\
	lib/colors.cpp \
	lib/delta_timer.cpp \
	lib/expat_parser.cpp \
	lib/logger.cpp \
	lib/node.cpp \
	lib/timer.cpp \
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
	common/player.cpp \
	common/region.cpp \
	common/renderer.cpp \
	common/skeleton.cpp \
	common/smil_handler.cpp \
	common/timeline_builder.cpp \
	common/timelines.cpp \
	\
	gui/none/none_gui.cpp

EXTRA_DIST= \
	gui/cocoa/cocoa_gui.m gui/cocoa/cocoa_gui.mm \
	gui/cocoa/cocoa_image.m gui/cocoa/cocoa_image.mm \
	gui/cocoa/cocoa_text.m gui/cocoa/cocoa_text.mm \
	gui/cocoa/cocoa_audio.m gui/cocoa/cocoa_audio.mm
