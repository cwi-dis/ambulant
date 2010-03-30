-------------------------------------------------------------------------------
The information in this file is probably outdated. As of Ambulant 2.2
you should read the toplevel INSTALL file, which will give you a much easier
recipe for building the 3rd party packages, using the
"build-third-party-packages.py" script. Only consult this file if you really
need to build manually, or if somehow the automatic procedure does not work.
-------------------------------------------------------------------------------

This directory contains third-party packages used to build Ambulant.
Note that for Windows and Windows CE we make available a zipfile with
all required packages, patched and ready to build, see the toplevel
INSTALL file (section "building for Windows") for details.

Here is a list of the packages, where they come from, plus the versions:

Windows Desktop, Windows Mobile 5
=================================

xerces:
	windows desktop only.
	
	Download xerces-C++ Version 3.0.1 Source Release from
	<http://xerces.apache.org>, extract, and put its
	content in the directory .../ambulant/third_party_packages/.

	Build according to instructions.

	If you want to compile AmbulantPlayer without xerces support use the
	NoXerces configurations in the Ambulant solution.
	
libxml2:
	Only for SMIL 3 support. Download source from <http://www.xmlsoft.org>.
	There are no releases, only hourly snapshots it seems. 2.7.6 is known to
	work, but so are many other versions.
    
	Building libxml2 should be handled by the projects/vc9/third_party_packages.sln.
	You may have to modify Ambulant-win32.vsprops if you downloaded a different
	release than 2.7.6 to fix the pathname.
    
	Do not install the libs and binaries.
    
	WM5 notes TBD.

ffmpeg:
    Version 0.5 or later.
    
	Needs to be built using MinGW, can then be linked into Visual Studio projects.
	
	The easiest way to get this is to download the prebuilt version from
	our sourceforge download page. Package "ffmpeg for Ambulant", release
	"ffmpeg-20100122-win32-prebuilt"

	If you want to build from source, follow the build instructions on 
	<http://ffmpeg.arrozcru.org/wiki/index.php>. There are other webpages
	with instructions, but these are the only ones that worked for me.
	
SDL:

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

Again, as stated above: unless you really want to do this manually please try
the automatic procedure, mentioned at the top of this file.
If you decide to build manually: you make your life easier by using
--prefix=.../ambulant/third_party_packages/installed on all your configures.
The Ambulant configure knows about this location, and automatically add it to
$PATH, $PKGCONFIG_PATH, etc.
	
expat-unix:	
	Build from source. Download the expat source via
	<http://expat.sourceforge.net> and extract into expat-2.0.1 in this
	directory (.../ambulant/third_party_packages).
	Then do the following:
		$ tppdir=`pwd`  # or setenv tppdir `pwd` if you use tcsh
		$ cd expat-2.0.1
		$ patch < ../expat.patch
		$ ./configure --prefix=...where-ever...
		$ make
		$ make install
	The patch is needed to add pkgconfig support to expat. Make sure the expat prefix directory
	(actually lib/pkgconfig therein) is searched by pkgconfig.

xerces-unix:
	Download xerces-C++ Version 3.0.1 Source Release from
	<http://xerces.apache.org> and extract
	into third_party_packages. Build according to instructions.

    Xerces (as of version 3.0) now also has pkgconfig support, so again: if you use
    --prefix when building xerces, make sure that pkgconfig searches the correct
    location.
	
ffmpeg:
	First, for AAC audio decoding, libfaad2 is needs to be installed.
	See: http://www.audiocoding.com/faad2.html how to download and install.

	Warning: while AmbulantPlayer by itself is licensed LGPL, by including
	libfaad2 the licensing scheme falls back to GPL	(AAC decoding is currently
	only needed by AmbulantPlayer for audio streaming via RTSP).

	You can get ffmpeg 0.5 from www.ffmpeg.org, but this version has a nasty
	bug with playback of audio/video over http. If this is important to you:
	get ffmpeg-export-2010-01-22.tar.gz from the Ambulant sourceforge download
	page. This is a source snapshot that works with ambulant, and has this bug
	fixed.
	
	Build according to what you want to support.
	Configure options that may be useful:
	--enable-libfaad --enable-gpk
	    Use these to include AAC audio support (as mentioned above)
	--extra-cflags=-fPIC
	    May be needed on some 64bit linux systems
	Also: read the build-third-party-packages.py script to see the options we've used.
	
sdl:
	Ambulant has been tested with sdl 1.2 and 1.3. For 1.2, make sure sdl-config
	is on your $PATH when building Ambulant. 1.3 uses standard pkgconfig.
	
live555.com:
	You need to download live555-latest.tar.gz at
	<http://www.live555.com/liveMedia/public/>. Extract the tar file in
	.../ambulant/third_party_packages. Build live according to supplied instructions.
		
	Live does not have an install procedure. If you have installed live
	in .../ambulant/third_party_packages/live, the Ambulant configure will detect
	it. Otherwise you need to supply --with-live=...pathname...

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
    using --with-npambulant
 

    
