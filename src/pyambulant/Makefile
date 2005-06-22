BGENADDITIONS=bgenCxxSupport.py bgenBackSupport.py
BGENOUTPUT=ambulantmodule.cpp ambulantinterface.cpp
BGENTEMP=ambulantgen.py
BGENINPUT=ambulantscan.py ambulantsupport.py
GENOBJOUTPUT=ambulantobjgen.py ambulantincludegen.py ambulantmodule.h

all: ambulantmodule.so

clean:
	rm -f $(BGENOUTPUT) $(BGENTEMP) $(GENOBJOUTPUT)
	rm -fr build
	
$(GENOBJOUTPUT): genobjects.py 
	python genobjects.py
	
$(BGENOUTPUT) $(BGENTEMP): $(BGENINPUT) $(GENOBJOUTPUT) $(BGENADDITIONS)
	python ambulantscan.py

ambulantmodule.so: setup.py $(BGENOUTPUT) ambulantutilities.cpp ambulantutilities.h ambulantmodule.h
	python setup.py build
