EXTRA_DIST=MyAmbulantView.mm MyDocument.mm LogController.mm mainloop.cpp MyAppDelegate.mm main.m \
	mypreferences.mm mypreferences.h \
	MyAmbulantView.h MyAppDelegate.h MyDocument.h LogController.h mainloop.h \
	cocoambulant_Prefix.pch \
	English.lproj Dutch.lproj \
	Info.plist version.plist \
	fullcocoambulant.xcodeproj \
	Ambulant.icns \
	AmbulantDocSmil.icns \
	AmbulantDocGrins.icns \
	AmbulantDocDaisy.icns \
	play.png \
	pause.png \
	playactive.png \
	stop.png \
	systemTestSettings.xml \
	genappleindex.sh \
	internalize_dylibs.py \
	testdependencies \
	ui_icons.psd

PYTHON=python

if WITH_CG
extra_cg_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_cg.a
else
extra_cg_LDADD=
endif

if WITH_EXPAT
extra_expat_DEFINES=$(EXPAT_CFLAGS) -DWITH_EXPAT
extra_expat_LDADD=$(EXPAT_LIBS)
else
extra_expat_DEFINES=
extra_expat_LDADD=
endif

if WITH_FFMPEG
extra_ffmpeg_DEFINES=-DWITH_FFMPEG $(FFMPEG_CFLAGS)
extra_ffmpeg_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_ffmpeg.a $(FFMPEG_LIBS)
else
extra_ffmpeg_DEFINES=
extra_ffmpeg_LDADD=
endif

if WITH_LIVE
extra_live_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_live.a $(LIVE_LIBS)
extra_live_DEFINES=$(LIVE_CFLAGS) -DWITH_LIVE
else
extra_live_DEFINES=
extra_live_LDADD =
endif

if WITH_SDL
extra_sdl_DEFINES=$(SDL_CFLAGS) -DWITH_SDL
extra_sdl_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_sdl.a $(SDL_LIBS)
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

# Sometimes we seem to have a .so suffix, sometimes not.
SO=.so
#SO=
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
BUILDAPPNAME=AmbulantPlayer

# Script to test dependencies
TESTDEP=$(srcdir)/testdependencies

cocoambulant_bundle: $(BUILDAPPNAME).app
$(BUILDAPPNAME).app: cocoambulant_executable
	mkdir -p $(BUILDAPPNAME).app/Contents/MacOS
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources
	echo -n "APPLAmbl" > $(BUILDAPPNAME).app/Contents/PkgInfo
	$(INSTALL_DATA) $(srcdir)/Info.plist $(BUILDAPPNAME).app/Contents/Info.plist
	$(INSTALL_DATA) $(srcdir)/Ambulant.icns $(BUILDAPPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/AmbulantDocDaisy.icns $(BUILDAPPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/AmbulantDocGrins.icns $(BUILDAPPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/AmbulantDocSmil.icns $(BUILDAPPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/systemTestSettings.xml $(BUILDAPPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/play.png $(BUILDAPPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/pause.png $(BUILDAPPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/playactive.png $(BUILDAPPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/stop.png $(BUILDAPPNAME).app/Contents/Resources

#	Install plugins
#	if test -d ../plugins/.libs; then cp -R ../plugins/.libs $(BUILDAPPNAME).app/Contents/PlugIns; fi
	pkglibdir=`pwd`/$(BUILDAPPNAME).app/Contents/PlugIns; \
	cd ../plugins; \
	$(MAKE) $(AM_MAKEFLAGS) pkglibdir=$$pkglibdir install 
	
#	Install gettext localizations
	localedir=`pwd`/$(BUILDAPPNAME).app/Contents/Resources/locale; \
	cd ../../po ; \
	$(MAKE) $(AM_MAKEFLAGS) localedir=$$localedir install
	
#	Install Cocoa localizations
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/English.lproj
	$(INSTALL_DATA) $(srcdir)/English.lproj/Credits.rtf $(BUILDAPPNAME).app/Contents/Resources/English.lproj
	$(INSTALL_DATA) $(srcdir)/English.lproj/InfoPlist.strings $(BUILDAPPNAME).app/Contents/Resources/English.lproj
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/English.lproj/MainMenu.nib
	$(INSTALL_DATA) $(srcdir)/English.lproj/MainMenu.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/English.lproj/MainMenu.nib
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/English.lproj/MyDocument.nib
	$(INSTALL_DATA) $(srcdir)/English.lproj/MyDocument.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/English.lproj/MyDocument.nib
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/English.lproj/Log.nib
	$(INSTALL_DATA) $(srcdir)/English.lproj/Log.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/English.lproj/Log.nib

	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/Dutch.lproj
	$(INSTALL_DATA) $(srcdir)/Dutch.lproj/Credits.rtf $(BUILDAPPNAME).app/Contents/Resources/Dutch.lproj
	$(INSTALL_DATA) $(srcdir)/Dutch.lproj/InfoPlist.strings $(BUILDAPPNAME).app/Contents/Resources/Dutch.lproj
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/Dutch.lproj/MainMenu.nib
	$(INSTALL_DATA) $(srcdir)/Dutch.lproj/MainMenu.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/Dutch.lproj/MainMenu.nib
#	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/Dutch.lproj/MyDocument.nib
#	$(INSTALL_DATA) $(srcdir)/Dutch.lproj/MyDocument.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/Dutch.lproj/MyDocument.nib
#	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/Dutch.lproj/Log.nib
#	$(INSTALL_DATA) $(srcdir)/Dutch.lproj/Log.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/Dutch.lproj/Log.nib

##	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/Hindi.lproj
##	$(INSTALL_DATA) $(srcdir)/Hindi.lproj/Credits.rtf $(BUILDAPPNAME).app/Contents/Resources/Hindi.lproj
##	$(INSTALL_DATA) $(srcdir)/Hindi.lproj/InfoPlist.strings $(BUILDAPPNAME).app/Contents/Resources/Hindi.lproj
##	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/Hindi.lproj/MainMenu.nib
##	$(INSTALL_DATA) $(srcdir)/Hindi.lproj/MainMenu.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/Hindi.lproj/MainMenu.nib
##	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/Hindi.lproj/MyDocument.nib
##	$(INSTALL_DATA) $(srcdir)/Hindi.lproj/MyDocument.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/Hindi.lproj/MyDocument.nib
##	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/Hindi.lproj/Log.nib
##	$(INSTALL_DATA) $(srcdir)/Hindi.lproj/Log.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/Hindi.lproj/Log.nib

#	$(INSTALL_DATA) $(srcdir)/*.png $(BUILDAPPNAME).app/Contents/Resources

#	Install documentation
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/English.lproj/Ambulant\ Player\ Help
	sh $(srcdir)/genappleindex.sh $(srcdir)/../../Documentation/user
	$(INSTALL_DATA) $(srcdir)/../../Documentation/user/*.* $(BUILDAPPNAME).app/Contents/Resources/English.lproj/Ambulant\ Player\ Help
#	sh $(srcdir)/genappleindex.sh $(BUILDAPPNAME).app/Contents/Resources/English.lproj/Ambulant\ Player\ Help

#	Install welcome document
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/data
	$(INSTALL_DATA) $(top_srcdir)/Extras/Welcome/Welcome.smil $(BUILDAPPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(top_srcdir)/Extras/Welcome/data/*.* $(BUILDAPPNAME).app/Contents/Resources/data

#	Install DTD cache data
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/DTDCache
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/DTDCache/SMIL20
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/DTDCache/SMIL21
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/DTDCache/SMIL30
	$(INSTALL_DATA) $(top_srcdir)/Extras/DTDCache/mapping.txt $(BUILDAPPNAME).app/Contents/Resources/DTDCache
	$(INSTALL_DATA) $(top_srcdir)/Extras/DTDCache/SMIL20/*.* $(BUILDAPPNAME).app/Contents/Resources/DTDCache/SMIL20
	$(INSTALL_DATA) $(top_srcdir)/Extras/DTDCache/SMIL21/*.* $(BUILDAPPNAME).app/Contents/Resources/DTDCache/SMIL21
	$(INSTALL_DATA) $(top_srcdir)/Extras/DTDCache/SMIL30/*.* $(BUILDAPPNAME).app/Contents/Resources/DTDCache/SMIL30

#	Install executable
	$(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) cocoambulant_executable `pwd`/$(BUILDAPPNAME).app/Contents/MacOS/Ambulant-mac
	strip -x `pwd`/$(BUILDAPPNAME).app/Contents/MacOS/Ambulant-mac
	
#	Internalize any non-system shared libraries we use
	$(PYTHON) $(srcdir)/internalize_dylibs.py -v $(BUILDAPPNAME).app

#	Test for problematic dependencies
	$(TESTDEP) `pwd`/$(BUILDAPPNAME).app/Contents/MacOS/Ambulant-mac
	$(TESTDEP) `pwd`/$(BUILDAPPNAME).app/Contents/PlugIns/*
	$(TESTDEP) `pwd`/$(BUILDAPPNAME).app/Contents/Frameworks/*
	$(PYTHON) $(srcdir)/internalize_dylibs.py -c $(BUILDAPPNAME).app
	
LDADD = \
	$(top_builddir)/src/libambulant/.libs/libambulant_cocoa.a \
	$(extra_cg_LDADD) \
	$(extra_expat_LDADD) \
	$(extra_ffmpeg_LDADD) \
	$(extra_live_LDADD) \
	$(extra_sdl_LDADD) \
	$(extra_xerces_LDADD) \
	$(LIBINTL) \
	$(LIBICONV) \
	$(LIBLTDL) \
	$(top_builddir)/src/libambulant/.libs/libambulant.a \
	-lstdc++
AM_LDFLAGS= \
	-framework Cocoa \
	-framework QTKit \
	-framework WebKit \
	-weak_framework QuickTime 

AM_CPPFLAGS = \
	-I$(top_builddir)/include -I$(top_srcdir)/include \
	$(extra_expat_DEFINES) \
	$(extra_ffmpeg_DEFINES) \
	$(extra_nls_DEFINES) \
	$(extra_live_DEFINES) \
	$(extra_sdl_DEFINES) \
	$(extra_xerces_DEFINES)

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	
install-exec-hook: cocoambulant_bundle
	rm -rf $(DESTDIR)/Applications/$(APPNAME).app
	cp -R $(BUILDAPPNAME).app $(DESTDIR)/Applications/$(APPNAME).app

uninstall-hook:
	rm -rf $(DESTDIR)/Applications/$(APPNAME).app
	
