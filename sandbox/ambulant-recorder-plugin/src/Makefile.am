AM_CPPFLAGS = \
			-I$(top_srcdir)/include $(AMBULANT_CFLAGS) $(SDL_CFLAGS)

LIBS = \
			$(AMBULANT_LIBS) $(SDL_LIBS)

libdir = \
		       $(prefix)/lib/ambulant

lib_LTLIBRARIES = \
		       libamplugin_recorder.la


libamplugin_recorder_la_SOURCES = \
			recorder_plugin.cpp 

libamplugin_recorder_la_LIBADD =			\
			-lpthread			\
			-lambulant

libamplugin_recorder_la_LIBFLAGS = \
			-no-undefined

MAINTAINERCLEANFILES = \
			Makefile.in

EXTRA_DIST = \
			pyamplugin_recorder.py


install-data-local:
	echo Installing in: $(libdir)
	$(INSTALL) $(srcdir)/pyamplugin_recorder.py $(libdir)

