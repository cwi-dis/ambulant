#
# Conditionals for various third party libraries
#

ambulantlibdir=$(top_builddir)/src/libambulant/.libs
if WITH_AMBULANT_SHARED_LIB
# If we link with shared libs: use these. Needed for plugin support.
libambulant_core=-L$(ambulantlibdir) -lambulant
libambulant_ffmpeg=-L$(ambulantlibdir) -lambulant_ffmpeg
libambulant_sdl=-L$(ambulantlibdir) -lambulant_sdl
libambulant_gstreamer=-L$(ambulantlibdir) -lambulant_gstreamer
libambulant_gtk=-L$(ambulantlibdir) -lambulant_gtk
else
# Otherwise we link the ambulant libraries statically.
libambulant_core=$(ambulantlibdir)/libambulant.a
libambulant_ffmpeg=$(ambulantlibdir)/libambulant_ffmpeg.a
libambulant_sdl=$(ambulantlibdir)/libambulant_sdl.a
libambulant_gstreamer=$(ambulantlibdir)/libambulant_gstreamer.a
libambulant_gtk=$(ambulantlibdir)/libambulant_gtk.a
endif

# This target is magic for automake
check-am: check-ambulantplayer_gtk

if WITH_EXPAT
extra_expat_DEFINES=$(EXPAT_CFLAGS) -DWITH_EXPAT
extra_expat_LDADD=$(EXPAT_LIBS)
else
extra_expat_DEFINES=
extra_expat_LDADD=
endif

if WITH_FFMPEG
extra_ffmpeg_DEFINES=-DWITH_FFMPEG $(FFMPEG_CFLAGS)
extra_ffmpeg_LDADD=$(libambulant_ffmpeg) $(FFMPEG_LIBS)
else
extra_ffmpeg_DEFINES=
extra_ffmpeg_LDADD=
endif

if WITH_GSTREAMER
extra_gstreamer_LDADD=$(libambulant_gstreamer) $(GSTREAMER_LIBS)
extra_gstreamer_DEFINES=-DWITH_GSTREAMER $(GSTREAMER_CFLAGS)
else
extra_gstreamer_LDADD=
extra_gstreamer_DEFINES=
endif

if WITH_SDL
extra_sdl_DEFINES=$(SDL_CFLAGS) -DWITH_SDL
extra_sdl_LDADD=$(libambulant_sdl) $(SDL_LIBS)
else
extra_sdl_DEFINES=
extra_sdl_LDADD=
endif

if WITH_XERCES_BUILTIN
extra_xerces_DEFINES=$(XERCES_CFLAGS) -DWITH_XERCES_BUILTIN
extra_xerces_LDADD=$(XERCES_LIBS)
else
extra_xerces_DEFINES=
extra_xerces_LDADD=
endif
#if USE_NLS
extra_nls_DEFINES=-DLOCALEDIR=\"$(datadir)/locale\"
#else
#extra_nls_DEFINES=
#endif


#------------------------------------------------------
if WITH_GTK
if WITH_GCD_EVENT_PROCESSOR
gtk_libs=$(GTK_LIBS) $(GCD_LIBS)
else
gtk_libs=$(GTK_LIBS)
endif #WITH_GCD_EVENT_PROCESSOR
gtk_includes=$(GTK_CFLAGS)
else
gtk_libs=
gtk_includes=
endif # WITH_GTK

#------------------------------------------------------

bin_PROGRAMS = AmbulantPlayer_gtk
AmbulantPlayer_gtk_SOURCES = \
	gtk_gui.cpp \
	gtk_gui.h \
	gtk_logger.cpp \
	gtk_logger.h \
	gtk_mainloop.cpp \
	gtk_mainloop.h \
	gtk_settings.cpp \
	gtk_settings.h \
	unix_preferences.cpp \
	unix_preferences.h

AmbulantPlayer_gtk_LDADD = $(libambulant_gtk) \
	$(extra_expat_LDADD) \
	$(extra_ffmpeg_LDADD) \
	$(extra_gstreamer_LDADD) \
	$(extra_sdl_LDADD) \
	$(extra_xerces_LDADD) \
	$(libambulant_core) \
	$(gtk_libs) \
	$(LIBLTDL) \
	$(PTHREAD_LIBS)

AM_LDFLAGS= \
	   -export-dynamic \
	   -R $(libdir)

AM_CPPFLAGS = \
		$(extra_gstreamer_CPPFLAGS) \
		-DAMBULANT_DATADIR=\"$(pkgdatadir)\" \
		$(extra_expat_DEFINES) \
		$(extra_ffmpeg_DEFINES) \
		$(extra_gstreamer_DEFINES) \
		$(extra_nls_DEFINES) \
		$(extra_sdl_DEFINES) \
		$(extra_xerces_DEFINES) \
		$(gtk_includes) \
		-I$(top_builddir)/include \
		-I$(top_srcdir)/include \
		-I$(top_builddir)/include/ambulant/gui/gtk \
		-I$(top_srcdir)/include/ambulant/gui/gtk

AM_CXXFLAGS=-fPIC

install-exec-hook: install-data-local
	cd $(DESTDIR)$(bindir) && \
	$(LN_S) -f $(bin_PROGRAMS) AmbulantPlayer
	if test x`which gconftool-2` != x ; then GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source` gconftool-2 --makefile-install-rule $(srcdir)/ambulant.schemas; fi
##	gconftool-2 -s /desktop/gnome/url-handlers/ambulant/enabled --type Boolean true
###	gconftool-2 -s /desktop/gnome/url-handlers/ambulant/command "$(DESTDIR)$(bindir)/AmbulantPlayer  %s" --type String

check-ambulantplayer_gtk:
	if test x$$DISPLAY = x; then \
		echo Skipping tests, DISPLAY not set; \
	else \
		set -e; \
		set +x; \
		pyv=`$(PYTHON) -V 2>&1 | sed -e 's/Python \([1-9].[1-9]\).*/\1/'`; \
		pyp=`$(PYTHON) -c 'import sys; print sys.prefix'`; \
		lib=/lib; \
		if test `uname -p` = "x86_64"; then lib=/lib64; fi;\
		export AMBULANT_PLUGIN_PATH=$(top_srcdir)/src/pyambulant/test; \
		export PYTHONPATH=:$(top_srcdir)/src/pyambulant/build/lib.*; \
		export LD_PRELOAD=$$pyp/$$lib/python$$pyv/config/libpython$$pyv.so; \
		./AmbulantPlayer_gtk $(top_srcdir)/Extras/Welcome/Welcome.smil; \
		if test -e AM_TEST-output.txt; then cat AM_TEST-output.txt; fi \
	fi

uninstall-local:
#XXXX quick fix to make distcheck work: remove make check results
	-rm -fr AM_TEST-output.txt 
	-rm -f $(DESTDIR)$(datadir)/applications/ambulant-gtk.desktop
	-rm -f $(DESTDIR)$(datadir)/ambulant/ambulant-gtk_gui.xml

install-data-local:
	-cp $(srcdir)/ambulant-gtk_gui.xml $(DESTDIR)$(datadir)/ambulant/
	mkdir -p  $(DESTDIR)$(datadir)/applications
	-cp $(srcdir)/ambulant-gtk.desktop $(DESTDIR)$(datadir)/applications

EXTRA_DIST=ambulant-gtk.desktop ambulant.schemas ambulant-gtk_gui.xml
