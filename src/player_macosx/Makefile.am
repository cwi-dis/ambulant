EXTRA_DIST=MyAmbulantView.mm MyDocument.mm mainloop.cpp MyAppDelegate.mm main.m \
	MyAmbulantView.h MyAppDelegate.h MyDocument.h mainloop.h \
	cocoambulant_Prefix.pch \
	English.lproj \
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
	-framework OpenGL
else
extra_sdl_ldflags=
extra_sdl_DEFINES=
endif

if WITH_FFMPEG
extra_ffmpeg_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_ffmpeg.a \
	-L$(FFMPEG_CODEC_LIB)  -lavcodec \
	-L$(FFMPEG_FORMAT_LIB) -lavformat -lz
extra_ffmpeg_ldflags=
extra_ffmpeg_DEFINES=-I$(FFMPEG_CODEC_INCLUDE) -DWITH_FFMPEG \
	-I$(FFMPEG_FORMAT_INCLUDE) -DWITH_FFMPEG_AVFORMAT
else
extra_ffmpeg_LDADD=
extra_ffmpeg_ldflags=
extra_ffmpeg_DEFINES=
endif


# Building a MacOSX .app bundle is a bit tricky.
noinst_PROGRAMS = cocoambulant_executable
cocoambulant_executable_SOURCES= \
	main.m \
	mainloop.cpp \
	MyAmbulantView.m \
	MyDocument.m \
	MyAppDelegate.m
	
APPNAME="Ambulant Player"
cocoambulant_bundle: cocoambulant_executable
	mkdir -p $(APPNAME).app/Contents/MacOS
	mkdir -p $(APPNAME).app/Contents/Resources
	echo "APPL????" > $(APPNAME).app/Contents/PkgInfo
	$(INSTALL_DATA) $(srcdir)/Info.plist $(APPNAME).app/Contents/Info.plist
	mkdir -p $(APPNAME).app/Contents/Resources/English.lproj/MainMenu.nib
	mkdir -p $(APPNAME).app/Contents/Resources/English.lproj/MyDocument.nib
	$(INSTALL_DATA) $(srcdir)/cocoambulant.icns $(APPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/English.lproj/Credits.rtf $(APPNAME).app/Contents/Resources/English.lproj
	$(INSTALL_DATA) $(srcdir)/English.lproj/InfoPlist.strings $(APPNAME).app/Contents/Resources/English.lproj
	$(INSTALL_DATA) $(srcdir)/English.lproj/locversion.plist $(APPNAME).app/Contents/Resources/English.lproj
	$(INSTALL_DATA) $(srcdir)/English.lproj/MainMenu.nib/*.nib $(APPNAME).app/Contents/Resources/English.lproj/MainMenu.nib
	$(INSTALL_DATA) $(srcdir)/English.lproj/MyDocument.nib/*.nib $(APPNAME).app/Contents/Resources/English.lproj/MyDocument.nib
	$(INSTALL_DATA) $(srcdir)/*.png $(APPNAME).app/Contents/Resources
	mkdir -p $(APPNAME).app/Contents/Resources/data
	$(INSTALL_DATA) $(top_srcdir)/Extras/Welcome/Welcome.smil $(APPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(top_srcdir)/Extras/Welcome/data/*.* $(APPNAME).app/Contents/Resources/data
	$(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) -s cocoambulant_executable `pwd`/$(APPNAME).app/Contents/MacOS/cocoambulant
	
LDADD = $(top_builddir)/src/libambulant/.libs/libambulant.a \
	$(extra_sdl_LDADD) \
	$(extra_ffmpeg_LDADD) \
	$(top_builddir)/src/libambulant/.libs/libambulant_cocoa.a \
	$(EXPAT_PREFIX)/lib/libexpat.a -lstdc++
AM_LDFLAGS=$(extra_sdl_ldflags) $(extra_ffmpeg_ldflags) -framework Cocoa -framework QuickTime

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include -I$(EXPAT_PREFIX)/include \
	$(extra_sdl_DEFINES) \
	$(extra_ffmpeg_DEFINES)

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	
install-exec-hook: cocoambulant_bundle
	rm -rf $(DESTDIR)/Applications/$(APPNAME).app
	mkdir -p $(DESTDIR)/Applications/$(APPNAME).app
	cp -r $(APPNAME).app $(DESTDIR)/Applications
