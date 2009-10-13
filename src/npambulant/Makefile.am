#------------------------------------------------------
FIREFOX=firefox
ambulantlibdir=$(top_builddir)/src/libambulant/.libs

if WITH_CG
NPAMBULANT=libnpambulant.so
NPAMBULANT_PLUGIN=npambulant.plugin
MOZILLA_PLUGINS="/Library/Internet Plug-Ins"
mainloop=cg_mainloop.cpp
ambulant_LDADD=\
	$(ambulantlibdir)/libambulant.a \
	$(EXPAT_LIBS) 
cgdefines=-DXP_MACOSX
else
NPAMBULANT=libnpambulant.so.0.0.0
MOZILLA_PLUGINS=mozilla/plugins
ambulant_LDADD=-L$(ambulantlibdir) -lambulant
mainloop=
cgdefines=
endif




# Various flags for optional features
#------------------------------------------------------
if WITH_GTK
extra_gtk_DEFINES=-DWITH_GTK -DMOZ_X11 $(GTK_CFLAGS)
extra_gtk_LDADD=../player_gtk/gtk_mainloop.o -lambulant_gtk $(GTK_LIBS)
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
extra_ffmpeg_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_ffmpeg.a $(FFMPEG_LIBS)
endif
else
extra_ffmpeg_DEFINES=
extra_ffmpeg_LDADD=
endif
#------------------------------------------------------
if WITH_LIVE
extra_live_DEFINES=$(LIVE_CFLAGS) -DWITH_LIVE
extra_live_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_live.a $(LIVE_LIBS)
else
extra_live_DEFINES=
extra_live_LDADD =
endif
#------------------------------------------------------
if WITH_SDL
extra_sdl_DEFINES=$(SDL_CFLAGS) -DWITH_SDL
extra_sdl_LDADD=$(top_builddir)/src/libambulant/.libs/libambulant_sdl.a $(SDL_LIBS)
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

# KB temp MOZ_X11
# XXXJACK AM_CXXFLAGS=$(extra_gtk_cflags)
AM_CPPFLAGS = \
	$(cgdefines) \
	$(extra_gtk_DEFINES)	\
	$(extra_ffmpeg_DEFINES)	\
	$(extra_sdl_DEFINES)	\
	$(extra_xerces_DEFINES)	\
	$(extra_live_DEFINES)	\
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/include/ambulant/gui \
	-I$(top_srcdir)/src/player_gtk \
	-I$(XULRUNNER_ROOT)/sdk/include \
	$(INCLTDL)

# XXXJACK removed:
##	-I$(XULRUNNER_ROOT)/include/xulrunner-sdk-1.9/plugin \
##	-I$(XULRUNNER_ROOT)/include/xulrunner-sdk-1.9/system_wrappers \
##	-I$(XULRUNNER_ROOT)/include/nspr4 \
##	-I$(XULRUNNER_ROOT)/include/xulrunner-sdk-1.9/java  \
##	-I$(XULRUNNER_ROOT)/include/xulrunner-sdk-1.9/xpcom \
##	-I$(XULRUNNER_ROOT)/include/plugin \
##	-I$(XULRUNNER_ROOT)/include/system_wrappers \
##	-I$(XULRUNNER_ROOT)/include/nspr  \
##	-I$(XULRUNNER_ROOT)/include/java  \
##	-I$(XULRUNNER_ROOT)/include/xpcom \

# These targets are magic for automake

all-am: build-npambulant

check-am: check-npambulant

install-exec-hook: install-npambulant

uninstall-local: clean-npambulant

# Set these variables to change the way the tests are run
DEBUGGER=
#DEBUGGER=gdb --args
FIREFOXARGS=
#FIREFOXARGS=-i
FIREFOXENV=

BUILDLIBDIR=
lib_LTLIBRARIES = libnpambulant.la
libnpambulant_la_SOURCES=\
	np_entry.cpp \
	npn_gate.cpp \
	npp_gate.cpp \
	ScriptablePluginObjectBase.cpp \
	ScriptablePluginObject.cpp \
	npambulant.cpp \
	$(mainloop)
	
nodist_libnpambulant_la_INCLUDES=npambulant.h

libnpambulant_la_LDFLAGS=	\
	-module -shared		\
	$(LIBLTDL)		\
	$(extra_gtk_LDADD)	\
	$(extra_ffmpeg_LDADD)	\
	$(extra_sdl_LDADD)	\
	$(extra_xerces_LDADD)	\
	$(extra_live_LDADD)	\
	$(ambulant_LDADD)	\
	-o $(NPAMBULANT)

build-npambulant: $(PLUGIN_FILES)
	AM_LDFLAGS="-o libnpambulant.so"

check-npambulant:
	( a=`cd $(ambulantlibdir); pwd` ; \
	  cd $(srcdir) ; \
	  LD_LIBRARY_PATH=-L"$$a" $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) test/test_npambulant.html ; \
	)

if WITH_CG
$(NPAMBULANT_PLUGIN): .libs/$(NPAMBULANT) $(srcdir)/Info.plist
	-rm -fr $(NPAMBULANT_PLUGIN)

#	Install boilerplate, Info.plist, etc.
	mkdir $(NPAMBULANT_PLUGIN)
	mkdir $(NPAMBULANT_PLUGIN)/Contents
	cp $(srcdir)/Info.plist $(NPAMBULANT_PLUGIN)/Contents
	
#	Install npambulant plugin binary
	mkdir $(NPAMBULANT_PLUGIN)/Contents/MacOS
	cp .libs/$(NPAMBULANT) $(NPAMBULANT_PLUGIN)/Contents/MacOS/$(NPAMBULANT)
	
#	Install ambulant core libraries

#	Install plugins for ambulant engine
	mkdir $(NPAMBULANT_PLUGIN)/Contents/PlugIns
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
	
else

install-npambulant:  .libs/$(NPAMBULANT)
	if [ `id -u` == "0" ]; then MOZILLA_PLUGINS=/usr/lib/$(MOZILLA_PLUGINS); else MOZILLA_PLUGINS=~/.$(MOZILLA_PLUGINS);fi; \
	$(INSTALL) -d $(MOZILLA_PLUGINS);	\
	$(INSTALL_PROGRAM) .libs/$(NPAMBULANT) $$MOZILLA_PLUGINS/npambulant.so;	\
	./selinux_patch.sh $$MOZILLA_PLUGINS/npambulant.so
# when selinux is enabled, identify the installed shared library
endif

if WITH_CG
clean-npambulant:
	-rm -rf $(NPAMBULANT_PLUGIN)
	-rm -rf $(MOZILLA_PLUGINS)/$(NPAMBULANT_PLUGIN)
else
clean-npambulant:
	if [ `id -u` == "0" ]; then MOZILLA_PLUGINS=/usr/lib/$(MOZILLA_PLUGINS); else MOZILLA_PLUGINS=~/.$(MOZILLA_PLUGINS);fi; \
	rm -fr $$MOZILLA_PLUGINS/npambulant.so;
endif

mostlyclean-local:
	rm -fr .libs *o $(NPAMBULANT)

# Debug target

testplay:
	cd $(srcdir) ; $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) test/test_npambulant.html
