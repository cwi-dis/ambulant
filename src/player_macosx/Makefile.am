EXTRA_DIST=MyAmbulantView.mm MyDocument.mm mainloop.cpp MyAppDelegate.mm main.m \
	MyAmbulantView.h MyAppDelegate.h MyDocument.h mainloop.h \
	cocoambulant_Prefix.pch \
	English.lproj \
	Info.plist version.plist \
	cocoambulant.pbproj fullcocoambulant.pbproj

if WITH_SDL
extra_sdl_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_sdl.a
extra_sdl_DEFINES=-DWITH_SDL
extra_sdl_ldflags= `sdl-config --libs` -lSDL_mixer
else
extra_sdl_ldflags=
extra_sdl_DEFINES=
endif

if WITH_FFMPEG
extra_ffmpeg_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_ffmpeg.a -L$(FFMPEG_LIB)  -lavcodec
extra_ffmpeg_ldflags=
extra_ffmpeg_DEFINES=-I$(FFMPEG_INCLUDE) -DWITH_FFMPEG
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
	
cocoambulant_bundle: cocoambulant_executable
	mkdir -p cocoambulant.app/Contents/MacOS
	mkdir -p cocoambulant.app/Contents/Resources
	echo "APPL????" > cocoambulant.app/Contents/PkgInfo
	$(INSTALL_DATA) $(srcdir)/Info.plist cocoambulant.app/Contents/Info.plist
	mkdir -p cocoambulant.app/Contents/Resources/English.lproj/MainMenu.nib
	mkdir -p cocoambulant.app/Contents/Resources/English.lproj/MyDocument.nib
	$(INSTALL_DATA) $(srcdir)/cocoambulant.icns cocoambulant.app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/English.lproj/Credits.rtf cocoambulant.app/Contents/Resources/English.lproj
	$(INSTALL_DATA) $(srcdir)/English.lproj/InfoPlist.strings cocoambulant.app/Contents/Resources/English.lproj
	$(INSTALL_DATA) $(srcdir)/English.lproj/locversion.plist cocoambulant.app/Contents/Resources/English.lproj
	$(INSTALL_DATA) $(srcdir)/English.lproj/MainMenu.nib/*.nib cocoambulant.app/Contents/Resources/English.lproj/MainMenu.nib
	$(INSTALL_DATA) $(srcdir)/English.lproj/MyDocument.nib/*.nib cocoambulant.app/Contents/Resources/English.lproj/MyDocument.nib
	mkdir -p cocoambulant.app/Contents/Resources/data
	$(INSTALL_DATA) $(top_srcdir)/Extras/Welcome/Welcome.smil cocoambulant.app/Contents/Resources
	$(INSTALL_DATA) $(top_srcdir)/Extras/Welcome/data/*.* cocoambulant.app/Contents/Resources/data
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL_PROGRAM) cocoambulant_executable `pwd`/cocoambulant.app/Contents/MacOS/cocoambulant
	
LDADD = $(top_builddir)/src/libambulant/.libs/libambulant.a \
	$(extra_sdl_LDADD) \
	$(extra_ffmpeg_LDADD) \
	$(top_builddir)/src/libambulant/.libs/libambulant_cocoa.a \
	$(EXPAT_PREFIX)/lib/libexpat.a -lstdc++
AM_LDFLAGS=$(extra_sdl_ldflags) $(extra_ffmpeg_ldflags) -framework Cocoa

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include -I$(EXPAT_PREFIX)/include \
	$(extra_sdl_DEFINES) \
	$(extra_ffmpeg_DEFINES)

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	
install-exec-hook: cocoambulant_bundle
	rm -rf $(DESTDIR)/Applications/cocoambulant.app
	mkdir -p $(DESTDIR)/Applications/cocoambulant.app
	cp -r cocoambulant.app $(DESTDIR)/Applications
	