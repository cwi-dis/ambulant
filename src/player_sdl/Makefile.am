#
# Conditionals for various third party libraries
#

ambulantlibdir=$(top_builddir)/src/libambulant/.libs
if WITH_AMBULANT_SHARED_LIB
# If we link with shared libs: use these. Needed for plugin support.
libambulant_core=-L$(ambulantlibdir) -lambulant
libambulant_ffmpeg=-L$(ambulantlibdir) -lambulant_ffmpeg
libambulant_sdl=-L$(ambulantlibdir) -lambulant_sdl
else
# Otherwise we link the ambulant libraries statically.
libambulant_core=$(ambulantlibdir)/libambulant.a
libambulant_ffmpeg=$(ambulantlibdir)/libambulant_ffmpeg.a
libambulant_sdl=$(ambulantlibdir)/libambulant_sdl.a
endif

# This target is magic for automake
check-am: check-ambulantplayer_sdl

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

if WITH_SDL2
extra_sdl_DEFINES=$(SDL_CFLAGS) -DWITH_SDL2
extra_sdl_LDADD=$(libambulant_sdl) $(FFMPEG_LIBS) $(SDL_LIBS)
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
if WITH_SDL
if WITH_GCD_EVENT_PROCESSOR
sdl_libs=$(SDL_LIBS) $(GCD_LIBS)
else
sdl_libs=$(SDL_LIBS)
endif #WITH_GCD_EVENT_PROCESSOR
sdl_includes=$(SDL_CFLAGS)
else
sdl_libs=
sdl_includes=
endif # WITH_SDL

#------------------------------------------------------

bin_PROGRAMS = AmbulantPlayer_sdl
AmbulantPlayer_sdl_SOURCES = \
	sdl_gui_player.cpp \
	sdl_gui.cpp \
	sdl_gui_player.h \
	sdl_gui.h \
	sdl_logger.cpp \
	sdl_logger.h \
	unix_preferences.cpp \
	unix_preferences.h

##X	sdl_mainloop.cpp \
##X	sdl_mainloop.h \
##X	sdl_settings.cpp \
##X	sdl_settings.h

LDADD = $(libambulant_sdl) \
	$(extra_expat_LDADD) \
	$(extra_ffmpeg_LDADD) \
	$(extra_sdl_LDADD) \
	$(extra_xerces_LDADD) \
	$(libambulant_core) \
	$(sdl_libs) \
	$(LIBLTDL) \
	$(PTHREAD_LIBS)

AM_LDFLAGS= \
	   -export-dynamic

AM_CPPFLAGS = \
		$(extra_gstreamer_CPPFLAGS) \
		-DAMBULANT_DATADIR=\"$(pkgdatadir)\" \
		$(extra_expat_DEFINES) \
		$(extra_ffmpeg_DEFINES) \
		$(extra_nls_DEFINES) \
		$(extra_sdl_DEFINES) \
		$(extra_xerces_DEFINES) \
		$(sdl_includes) \
		-I$(top_builddir)/include \
		-I$(top_srcdir)/include \
		-I$(top_builddir)/include/ambulant/gui/sdl \
		-I$(top_srcdir)/include/ambulant/gui/sdl

AM_CXXFLAGS=-fPIC

install-exec-hook: install-data-local

# Set this player as default AmbualntPlayer 
#	cd $(DESTDIR)$(bindir) && \
#	$(LN_S) -f $(bin_PROGRAMS) AmbulantPlayer

check-ambulantplayer_sdl:
	echo "Test TBD"

# 	if test x$$DISPLAY = x; then \
# 		echo Skipping tests, DISPLAY not set; \
# 	else \
# 		set -e; \
# 		set +x; \
# 		pyv=`$(PYTHON) -V 2>&1 | sed -e 's/Python \([1-9].[1-9]\).*/\1/'`; \
# 		pyp=`$(PYTHON) -c 'import sys; print sys.prefix'`; \
# 		lib=/lib; \
# 		if test `uname -p` = "x86_64"; then lib=/lib64; fi;\
# 		export AMBULANT_PLUGIN_PATH=$(top_srcdir)/src/pyambulant/test; \
# 		export PYTHONPATH=:$(top_srcdir)/src/pyambulant/build/lib.*; \
# 		export LD_PRELOAD=$$pyp/$$lib/python$$pyv/config/libpython$$pyv.so; \
# 		./AmbulantPlayer_sdl $(top_srcdir)/Extras/Welcome/Welcome.smil; \
# 		if test -e AM_TEST-output.txt; then cat AM_TEST-output.txt; fi \
# 	fi

uninstall-local:
#XXXX quick fix to make distcheck work: remove make check results
	-rm -fr AM_TEST-output.txt 

install-data-local:

EXTRA_DIST=

