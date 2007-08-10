This directory contains third-party packages used to build Ambulant.
Note that for Windows and Windows CE we make available a zipfile with
all required packages, patched and ready to build, see the toplevel
README file (section "building for Windows") for details.

Here is a list of the packages, where they come from, plus the versions:

Windows Desktop, Windows Mobile 5
=================================

xerces:
	windows desktop only.
	
	Download xerces-C++ Version 2.7.0 Source Release from
	<http://xerces.apache.org> and extract xerces-c-src_2_7_0 and put its
	content in the directory .../ambulant/third_party_packages/

	Next read the Readme.html included in the xerces distribution and
	follow the MSVC Version 7.1 build instructions. After building xerces
	copy xerces-c_2_7.dll and xerces-depcom_2_7.dll to
	ambulant/bin/win32 and the xerces-c_2.lib and xerces-depdom_2.lib to
	ambulant/lib/win32. The files are located in
	xerces-c-src_2_7_0\Build\Win32\VC7.1\Release.

	If you want to compile AmbulantPlayer without xerces support use the
	NoXerces configurations in the Ambulant solution.
	
libxml2:
    Only for SMIL 3 support. Download source from <http://www.xmlsoft.org>.
    There are no releases, only hourly snapshots it seems. Copy
    libxml2-2.6.69 into third_party_packages (if the name has changed you
    may need to change some Ambulant projects to refer to the new name.
    
    You need to configure and build libxml2 manually (the solution tries to
    do it but fails), in libxml2-2.6.69\win32. Readme.txt has the details,
    but it boils down to doing the following from a Command Prompt window:
    
    - Set MSVC paths with ...\Microsoft Vistal Studio 7or8\VC\vcvarsall.bat
    - cscript configure.js iconv=no
    - nmake
    
    Do not install the libs and binaries.
    
    WM5 notes TBD.
	
others:
	For this release we don't fully remember where we got the other
	libraries (jpeg, zlib, mp3lib, lpng128), so it is really in your
	best interest to use the third_party_packages_win_20051212.zip 
	archive we have prepared.
	
	After downloading and unpacking the zipfile move the 4 individual
	folders from "INTO_third_party_packages" into this directory. Then
	goto either projects\vc7, projects\vc8 or projects\vc8-wince5, depending
	on the platform you are interested in.
	Ppen third_party_packages.sln and build "Release".
	NOTE THAT LAST SENTENCE: you must
	build the Release configuration, the default configuration opened is
	not what you want (and we don't know how to fix this).
	
	If you really want to build from original distributions: don't do this.
	If you still want it after this warning, and it turns out to be easy:
	please let us know.

	
Unix (Linux, MacOSX)
====================
	
expat-unix:	
	Build from source. Download the expat source via
	<http://expat.sourceforge.net> and extract into expat-2.0.0 in this
	directory (.../ambulant/third_party_packages).
	Then do the following:
		$ tppdir=`pwd`  # or setenv tppdir `pwd` if you use tcsh
		$ cd expat-1.95.7
		$ ./configure --prefix=$tppdir/expat-unix
		$ make
		$ make install

xerces-unix:
	Download xerces-C++ Version 2.7.0 Source Release from
	<http://xerces.apache.org> and extract xerces-c-src_2_7_0 and extract
	into xerces-c-src_2_7_0 in the directory
	(.../ambulant/third_party_packages).
	Then do the following:
		$ tppdir=`pwd`  # or setenv tppdir `pwd` if you use tcsh

	Next, follow the instructions for building xerces, open Readme.html
	and navigate to "Building". You really want to do this, as they have
	created some wrappers around the normal configure/make combo. You
	need to add the prefix directory, and for MacOSX the "-n native"
	option:
	    linux$ ./runConfigure -p linux -P $tppdir/xerces-unix
	or
		macosx$ ./runConfigure -p macosx -n native -t native -P $tppdir/xerces-unix
	After this "gmake" and "gmake install" will do the job.
	
	At this point in time you must specify --with-xerces to the Ambulant
	configure, by default AmbulantPlayer is built with only expat
	support.
	
	Note that if you want to install Xerces somewhere else (or you have
	already installed it) you should specify the location to the
	--with-xerces option on the main ambulant configure: it will
	normally only look in third_party_packages/xerces-unix.

	To run Ambulantplayer make sure that you have set LD_LIBRARY_PATH to
	the directory containing libxerces-c.so.
	
ffmpeg:
    The best option is to use the the fairly recent frozen cvs-ffmpeg
    that was created especially for Ambulant. You find this on the
    Ambulant SF download pages as ffmpeg-cvs-20051121.tgz. Unpack this
    into this directory (third_party_packages) as ffmpeg-cvs, and configure
    and build it. No need to install, if you have kept all pathnames as
    stated here the Ambulant configure script will pick it up.
    
	Alternatively, use ffmpeg 0.4.8 or 0.4.9pre, but some functionality
	will not work in this case. Download from
	<http://ffmpeg.sourceforge.net/> and unpack into ffmpeg-0.4.8 in
	this directory (.../ambulant/third_party_packages).

	After downloading and unpacking, for MacOSX you must apply the patch
	from ffmpeg (it should do no harm applying the patch for other Unix
	systems bt I don't think it is needed). Then build ffmpeg (there is
	no need to install):
		$ cd ffmpeg-0.4.8
		$ patch -p0 < ../ffmpeg-macosx-patch    # For Mac OS X only
		$ ./configure       (*)
		$ make
		
	(*) Use configure --disable-opts if you get a lot of linker errors

	Note that there is no reason to install ffmpeg (and it may actually
	fail to install cleanly on some systems): Ambulant Player links
	against the static libraries in the build directory.

	Also note that as of this writing you cannot use an ffmpeg
	installation as comes pre-installed with some RedHat distributions:
	not all libraries and include files seem to be installed.

	In principle the ffmpeg package is optional, but failing to supply
	it will result in an ambulant player that can play no audio (Mac OS
	X) or no audio and video (Linux).
	
sdl:
	Ambulant has been tested with sdl 1.2.5 thru 1.2.11. You find this at
	<http://www.libsdl.org>. Build and install normally, and make sure
	the sdl-config utility is on your $PATH when running the configure
	for Ambulant.

	Notes for MacOSX:
	- the SDL install will fail if there are spaces in the 
	  pathname *of the source directory pathname*.
	- You may need to use "./configure --disable-cdrom" to build SDL
	  if you've installed xcode 2.2.
	- SDL always seems to build the dynamic libraries. IF YOU WANT TO
	  CREATE A SELFCONTAINED INSTALLER YOU MUST REMOVE THESE BEFORE BUILDING,
	  use "sudo rm /usr/local/bin/libSDL*dylib".
	
arts:
	Linux only.
	
	At some point Ambulant was also able to use Arts audio library
	instead of SDL, but for this release that support is untested, and
	believed to be very buggy.

live555.com:
	You need to download live555-latest.tar.gz at
	<http://www.live555.com/liveMedia/public/>. Extract the tar file in
	.../ambulant/third_party_packages (or at anyother place, but
	remember to use the configure option --with-live=.....). Build live
	with the following commands:
			$ cd live
			$ ./genMakefiles linux
			$ make
	For MacOSX you need to supply "macosx" or "macosx-before-version-10.4"
	to genMakefiles in stead of "linux".
	
	Live does not have an install procedure. If you have installed live
	in .../ambulant/third_party_packages/live configure will detect
	live.com.

gettext:
	On Linux you will usually have gettext pre-installed, on Mac OS X probably
	not. Download from <http://www.gnu.org> and install in the normal way.

	On some systems (Fedora Core 6, for example) you may have to install
	the gettext-devel package (in addition to the usually pre-installed
	gettext package).
	
	Alternatively, configure Ambulant with --disable-nls to disable libintl
	support (and, therefore, localization).
	
libxml2:
    Only needed for SMIL3 support. Download from <http://www.xmlsoft.org>
    and install normally. Make sure xml2-config is in your $PATH when building
    Ambulant.
    
    