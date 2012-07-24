# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstambulantsrc.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgstambulantsrc.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
libgstambulantsrc_la_SOURCES = gstambulantsrc.c gstambulantsrc.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstambulantsrc_la_CFLAGS = $(GST_CFLAGS)
libgstambulantsrc_la_LIBADD = $(GST_LIBS)
libgstambulantsrc_la_LDFLAGS = $(GST_AMBULANTSRC_LDFLAGS)
libgstambulantsrc_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstambulantsrc.h
