# The default (no-op) plugin. Always build

dummy_plugin = libamplugin_dummy.la
libamplugin_dummy_la_SOURCES = plugin.cpp
libamplugin_dummy_la_LDFLAGS = -module

# The arts plugin
if WITH_ARTS
arts_plugin = libamplugin_arts.la
arts_includes = `artsc-config --cflags`
libamplugin_arts_la_LDFLAGS = -module `artsc-config --libs`
libamplugin_arts_la_SOURCES = artsplugin.cpp
else
arts_plugin=
arts_includes=

endif

# Now tie it all together.
# XXXX Need to set install directory

noinst_LTLIBRARIES = $(dummy_plugin) $(arts_plugin)
AM_CPPFLAGS=-shared
#LDADD=$(top_builddir)/src/libambulant/.libs/libambulant.a
INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include \
	 $(arts_includes)

