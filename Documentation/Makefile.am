EXTRA_DIST=README API Design man1 misc user
man1_MANS=man1/ambulantplayer.1

install-data-local:
	$(MKINSTALLDIRS) $(DESTDIR)$(pkgdatadir)/AmbulantPlayerHelp
	$(INSTALL_DATA) $(srcdir)/user/*.* $(DESTDIR)$(pkgdatadir)/AmbulantPlayerHelp
	