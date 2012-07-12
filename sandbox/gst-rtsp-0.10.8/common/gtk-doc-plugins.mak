# This is an include file specifically tuned for building documentation
# for GStreamer plug-ins

help:
	@echo
	@echo "If you are a doc maintainer, run 'make update' to update"
	@echo "the documentation files maintained in git"
	@echo
	@echo Other useful make targets:
	@echo
	@echo  check-inspected-versions: make sure the inspected plugin info
	@echo                            is up to date before a release
	@echo

# update the stuff maintained by doc maintainers
update:
	$(MAKE) scanobj-update
	$(MAKE) check-outdated-docs

# We set GPATH here; this gives us semantics for GNU make
# which are more like other make's VPATH, when it comes to
# whether a source that is a target of one rule is then
# searched for in VPATH/GPATH.
#
GPATH = $(srcdir)

# thomas: make docs parallel installable
TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)-@GST_MAJORMINOR@

MAINTAINER_DOC_STAMPS =			\
	scanobj-build.stamp

EXTRA_DIST = 				\
	$(MAINTAINER_DOC_STAMPS)		\
	$(srcdir)/inspect/*.xml		\
	$(SCANOBJ_FILES)		\
	$(content_files)		\
	$(extra_files)			\
	$(HTML_IMAGES)			\
	$(DOC_MAIN_SGML_FILE)	\
	$(DOC_OVERRIDES)		\
	$(DOC_MODULE)-sections.txt

# we don't add scanobj-build.stamp here since they are built manually by docs
# maintainers and result is commited to git
DOC_STAMPS =				\
	scan-build.stamp		\
	tmpl-build.stamp		\
	sgml-build.stamp		\
	html-build.stamp		\
	scan.stamp			\
	tmpl.stamp			\
	sgml.stamp			\
	html.stamp

# files generated/updated by gtkdoc-scangobj
SCANOBJ_FILES =				\
	$(DOC_MODULE).signals           \
	$(DOC_MODULE).hierarchy         \
	$(DOC_MODULE).interfaces        \
	$(DOC_MODULE).prerequisites     \
	$(DOC_MODULE).types		\
	$(DOC_MODULE).args

SCANOBJ_FILES_O =			\
	.libs/$(DOC_MODULE)-scan.o

# files generated/updated by gtkdoc-scan
SCAN_FILES =				\
	$(DOC_MODULE)-sections.txt	\
	$(DOC_MODULE)-overrides.txt	\
	$(DOC_MODULE)-undocumented.txt	\
	$(DOC_MODULE)-decl.txt		\
	$(DOC_MODULE)-decl-list.txt


REPORT_FILES = \
	$(DOC_MODULE)-undocumented.txt \
	$(DOC_MODULE)-undeclared.txt \
	$(DOC_MODULE)-unused.txt

# FC3 seems to need -scan.c to be part of CLEANFILES for distcheck
# no idea why FC4 can do without
CLEANFILES = \
	$(SCANOBJ_FILES_O) \
	$(DOC_MODULE)-scan.c \
	$(REPORT_FILES) \
	$(DOC_STAMPS) \
	inspect-registry.xml


if ENABLE_GTK_DOC
all-local: html-build.stamp

### inspect GStreamer plug-ins; done by documentation maintainer ###

# only look at the plugins in this module when building inspect .xml stuff
INSPECT_REGISTRY=$(top_builddir)/docs/plugins/inspect-registry.xml
INSPECT_ENVIRONMENT=\
	GST_PLUGIN_SYSTEM_PATH= \
	GST_PLUGIN_PATH=$(top_builddir)/gst:$(top_builddir)/sys:$(top_builddir)/ext:$(top_builddir)/plugins:$(top_builddir)/src:$(top_builddir)/gnl \
	GST_REGISTRY=$(INSPECT_REGISTRY) \
	PKG_CONFIG_PATH="$(GST_PKG_CONFIG_PATH)" \
	$(INSPECT_EXTRA_ENVIRONMENT)

# update the element and plugin XML descriptions; store in inspect/
inspect:
	mkdir inspect

#### scan gobjects; done by documentation maintainer ####
scanobj-update:
	-rm scanobj-build.stamp
	$(MAKE) scanobj-build.stamp

# in the case of non-srcdir builds, the built gst directory gets added
# to gtk-doc scanning; but only then, to avoid duplicates
# FIXME: since we don't have the scan step as part of the build anymore,
# we could remove that
# TODO: finish elite script that updates the output files of this step
# instead of rewriting them, so that multiple maintainers can generate
# a collective set of args and signals
scanobj-build.stamp: $(SCANOBJ_DEPS) $(basefiles) inspect
	@echo '*** Scanning GObjects ***'
	if test x"$(srcdir)" != x. ; then				\
	    for f in $(SCANOBJ_FILES);					\
	    do								\
	        cp $(srcdir)/$$f . ;					\
	    done;							\
	else								\
	    $(INSPECT_ENVIRONMENT) 					\
	    CC="$(GTKDOC_CC)" LD="$(GTKDOC_LD)"				\
	    CFLAGS="$(GTKDOC_CFLAGS) $(CFLAGS) $(WARNING_CFLAGS)"	\
	    LDFLAGS="$(GTKDOC_LIBS) $(LDFLAGS)"				\
	    $(GST_DOC_SCANOBJ) --type-init-func="gst_init(NULL,NULL)"	\
	        --module=$(DOC_MODULE) --source=$(PACKAGE) --inspect-dir="inspect" &&		\
		$(PYTHON)						\
		$(top_srcdir)/common/scangobj-merge.py $(DOC_MODULE);	\
	fi
	touch scanobj-build.stamp

$(DOC_MODULE)-decl.txt $(SCANOBJ_FILES) $(SCANOBJ_FILES_O): scan-build.stamp
	@true

### scan headers; done on every build ###
scan-build.stamp: $(HFILE_GLOB) $(EXTRA_HFILES) $(basefiles) scanobj-build.stamp
	if test "x$(top_srcdir)" != "x$(top_builddir)" &&		\
	   test -d "$(top_builddir)/gst";				\
	then								\
	    export BUILT_OPTIONS="--source-dir=$(top_builddir)/gst";	\
	fi;								\
	gtkdoc-scan							\
	    $(SCAN_OPTIONS) $(EXTRA_HFILES)				\
	    --module=$(DOC_MODULE)					\
	    $$BUILT_OPTIONS						\
	    --ignore-headers="$(IGNORE_HFILES)";			\
	touch scan-build.stamp

#### update templates; done on every build ####

### FIXME: make this error out again when docs are fixed for 0.9
# in a non-srcdir build, we need to copy files from the previous step
# and the files from previous runs of this step
tmpl-build.stamp: $(DOC_MODULE)-decl.txt $(SCANOBJ_FILES) $(DOC_MODULE)-sections.txt $(DOC_OVERRIDES)
	@echo '*** Rebuilding template files ***'
	if test x"$(srcdir)" != x. ; then				\
	    for f in $(SCANOBJ_FILES) $(SCAN_FILES);			\
	    do								\
	        if test -e $(srcdir)/$$f; then cp $(srcdir)/$$f . ; fi; \
	    done;							\
	fi
	gtkdoc-mktmpl --module=$(DOC_MODULE) | tee tmpl-build.log
	$(PYTHON) \
		$(top_srcdir)/common/mangle-tmpl.py $(srcdir)/inspect tmpl
	@cat $(DOC_MODULE)-unused.txt
	rm -f tmpl-build.log
	touch tmpl-build.stamp

tmpl.stamp: tmpl-build.stamp
	@true

#### build xml; done on every build ####

### FIXME: make this error out again when docs are fixed for 0.9
sgml-build.stamp: tmpl.stamp scan-build.stamp $(CFILE_GLOB) $(top_srcdir)/common/plugins.xsl $(expand_content_files)
	@echo '*** Building XML ***'
	@-mkdir -p xml
	@for a in $(srcdir)/inspect/*.xml; do \
	    xsltproc --stringparam module $(MODULE) \
		$(top_srcdir)/common/plugins.xsl $$a > xml/`basename $$a`; done
	@for f in $(EXAMPLE_CFILES); do \
		$(PYTHON) $(top_srcdir)/common/c-to-xml.py $$f > xml/element-`basename $$f .c`.xml; done
	gtkdoc-mkdb \
		--module=$(DOC_MODULE) \
		--source-dir=$(DOC_SOURCE_DIR) \
		 --expand-content-files="$(expand_content_files)" \
		--main-sgml-file=$(srcdir)/$(DOC_MAIN_SGML_FILE) \
		--output-format=xml \
		--ignore-files="$(IGNORE_HFILES) $(IGNORE_CFILES)" \
		$(MKDB_OPTIONS) \
		| tee sgml-build.log
	@if grep "WARNING:" sgml-build.log > /dev/null; then true; fi # exit 1; fi
	cp ../version.entities xml
	rm sgml-build.log
	touch sgml-build.stamp

sgml.stamp: sgml-build.stamp
	@true

#### build html; done on every step ####

html-build.stamp: sgml.stamp $(DOC_MAIN_SGML_FILE) $(content_files)
	@echo '*** Building HTML ***'
	if test -d html; then rm -rf html; fi
	mkdir html
	cp $(srcdir)/$(DOC_MAIN_SGML_FILE) html
	@for f in $(content_files); do cp $(srcdir)/$$f html; done
	cp -pr xml html
	cp ../version.entities html
	cd html && gtkdoc-mkhtml $(DOC_MODULE) $(DOC_MAIN_SGML_FILE)
	mv html/index.sgml html/index.sgml.bak
	$(SED) "s/ href=\"$(DOC_MODULE)\// href=\"$(DOC_MODULE)-@GST_MAJORMINOR@\//g" html/index.sgml.bak >html/index.sgml
	rm -f html/index.sgml.bak
	rm -f html/$(DOC_MAIN_SGML_FILE)
	rm -rf html/xml
	rm -f html/version.entities
	test "x$(HTML_IMAGES)" = "x" || for i in "" $(HTML_IMAGES) ; do \
	    if test "$$i" != ""; then cp $(srcdir)/$$i html ; fi; done
	@echo '-- Fixing Crossreferences'
	gtkdoc-fixxref --module=$(DOC_MODULE) --module-dir=html --html-dir=$(HTML_DIR) $(FIXXREF_OPTIONS)
	touch html-build.stamp

clean-local-gtkdoc:
	rm -rf xml tmpl html
# clean files copied for nonsrcdir templates build
	if test x"$(srcdir)" != x. ; then \
	    rm -rf $(SCANOBJ_FILES) $(SCAN_FILES) $(MAINTAINER_DOC_STAMPS); \
	fi
else
all-local:
clean-local-gtkdoc:
endif

clean-local: clean-local-gtkdoc
	rm -f *~ *.bak
	rm -rf .libs

distclean-local:
	rm -rf tmpl/*.sgml.bak
	rm -rf *.o

MAINTAINERCLEANFILES = $(MAINTAINER_DOC_STAMPS)

# thomas: make docs parallel installable; devhelp requires majorminor too
install-data-local:
	(installfiles=`echo $(builddir)/html/*.sgml $(builddir)/html/*.html $(builddir)/html/*.png $(builddir)/html/*.css`; \
	if test "$$installfiles" = '$(builddir)/html/*.sgml $(builddir)/html/*.html $(builddir)/html/*.png $(builddir)/html/*.css'; \
	then echo '-- Nothing to install' ; \
	else \
	  $(mkinstalldirs) $(DESTDIR)$(TARGET_DIR); \
	  for i in $$installfiles; do \
	    echo '-- Installing '$$i ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
	  done; \
	  pngfiles=`echo ./html/*.png`; \
	  if test "$$pngfiles" != './html/*.png'; then \
	    for i in $$pngfiles; do \
	      echo '-- Installing '$$i ; \
	      $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
	    done; \
	  fi; \
	  echo '-- Installing $(builddir)/html/$(DOC_MODULE).devhelp' ; \
	  $(INSTALL_DATA) $(builddir)/html/$(DOC_MODULE).devhelp \
	    $(DESTDIR)$(TARGET_DIR)/$(DOC_MODULE)-@GST_MAJORMINOR@.devhelp; \
	  if test -e $(builddir)/html/$(DOC_MODULE).devhelp2; then \
	            $(INSTALL_DATA) $(builddir)/html/$(DOC_MODULE).devhelp2 \
	            $(DESTDIR)$(TARGET_DIR)/$(DOC_MODULE)-@GST_MAJORMINOR@.devhelp2; \
	  fi; \
	  (which gtkdoc-rebase >/dev/null && \
	    gtkdoc-rebase --relative --dest-dir=$(DESTDIR) --html-dir=$(DESTDIR)$(TARGET_DIR)) || true ; \
	fi)
uninstall-local:
	if test -d $(DESTDIR)$(TARGET_DIR); then \
	  rm -rf $(DESTDIR)$(TARGET_DIR)/*; \
	  rmdir -p $(DESTDIR)$(TARGET_DIR) 2>/dev/null || true; \
	else \
	  echo '-- Nothing to uninstall' ; \
	fi;

#
# Checks
#
check-hierarchy: $(DOC_MODULE).hierarchy
	@if grep '	' $(DOC_MODULE).hierarchy; then \
	    echo "$(DOC_MODULE).hierarchy contains tabs, please fix"; \
	    /bin/false; \
	fi

check: check-hierarchy

# wildcard is apparently not portable to other makes, hence the use of find
inspect_files = $(shell find $(srcdir)/inspect -name '*.xml')

check-inspected-versions:
	@echo Checking plugin versions of inspected plugin data ...; \
	fail=0 ; \
	for each in $(inspect_files) ; do \
	  if (grep -H '<version>' $$each | grep -v '<version>$(VERSION)'); then \
	    echo $$each should be fixed to say version $(VERSION) or be removed ; \
	    echo "sed -i -e 's/<version.*version>/<version>$(VERSION)<\/version>/'" $$each; \
	    echo ; \
	    fail=1; \
	  fi ; \
	done ; \
	exit $$fail

check-outdated-docs:
	$(AM_V_GEN)echo Checking for outdated plugin inspect data ...; \
	fail=0 ; \
	if [ -d $(top_srcdir)/.git/ ]; then \
	  files=`find $(srcdir)/inspect/ -name '*xml'`; \
	  for f in $$files; do \
	    ver=`grep '<version>$(PACKAGE_VERSION)</version>' $$f`; \
	    if test "x$$ver" = "x"; then \
	      plugin=`echo $$f | sed -e 's/^.*plugin-//' -e 's/.xml//'`; \
	      # echo "Checking $$plugin $$f"; \
	      pushd "$(top_srcdir)" >/dev/null; \
	      pinit=`git grep -A3 GST_PLUGIN_DEFINE -- ext/ gst/ sys/ | grep "\"$$plugin\""`; \
	      popd >/dev/null; \
	      # echo "[$$pinit]"; \
	      if test "x$$pinit" = "x"; then \
	        printf " **** outdated docs for plugin %-15s: %s\n" $$plugin $$f; \
	        fail=1; \
	      fi; \
	    fi; \
	  done; \
	fi ; \
	exit $$fail

#
# Require gtk-doc when making dist
#
if ENABLE_GTK_DOC
dist-check-gtkdoc:
else
dist-check-gtkdoc:
	@echo "*** gtk-doc must be installed and enabled in order to make dist"
	@false
endif

# FIXME: decide whether we want to dist generated html or not
# also this only works, if the project has been build before
# we could dist html only if its there, but that might lead to missing html in
# tarballs
dist-hook: dist-check-gtkdoc dist-hook-local
	mkdir $(distdir)/html
	cp html/* $(distdir)/html
	-cp $(srcdir)/$(DOC_MODULE).types $(distdir)/
	-cp $(srcdir)/$(DOC_MODULE)-sections.txt $(distdir)/
	cd $(distdir) && rm -f $(DISTCLEANFILES)
	-gtkdoc-rebase --online --relative --html-dir=$(distdir)/html

.PHONY : dist-hook-local docs check-outdated-docs

# avoid spurious build errors when distchecking with -jN
.NOTPARALLEL:
