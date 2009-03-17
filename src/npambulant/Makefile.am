#------------------------------------------------------
FIREFOX=firefox
NPAMBULANT=libnpambulant.0.0.0
MOZILLA_PLUGINS=mozilla/plugins

ambulantlibdir=$(top_builddir)/src/libambulant/.libs

ambulant_ldflags=-L$(ambulantlibdir) -lambulant

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
extra_live_ldflags=$(top_builddir)/src/libambulant/.libs/libambulant_live.a \
			  -L$(LIVE_ROOT)/liveMedia/ -lliveMedia \
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
AM_CPPFLAGS = \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/include/ambulant/gui \
	-I$(top_srcdir)/src/player_gtk \
	$(gtkincludes) \
	-I$(XULRUNNER_ROOT)/include/xulrunner-sdk-1.9/plugin \
	-I$(XULRUNNER_ROOT)/include/xulrunner-sdk-1.9/system_wrappers \
	-I$(XULRUNNER_ROOT)/include/nspr4 \
	-I$(XULRUNNER_ROOT)/include/xulrunner-sdk-1.9/java  \
	-I$(XULRUNNER_ROOT)/include/xulrunner-sdk-1.9/xpcom \
	-I$(XULRUNNER_ROOT)/include/plugin \
	-I$(XULRUNNER_ROOT)/include/system_wrappers \
	-I$(XULRUNNER_ROOT)/include/nspr  \
	-I$(XULRUNNER_ROOT)/include/java  \
	-I$(XULRUNNER_ROOT)/include/xpcom \
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
libnpambulant_la_SOURCES=np_entry.cpp npn_gate.cpp npp_gate.cpp ScriptablePluginObjectBase.cpp ScriptablePluginObject.cpp npambulant.cpp
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

build-npambulant: $(PLUGIN_FILES)
	AM_LDFLAGS="-o libnpambulant.so"

check-npambulant:
	( a=`cd $(ambulantlibdir); pwd` ; \
	  cd $(srcdir) ; \
	  LD_LIBRARY_PATH=-L"$$a" $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) test/test_npambulant.html ; \
	)

install-npambulant:  .libs/$(NPAMBULANT)
	if [ `id -u` == "0" ]; then MOZILLA_PLUGINS=/usr/lib/$(MOZILLA_PLUGINS); else MOZILLA_PLUGINS=~/.$(MOZILLA_PLUGINS);fi; \
	$(INSTALL) -d $(MOZILLA_PLUGINS);	\
	$(INSTALL_PROGRAM) .libs/$(NPAMBULANT) $$MOZILLA_PLUGINS/npambulant.so;	\
	./selinux_patch.sh $$MOZILLA_PLUGINS/npambulant.so
# when selinux is enabled, identify the installed shared library


clean-npambulant:
	if [ `id -u` == "0" ]; then MOZILLA_PLUGINS=/usr/lib/$(MOZILLA_PLUGINS); else MOZILLA_PLUGINS=~/.$(MOZILLA_PLUGINS);fi; \
	rm -fr $$MOZILLA_PLUGINS/npambulant.so;

mostlyclean-local:
	rm -fr .libs *o $(NPAMBULANT)

# Debug target

testplay:
	cd $(srcdir) ; $(FIREFOXENV) FIREFOXPATH=$(BUILDLIBDIR) $(DEBUGGER) $(FIREFOX) $(FIREFOXARGS) test/test_npambulant.html
