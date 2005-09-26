BGENADDITIONS=bgenCxxSupport.py bgenBackSupport.py
BGENOUTPUT=ambulantmodule.cpp ambulantinterface.cpp
BGENTEMP=ambulantgen.py
BGENINPUT=ambulantscan.py ambulantsupport.py
GENOBJOUTPUT=ambulantobjgen.py ambulantincludegen.py ambulantmodule.h
PYTHON=python2.5

# Set these variables to change the way the tests are run
DEBUGGER=
#DEBUGGER=gdb --args
PYTHONARGS=
#PYTHONARGS=-i
PYTHONENV=
#PYTHONENV=DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib

BUILDLIBDIR=./build/lib.darwin-8.2.0-Power_Macintosh-2.5

all: ambulantmodule.so

clean:
	rm -f $(BGENOUTPUT) $(BGENTEMP) $(GENOBJOUTPUT)
	rm -fr build
	
install:
	$(PYTHON) setup.py install
	
tests:
	$(PYTHONENV) PYTHONPATH=$(BUILDLIBDIR) $(DEBUGGER) $(PYTHON) $(PYTHONARGS) test/test_pyambulant.py
	
testplay:
	$(PYTHONENV) PYTHONPATH=$(BUILDLIBDIR) $(DEBUGGER) $(PYTHON) $(PYTHONARGS) test/nogui_player.py ../../Extras/DemoPresentation/NYC-SMIL2.smil
	
$(GENOBJOUTPUT): genobjects.py 
	$(PYTHON) genobjects.py
	
$(BGENOUTPUT) $(BGENTEMP): $(BGENINPUT) $(GENOBJOUTPUT) $(BGENADDITIONS)
	$(PYTHON) ambulantscan.py

ambulantmodule.so: setup.py $(BGENOUTPUT) ambulantutilities.cpp ambulantutilities.h ambulantmodule.h
	$(PYTHON) setup.py build
