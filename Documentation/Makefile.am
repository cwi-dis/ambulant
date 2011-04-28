SUBDIRS=API
EXTRA_DIST=README Design man1 misc user user-htmlhelp
man1_MANS=man1/ambulantplayer.1
MKDIR_P=mkdir -p

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(pkgdatadir)/AmbulantPlayerHelp
	$(INSTALL_DATA) $(srcdir)/user/*.* $(DESTDIR)$(pkgdatadir)/AmbulantPlayerHelp

uninstall-local:
	rm -rf $(DESTDIR)$(pkgdatadir)/AmbulantPlayerHelp
