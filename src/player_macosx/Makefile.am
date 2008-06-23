EXTRA_DIST=MyAmbulantView.mm MyDocument.mm LogController.mm mainloop.cpp MyAppDelegate.mm main.m \
	mypreferences.mm mypreferences.h \
	MyAmbulantView.h MyAppDelegate.h MyDocument.h LogController.h mainloop.h \
	cocoambulant_Prefix.pch \
	English.lproj Dutch.lproj \
	Info.plist version.plist \
	fullcocoambulant.xcodeproj \
	cocoambulant.icns \
	play.png \
	pause.png \
	playactive.png \
	stop.png \
	systemTestSettings.xml \
	genappleindex.sh \
	ui_icons.psd

if WITH_CG
extra_cg_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_cg.a
extra_cg_ldflags=
else
extra_cg_LDADD=
extra_cg_ldflags=
endif

if WITH_SDL
extra_sdl_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_sdl.a
extra_sdl_DEFINES=-DWITH_SDL
extra_sdl_ldflags=-static `sdl-config --libs` \
	-framework Carbon \
	-framework IOKit \
	-framework OpenGL \
	-framework AudioUnit \
	-framework AudioToolbox
else
extra_sdl_ldflags=
extra_sdl_DEFINES=
endif

if WITH_FFMPEG
extra_ffmpeg_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_ffmpeg.a
extra_ffmpeg_ldflags=$(FFMPEG_LIBS) -lz
extra_ffmpeg_DEFINES=-DWITH_FFMPEG -DWITH_FFMPEG_AVFORMAT $(FFMPEG_CFLAGS)
else
extra_ffmpeg_LDADD=
extra_ffmpeg_ldflags=
extra_ffmpeg_DEFINES=
endif

if WITH_EXPAT
extra_expat_DEFINES=-I$(EXPAT_PREFIX)/include
extra_expat_ldflags=-L$(EXPAT_PREFIX)/lib -lexpat
extra_expat_LDADD=
else
extra_expat_DEFINES=
extra_expat_ldflags=
extra_expat_LDADD=
endif

if WITH_XERCES_BUILTIN
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

# Sometimes we seem to have a .so suffix, sometimes not.
#SO=.so
SO=
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
BUILDAPPNAME = "AmbulantPlayer"

# Script to test dependencies
TESTDEP=$(srcdir)/testdependencies

cocoambulant_bundle: cocoambulant_executable
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
	
if WITH_PYTHON_PLUGIN
#   Install python-based plugins, when applicable
if WITH_SMIL30
#   Install the SMIL State plugin
	cp -r $(top_srcdir)/src/plugins/pyamplugin_scripting $(BUILDAPPNAME).app/Contents/PlugIns
	$(PYTHON) -c "import compileall;compileall.main()" $(BUILDAPPNAME).app/Contents/PlugIns
endif
endif

#	The next bit is gross. Xerces only builds as a dynamic library. This means
#	we must copy that library into the .app bundle (in the Frameworks dir) and
#	change the plugin to refer to that copy of the xerces library in stead of
#	the one it was linked to.
if WITH_XERCES_PLUGIN
	mkdir -p $(BUILDAPPNAME).app/Contents/Frameworks
	cp $(top_builddir)/third_party_packages/xerces-unix/lib/libxerces-c.27.dylib $(BUILDAPPNAME).app/Contents/Frameworks
	install_name_tool -change `otool -D $(top_builddir)/third_party_packages/xerces-unix/lib/libxerces-c.27.dylib | tail -n 1` \
		@executable_path/../Frameworks/libxerces-c.27.dylib $(BUILDAPPNAME).app/Contents/PlugIns/libamplugin_xerces$(SO)
	install_name_tool -id @executable_path/../Frameworks/libxerces-c.27.dylib $(BUILDAPPNAME).app/Contents/Frameworks/libxerces-c.27.dylib
endif

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
	$(INSTALL_DATA) $(top_srcdir)/Extras/DTDCache/mapping.txt $(BUILDAPPNAME).app/Contents/Resources/DTDCache
	$(INSTALL_DATA) $(top_srcdir)/Extras/DTDCache/SMIL20/*.* $(BUILDAPPNAME).app/Contents/Resources/DTDCache/SMIL20
	$(INSTALL_DATA) $(top_srcdir)/Extras/DTDCache/SMIL21/*.* $(BUILDAPPNAME).app/Contents/Resources/DTDCache/SMIL21

#	Install executable
	$(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) cocoambulant_executable `pwd`/$(BUILDAPPNAME).app/Contents/MacOS/Ambulant-mac
	strip -x `pwd`/$(BUILDAPPNAME).app/Contents/MacOS/Ambulant-mac
	
#   Test for problematic dependencies
	$(TESTDEP) `pwd`/$(BUILDAPPNAME).app/Contents/MacOS/Ambulant-mac
	$(TESTDEP) `pwd`/$(BUILDAPPNAME).app/Contents/PlugIns/*
	$(TESTDEP) `pwd`/$(BUILDAPPNAME).app/Contents/Frameworks/*
	
LDADD = \
	$(top_builddir)/src/libambulant/.libs/libambulant_cocoa.a \
	$(extra_cg_LDADD) \
	$(extra_sdl_LDADD) \
	$(extra_ffmpeg_LDADD) \
	$(extra_expat_LDADD) \
	$(extra_xerces_LDADD) \
	$(LIBINTL) \
	$(LIBICONV) \
    $(top_builddir)/src/libambulant/.libs/libambulant.a \
    -lstdc++
AM_LDFLAGS= \
	$(extra_cg_ldflags) \
    $(extra_sdl_ldflags) \
    $(extra_ffmpeg_ldflags) \
    $(extra_expat_ldflags) \
    -framework Cocoa \
    -framework QTKit \
    -framework QuickTime \
    -framework WebKit

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
	cp -R $(BUILDAPPNAME).app $(DESTDIR)/Applications/$(APPNAME).app

uninstall-hook:
	rm -rf $(DESTDIR)/Applications/$(APPNAME).app
	
