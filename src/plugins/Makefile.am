lib_LTLIBRARIES = libplugin.la
plugindir = $(top_builddir)/src/plugins
libplugin_la_SOURCES = plugin.cpp

lib_LTLIBRARIES = libarts.la
artsplugin_la_LDFLAGS = `artsc-config --libs`
artsincludes = `artsc-config --cflags`
libarts_la_SOURCES = artsplugin.cpp

AM_CPPFLAGS=-shared
LDADD=$(top_builddir)/src/libambulant/.libs/libambulant.a
INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include \
	 $(artsincludes)



