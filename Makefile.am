SUBDIRS = m4 po libltdl src include third_party_packages
EXTRA_DIST = config.rpath  Documentation Extras 

AM_CPPFLAGS=-rdynamic
dist-hook:
	rm -rf `find $(distdir) -name CVS -o -name '.#*' -o -name '*~' -o -name '*~.*'`

# Special rule for building MacOSX binary installer.
# Will only work on OSX 10.3 or later
MACOS_INSTALLDIR=AmbulantPlayer
MACOS_DMGNAME=AmbulantPlayer-$(VERSION)
macosbininstaller:
	-rmdir dist/$(MACOS_INSTALLDIR)
	mkdir -p dist/$(MACOS_INSTALLDIR)
	cp $(srcdir)/README dist/$(MACOS_INSTALLDIR)
	cp $(srcdir)/COPYING dist/$(MACOS_INSTALLDIR)
	cp -R $(DESTDIR)/Applications/"Ambulant Player".app dist/$(MACOS_INSTALLDIR)
	cp -R $(srcdir)/Extras/DemoPresentation dist/$(MACOS_INSTALLDIR)
	rm -rf `find dist -name CVS`
	hdiutil create -srcfolder dist/$(MACOS_INSTALLDIR) dist/$(MACOS_DMGNAME).dmg
	

ACLOCAL_AMFLAGS = -I m4
