EXTRA_DIST=Welcome DemoPresentation

install-data-local:
	mkdir -p $(DESTDIR)$(pkgdatadir)
	cp -R $(srcdir)/Welcome $(DESTDIR)$(pkgdatadir)
	cp -R $(srcdir)/DemoPresentation $(DESTDIR)$(pkgdatadir)

uninstall-local:
	rm -r $(DESTDIR)$(pkgdatadir)/Welcome
	rm -r $(DESTDIR)$(pkgdatadir)/DemoPresentation
	