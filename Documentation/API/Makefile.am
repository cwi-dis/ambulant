EXTRA_DIST=Doxyfile

if WITH_DOXYGEN
html:
	-rm -r html
	cd $(srcdir) && $(DOXYGEN)
	mv $(srcdir)/html .
endif

docset: html html/Makefile
	cd html ; make
	-rm -r org.ambulantplayer.ambulantplayer.docset
	mv html/org.ambulantplayer.ambulantplayer.docset .
	zip -r org.ambulantplayer.ambulantplayer.docset.zip org.ambulantplayer.ambulantplayer.docset
	