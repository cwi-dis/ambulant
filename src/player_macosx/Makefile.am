EXTRA_DIST=MyAmbulantView.mm MyDocument.mm mainloop.cpp MyAppDelegate.m main.m \
	MyAmbulantView.h MyAppDelegate.h MyDocument.h mainloop.h \
	cocoambulant_Prefix.pch \
	English.lproj \
	Info.plist version.plist \
	cocoambulant.pbproj fullcocoambulant.pbproj

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
	$(INSTALL_DATA) Info.plist cocoambulant.app/Contents/Info.plist
	mkdir -p cocoambulant.app/Contents/Resources/English.lproj/MainMenu.nib
	mkdir -p cocoambulant.app/Contents/Resources/English.lproj/MyDocument.nib
	$(INSTALL_DATA) cocoambulant.icns cocoambulant.app/Contents/Resources
	$(INSTALL_DATA) English.lproj/Credits.rtf cocoambulant.app/Contents/Resources/English.lproj
	$(INSTALL_DATA) English.lproj/InfoPlist.strings cocoambulant.app/Contents/Resources/English.lproj
	$(INSTALL_DATA) English.lproj/locversion.plist cocoambulant.app/Contents/Resources/English.lproj
	$(INSTALL_DATA) English.lproj/MainMenu.nib/*.nib cocoambulant.app/Contents/Resources/English.lproj/MainMenu.nib
	$(INSTALL_DATA) English.lproj/MyDocument.nib/*.nib cocoambulant.app/Contents/Resources/English.lproj/MyDocument.nib
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL_PROGRAM) cocoambulant_executable `pwd`/cocoambulant.app/Contents/MacOS/cocoambulant
	
LDADD = $(top_builddir)/src/libambulant/.libs/libambulant.a \
	$(top_builddir)/src/libambulant/.libs/libambulant_cocoa.a \
	$(EXPAT_PREFIX)/lib/libexpat.a -lstdc++
AM_LDFLAGS=-framework Cocoa

INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	
install-exec-hook: cocoambulant_bundle
	rm -rf $(DESTDIR)/Applications/cocoambulant.app
	mkdir -p $(DESTDIR)/Applications/cocoambulant.app
	cp -r cocoambulant.app $(DESTDIR)/Applications
	