EXTRA_DIST=plugin.cpp artsplugin.cpp xercesplugin.cpp plugin.vcproj
# The default (no-op) plugin. only build when asked to buid

if WITH_DUMMY_PLUGIN
dummy_plugin = libamplugin_dummy.la
libamplugin_dummy_la_SOURCES = plugin.cpp
libamplugin_dummy_la_LDFLAGS = -module

rot13_plugin = libamplugin_rot13.la
libamplugin_rot13_la_SOURCES = rot13plugin.cpp
libamplugin_rot13_la_LDFLAGS = -module
else
dummy_plugin=
rot13_plugin =
endif

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
if WITH_XERCES_PLUGIN
xerces_plugin = libamplugin_xerces.la
xerces_includes = $(XERCES_INCLUDES)
libamplugin_xerces_la_LDFLAGS = -module $(XERCES_LIBS)
libamplugin_xerces_la_SOURCES = xercesplugin.cpp \
		$(top_builddir)/src/libambulant/lib/xerces_parser.cpp

else
xerces_plugin=
xerces_include=
endif

# The Python plugin
if WITH_PYTHON_PLUGIN
# XXX These should be elsewhere
pymodule = $(top_srcdir)/src/pyambulant
pymodule_includes = -I$(pymodule)
pymodule_tmp = $(pymodule)/build/temp.*
pymodule_libs = \
    $(pymodule_tmp)/ambulantmodule.o \
    $(pymodule_tmp)/ambulantinterface.o \
    $(pymodule_tmp)/ambulantutilities.o

python_plugin = libamplugin_python.la
python_includes = $(PYTHON_INCLUDES) $(pymodule_includes)
libamplugin_python_la_LDFLAGS = -module $(PYTHON_LIBS) $(pymodule_libs)
libamplugin_python_la_SOURCES = pythonplugin.cpp

else
python_plugin=
python_include=
endif

# Now tie it all together.
# XXXX Need to set install directory

pkglib_LTLIBRARIES = $(dummy_plugin) $(rot13_plugin) $(arts_plugin) $(xerces_plugin) $(python_plugin)

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include \
	 $(arts_includes) $(xerces_includes) $(python_includes)

