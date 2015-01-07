EXTRA_DIST=MyAmbulantView.mm MyDocument.mm LogController.mm mainloop.cpp MyAppDelegate.mm main.m \
	mypreferences.mm mypreferences.h \
	MyAmbulantView.h MyAppDelegate.h MyDocument.h LogController.h mainloop.h \
	cocoambulant_Prefix.pch \
	Resources \
	Resources-cg \
	version.plist \
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
	testdependencies \
	ui_icons.psd

PYTHON=python

ambulantlibdir=$(top_builddir)/src/libambulant/.libs
if WITH_AMBULANT_SHARED_LIB
#
# If we link with shared libs: use these. Needed for plugin support.
#
libambulant_core=-L$(ambulantlibdir) -lambulant
libambulant_ffmpeg=-L$(ambulantlibdir) -lambulant_ffmpeg
libambulant_sdl=-L$(ambulantlibdir) -lambulant_sdl
libambulant_cg=-L$(ambulantlibdir) -lambulant_cg
else
#
# Otherwise we link the ambulant libraries statically, and we
# do not include these here (the final executable will
# link with them).
#
libambulant_core=$(ambulantlibdir)/libambulant.a
libambulant_ffmpeg=$(ambulantlibdir)/libambulant_ffmpeg.a
libambulant_sdl=$(ambulantlibdir)/libambulant_sdl.a
libambulant_cg=$(ambulantlibdir)/libambulant_cg.a
endif

if WITH_CG
extra_cg_LDADD=$(libambulant_cg)
toolkit_resources=Resources-cg
else
extra_cg_LDADD=
toolkit_resources=
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
extra_ffmpeg_LDADD=$(libambulant_ffmpeg) $(FFMPEG_LIBS)
else
extra_ffmpeg_DEFINES=
extra_ffmpeg_LDADD=
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

# Sometimes we seem to have a .so suffix, sometimes not.
SO=.so
#SO=
# Building a MacOSX .app bundle is a bit tricky.
noinst_PROGRAMS = ambulant_executable
ambulant_executable_SOURCES= \
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

# The target all-am is magic for automake

all-am: $(BUILDAPPNAME).app

signedapp: $(BUILDAPPNAME).app
# For Code Signing, the key needs to be imported into the KeyChain
	codesign -s "Developer ID Application" -f --deep -v $(BUILDAPPNAME).app
#	codesign -s "Stichting CWI" -f --deep -v $(BUILDAPPNAME).app

$(BUILDAPPNAME).app: ambulant_executable
	mkdir -p $(BUILDAPPNAME).app/Contents/MacOS
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources
	echo -n "APPLAmbl" > $(BUILDAPPNAME).app/Contents/PkgInfo
#	$(INSTALL_DATA) $(srcdir)/Resources/Info.plist $(BUILDAPPNAME).app/Contents/Info.plist
	sed -e 's/..PRODUCT_NAME./AmbulantPlayer/' $(srcdir)/Resources/Info.plist > $(BUILDAPPNAME).app/Contents/Info.plist
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
#	Remove plugin cruft
	find $(BUILDAPPNAME).app/Contents/PlugIns -type l -exec rm '{}' ';'
	-rm $(BUILDAPPNAME).app/Contents/PlugIns/*.a

#	Install gettext localizations
	localedir=`pwd`/$(BUILDAPPNAME).app/Contents/Resources/locale; \
	cd ../../po ; \
	$(MAKE) $(AM_MAKEFLAGS) localedir=$$localedir install

	mkdir -p $(BUILDAPPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/Resources/Credits.rtf $(BUILDAPPNAME).app/Contents/Resources
	$(INSTALL_DATA) $(srcdir)/Resources/InfoPlist.strings $(BUILDAPPNAME).app/Contents/Resources
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/MainMenu.nib
	$(INSTALL_DATA) $(srcdir)/$(toolkit_resources)/MainMenu.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/MainMenu.nib
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/MyDocument.nib
	$(INSTALL_DATA) $(srcdir)/$(toolkit_resources)/MyDocument.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/MyDocument.nib
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/Log.nib
	$(INSTALL_DATA) $(srcdir)/$(toolkit_resources)/Log.nib/*.nib $(BUILDAPPNAME).app/Contents/Resources/Log.nib

#	$(INSTALL_DATA) $(srcdir)/*.png $(BUILDAPPNAME).app/Contents/Resources

#	Install documentation
	mkdir -p $(BUILDAPPNAME).app/Contents/Resources/Ambulant\ Player\ Help
	sh $(srcdir)/genappleindex.sh $(srcdir)/../../Documentation/user
	$(INSTALL_DATA) $(srcdir)/../../Documentation/user/*.* $(BUILDAPPNAME).app/Contents/Resources/Ambulant\ Player\ Help
#	sh $(srcdir)/genappleindex.sh $(BUILDAPPNAME).app/Contents/Resources/Ambulant\ Player\ Help

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
	$(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) .libs/ambulant_executable `pwd`/$(BUILDAPPNAME).app/Contents/MacOS/AmbulantPlayer
	strip -x `pwd`/$(BUILDAPPNAME).app/Contents/MacOS/AmbulantPlayer

#	Internalize any non-system shared libraries we use
	$(PYTHON) $(top_srcdir)/scripts/internalize_dylibs.py -v -s $(prefix)/lib:$(ambulantlibdir) $(BUILDAPPNAME).app

#	Test for problematic dependencies
	$(TESTDEP) `pwd`/$(BUILDAPPNAME).app/Contents/MacOS/AmbulantPlayer
	$(TESTDEP) `pwd`/$(BUILDAPPNAME).app/Contents/PlugIns/*
	$(TESTDEP) `pwd`/$(BUILDAPPNAME).app/Contents/Frameworks/*
	$(PYTHON) $(top_srcdir)/scripts/internalize_dylibs.py -c $(BUILDAPPNAME).app

LDADD = \
	$(extra_cg_LDADD) \
	$(extra_expat_LDADD) \
	$(extra_ffmpeg_LDADD) \
	$(extra_sdl_LDADD) \
	$(extra_xerces_LDADD) \
	$(LIBINTL) \
	$(LIBICONV) \
	$(LIBLTDL) \
	$(PTHREAD_LIBS) \
	$(libambulant_core) \
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
	$(extra_sdl_DEFINES) \
	$(extra_xerces_DEFINES)

dist-hook:
	rm -rf `find $(distdir) -name CVS`

install-exec-hook: $(BUILDAPPNAME).app
	if test $(prefix) = "/usr" -o $(prefix) = "/usr/local"; then \
		rm -rf $(DESTDIR)/Applications/$(APPNAME).app ; \
		mkdir -p $(DESTDIR)/Applications ; \
		cp -R $(BUILDAPPNAME).app $(DESTDIR)/Applications/$(APPNAME).app ; \
	else \
		rm -rf $(DESTDIR)/$(prefix)/Applications/$(APPNAME).app ; \
		mkdir -p $(DESTDIR)/$(prefix)/Applications ; \
		cp -R $(BUILDAPPNAME).app $(DESTDIR)/$(prefix)/Applications/$(APPNAME).app ; \
	fi

clean-local:
	-rm -fr AM_TEST-output.txt
	-rm -fr $(BUILDAPPNAME).app

uninstall-hook:
	if test $(prefix) = "/usr" -o $(prefix) = "/usr/local"; then \
		rm -rf $(DESTDIR)/Applications/$(APPNAME).app ; \
	else \
		rm -rf $(DESTDIR)/$(prefix)/Applications/$(APPNAME).app ; \
	fi

# This target is magic for automake
check-am: check-ambulantplayer_macos


orig-check-ambulantplayer_macos:
	( set -x;export PYAMBULANTBUILDDIR=`echo ../pyambulant/build/lib*` ; \
	  AMBULANT_BUNDLE_DIR=$$PWD/AmbulantPlayer.app/Contents ; \
	  export AMBULANT_PLUGIN_PATH=$$AMBULANT_BUNDLE_DIR/Plugins ; \
	  export AMBULANT_PLAYER=$$AMBULANT_BUNDLE_DIR/MacOS/AmbulantPlayer ; \
	  export AMBULANT_WELCOME=$$AMBULANT_BUNDLE_DIR/Resources/Welcome.smil ; \
	  cp ../pyambulant/test/pyamplugin_test.py $$AMBULANT_PLUGIN_PATH ; \
	  a=`cd $(ambulantlibdir); pwd` ; \
	  cd $(srcdir) ; \
	  LD_LIBRARY_PATH="$$a" DYLD_LIBRARY_PATH="$$a" $(PYTHONENV) PYTHONPATH=$AMBULANT_PLUGIN_PATH $(DEBUGGER) $$AMBULANT_PLAYER $$AMBULANT_WELCOME ; \
	)

check-ambulantplayer_macos:
# We run the player with one extra plugin (the test harness), obtained from src/pyambulant/test.
# This also causes exit-on-stop to happen. It's output will be written on AM_TEST-output.txt, which
# we will parse later.
	AMBULANT_PLUGIN_PATH=$(top_srcdir)/src/pyambulant/test \
		arch -i386 ./$(BUILDAPPNAME).app/Contents/MacOS/AmbulantPlayer \
			./$(BUILDAPPNAME).app/Contents/Resources/Welcome.smil
	AMBULANT_PLUGIN_PATH=$(top_srcdir)/src/pyambulant/test \
		arch -x86_64 ./$(BUILDAPPNAME).app/Contents/MacOS/AmbulantPlayer \
			./$(BUILDAPPNAME).app/Contents/Resources/Welcome.smil
