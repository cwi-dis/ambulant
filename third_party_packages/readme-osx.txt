This file explains how to build the third party packages as Universal Binaries:
- for MacOSX 10.4 and 10.5, to run on both PowerPC and Intel, or
- For MacOSX 10.6, to build for both 32 and 64 bit Intel.

It is an addition to the information contained in readme.txt (which you should also read).

When building on 10.5, for the time being please make sure you have
MACOSX_DEPLOYMENT_TARGET=10.4 in your environment, some things need
to be fixed for native 10.5 development.

expat:
    If you want to do a Intel/PPC universal build you need to build
    expat universally too, by making sure the following environment
    variable is set when running configure:
        CFLAGS="-arch i386 -arch ppc -isysroot /Developer/SDKs/MacOSX10.4u.sdk"
        
    For 10.6 32/64 bit universal, build with
        CFLAGS="-arch i386 -arch x86_64"

xerces-c 2.7 or 2.8:
	If you want to build an Intel/PPC universal binary of Ambulant: building
	Xerces universally is a royal pain, due to the automated build
	process. The following seems to work:
	- call runConfigure as:
	   ./runConfigure -p macosx \
	       -z -arch -z ppc -z -arch -z i386 -z -isysroot \
	       -z /Developer/SDKs/MacOSX10.4u.sdk -n native
	- then call make as:
	   make LDFLAGS="-arch ppc -arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk"
	   
xerces-c 3.0:
	To build 32/64-bit Intel on 10.6 do not use Xerces 2.X, use 3.0 in stead.
	Configure with
	    ./configure \
	        --disable-dependency-tracking \
	        --prefix=$HOME/xerces-3-installed \
	        CFLAGS="-arch i386 -arch x86_64" CXXFLAGS="-arch i386 -arch x86_64"
	Then, when building ambulant, configure with --with-xerces=$HOME/xerces-3-installed

ffmpeg 0.5:
	If you want to do a MacOSX universal build of Ambulant you also need
	to build ffmpeg universally. This is a bit tricky, but there's a script
	here to help you: ffmpeg-osx-fatbuild.sh. 
	- Create an empty directory "ffmpeg-universal".
	- Make sure the ffmpeg source directory is "make clean"ed.
	- Edit ffmpeg-osx-fatbuild.sh, and examine the defines at the beginning.
	  The comments should be good enough (we hope:-) to allow you to select the
	  defines you need.
	- In ffmpeg-universal, run ffmpeg-osx-fatbuild.sh supplying the full
	  pathname of your ffmpeg sources. This configures, builds and merges the
	  fat ffmpeg libraries.
	- Now pass this folder to the Ambulant configure, with 
	  "--with-ffmpeg=third_party_packages/ffmpeg-universal".
	
libfaad:
    (Only if you build ffmpeg with libfaad support)
    For ppc/build with
    
        ./configure \
            CFLAGS="-arch i386 -arch ppc -isysroot /Developer/SDKs/MacOSX10.4u.sdk" \
            --disable-dependency-tracking
	
	For 32/64-bit intel, build with
        ./configure CFLAGS="-arch i386 -arch x86_64" --disable-dependency-tracking

    For both, you may need to edit the Makefile (if you get "Missing Separator") and
    replace some spaces with tabs.
    
sdl 1.2.X:
	If you want to build a universal (ppc/intel) Ambulant binary you must
	build SDL in a different way. The script sdl-osx-fatbuild.sh in this
	directory will do the right thing.
	
	Also note that readme.txt has some important notes on building SDL for
	MacOSX.
	
sdl 1.3:
    Use this for building 32/64 bit on MacOSX 10.6. As of this writing (September 2009)
    this is still in beta, but seems to be good enough for Ambulant.
    Configure with
        ./configure \
            CFLAGS="-arch i386 -arch x86_64 -framework ForceFeedback" \
            LDFLAGS="-framework ForceFeedback"
        
live555.com:
	If you want to create a MacOSX universal installer you need some patches
	to live first. Extract live-osx-fatbuild-patches.tar into the live
	source directory, do 
        $ export MACOSX_DEPLOYMENT_TARGET=10.4 
        $./genMakefiles macosxfat 
        $ make
	and you're all set.
	
	For 32/64 bit builds: again first extract live-osx-fatbuild-patches.tar into
	the live distribution. Then:
        $ genMakefiles macosx3264
        $ make
	
gettext:
	If you want to build a ppc/intel MacOSX installer, you need to run configure
	as follows:

        CFLAGS="-arch ppc -arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk" \
            configure --disable-csharp
	     
	For a 32/64 bit build: configure with
	
	    ./configure --disable-csharp --disable-shared CFLAGS="-arch i386 -arch x86_64"
	 
libxml2:
    To build ppc/intel universally, configure with
        MACOSX_DEPLOYMENT_TARGET=10.4 \
            CFLAGS="-arch i386 -arch ppc -isysroot /Developer/SDKs/MacOSX10.4u.sdk" \
            ./configure --disable-dependency-tracking

    To build 32/64 bit on OSX 10.6: configure with
    
        ./configure --disable-dependency-tracking CFLAGS="-arch i386 -arch x86_64"
        

