This directory contains third-party packages used to build Ambulant.
Here is a list of the packages, where they come from, plus the versions:

expat-win32:
	James Clark's expat XML parser library, version 1.95.6. Download 
	via <http://expat.sourceforge.net>. Installed with the win32 binary installer 
	(it seems to be difficult to build from the source).
expat-unix:
	Ditto, but built from source. Download the expat source into
	expat-1.95.6 in this directory (.../ambulant/third_party_packages).
	Apply the expat.h patch:
	(cd expat-1.95.6/include;patch < ../.../expat.h.patch)

	Then do:
	./configure --prefix=.../ambulant/third_party_packages/expat-unix
	make; make install"

	NOTE: on MacOSX you need to apply expat.h.patch.
