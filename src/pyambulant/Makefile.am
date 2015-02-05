#
# This automake file is quite a bit of a hack, mainly because (a) automake
# and Python distutils aren't very friendly to each other and (b) distutils
# does not like to be run outside of the source directory.
#
# (a) is solved by pretty much ignoring automake and using the all-am and related
# targets to depend on targets that build and install through distutils.
# (b) is solved by running setup.py in the source directory. This should also
# work for building for multiple platforms, because distutils has its own
# platform structure under its build directory (but it will not work for building
# with multiple pythons, etc).
#

ambulantlibdir=$(top_builddir)/src/libambulant/.libs

DESTDIR=/

EXTRA_DIST=setup.py \
	bgenCxxSupport.py bgenBackSupport.py \
	ambulantscan.py ambulantsupport.py genobjects.py \
	ambulantutilities.h ambulantutilities.cpp \
	ambulantmodule.h ambulantmodule.cpp \
	ambulantinterface.h ambulantinterface.cpp \
	player_pygtk test

ambulant_lib=libambulant.so

# Various flags for optional features

if WITH_GTK
extra_gtk_cflags=-DWITH_GTK=1 $(GTK_CFLAGS) `pkg-config --cflags pygtk-2.0`
extra_gtk_ldflags=-lambulant_gtk $(GTK_LIBS) `pkg-config --libs pygtk-2.0`
else
extra_gtk_cflags=
extra_gtk_ldflags=
endif
if WITH_FFMPEG
extra_ffmpeg_cflags=-DWITH_FFMPEG=1
extra_ffmpeg_ldflags=-lambulant_ffmpeg $(FFMPEG_LIBS)
else
extra_ffmpeg_ldflags=
endif
if WITH_SDL
extra_sdl_cflags=-DWITH_SDL=1 $(SDL_CFLAGS)
extra_sdl_ldflags=-lambulant_sdl $(SDL_LIBS)
else
extra_sdl_cflags=
extra_sdl_ldflags=
endif

# Get all the extra bits together
extra_cflags=$(extra_gtk_cflags) $(extra_ffmpeg_cflags) $(extra_sdl_cflags)
extra_ldflags=$(extra_gtk_ldflags) $(extra_ffmpeg_ldflags) $(extra_sdl_ldflags) $(LIBINTL)
all_ldflags=-L$$a $(extra_ldflags) $(LDFLAGS) $(ambulantlib)
all_cflags=$(extra_cflags) $(DEFS) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS)

# These targets are magic for automake

all-am: build-pyambulant

check-am: check-pyambulant

install-exec-hook: install-pyambulant

uninstall-local:
#XXXX quick fix to make distcheck work from nightly build
	-rm -fr  $(DESTDIR)/$(prefix)/lib64/python/ambulant.so $(DESTDIR)/$(prefix)/lib64/python/ambulant-2.6*.egg-info
	-rm -fr  $(DESTDIR)/$(prefix)/lib/python/ambulant.so $(DESTDIR)/$(prefix)/lib/python/ambulant-2.6*.egg-info
	echo Cannot uninstall pyambulant module, please remove manually $(DESTDIR)/$(prefix)

# This target re-creates the cpp files. Use only when you know what you
# are doing.

regen: regenerate-pyambulant

BGENADDITIONS=$(srcdir)/bgenCxxSupport.py $(srcdir)/bgenBackSupport.py
BGENOUTPUT=$(srcdir)/ambulantmodule.cpp $(srcdir)/ambulantinterface.cpp
BGENTEMP=$(srcdir)/ambulantgen.py
BGENINPUT=$(srcdir)/ambulantscan.py $(srcdir)/ambulantsupport.py
GENOBJOUTPUT=$(srcdir)/ambulantobjgen.py $(srcdir)/ambulantincludegen.py $(srcdir)/ambulantmodule.h

# Set these variables to change the way the tests are run
DEBUGGER=
#DEBUGGER=gdb --args
PYTHONARGS=
#PYTHONARGS=-i
PYTHONENV=
#PYTHONENV=DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib

BUILDLIBDIR=`echo ./build/lib.*`

# XXX Note by Jack: the unset MACOSX_DEPLOYMENT_TARGET is a hack to get OSX 10.6 builds working
# on OSX 10.7. Unsuer whether it is a good idea.
# XXX Removed it: 	  unset MACOSX_DEPLOYMENT_TARGET ; 
build-pyambulant: $(srcdir)/setup.py $(BGENOUTPUT) $(srcdir)/ambulantutilities.cpp $(srcdir)/ambulantutilities.h $(srcdir)/ambulantmodule.h
	( a=`cd $(ambulantlibdir); pwd` ; \
	  cd $(srcdir) ; chmod +w . ; \
	  unset MACOSX_DEPLOYMENT_TARGET ; \
	  LDFLAGS="$(all_ldflags)" CFLAGS="$(all_cflags)" CC=$(CC) $(PYTHON) setup.py build ; \
	)

check-pyambulant:
	( a=`cd $(ambulantlibdir); pwd`:../../third_party_packages/installed/lib ; \
	  cd $(srcdir) ; \
	  LD_LIBRARY_PATH="$$a:$$LD_LIBRARY_PATH" DYLD_LIBRARY_PATH="$$a:$$LD_LIBRARY_PATH" $(PYTHONENV) PYTHONPATH=$(BUILDLIBDIR) $(DEBUGGER) $(PYTHON) $(PYTHONARGS) test/test_pyambulant.py ; \
	)

install-pyambulant:
	layout= ;\
	if $(PYTHON) -c "import sys; print sys.path" | grep -q dist-packages; then \
		layout=--install-layout=deb ; \
	fi ; \
	if test $(prefix) = "/usr" -o $(prefix) = "/usr/local"; then \
		cd $(srcdir) ; $(PYTHON) setup.py install $$layout --root $(DESTDIR) --prefix `$(PYTHON) -c "import sys; print sys.prefix"` ; \
	else \
		cd $(srcdir) ; $(PYTHON) setup.py install $$layout --home $(DESTDIR)/$(prefix) ; \
	fi

mostlyclean-local:
	-cd $(srcdir) ; $(PYTHON) setup.py clean
	-rm -r $(srcdir)/build

DISTCLEANFILES=ambulant.so ambulant-2.6-py2.7.egg-info

# Targets to regenerate ambulantmodule.cpp and ambulantinterface.cpp

regenerate-pyambulant:
	cd $(srcdir) ; $(PYTHON) genobjects.py
	cd $(srcdir) ; $(PYTHON) ambulantscan.py

# Debug target

testplay:
	cd $(srcdir) ; $(PYTHONENV) PYTHONPATH=$(BUILDLIBDIR) $(DEBUGGER) $(PYTHON) $(PYTHONARGS) test/nogui_player.py ../../Extras/DemoPresentation/NYC-SMIL2.smil
