EXTRA_DIST=Welcome DemoPresentation DTDCache

install-data-local:
	mkdir -p $(DESTDIR)$(pkgdatadir)
	mkdir -p $(DESTDIR)$(pkgdatadir)/Welcome
	mkdir -p $(DESTDIR)$(pkgdatadir)/DemoPresentation
	cp -R $(srcdir)/Welcome/src $(DESTDIR)$(pkgdatadir)/Welcome
	cp -R $(srcdir)/Welcome/data $(DESTDIR)$(pkgdatadir)/Welcome
	cp -R $(srcdir)/Welcome/Welcome.smil $(DESTDIR)$(pkgdatadir)/Welcome
	cp -R $(srcdir)/DemoPresentation/NYC* $(DESTDIR)$(pkgdatadir)/DemoPresentation
	cp -R $(srcdir)/DemoPresentation/*.xml $(DESTDIR)$(pkgdatadir)/DemoPresentation
if WITH_XERCES
	cp -R $(srcdir)/DTDCache $(DESTDIR)$(pkgdatadir)
endif

uninstall-local:
	rm -fr $(DESTDIR)$(pkgdatadir)/Welcome
	rm -fr $(DESTDIR)$(pkgdatadir)/DemoPresentation
if WITH_XERCES
	rm -fr $(DESTDIR)$(pkgdatadir)/DTDCache
endif
	
