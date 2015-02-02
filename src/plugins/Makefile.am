EXTRA_DIST=plugin.cpp plugin_ffmpeg.cpp pyamplugin_state pythonplugin.cpp \
    wkdombridge.h wkdombridge.m wkdombridge.mm wkdomplugin.cpp \
    xercesplugin.cpp xpath_stateplugin.cpp plugin_timesync.cpp
    
ambulantlibdir=$(top_builddir)/src/libambulant/.libs
if WITH_AMBULANT_SHARED_LIB
# If we link with shared libs: use these. Needed for plugin support.
libambulant_core=-L$(ambulantlibdir) -lambulant
else
# Otherwise we link the ambulant libraries statically.
libambulant_core=
endif

# The default (no-op) plugin. only build when asked to buid

if WITH_DUMMY_PLUGIN
dummy_plugin = libamplugin_dummy.la
libamplugin_dummy_la_SOURCES = plugin.cpp
libamplugin_dummy_la_LDFLAGS = -module
libamplugin_dummy_la_LIBADD = $(libambulant_core)

rot13_plugin = libamplugin_rot13.la
libamplugin_rot13_la_SOURCES = rot13plugin.cpp
libamplugin_rot13_la_LDFLAGS = -module
else
dummy_plugin=
rot13_plugin =
endif

# The XERCES plugin
if WITH_XERCES_PLUGIN
xerces_plugin = libamplugin_xerces.la
xerces_includes = $(XERCES_CFLAGS)
libamplugin_xerces_la_LDFLAGS = -module $(XERCES_LIBS)
libamplugin_xerces_la_LIBADD = $(libambulant_core)
libamplugin_xerces_la_SOURCES = xercesplugin.cpp xerces_parser.cpp
else
xerces_plugin=
xerces_include=
endif

# The libxml2 XPath SMIL 3.0 state plugin
xpath_state_plugin = libamplugin_xpath_state.la
xpath_state_includes = $(LIBXML2_CFLAGS)
libamplugin_xpath_state_la_SOURCES = xpath_stateplugin.cpp
libamplugin_xpath_state_la_LDFLAGS = -module $(LIBXML2_LIBS)
libamplugin_xpath_state_la_LIBADD = $(libambulant_core)

# The wekkit DOM implementation
if WITH_WEBKIT_DOM
wkdom_plugin = libamplugin_wkdom.la
wkdom_includes = 
libamplugin_wkdom_la_SOURCES = wkdomplugin.cpp wkdombridge.m
libamplugin_wkdom_la_LDFLAGS = -module
libamplugin_wkdom_la_LIBADD = $(libambulant_core)
else
wkdom_plugin=
wkdom_includes=
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
libamplugin_python_la_LDFLAGS = -module $(PYTHON_LIBS) $(pymodule_libs) $(LIBINTL)
libamplugin_python_la_LIBADD = $(libambulant_core)
libamplugin_python_la_SOURCES = pythonplugin.cpp

else
python_plugin=
python_include=
endif

# Now tie it all together.
# XXXX Need to set install directory

pkglib_LTLIBRARIES = \
    $(dummy_plugin) \
    $(rot13_plugin) \
    $(xerces_plugin) \
    $(dummy_state_plugin) \
    $(xpath_state_plugin) \
    $(wkdom_plugin) \
    $(python_plugin)

AM_CPPFLAGS = -I$(top_builddir)/include -I$(top_srcdir)/include \
	 $(xerces_includes) $(python_includes) \
	 $(xpath_state_includes)

AM_LDFLAGS = \
	   -export-dynamic \
	   -R $(libdir)

