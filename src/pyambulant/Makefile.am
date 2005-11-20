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

EXTRA_DIST=setup.py \
    bgenCxxSupport.py bgenBackSupport.py \
    ambulantscan.py ambulantsupport.py genobjects.py \
    ambulantutilities.h ambulantutilities.cpp \
    ambulantmodule.h ambulantmodule.cpp \
    ambulantinterface.h ambulantinterface.cpp
    
# These targets are magic for automake

all-am: build-pyambulant

check-am: check-pyambulant

install-exec-hook: install-pyambulant

uninstall-local: clean-pyambulant

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
	
build-pyambulant: $(srcdir)/setup.py $(BGENOUTPUT) $(srcdir)/ambulantutilities.cpp $(srcdir)/ambulantutilities.h $(srcdir)/ambulantmodule.h
	cd $(srcdir) ; LDFLAGS="-L$(libdir)" $(PYTHON) setup.py build

check-pyambulant:
	cd $(srcdir) ; LD_LIBRARY_PATH=$(libdir) $(PYTHONENV) PYTHONPATH=$(BUILDLIBDIR) $(DEBUGGER) $(PYTHON) $(PYTHONARGS) test/test_pyambulant.py

install-pyambulant:
	cd $(srcdir) ; $(PYTHON) setup.py install
	
mostlyclean-am:
	-cd $(srcdir) ; $(PYTHON) setup.py clean
	-rm -r $(srcdir)/build

# Targets to regenerate ambulantmodule.cpp and ambulantinterface.cpp

regenerate-pyambulant:
	cd $(srcdir) ; $(PYTHON) genobjects.py
	cd $(srcdir) ; $(PYTHON) ambulantscan.py
	
# Debug target

testplay:
	cd $(srcdir) ; $(PYTHONENV) PYTHONPATH=$(BUILDLIBDIR) $(DEBUGGER) $(PYTHON) $(PYTHONARGS) test/nogui_player.py ../../Extras/DemoPresentation/NYC-SMIL2.smil
	
