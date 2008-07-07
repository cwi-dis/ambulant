#
#
FIREFOX=firefox
FIREFOX_PLUGIN=libambulantplugin.0.0.0
FIREFOX_INSTALLED_PLUGINS=~/.mozilla/plugins

ambulantlibdir=$(top_builddir)/src/libambulant/.libs

ambulant_ldflags=-L$(ambulantlibdir) -lambulant

EXTRA_DIST=plugin.h plugin.cpp \
    ambulantmodule.h ambulantmodule.cpp \
    ambulantinterface.h ambulantinterface.cpp
    
# Various flags for optional features
#------------------------------------------------------
if WITH_GTK
gtkincludes=-I$(GTK_CFLAGS)
extra_gtk_cflags=-DWITH_GTK -DMOZ_X11
extra_gtk_ldflags=../player_gtk/gtk_mainloop.o -lambulant_gtk $(GTK_LIBS)
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
extra_live_ldflags= -L$(LIVE_ROOT)/liveMedia/ -lliveMedia \
			  -L$(LIVE_ROOT)/BasicUsageEnvironment/ -lBasicUsageEnvironment  \
			  -L$(LIVE_ROOT)/groupsock/ -lgroupsock  \
			  -L$(LIVE_ROOT)/UsageEnvironment/ -lUsageEnvironment
else
livelibs =
extra_live_ldflags=
endif # WITH_LIVE
#------------------------------------------------------
if WITH_SDL
sdl_config=`sdl-config --libs`
extra_sdl_ldflags=-lambulant_sdl $(sdl_config)
#extra_sdl_ldflags=-lambulant_sdl $(sdl_config) \
#	-Wl,-framework,Carbon \
#	-Wl,-framework,IOKit \
#	-Wl,-framework,OpenGL \
#	-Wl,-framework,AudioUnit \
#	-Wl,-framework,AudioToolbox \
#	-Wl,-framework,QuickTime
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

#EXTRA_CFLAGS=$(extra_gtk_cflags)
# KB temp MOZ_X11
AM_CXXFLAGS=$(extra_gtk_cflags)
#EXTRA_CXXFLAGS=$(extra_gtk_cflags)
#EXTRA_LDFLAGS=$(extra_gtk_ldflags)
INCLUDES = \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/include/ambulant/gui \
	-I$(top_srcdir)/src/player_gtk \
	$(gtkincludes) \
	-I$(GECKO_ROOT)/include \
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
libambulantplugin_la_SOURCES=np_entry.cpp npn_gate.cpp npp_gate.cpp plugin.cpp nsScriptablePeer.cpp
nodist_libambulantplugin_la_INCLUDES=nsIAmbulantPlugin.h

libambulantplugin_la_LDFLAGS=	\
	-module \
	$(LIBLTDL)	\
	$(extra_gtk_ldflags)	\
	$(extra_ffmpeg_ldflags)	\
	$(extra_sdl_ldflags)	\
	$(extra_xerces_ldflags)	\
	$(extra_live_ldflags)	\
	$(ambulant_ldflags) \
	-o $(FIREFOX_PLUGIN)

$(libambulantplugin_la_OBJECTS): nsIAmbulantPlugin.h	nsIAmbulantPlugin.xpt
build-firefox-plugin: $(PLUGIN_FILES) plugin.h nsIAmbulantPlugin.h
	AM_LDFLAGS="-o libambulantplugin.so"

nsIAmbulantPlugin.h: $(srcdir)/nsIAmbulantPlugin.idl
	$(XPIDL) -m header  -w -I$(GECKO_ROOT)/idl $(srcdir)/nsIAmbulantPlugin.idl

nsIAmbulantPlugin.xpt: $(srcdir)/nsIAmbulantPlugin.idl
	$(XPIDL) -m typelib -w -I$(GECKO_ROOT)/idl $(srcdir)/nsIAmbulantPlugin.idl

check-firefox-plugin:
	( a=`cd $(ambulantlibdir); pwd` ; \
	  cd $(srcdir) ; \
	  LD_LIBRARY_PATH=-L"$$a" $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) test/test_ambulantplugin.html ; \
	)

install-firefox-plugin: nsIAmbulantPlugin.xpt .libs/$(FIREFOX_PLUGIN)
	$(INSTALL) -d $(FIREFOX_INSTALLED_PLUGINS)
	$(INSTALL_PROGRAM) .libs/$(FIREFOX_PLUGIN) $(FIREFOX_INSTALLED_PLUGINS)/nsIAmbulantPlugin.so
	$(INSTALL_DATA) nsIAmbulantPlugin.xpt $(FIREFOX_INSTALLED_PLUGINS)/nsIAmbulantPlugin.xpt
	
mostlyclean-am:
	rm -fr .libs *o $(FIREFOX_PLUGIN)
	rm -fr nsIAmbulantPlugin.h nsIAmbulantPlugin.xpt

# Debug target

testplay:
	cd $(srcdir) ; $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) test/test_ambulantplugin.html
	
