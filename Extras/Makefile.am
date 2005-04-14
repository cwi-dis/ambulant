EXTRA_DIST=Welcome DemoPresentation DTDCache

install-data-local:
	mkdir -p $(DESTDIR)$(pkgdatadir)
	cp -R $(srcdir)/Welcome $(DESTDIR)$(pkgdatadir)
	cp -R $(srcdir)/DemoPresentation $(DESTDIR)$(pkgdatadir)
if WITH_XERCES
	cp -R $(srcdir)/DTDCache $(DESTDIR)$(pkgdatadir)
endif

uninstall-local:
	rm -r $(DESTDIR)$(pkgdatadir)/Welcome
	rm -r $(DESTDIR)$(pkgdatadir)/DemoPresentation
if WITH_XERCES
	rm -r $(DESTDIR)$(pkgdatadir)/DTDCache
endif
	