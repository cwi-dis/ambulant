This directory contains third-party packages used to build Ambulant.
Here is a list of the packages, where they come from, plus the versions:

expat-win32:
	James Clark's expat XML parser library, version 1.95.7. Download 
	via <http://expat.sourceforge.net>. Installed with the win32 binary installer 
	(it seems to be difficult to build from the source).
expat-unix:
	Ditto, but built from source. Download the expat source into
	expat-1.95.7 in this directory (.../ambulant/third_party_packages).
	./configure --prefix=.../ambulant/third_party_packages/expat-unix
	make; make install"

	Note that you really want expat-1.95.7, not 1.95.6: the older version
	has a bug in the expat.h header file that causes compile time errors for
	Ambulant.

The following packages are optional. You also need not put them here in
third_party_packages.

ffmpeg:
	You need ffmpeg 0.4.8, or 0.4.7 with the lame mp3 decoder. Download
	from <http://ffmpeg.sourceforge.net/>, unless it is already
	installed in /usr/lib and /usr/include/ffmpeg (as on RedHat).
	If you have downloaded ffmpeg
	yourself you must point --with-ffmpeg to the libavcodec subdirectory
	*in the sourcetree*, not as installed (the header files aren't installed).
	For RedHat with ffmpeg installed from a Redhat RPM you can simply specify --with-ffmpeg.
	
	For Macintosh G5 machines you may need to change configure and/or
	some of the Makefiles. Linker errors will show the way.
	
sdl:
	Ambulant has been tested with sdl 1.2.6 and sdl_mixer 1.2.5. You need to
	download and install the latter package separately. You find both at
	<http://www.libsdl.org>.
