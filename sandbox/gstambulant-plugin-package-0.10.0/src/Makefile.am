# Note: plugindir is set in configure
plugin_LTLIBRARIES = libgstambulantsrc.la

# sources used to compile this plug-in
libgstambulantsrc_la_SOURCES = gstambulantsrc.c gstambulantsrc.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstambulantsrc_la_CFLAGS = $(GST_CFLAGS)
libgstambulantsrc_la_LIBADD = $(GST_LIBS)
libgstambulantsrc_la_LDFLAGS = $(GST_AMBULANTSRC_LDFLAGS)
libgstambulantsrc_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstambulantsrc.h
