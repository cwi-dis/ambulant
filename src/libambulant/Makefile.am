CXXFLAGS=-g -O0 -fno-inline $(PTHREAD_CFLAGS)
expat_dir = $(top_builddir)/third_party_packages/expat-unix
INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include -I$(expat_dir)/include

# See which toolkit-dependent libraries we want to make
if WITH_COCOA
toolkit_cocoa_libraries = libambulant_cocoa.la
libambulant_cocoa_la_SOURCES = gui/cocoa/cocoa_gui.m
else
toolkit_cocoa_libraries =
endif
# if WITH_QT
# toolkit_qr_libraries = libambulant_qt.la
# libambulant_qt_la_SOURCES = gui/qt/qt_gui.cpp
# else
# toolkit_qt_libraries =
# endif

lib_LTLIBRARIES = libambulant.la $(toolkit_cocoa_libraries)

libambulant_la_SOURCES = \
	get_version.cpp \
	\
	lib/colors.cpp \
	lib/delta_timer.cpp \
	lib/expat_parser.cpp \
	lib/logger.cpp \
	lib/node.cpp \
	lib/player.cpp \
	lib/region.cpp \
	lib/renderer.cpp \
	lib/skeleton.cpp \
	lib/smil_handler.cpp \
	lib/timeline_builder.cpp \
	lib/timelines.cpp \
	lib/tree_builder.cpp \
	\
	lib/unix/unix_event_processor.cpp \
	lib/unix/unix_mtsync.cpp \
	lib/unix/unix_thread.cpp \
	lib/unix/unix_timer.cpp \
	\
	net/datasource.cpp \
	\
	gui/none/none_gui.cpp
