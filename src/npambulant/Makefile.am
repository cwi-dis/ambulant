#
#
FIREFOX=firefox
NPAMBULANT=libnpambulant.0.0.0
MOZILLA_PLUGINS=~/.mozilla/plugins

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
libnpambulant_la_SOURCES=np_entry.cpp npn_gate.cpp npp_gate.cpp plugin.cpp nsScriptablePeer.cpp
nodist_libnpambulant_la_INCLUDES=npambulant.h

libnpambulant_la_LDFLAGS=	\
	-module -shared		\
	$(LIBLTDL)		\
	$(extra_gtk_ldflags)	\
	$(extra_ffmpeg_ldflags)	\
	$(extra_sdl_ldflags)	\
	$(extra_xerces_ldflags)	\
	$(extra_live_ldflags)	\
	$(ambulant_ldflags)	\
	-o $(NPAMBULANT)

$(libnpambulant_la_OBJECTS): npambulant.h	npambulant.xpt
build-npambulant: $(PLUGIN_FILES) plugin.h npambulant.h
	AM_LDFLAGS="-o libnpambulant.so"

npambulant.h: $(srcdir)/npambulant.idl
	$(XPIDL) -m header  -w -I$(GECKO_ROOT)/idl $(srcdir)/npambulant.idl

npambulant.xpt: $(srcdir)/npambulant.idl
	$(XPIDL) -m typelib -w -I$(GECKO_ROOT)/idl $(srcdir)/npambulant.idl

check-npambulant:
	( a=`cd $(ambulantlibdir); pwd` ; \
	  cd $(srcdir) ; \
	  LD_LIBRARY_PATH=-L"$$a" $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) test/test_npambulant.html ; \
	)

install-npambulant: npambulant.xpt .libs/$(NPAMBULANT)
	$(INSTALL) -d $(MOZILLA_PLUGINS)
	$(INSTALL_PROGRAM) .libs/$(NPAMBULANT) $(MOZILLA_PLUGINS)/npambulant.so
	$(INSTALL_DATA) npambulant.xpt $(MOZILLA_PLUGINS)/npambulant.xpt
# when selinux is enabled, identify the installed shared library 
	./selinux_patch.sh $(MOZILLA_PLUGINS)/npambulant.so
	
clean-npambulant:
	rm -fr $(MOZILLA_PLUGINS)/npambulant.so
	rm -fr $(MOZILLA_PLUGINS)/npambulant.xpt

mostlyclean-am:
	rm -fr .libs *o $(NPAMBULANT)
	rm -fr npambulant.h npambulant.xpt

# Debug target

testplay:
	cd $(srcdir) ; $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) test/test_npambulant.html
	
