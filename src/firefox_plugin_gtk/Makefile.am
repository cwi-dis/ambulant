#
#
FIREFOX=firefox
FIREFOX_PLUGIN=libAmbulantPlugin.so

ambulantlibdir=$(top_builddir)/src/libambulant/.libs

ambulant_ldflags=-L$(ambulantlibdir) -lambulant

EXTRA_DIST=plugin.h plugin.cpp \
    ambulantmodule.h ambulantmodule.cpp \
    ambulantinterface.h ambulantinterface.cpp
    
# Various flags for optional features
#------------------------------------------------------
if WITH_GTK
gtkincludes=-I$(GTK_CFLAGS)
extra_gtk_cflags=-DWITH_GTK -DAMBULANT_FIREFOX_PLUGIN
extra_gtk_ldflags=$(GTK_LIBS)
#-lambulant_gtk -lgtk -lpango
else
gtkincludes=
extra_gtk_cflags=
extra_gtk_ldflags=
endif
#------------------------------------------------------
if WITH_FFMPEG
extra_ffmpeg_ldflags=-lambulant_ffmpeg $(FFMPEG_LIBS)
else
extra_ffmpeg_ldflags=
endif
#------------------------------------------------------
if WITH_LIVE
livelibs = -L$(LIVE_ROOT)/liveMedia/ -lliveMedia \
			  -L$(LIVE_ROOT)/BasicUsageEnvironment/ -lBasicUsageEnvironment  \
			  -L$(LIVE_ROOT)/groupsock/ -lgroupsock  \
			  -L$(LIVE_ROOT)/UsageEnvironment/ -lUsageEnvironment
extra_live_ldflags=
else
livelibs =
extra_live_ldflags=
endif # WITH_LIVE
#------------------------------------------------------
if WITH_SDL
sdl_config=`sdl-config --libs`
extra_sdl_ldflags=-lambulant_sdl $(sdl_config)
else
extra_sdl_ldflags=
endif
#------------------------------------------------------
if WITH_XERCES_BUILTIN
extra_xerces_ldflags=-L$(XERCES_PREFIX)/lib -lxerces-c
else
extra_xerces_ldflags=
endif # WITH_XERCES_BUILTIN
#------------------------------------------------------
#KB hack
extra_hack1_ldflags=../player_gtk/gtk_mainloop.o

EXTRA_CFLAGS=$(extra_gtk_cflags)
# KB temp MOZ_X11
AM_CXXFLAGS = -export-dynamic -DXP_UNIX -DMOZ_X11 -DAMBULANT_FIREFOX_PLUGIN
#EXTRA_CXXFLAGS=-DXP_UNIX -DMOZ_X11
#EXTRA_LDFLAGS=$(extra_gtk_ldflags)
# KB mozilla trunk: java js nspr plugin xpcom
INCLUDES = \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/include/ambulant/gui \
	-I$(top_srcdir)/src/player_gtk \
	$(gtkincludes) \
	-I$(GECKO_ROOT)/include \
	-I$(GECKO_ROOT)/include/java \
	-I$(GECKO_ROOT)/include/js \
	-I$(GECKO_ROOT)/include/nspr \
	-I$(GECKO_ROOT)/include/plugin \
	-I$(GECKO_ROOT)/include/xpcom \
	$(INCLTDL)

# These targets are magic for automake

all-am: build-firefox-plugin

check-am: check-firefox-plugin

install-exec-hook: install-firefox-plugin

uninstall-local: clean-firefox-plugin

# Set these variables to change the way the tests are run
DEBUGGER=
#DEBUGGER=gdb --args
FIREFOXARGS=
#FIREFOXARGS=-i
FIREFOXENV=

BUILDLIBDIR=
lib_LTLIBRARIES = libambulantplugin.la
libambulantplugin_la_SOURCES=np_entry.cpp npn_gate.cpp npp_gate.cpp npplat.h pluginbase.h plugin.cpp plugin.h nsScriptablePeer.cpp  nsScriptablePeer.h

libambulantplugin_la_LDFLAGS=	\
	-shared $(LIBLTDL)	\
	$(extra_hack1_ldflags)	\
	 -lambulant_gtk \
	$(extra_gtk_ldflags)	\
	$(extra_ffmpeg_ldflags)	\
	$(extra_sdl_ldflags)	\
	$(extra_xerces_ldflags)	\
	$(extra_live_ldflags)	\
	$(ambulant_ldflags) \
	-o $(FIREFOX_PLUGIN)

	
build-firefox-plugin: $(PLUGIN_FILES) $(srcdir)/plugin.h
	AM_LDFLAGS="-o libambulantplugin.so"

check-firefox-plugin:
	( a=`cd $(ambulantlibdir); pwd` ; \
	  cd $(srcdir) ; \
	  LD_LIBRARY_PATH=-L"$$a" $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) test/test_ambulantplugin.html ; \
	)

install-firefox-plugin:
	cd $(srcdir) ; nsinstall  .libs/$(FIREFOX_PLUGIN) install_dir/$(FIREFOX_PLUGIN).so
	
mostlyclean-am:
	-cd $(srcdir) ; rm -fr .libs *o $(FIREFOX_PLUGIN)

# Debug target

testplay:
	cd $(srcdir) ; $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) test/test_ambulantplugin.html
	
