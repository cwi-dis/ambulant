EXTRA_DIST=Welcome DemoPresentation DTDCache

install-data-local:
	mkdir -p $(DESTDIR)$(pkgdatadir)
	cp -R $(srcdir)/Welcome $(DESTDIR)$(pkgdatadir)
	cp -R $(srcdir)/DemoPresentation $(DESTDIR)$(pkgdatadir)
if WITH_XERCES
	cp -R $(srcdir)/DTDCache $(DESTDIR)$(pkgdatadir)
endif

uninstall-local:
	chmod -R +w $(DESTDIR)$(pkgdatadir)/Welcome
	rm -fr $(DESTDIR)$(pkgdatadir)/Welcome
	chmod -R +w $(DESTDIR)$(pkgdatadir)/DemoPresentation
	rm -fr $(DESTDIR)$(pkgdatadir)/DemoPresentation
if WITH_XERCES
	chmod -R +w $(DESTDIR)$(pkgdatadir)/DTDCache
	rm -fr $(DESTDIR)$(pkgdatadir)/DTDCache
endif
	