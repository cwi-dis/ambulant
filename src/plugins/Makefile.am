EXTRA_DIST=plugin.cpp xercesplugin.cpp
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

# The dummy SMIL 3.0 state plugin
if WITH_SMIL30
dummy_state_plugin = libamplugin_dummy_state.la
libamplugin_dummy_state_la_SOURCES = dummy_stateplugin.cpp
libamplugin_dummy_state_la_LDFLAGS = -module
else
dummy_state_plugin=
endif

# The libxml2 XPath SMIL 3.0 state plugin
if WITH_SMIL30
xpath_state_plugin = libamplugin_xpath_state.la
xpath_state_includes = `xml2-config --cflags`
libamplugin_xpath_state_la_SOURCES = xpath_stateplugin.cpp
libamplugin_xpath_state_la_LDFLAGS = -module `xml2-config --libs`
else
xpath_state_plugin=
xpath_state_includes=
endif

# The wekkit DOM implementation
if WITH_WEBKIT_DOM
wkdom_plugin = libamplugin_wkdom.la
wkdom_includes = 
libamplugin_wkdom_la_SOURCES = wkdomplugin.cpp wkdombridge.m
libamplugin_wkdom_la_LDFLAGS = -module
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
libamplugin_python_la_LDFLAGS = -module $(PYTHON_LIBS) $(pymodule_libs)
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

