noinst_LIBRARIES = libambulant.a
expat_dir = $(top_builddir)/third_party_packages/expat-unix
INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include -I$(expat_dir)/include

libambulant_a_SOURCES = \
	get_version.cpp \
	\
	unix/unix_logger.cpp \
	\
	lib/timelines.cpp \
	lib/region.cpp \
	lib/player.cpp \
	lib/skeleton.cpp \
	lib/timeline_builder.cpp \
	\
	lib/unix/unix_event_processor.cpp \
	lib/unix/unix_mtsync.cpp \
	lib/unix/unix_thread.cpp \
	lib/unix/unix_timer.cpp \
	\
	net/datasource.cpp
