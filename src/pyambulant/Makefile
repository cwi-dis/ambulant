all: ambulantmodule.so

clean:
	rm -f ambulantobjgen.py ambulantincludegen.py ambulantmodule.cpp
	rm -fr build
	
ambulantobjgen.py ambulantincludegen.py: genobjects.py 
	python genobjects.py
	
ambulantmodule.cpp ambulantgen.py: ambulantscan.py ambulantsupport.py ambulantobjgen.py ambulantincludegen.py
	python ambulantscan.py

ambulantmodule.so: ambulantmodule.cpp setup.py
	python setup.py build
