EXTRA_DIST=MyAmbulantView.mm MyDocument.mm LogController.mm mainloop.cpp MyAppDelegate.mm main.m \
	mypreferences.mm mypreferences.h \
	MyAmbulantView.h MyAppDelegate.h MyDocument.h LogController.h mainloop.h \
	cocoambulant_Prefix.pch \
	English.lproj Dutch.lproj Japanese.lproj \
	Info.plist version.plist \
	cocoambulant.pbproj fullcocoambulant.pbproj \
	cocoambulant.icns \
	play.png \
	pause.png \
	playactive.png \
	stop.png \
	systemTestSettings.xml

if WITH_SDL
extra_sdl_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_sdl.a
extra_sdl_DEFINES=-DWITH_SDL
extra_sdl_ldflags=-static `sdl-config --libs` \
	-framework Carbon \
	-framework IOKit \
	-framework QuickTime \
	-framework OpenGL \
	-framework AudioUnit \
	-framework AudioToolbox
else
extra_sdl_ldflags=
extra_sdl_DEFINES=
endif

if WITH_FFMPEG
#
# Using the dynamic ffmpeg libraries will fail on RedHat 9, which has
# incompatible ffmpeg libraries installed into /usr/lib, which will be
# preferred over our libraries because of linker flags provided by sdl_config:-(
#
##extra_ffmpeg_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_ffmpeg.a \
##	-L$(FFMPEG_CODEC_LIB)  -lavcodec \
##	-L$(FFMPEG_FORMAT_LIB) -lavformat -lz
extra_ffmpeg_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_ffmpeg.a \
	$(FFMPEG_FORMAT_LIB)/libavformat.a \
	$(FFMPEG_CODEC_LIB)/libavcodec.a -lz
extra_ffmpeg_ldflags=
extra_ffmpeg_DEFINES=-I$(FFMPEG_CODEC_INCLUDE) -DWITH_FFMPEG \
	-I$(FFMPEG_FORMAT_INCLUDE) -DWITH_FFMPEG_AVFORMAT
else
extra_ffmpeg_LDADD=
extra_ffmpeg_ldflags=
extra_ffmpeg_DEFINES=
endif

if WITH_EXPAT
extra_expat_DEFINES=-I$(EXPAT_PREFIX)/include
extra_expat_LDADD=$(EXPAT_PREFIX)/lib/libexpat.a
else
extra_expat_DEFINES=
extra_expat_LDADD=
endif

if WITH_XERCES
extra_xerces_DEFINES=-I$(XERCES_PREFIX)/include
extra_xerces_LDADD=-L$(XERCES_PREFIX)/lib -lxerces-c
else
extra_xerces_DEFINES=
extra_xerces_LDADD=
endif

#if USE_NLS
extra_nls_DEFINES=-DLOCALEDIR=\"$(datadir)/locale\"
#else
#extra_nls_DEFINES=
#endif

# Building a MacOSX .app bundle is a bit tricky.
noinst_PROGRAMS = cocoambulant_executable
cocoambulant_executable_SOURCES= \
	main.m \
	mainloop.cpp \
	mypreferences.m \
	MyAmbulantView.m \
	MyDocument.m \
	LogController.m \
	MyAppDelegate.m 
	
APPNAME="Ambulant Player"
cocoambulant_bundle: cocoambulant_executable
	mkdir -p $(APPNAME).app/Contents/MacOS
	mkdir -p $(APPNAME).app/Contents/Resources
	echo -n "APPLAmbl" > $(APPNAME).app/Contents/PkgInfo
	$(INSTALL_DATA) $(srcdir)/Info.plist $(APPNAME).app/Contents/Info.plist
	$(INSTALL_DATA) $(srcdir)/cocoambulant.icns $(APPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/systemTestSettings.xml $(APPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/play.png $(APPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/pause.png $(APPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/playactive.png $(APPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/stop.png $(APPNAME).app/Contents/Resources

    # Install localizations
	mkdir -p $(APPNAME).app/Contents/Resources/English.lproj
	$(INSTALL_DATA) $(srcdir)/English.lproj/Credits.rtf $(APPNAME).app/Contents/Resources/English.lproj
	$(INSTALL_DATA) $(srcdir)/English.lproj/InfoPlist.strings $(APPNAME).app/Contents/Resources/English.lproj
	mkdir -p $(APPNAME).app/Contents/Resources/English.lproj/MainMenu.nib
	$(INSTALL_DATA) $(srcdir)/English.lproj/MainMenu.nib/*.nib $(APPNAME).app/Contents/Resources/English.lproj/MainMenu.nib
	mkdir -p $(APPNAME).app/Contents/Resources/English.lproj/MyDocument.nib
	$(INSTALL_DATA) $(srcdir)/English.lproj/MyDocument.nib/*.nib $(APPNAME).app/Contents/Resources/English.lproj/MyDocument.nib
	mkdir -p $(APPNAME).app/Contents/Resources/English.lproj/Log.nib
	$(INSTALL_DATA) $(srcdir)/English.lproj/Log.nib/*.nib $(APPNAME).app/Contents/Resources/English.lproj/Log.nib

	mkdir -p $(APPNAME).app/Contents/Resources/Dutch.lproj
	$(INSTALL_DATA) $(srcdir)/Dutch.lproj/Credits.rtf $(APPNAME).app/Contents/Resources/Dutch.lproj
	$(INSTALL_DATA) $(srcdir)/Dutch.lproj/InfoPlist.strings $(APPNAME).app/Contents/Resources/Dutch.lproj
	mkdir -p $(APPNAME).app/Contents/Resources/Dutch.lproj/MainMenu.nib
	$(INSTALL_DATA) $(srcdir)/Dutch.lproj/MainMenu.nib/*.nib $(APPNAME).app/Contents/Resources/Dutch.lproj/MainMenu.nib
#	mkdir -p $(APPNAME).app/Contents/Resources/Dutch.lproj/MyDocument.nib
#	$(INSTALL_DATA) $(srcdir)/Dutch.lproj/MyDocument.nib/*.nib $(APPNAME).app/Contents/Resources/Dutch.lproj/MyDocument.nib
#	mkdir -p $(APPNAME).app/Contents/Resources/Dutch.lproj/Log.nib
#	$(INSTALL_DATA) $(srcdir)/Dutch.lproj/Log.nib/*.nib $(APPNAME).app/Contents/Resources/Dutch.lproj/Log.nib

	mkdir -p $(APPNAME).app/Contents/Resources/Japanese.lproj
#	$(INSTALL_DATA) $(srcdir)/Japanese.lproj/Credits.rtf $(APPNAME).app/Contents/Resources/Japanese.lproj
#	$(INSTALL_DATA) $(srcdir)/Japanese.lproj/InfoPlist.strings $(APPNAME).app/Contents/Resources/Japanese.lproj
	mkdir -p $(APPNAME).app/Contents/Resources/Japanese.lproj/MainMenu.nib
	$(INSTALL_DATA) $(srcdir)/Japanese.lproj/MainMenu.nib/*.nib $(APPNAME).app/Contents/Resources/Japanese.lproj/MainMenu.nib
#	mkdir -p $(APPNAME).app/Contents/Resources/Japanese.lproj/MyDocument.nib
#	$(INSTALL_DATA) $(srcdir)/Japanese.lproj/MyDocument.nib/*.nib $(APPNAME).app/Contents/Resources/Japanese.lproj/MyDocument.nib
#	mkdir -p $(APPNAME).app/Contents/Resources/Japanese.lproj/Log.nib
#	$(INSTALL_DATA) $(srcdir)/Japanese.lproj/Log.nib/*.nib $(APPNAME).app/Contents/Resources/Japanese.lproj/Log.nib

#	$(INSTALL_DATA) $(srcdir)/*.png $(APPNAME).app/Contents/Resources
	mkdir -p $(APPNAME).app/Contents/Resources/data
	$(INSTALL_DATA) $(top_srcdir)/Extras/Welcome/Welcome.smil $(APPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(top_srcdir)/Extras/Welcome/data/*.* $(APPNAME).app/Contents/Resources/data
	$(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) -s cocoambulant_executable `pwd`/$(APPNAME).app/Contents/MacOS/cocoambulant
	
LDADD = $(top_builddir)/src/libambulant/.libs/libambulant.a \
	$(extra_sdl_LDADD) \
	$(extra_ffmpeg_LDADD) \
	$(extra_expat_LDADD) \
	$(extra_xerces_LDADD) \
	$(LIBINTL) \
	$(top_builddir)/src/libambulant/.libs/libambulant_cocoa.a \
    -lstdc++
AM_LDFLAGS=$(extra_sdl_ldflags) $(extra_ffmpeg_ldflags) -framework Cocoa -framework QuickTime

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include \
    $(extra_expat_DEFINES) \
    $(extra_xerces_DEFINES) \
	$(extra_sdl_DEFINES) \
	$(extra_ffmpeg_DEFINES) \
	$(extra_nls_DEFINES)

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	
install-exec-hook: cocoambulant_bundle
	rm -rf $(DESTDIR)/Applications/$(APPNAME).app
	mkdir -p $(DESTDIR)/Applications/$(APPNAME).app
	cp -r $(APPNAME).app $(DESTDIR)/Applications

uninstall-hook:
	rm -rf $(DESTDIR)/Applications/$(APPNAME).app
	
