noinst_LIBRARIES = libambulant.a
expat_dir = $(top_builddir)/third_party_packages/expat-unix
INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include -I$(expat_dir)/include

libambulant_a_SOURCES = \
	get_version.cpp \
	\
	lib/colors.cpp \
	lib/delta_timer.cpp \
	lib/expat_parser.cpp \
	lib/logger.cpp \
	lib/node.cpp \
	lib/player.cpp \
	lib/region.cpp \
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
	net/datasource.cpp
