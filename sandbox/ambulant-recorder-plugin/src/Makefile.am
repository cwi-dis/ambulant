AM_CPPFLAGS = \
			-I$(top_srcdir)/include $(AMBULANT_CFLAGS) $(SDL_CFLAGS)

LIBS = \
			$(AMBULANT_LIBS) $(SDL_LIBS)

libdir = \
		       $(prefix)/lib/ambulant

if ! WITH_PYTHON
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
	rm -fr $(libdir)/pyamplugin_recorder.py*

else

install-data-local:
	echo Installing in: $(libdir)
	rm -fr $(libdir)/libamplugin_recorder.*
	rm -fr $(libdir)/pyamplugin_recorder.py*
	$(INSTALL) $(srcdir)/pyamplugin_recorder.py $(libdir)
endif

uninstall:
	rm -fr $(libdir)/libamplugin_recorder.*
	rm -fr $(libdir)/pyamplugin_recorder.py*

