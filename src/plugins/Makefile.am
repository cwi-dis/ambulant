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

# The XERCES plugin
if WITH_XERCES
xerces_plugin = libamplugin_xerces.la
xerces_includes = -I$(XERCES_PREFIX)/src/
libamplugin_xerces_la_LDFLAGS = -module -L$(XERCES_PREFIX)/lib/ -lxerces-c 
libamplugin_xerces_la_SOURCES = xercesplugin.cpp \
		$(top_builddir)/src/libambulant/lib/xerces_parser.cpp

else
xerces_plugin=
xerces_include=
endif

# Now tie it all together.
# XXXX Need to set install directory

pkglib_LTLIBRARIES = $(dummy_plugin) $(arts_plugin) $(xerces_plugin)
AM_CPPFLAGS=-DWITH_XERCES
#LDADD=$(top_builddir)/src/libambulant/.libs/libambulant.a
INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include \
	 $(arts_includes) $(xerces_includes)

