#------------------------------------------------------
ambulantlibdir=$(top_builddir)/src/libambulant/.libs

if WITH_AMBULANT_SHARED_LIB
# If we link with shared libs: use these. Needed for plugin support.
libambulant_core=-L$(ambulantlibdir) -lambulant
libambulant_ffmpeg=-L$(ambulantlibdir) -lambulant_ffmpeg
libambulant_sdl=-L$(ambulantlibdir) -lambulant_sdl
else
# Otherwise we link the ambulant libraries statically.
libambulant_core=$(ambulantlibdir)/libambulant.a
libambulant_ffmpeg=$(ambulantlibdir)/libambulant_ffmpeg.a
libambulant_sdl=$(ambulantlibdir)/libambulant_sdl.a
endif

FIREFOX=firefox
ARCH=`arch`
INSTALLER_ARCHIVE=npambulant-$(VERSION)-linux-$(ARCH).xpi

if WITH_CG
NPAMBULANT=libnpambulant.so
NPAMBULANT_PLUGIN=npambulant.plugin
MOZILLA_PLUGINS="/Library/Internet Plug-Ins"
mainloop=cg_mainloop.cpp
ambulant_LDADD=$(libambulant_core) $(EXPAT_LIBS) 
extra_cg_DEFINES=-DXP_MACOSX
extra_cg_LDADD=-L$(ambulantlibdir) -lambulant_cg
else
NPAMBULANT=libnpambulant.so.0.0.0
MOZILLA_PLUGINS=mozilla/plugins
ambulant_LDADD=$(libambulant_core)
mainloop=
extra_cg_DEFINES=
extra_cg_LDADD=
endif
PYTHON=python
#
# Various compile/link flags for optional features
#

#------------------------------------------------------
if WITH_GTK
extra_gtk_DEFINES=-DWITH_GTK -DMOZ_X11 $(GTK_CFLAGS)
extra_gtk_LDADD=../player_gtk/gtk_mainloop.o -L$(ambulantlibdir) -lambulant_gtk $(GTK_LIBS)
else
extra_gtk_DEFINES=
extra_gtk_LDADD=
endif
#------------------------------------------------------
if WITH_FFMPEG
extra_ffmpeg_DEFINES=-DWITH_FFMPEG $(FFMPEG_CFLAGS)
if WITH_CG
extra_ffmpeg_LDADD=-lambulant_ffmpeg $(FFMPEG_LIBS)
else
extra_ffmpeg_LDADD=$(libambulant_ffmpeg) $(FFMPEG_LIBS)
endif
else
extra_ffmpeg_DEFINES=
extra_ffmpeg_LDADD=
endif
#------------------------------------------------------
if WITH_SDL
extra_sdl_DEFINES=$(SDL_CFLAGS) -DWITH_SDL
extra_sdl_LDADD=$(libambulant_sdl) $(SDL_LIBS)
else
extra_sdl_DEFINES=
extra_sdl_LDADD=
endif
#------------------------------------------------------
if WITH_XERCES_BUILTIN
extra_xerces_DEFINES=$(XERCES_CFLAGS) -DWITH_XERCES_BUILTIN
extra_xerces_LDADD=$(XERCES_LIBS)
else
extra_xerces_DEFINES=
extra_xerces_LDADD=
endif
#------------------------------------------------------
#
# Bringing all the optional flags together
#
AM_CPPFLAGS = \
	$(extra_cg_DEFINES)	\
	$(extra_gtk_DEFINES)	\
	$(extra_ffmpeg_DEFINES)	\
	$(extra_sdl_DEFINES)	\
	$(extra_xerces_DEFINES)	\
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/include/ambulant/gui \
	-I$(top_srcdir)/src/player_gtk \
	-I$(top_srcdir)/src/npambulant \
	$(NPAMBULANT_INCLUDES) \
	$(INCLTDL)

lib_LTLIBRARIES = libnpambulant.la
libnpambulant_la_SOURCES=\
	np_entry.cpp \
	npn_gate.cpp \
	npp_gate.cpp \
	ScriptablePluginObjectBase.h \
	ConstructablePluginObject.h \
	ScriptablePluginObject.h \
	ScriptablePluginObjectBase.cpp \
	ScriptablePluginObject.cpp \
	npambulant.h \
	npambulant.cpp \
	$(mainloop)

libnpambulant_la_LDFLAGS=	\
	-module -shared	-export-dynamic	\
	$(LIBLTDL)		\
	$(extra_cg_LDADD)	\
	$(extra_gtk_LDADD)	\
	$(extra_ffmpeg_LDADD)	\
	$(extra_sdl_LDADD)	\
	$(extra_xerces_LDADD)	\
	$(ambulant_LDADD)	\
	-o $(NPAMBULANT)

#------------------------------------------------------
#
# These targets are magic for automake
#
all-am: build-npambulant

## Disabled: the test subdirectory doesn't exist...
##check-am: check-npambulant

install-exec-hook: install-npambulant

if WITH_CG
mostlyclean-local:
	rm -fr .libs *o $(NPAMBULANT) $(NPAMBULANT_PLUGIN)

uninstall-local:
	-rm -rf $(MOZILLA_PLUGINS)/$(NPAMBULANT_PLUGIN)
else
mostlyclean-local:
	-rm -fr .libs *o $(NPAMBULANT) npambulant_plugin_dir
	-rm -fr mozilla npambulant

uninstall-local:
	if [ `id -u` == "0" ]; then MOZILLA_PLUGINS=/usr/lib/$(MOZILLA_PLUGINS); else MOZILLA_PLUGINS=~/.$(MOZILLA_PLUGINS);fi; \
	rm -fr $$MOZILLA_PLUGINS/npambulant.so $$MOZILLA_PLUGINS/npambulant;
endif

#
# To build the shared library we want we should build the libtool library.
# You know this, and I know this, but "make" doesn't, so we tell it here:
#
.libs/$(NPAMBULANT): $(lib_LTLIBRARIES)

#------------------------------------------------------
#
# Test, check and debug section.
# Variables determining how things are run
#

DEBUGGER=
#DEBUGGER=gdb --args
FIREFOXARGS=
#FIREFOXARGS=-i
FIREFOXENV=

BUILDLIBDIR=
check-npambulant:
	if test x$$DISPLAY = x; then \
		echo Skipping tests, DISPLAY not set; \
	else \
		a=`cd $(ambulantlibdir); pwd` ; \
		cd $(srcdir) ; \
		LD_LIBRARY_PATH=-L"$$a" $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) `pwd`/test/test_npambulant.html ; \
	fi

testplay:
	cd $(srcdir) ; $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) test/test_npambulant.html

#------------------------------------------------------
#
# Create the structure that will eventually end up in the Firefox
# plugins directory.
#
if WITH_CG
build-npambulant: $(NPAMBULANT_PLUGIN)
$(NPAMBULANT_PLUGIN): .libs/$(NPAMBULANT) $(srcdir)/Info.plist
	-rm -fr $(NPAMBULANT_PLUGIN)

#	Install boilerplate, Info.plist, etc.
	mkdir $(NPAMBULANT_PLUGIN)
	mkdir $(NPAMBULANT_PLUGIN)/Contents
	cp $(srcdir)/Info.plist $(NPAMBULANT_PLUGIN)/Contents

#	Install npambulant plugin binary
	-mkdir $(NPAMBULANT_PLUGIN)/Contents/MacOS
	cp .libs/$(NPAMBULANT) $(NPAMBULANT_PLUGIN)/Contents/MacOS/$(NPAMBULANT)

#	Install ambulant core libraries

#	Install plugins for ambulant engine
	-mkdir $(NPAMBULANT_PLUGIN)/Contents/PlugIns
	pkglibdir=`pwd`/$(NPAMBULANT_PLUGIN)/Contents/PlugIns; \
	cd ../plugins; \
	$(MAKE) $(AM_MAKEFLAGS) pkglibdir=$$pkglibdir install 

#	Install gettext localizations
	localedir=`pwd`/$(BUILDAPPNAME).app/Contents/Resources/locale; \
	cd ../../po ; \
	$(MAKE) $(AM_MAKEFLAGS) localedir=$$localedir install

install-npambulant: $(NPAMBULANT_PLUGIN)
	-rm -fr $(MOZILLA_PLUGINS)/$(NPAMBULANT_PLUGIN)
	cp -R $(NPAMBULANT_PLUGIN) $(MOZILLA_PLUGINS)/$(NPAMBULANT_PLUGIN)
	-mkdir $(NPAMBULANT_PLUGIN)/Contents/MacOS
	cp .libs/$(NPAMBULANT) $(NPAMBULANT_PLUGIN)/Contents/MacOS/$(NPAMBULANT)

#	Install ambulant core libraries

#	Install plugins for ambulant engine
	-mkdir $(NPAMBULANT_PLUGIN)/Contents/PlugIns
	pkglibdir=`pwd`/$(NPAMBULANT_PLUGIN)/Contents/PlugIns; \
	cd ../plugins; \
	$(MAKE) $(AM_MAKEFLAGS) pkglibdir=$$pkglibdir install 

#	Copy in shared libraries and fix rpath
	DYLD_LIBRARY_PATH=$(top_builddir)/src/libambulant/.libs:$(top_builddir)/third_party_packages/installed/lib \
		$(PYTHON) $(top_srcdir)/scripts/internalize_dylibs.py -v\
		$(NPAMBULANT_PLUGIN)
#	Check that internalization worked
	$(PYTHON) $(top_srcdir)/scripts/internalize_dylibs.py -c $(NPAMBULANT_PLUGIN)
	strip  npambulant_plugin_dir/plugins/npambulant/*.so*

#	Install gettext localizations
	localedir=`pwd`/$(BUILDAPPNAME).app/Contents/Resources/locale; \
	cd ../../po ; \
	$(MAKE) $(AM_MAKEFLAGS) localedir=$$localedir install

else
build-npambulant: npambulant_plugin_dir
npambulant_plugin_dir: .libs/$(NPAMBULANT)
#	Create directory structure
	-rm -fr npambulant_plugin_dir
	mkdir npambulant_plugin_dir
	mkdir npambulant_plugin_dir/plugins
	mkdir npambulant_plugin_dir/plugins/npambulant
	mkdir npambulant_plugin_dir/plugins/npambulant/plugins

#	Copy browser plugin
	cp .libs/$(NPAMBULANT) npambulant_plugin_dir/plugins/npambulant.so
	strip npambulant_plugin_dir/plugins/npambulant.so

#	Install plugins for ambulant engine
		pkglibdir=`pwd`/npambulant_plugin_dir/plugins/npambulant/plugins; \
		cd ../plugins; \
		cp .libs/*.{so.0,la} $$pkglibdir

#	Copy in shared libraries and fix rpath
#	Ubuntu 11.04 32/64 bit
	LD_LIBRARY_PATH=$(top_builddir)/src/libambulant/.libs:$(top_builddir)/third_party_packages/installed/lib \
	$(PYTHON) $(top_srcdir)/scripts/internalize_shlibs.py \
		npambulant_plugin_dir/plugins
#	Check that internalization worked
	$(PYTHON) $(top_srcdir)/scripts/internalize_shlibs.py -c npambulant_plugin_dir/plugins
#	Copy manifest file (only used for xpt distributions)
	@case $(ARCH) in		    \
		x86_64|X86_64)      \
		 	cp $(srcdir)/install-linux64.rdf npambulant_plugin_dir/install.rdf ;; \
		i386|i686)			\
			cp $(srcdir)/install-linux.rdf npambulant_plugin_dir/install.rdf ;;	\
	esac
	cp $(srcdir)/manifest.json npambulant_plugin_dir/manifest.json
	strip  npambulant_plugin_dir/plugins/npambulant/*.so*

install-npambulant:  npambulant_plugin_dir
	if [ `id -u` == "0" ]; then \
		MOZILLA_PLUGINS=/usr/lib/$(MOZILLA_PLUGINS); \
	else \
		MOZILLA_PLUGINS=~/.$(MOZILLA_PLUGINS);\
	fi; \
	$(INSTALL) -d $(MOZILLA_PLUGINS);	\
	$(INSTALL_PROGRAM) npambulant_plugin_dir/plugins/npambulant.so $$MOZILLA_PLUGINS/npambulant.so;	\
	(cd  npambulant_plugin_dir/plugins ; tar cf - npambulant  ) | \
		(cd $$MOZILLA_PLUGINS ; tar xf -)
endif
#------------------------------------------------------
#
# Build the .xpi file which can be fed to Firefox to install the plugin
# and when Google Chrome browser is installed, the .crx for that browser as well
#
installer: $(INSTALLER_ARCHIVE)

$(INSTALLER_ARCHIVE): npambulant_plugin_dir
	-rm -f $(INSTALLER_ARCHIVE)
	-rm -f npambulant-$(VERSION)-linux-$(ARCH).crx
	cd npambulant_plugin_dir; \
		zip -r ../$(INSTALLER_ARCHIVE) *

signed_installer: installer
	codesign -s "Developer ID Application" -f --deep -v $(INSTALLER_ARCHIVE)
#	codesign -s "Stichting CWI" -f --deep -v $(INSTALLER_ARCHIVE)

npambulant-$(VERSION)-linux-$(ARCH).crx: npambulant_plugin_dir
	which google-chrome; \
	if test $$?; then cd ..; \
		rm -f  npambulant_plugin_dir.pem ; \
		Xvfb :829 & DISPLAY=:829 google-chrome --enable-apps --pack-extension=$$PWD/npambulant_plugin_dir; killall Xvfb; \
	fi
#	-rm -f /tmp/.X829
	-mv -f npambulant_plugin_dir.crx npambulant-$(VERSION)-linux-$(ARCH).crx
#------------------------------------------------------

EXTRA_DIST = \
	install-linux.rdf \
	install-linux64.rdf \
	install-win32.rdf  \
	manifest.json \
	npambulant.rc \
	npambulant.def \
	resource.h \
	cg_mainloop.h \
	Info.plist \
	README-linux \
	README-win32.txt
