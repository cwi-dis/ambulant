EXTRA_DIST=Welcome DemoPresentation DTDCache
MKDIR_P=mkdir -p

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)/Welcome
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)/Welcome/data
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)/DemoPresentation
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)/DemoPresentation/NYCdata
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)/DTDCache
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)/DTDCache/SMIL20
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)/DTDCache/SMIL21
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)/DTDCache/SMIL30
	$(INSTALL_DATA) $(srcdir)/Welcome/*.smil $(DESTDIR)$(pkgdatadir)/Welcome
	$(INSTALL_DATA) $(srcdir)/Welcome/data/*.* $(DESTDIR)$(pkgdatadir)/Welcome/data
	$(INSTALL_DATA) $(srcdir)/DemoPresentation/*.{smil,txt,xml} $(DESTDIR)$(pkgdatadir)/DemoPresentation
	$(INSTALL_DATA) $(srcdir)/DemoPresentation/NYCdata/*.* $(DESTDIR)$(pkgdatadir)/DemoPresentation/NYCdata
if WITH_XERCES
	$(INSTALL_DATA) $(srcdir)/DTDCache/mapping.txt $(DESTDIR)$(pkgdatadir)/DTDCache
	$(INSTALL_DATA) $(srcdir)/DTDCache/SMIL20/*.* $(DESTDIR)$(pkgdatadir)/DTDCache/SMIL20
	$(INSTALL_DATA) $(srcdir)/DTDCache/SMIL21/*.* $(DESTDIR)$(pkgdatadir)/DTDCache/SMIL21
	$(INSTALL_DATA) $(srcdir)/DTDCache/SMIL30/*.* $(DESTDIR)$(pkgdatadir)/DTDCache/SMIL30
endif

uninstall-local:
	rm -fr $(DESTDIR)$(pkgdatadir)/Welcome
	rm -fr $(DESTDIR)$(pkgdatadir)/DemoPresentation
if WITH_XERCES
	rm -fr $(DESTDIR)$(pkgdatadir)/DTDCache
endif
