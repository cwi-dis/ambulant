This directory contains third-party packages used to build Ambulant.
Here is a list of the packages, where they come from, plus the versions:

expat-win32:
	James Clark's expat XML parser library, version 1.95.6. Download 
	via <http://expat.sourceforge.net>. Installed with the win32 binary installer 
	(it seems to be difficult to build from the source).
expat-unix:
	Ditto, but built from source. Download the expat source into, say,
	expat-1.95.6, do "./configure --prefix=.../ambulant/third_party_packages/expat-unix"
	and then "make && make install".