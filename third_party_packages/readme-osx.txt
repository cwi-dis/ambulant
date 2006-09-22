This file explains how to build the third party packages as Universal Binaries
(which run on both powerpc and Intel) for MacOSX. It is an addition to the
information contained in readme.txt (which you should also read).

expat:
    If you want to do a MacOSX universal build you need to build
    expat universally too, by making sure the following environment
    variable is set when running configure:
        CFLAGS="-arch i386 -arch ppc -isysroot /Developer/SDKs/MacOSX10.4u.sdk"

xerces:
	If you want to build a MacOSX universal binary of Ambulant: building
	Xerces universally is a royal pain, due to the automated build
	process. The following seems to work:
	- call runConfigure as:
	   ./runConfigure -p macosx \
	       -z -arch -z ppc -z -arch -z i386 -z -isysroot \
	       -z /Developer/SDKs/MacOSX10.4u.sdk -n native
	- then call make as:
	   make LDFLAGS="-arch ppc -arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk"

ffmpeg:
	If you want to do a MacOSX universal build of Ambulant you also need
	to build ffmpeg universally. This is a bit tricky, but there's a script
	here to help you: ffmpeg-osx-fatbuild.sh. 
	- Create an empty directory "ffmpeg-universal".
	- Make sure the ffmpeg source directory is "make clean"ed.
	- In fmpeg-universal, run ffmpeg-osx-fatbuild.sh supplying the full
	  pathname of your ffmpeg sources. This configures, builds and merges the
	  fat ffmpeg libraries.
	- Now pass this folder to the Ambulant configure, with 
	  "--with-ffmpeg=third_party_packages/ffmpeg-universal".
	
sdl:
	If you want to build a universal (ppc/intel) Ambulant binary you must
	build SDL in a different way. The script sdl-osx-fatbuild.sh in this
	directory will do the right thing.
	
	Also note that readme.txt has some important notes on building SDL for
	MacOSX.
	
live555.com:
	If you want to create a MacOSX universal installer you need some patches
	to live first. Extract live-osx-fatbuild-patches.tar into the live
	source directory, do "./genMakefiles macosxfat" and make, and you're all set.
	
gettext:
	If you want to build a universal MacOSX installer, you need to run configure
	as follows:
	  CFLAGS="-arch ppc -arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk" \
	     configure --disable-csharp
	     
	

