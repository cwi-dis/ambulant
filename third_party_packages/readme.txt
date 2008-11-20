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
    
	Building libxml2 should be handled by the projects/*/third_party_packages.sln.
	In case it fails: read the instructions in libxml2-2.6.69/win32 and do it
	yourself.
    
	Do not install the libs and binaries.
    
	WM5 notes TBD.

ffmpeg:
	Experimental, for when you want to use the ffmpeg renderers on Windows.
	Needs to be built using MinGW,can then be linked into Visual Studio projects.
	
	The easiest way to get this is to download the prebuilt version from
	our sourceforge download page. Package "ffmpeg for Ambulant", release
	"20070828-win32-prebuilt".

	If you want to build from source, follow the build instructions on 
	<http://ffmpeg.arrozcru.org/wiki/index.php>. There are other webpages
	with instructions, but these are the only ones that worked for me.
	
SDL:
	Experimental, for when you want to use the SDL renderers on Windows. 
	There are two options:
	-Pre-built Development Libraries:
		Download development libraries for win32(VC8) from <http://www.libsdl.org/download-1.2.php>, 
		and then follow the instructions of section "Creating a Project with SDL" of VisualC.html 
		in the root folder of unzipped sdl folder.
	-Source Code:
		Download the source code of SDL from <http://www.libsdl.org/download-1.2.php>, and then follow
		the instructions of section "Building SDL" of VisualC.html in the root folder of unzipped sdl folder.

Live555:
	Experimental, for when you want to use the Live555 to provide rtsp support on Windows.
	
	Download source from <http://live555.com/liveMedia/public/>.
	Copy the extracted folder "live" into third_party_packages (if the name has changed you
	may need to change some Ambulant projects to refer to the new name). Check out the relevent 
	solution and project files from the path of /ambulant/third_party_packages/live_VC8 at ambulant's 
	CVS repository.
    
	Building Live555 should be handled by the /ambulant/third_party_packages/live_VC8/Live555.com.sln.
		
xulrunner-sdk:
	For building AmbulantPlayer as a plugin for Firefox Web browser, download:
	http://releases.mozilla.org/pub/mozilla.org/xulrunner/releases/1.9.0.1/sdk/xulrunner-1.9.0.1.en-US.win32.sdk.zip 	
    Install in /ambulant/third_party_packages

others:
	For this release we don't fully remember where we got the other
	libraries (jpeg, zlib, mp3lib, lpng128), so it is really in your
	best interest to use the third_party_packages_win_20051212.zip 
	archive we have prepared.
	
	After downloading and unpacking the zipfile move the 4 individual
	folders from "INTO_third_party_packages" into this directory. Then
	goto either projects\vc7, projects\vc8 or projects\vc8-wince5, depending
	on the platform you are interested in.
	Open third_party_packages.sln and build "Release".
	NOTE THAT LAST SENTENCE: you must
	build the Release configuration, the default configuration opened is
	not what you want (and we don't know how to fix this).

	By now the various third_party_packages.sln should all have copied the
	needed libraries into ambulant\lib\win32. Check this. If this has not happened
	we forgot to modify the solution you used, use the script copy_tpp_win32.bat
	to copy the libraries.
	
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

	To run AmbulantPlayer make sure that you have set LD_LIBRARY_PATH to
	the directory containing libxerces-c.so.
	
ffmpeg:
	In principle the ffmpeg package is optional, but failing to supply
	it will result in an AmbulantPlayer that can play no audio (Mac OS
	X) or no audio and video (Linux).

	As ffmpeg does not provide versioned distributions, and on all Linux
	distributions we have seen, system installed versions of ffmpeg have
	too little functionality to be useable for AmbulantPlayer, we need to
	extract the June 15th, 2008 version from the developers source tree.
	When configuring ffmpeg, do NOT use its '--enable-swscale' option.

	In addition, for AAC audio decoding, libfaad2 is needs to be installed.
	See: http://www.audiocoding.com/faad2.html how to download and install.

	Warning: while AmbulantPlayer by itself is licensed LGPL, by including
	libfaad2 the licensing scheme falls back to GPL	(AAC decoding is currently
	only needed by AmbulantPlayer for audio streaming via RTSP).

	Download ffmpeg from source in the directory 
	.../ambulant/third_party_packages/ffmpeg using svn:

	$ cd .../ambulant/third_party_packages
	$ svn checkout -r "{2008-06-15}" svn://svn.mplayerhq.hu/ffmpeg/trunk ffmpeg
	$ cd ffmpeg
	$ if [ `arch` = x86_64 ] ; then extracflags=--extra-cflags=-fPIC ; fi
	$ ./configure --enable-libfaad --enable-gpl $extracflags
	$ make

	("--extra-cflags=-fPIC" is needed a.o. on Fedora-8 64 bit installations).

	NOTE 1: on Mac OS X, the lib*-uninstalled.pc files have a problem
	(as of June 17, 2008): they refer to the libraries by filename in stead of with
	a -L/-l construct. You must change this manually: replace each 
	"${libdir}/libavxxxx.a" with "-L${libdir} -lavxxxx", otherwise libtool will fail
	later, while building the ambulant ffmpeg library.

	NOTE 2: for Nokia770/800, see special instructions in .../installers/nokia770/README.

sdl:
	Ambulant has been tested with sdl 1.2.5 thru 1.2.13. You find this at
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
    
xulrunner-sdk:
	For building AmbulantPlayer as a plugin for Firefox Web browser, download:
	http://releases.mozilla.org/pub/mozilla.org/xulrunner/releases/1.9.0.1/sdk/xulrunner-1.9.0.1.en-US.XXX.sdk.zip
	
	where XXX stands for the appropriate archtecture of the target machine.
    Install in /ambulant/third_party_packages and configure ambulant
    using --with -npambulant
 

    
