#------------------------------------------------------
if WITH_LIVE
extra_live_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_live.a
liveincludes = -I$(LIVE_ROOT)/BasicUsageEnvironment/include \
			   -I$(LIVE_ROOT)/groupsock/include \
			   -I$(LIVE_ROOT)/liveMedia/include \
			   -I$(LIVE_ROOT)/UsageEnvironment/include

livelibs = -L$(LIVE_ROOT)/liveMedia/ -lliveMedia \
			  -L$(LIVE_ROOT)/BasicUsageEnvironment/ -lBasicUsageEnvironment  \
			  -L$(LIVE_ROOT)/groupsock/ -lgroupsock  \
			  -L$(LIVE_ROOT)/UsageEnvironment/ -lUsageEnvironment
extra_live_DEFINES=-DWITH_LIVE
else
extra_live_DEFINES=
extra_live_LDADD =
liveincludes =
livelibs =
endif # WITH_LIVE

#------------------------------------------------------
if WITH_GSTREAMER
extra_gstreamer_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_gstreamer.a
extra_gstreamer_CPPFLAGS=-DWITH_GSTREAMER	\
		-I/usr/include/gstreamer-0.10 \
		-I/usr/include/glib-2.0	\
		-I/usr/lib/glib-2.0/include \
		-I/usr/include/libxml2

extra_gstreamer_ldflags=`pkg-config --libs gstreamer-0.10`
else
extra_gstreamer_ldflags=
extra_gstreamer_CFLAGS=
endif # WITH_GSTREAMER
#------------------------------------------------------
if WITH_SDL
extra_sdl_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_sdl.a
extra_sdl_DEFINES=-DWITH_SDL
extra_sdl_ldflags= `sdl-config --libs`
else
extra_sdl_ldflags=
extra_sdl_DEFINES=
endif # WITH_SDL

#------------------------------------------------------

#------------------------------------------------------
if WITH_EXPAT
extra_expat_DEFINES=-I$(EXPAT_PREFIX)/include -DWITH_EXPAT
extra_expat_LDADD=-L$(EXPAT_PREFIX)/lib -lexpat
else
extra_expat_DEFINES=
extra_expat_LDADD=
endif #  WITH_EXPAT_BUILTIN

#------------------------------------------------------
if WITH_FFMPEG
extra_ffmpeg_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_ffmpeg.a $(FFMPEG_LIBS)
extra_ffmpeg_ldflags=
extra_ffmpeg_DEFINES=-DWITH_FFMPEG -DWITH_FFMPEG_AVFORMAT $(FFMPEG_CFLAGS)
else
extra_ffmpeg_LDADD=
extra_ffmpeg_ldflags=
extra_ffmpeg_DEFINES=
endif # WITH_FFMPEG

#------------------------------------------------------
if WITH_XERCES_BUILTIN
extra_xerces_DEFINES=-I$(XERCES_PREFIX)/include -DWITH_XERCES_BUILTIN
extra_xerces_LDADD=-L$(XERCES_PREFIX)/lib -lxerces-c
else
extra_xerces_DEFINES=
extra_xerces_LDADD=
endif # WITH_XERCES_BUILTIN

#------------------------------------------------------
##if USE_NLS
extra_nls_DEFINES=-DLOCALEDIR=\"$(datadir)/locale\"
##else
##extra_nls_DEFINES=
##endif # USE_NLS

#------------------------------------------------------
if WITH_GTK
gtk_libs=$(GTK_LIBS)
gtk_includes=$(GTK_CFLAGS)
else
gtk_libs=
gtk_includes=
endif # WITH_GTK

#------------------------------------------------------
if WITH_NOKIA770
extra_nokia770_DEFINES=-DWITH_NOKIA770 $(HILDONBASE_CFLAGS)  $(HILDONLIBS_CFLAGS) $(OSSO_CFLAGS)
extra_nokia770_LDADD=$(OSSO_LIBS) $(HILDONBASE_LIBS) $(HILDONLIBS_LIBS)
# -s no symbol table in executable
# extra_nokia770_ldflags=-s
extra_nokia770_ldflags=
else
extra_nokia770_DEFINES=
extra_nokia770_LDADD=
extra_nokia770_ldflags=
endif # WITH_NOKIA770

#------------------------------------------------------
#------------------------------------------------------

bin_PROGRAMS = AmbulantPlayer_gtk
AmbulantPlayer_gtk_SOURCES =					\
	gtk_gui.cpp						\
	gtk_gui.h 						\
	gtk_logger.cpp						\
	gtk_logger.h						\
	gtk_mainloop.cpp 					\
	gtk_mainloop.h						\
	gtk_settings.cpp 					\
	gtk_settings.h 						\
	unix_preferences.cpp					\
	unix_preferences.h

LDADD = $(top_builddir)/src/libambulant/.libs/libambulant_gtk.a	\
	$(extra_gstreamer_LDADD) \
	$(extra_sdl_LDADD) \
	$(extra_expat_LDADD) \
	$(extra_ffmpeg_LDADD) \
	$(extra_xerces_LDADD) \
	$(extra_live_LDADD)  \
	$(top_builddir)/src/libambulant/.libs/libambulant.a	\
	$(gtk_libs) \
	$(LIBLTDL) \
	$(extra_nokia770_LDADD)

AM_LDFLAGS= \
	   $(extra_gstreamer_ldflags) \
	   $(extra_sdl_ldflags) \
	   $(extra_ffmpeg_ldflags) \
	   $(extra_nokia770_ldflags) \
	   -export-dynamic \
       $(livelibs)

AM_CPPFLAGS = \
		$(extra_gstreamer_CPPFLAGS) \
		-DAMBULANT_DATADIR=\"$(pkgdatadir)\" \
		$(extra_sdl_DEFINES) \
		$(extra_ffmpeg_DEFINES) \
		$(extra_nls_DEFINES) \
		$(extra_xerces_DEFINES) \
		$(extra_live_DEFINES) \
		$(extra_nokia770_DEFINES) \
		-I$(top_builddir)/include \
		-I$(top_srcdir)/include \
		-I$(top_builddir)/include/ambulant/gui/gtk \
		-I$(top_srcdir)/include/ambulant/gui/gtk \
	    $(EXPAT_INCLUDES) \
	    $(XERCES_INCLUDES) \
	    $(gtk_includes) \
	    $(liveincludes)

#------------------------------------------------------
# More special case code for the Nokia 770: install the auxiliary files.
if WITH_NOKIA770
# The icons

icondir = $(datadir)/icons/hicolor
installerdir = $(srcdir)/../../installers

# 	i=pkg_AmbulantPlayer-app;
install-data-local:
	pwd; \
	i=AmbulantPlayer; \
	echo "DESTDIR=" $(DESTDIR) " icondir=" $(icondir) " " $i $$i; \
	$(mkinstalldirs) $(DESTDIR)$(icondir)/26x26/hildon; \
	$(mkinstalldirs) $(DESTDIR)$(icondir)/40x40/hildon; \
	$(mkinstalldirs) $(DESTDIR)$(icondir)/scalable/hildon; \
	$(INSTALL_DATA) $(installerdir)/nokia770/$$i-26x26.png $(DESTDIR)$(icondir)/26x26/hildon/pkg_$$i-app.png; \
	$(INSTALL_DATA) $(installerdir)/nokia770/$$i-40x40.png $(DESTDIR)$(icondir)/40x40/hildon/pkg_$$i-app.png; \
	$(INSTALL_DATA) $(installerdir)/nokia770/$$i-scalable.png $(DESTDIR)$(icondir)/scalable/hildon/pkg_$$i-app.png;
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir);
	$(INSTALL_DATA) $(srcdir)/ui_manager.xml $(DESTDIR)$(pkgdatadir);
#	Install gettext localizations
	localedir=$(DESTDIR)/$(datadir)/locale \
	cd ../../po ; \
	$(MAKE) $(AM_MAKEFLAGS) localedir=$$localedir install

uninstall-local:
	i=pkg_AmbulantPlayer-app; \
	rm $(DESTDIR)$(icondir)/26x26/hildon/$$i.png; \
	rm $(DESTDIR)$(icondir)/40x40/hildon/$$i.png; \
	rm $(DESTDIR)$(icondir)/scalable/hildon/$$i.png;
	rm $(DESTDIR)$(pkgdatadir)/ui_manager.xml;

#	Install gettext localizations
	localedir=$(DESTDIR)/$(datadir)/locale \
	pwd;cd ../../po ; \
	$(MAKE) $(AM_MAKEFLAGS) localedir=$$localedir install

# XXXX Jack thinks the rest of this file looks like leftovers from a previous life:

desktopdir = $(datadir)/applications/hildon
desktop_DATA = AmbulantPlayer.desktop

servicedir = $(datadir)/dbus-1/services
service_DATA = $(top_srcdir)/installers/nokia770/com.nokia.ambulantplayer_app.service


# The backup/restore configuration

EXTRA_DIST = \
	autogen.sh \
	ui_manager.xml \
	debian/changelog \
	debian/compat \
	debian/copyright \
	debian/control \
	debian/rules \
	debian/AmbulantPlayer-app.links \
	AmbulantPlayer.desktop \
	AmbulantPlayer.conf \
	com.nokia.AmbulantPlayer_app.service \
	pkg_AmbulantPlayer-app.26.png \
	pkg_AmbulantPlayer-app.40.png \
	pkg_AmbulantPlayer-app.64.png
endif # WITH_NOKIA770
