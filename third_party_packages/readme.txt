This directory contains third-party packages used to build Ambulant. Note that
for Windows and Windows CE we make available a zipfile with all required
packages, patched and ready to build, see the toplevel README file (section
"building for Windows") for details.

Here is a list of the packages, where they come from, plus the versions:

***** Win32 *****

expat-win32:
	Windows only.
	
	James Clark's expat XML parser library, version 1.95.7. Download 
	via <http://expat.sourceforge.net>. Installed with the win32 binary installer 
	(it seems to be difficult to build from the source).


xerces-win32:
	windows only.
	
	Download xerces-C++ Version 2.5.0 Source Release from
	<http://xml.apache.org> and extract xerces-c-src_2_5_0 and put its 
	content in the directory .../ambulant/third_party_packages/
	
	Next read the Readme.html included in the xerces distribution and follow the MSVC Version 7
	buil instructions. After building xerces copy xerces-c_2_6.dll and xerces-depcom_2_6.dll to 
	ambulant/bin/win32 and the xerces-c_2.lib and xerces-depdom_2.lib to ambulant/lib.win32.
	The files are located in xerces-c-src_2_6_0\Build\Win32\VC7\Release. 

	But I don't want any xerces!
	If you want to compile AmbulantPlayer without xerces support use the Ambulant-win32-noxerces.sln
	
***** Unix *****
	
expat-unix:
	Linux and Mac OS X.
	
	Ditto, but built from source. Download the expat source via 
	<http://expat.sourceforge.net> and extract into
	expat-1.95.7 in this directory (.../ambulant/third_party_packages).
	Then do the following:
		$ tppdir=`pwd`  # or setenv tppdir `pwd` if you use tcsh
		$ cd expat-1.95.7
		$ ./configure --prefix=$tppdir/expat-unix
		$ make
		$ make install

	Note that you really want expat-1.95.7, not 1.95.6: the older version
	has a bug in the expat.h header file that causes compile time errors for
	Ambulant.
	
	Note that if you want to install expat 1.95.7 somewhere else (or you have
	already installed it) you should specify the location to the --with-expat
	option on the main ambulant configure: it will normally only look in
	third_party_packages/expat-unix.

xerces-unix:
	Linux and Mac OS X.
	
	For this release you need to build xerces from source, you cannot
	use a pre-installed xerces.
	
	Download xerces-C++ Version 2.6.0 Source Release from 
	<http://xml.apache.org> and extract xerces-c-src_2_6_0 and extract into
	xerces-c-src_2_6_0 in the directory (.../ambulant/third_party_packages).
	Then do the following:
		$ tppdir=`pwd`  # or setenv tppdir `pwd` if you use tcsh

	Next, follow the instructions for building xerces, open Readme.html
	and navigate to "Building". You really want to do this, as they have
	created some wrappers around the normal configure/make combo.
	After this :
		$ cd $tppdir/xerces-c-src_2_6_0/src/xercesc
		$ ./configure --prefix=$tppdir/xerces-unix
		$ gmake
		$ gmake install
	
	
	Mac OS X note: I had no success using the default net accessor (socket),
	but building xerces-c with the "native" net accessor does seem to work.
	For Linux there was no such problem.
	
	At this point in time you must specify --with-xerces to
	the Ambulant configure, by default AmbulantPlayer is built with only
	expat support. 
	
	Note that if you want to install xerces-c-src_2_5_0 somewhere
	else (or you have already installed it) you should specify the
	location to the --with-xerces option on the main ambulant
	configure: it will normally only look in third_party_packages/xerces-unix.
	And note that the --with-xerces pathname should refer to the base of a
	xerces source distribution, not to the installation prefix location.
	
	To run Ambulantplayer make sure that you have set LD_LIBRARY_PATH to the 
	directory containing libxerces-c.so.

ffmpeg:
	Linux and Mac OS X.
	
	You need ffmpeg 0.4.8. Download from <http://ffmpeg.sourceforge.net/> and
	unpack into ffmpeg-0.4.8 in this directory (.../ambulant/third_party_packages).
	
	After downloading and unpacking, for MacOSX you must apply the patch
	from ffmpeg (it should do no harm applying the patch for other Unix systems
	bt I don't think it is needed). Then build ffmpeg (there is no need to install):
		$ cd ffmpeg-0.4.8
		$ patch -p0 < ../ffmpeg-macosx-patch    # For Mac OS X only
		$ ./configure
		$ make
		
	Note that there is no reason to install ffmpeg (and it may actually fail to
	install cleanly on some systems): Ambulant Player links against the static
	libraries in the build directory.

	Also note that as of this writing you cannot use an ffmpeg installation as
	comes pre-installed with some RedHat distributions: not all libraries and
	include files seem to be installed.

	In principle the ffmpeg package is optional, but failing to supply it will
	result in an ambulant player that can play no audio (Mac OS X) or
	no audio and video (Linux).
	
sdl:
	Linux and Mac OS X.
	
	Ambulant has been tested with sdl 1.2.6 or 1.2.7. You find this at
	<http://www.libsdl.org>. Build and install normally, and make sure the
	sdl-config utility is on your $PATH when running the configure for Ambulant.
	
	Note for MacOSX: the SDL install will fail if there are spaces in the pathname
	*of the source directory pathname*.
	
arts:
	Linux only.
	
	At some point Ambulant was also able to use Arts audio library instead of
	SDL, but for this release that support is untested, and believed to be
	very buggy.
