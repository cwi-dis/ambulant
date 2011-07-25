#!/bin/sh
#
# Build universal MacOSX ffmpeg libraries.
#
# Script created by Jack.Jansen@cwi.nl, as part of the Ambulant project.
# This version of the script is public domain, so feel free to add any
# license you want if you modify it (but open would be preferred, of course!)
#
# Last tested with ffmpeg 0.5.
#

#
# Where do you want to install?
# Leave empty for no installation
#
PREFIX="--prefix=`cd ../installed ; pwd`"
#
# Set this to the global options you want to configure ffmpeg with.
#
#CONFIGOPTS="$PREFIX --disable-encoders --enable-swscale --enable-gpl --disable-vhook --disable-ffserver --disable-ffmpeg --disable-ffplay --disable-ffprobe --enable-static --enable-shared --enable-libfaad --disable-libfaac"
CONFIGOPTS="$PREFIX --disable-encoders --enable-runtime-cpudetect --enable-gpl --disable-ffserver --disable-ffmpeg --disable-ffplay --disable-ffprobe --enable-static --enable-shared --enable-libfaad --disable-libfaac"
#
# If you want to build for a different MacOSX version than the current one
# define SYSROOT and MACOSX_DEPLOYMENT_TARGET
#
###SYSROOT=" -isysroot /Developer/SDKs/MacOSX10.4u.sdk"
##export MACOSX_DEPLOYMENT_TARGET=10.4
##CONFIGOPTS="--cc=gcc-4.0  --extra-cflags=-I`cd ../installed/include; pwd` --extra-ldflags=-L`cd ../installed/lib; pwd` "$CONFIGOPTS
SYSROOT=" -isysroot /Developer/SDKs/MacOSX10.6.sdk"
CONFIGOPTS="--extra-cflags=-I`cd ../installed/include; pwd` --extra-ldflags=-L`cd ../installed/lib; pwd` "$CONFIGOPTS
#
# Set variables here to true to include the ABI
#
PPC=false
I386=true
PPC64=false
X86_64=true
#
# Set this variable to any of the architectures you are build
# (used to pick up .pc files)
#
ANY_RANDOM_ARCH=i386
#
# You may need to fiddle these, but probably not.
#
PPC_CONFIGOPTS="--arch=powerpc --cpu=g4 --enable-altivec --enable-shared"
I386_CONFIGOPTS="--arch=i686 --cpu=i686 --enable-shared"
PPC64_CONFIGOPTS="--enable-cross-compile --arch=ppc64 --cpu=g4 --enable-altivec --enable-shared"
X86_64_CONFIGOPTS="--arch=x86_64 --enable-shared"
#
# Should be boilerplate from here on
#
srcdir=`cd $1; pwd`
case x$srcdir in
    x)
        echo Usage: $0 ffmpeg-src-dir '[all|mkdirs|mklinks|configure|clean|build|merge|install] ...'
        exit
        ;;
esac

mkdirs=false
mklinks=false
configure=false
clean=false
build=false
merge=false
install=false
shift
for i in $*; do
    case $i in
        all)
            mkdirs=true
            mklinks=true
            configure=true
            clean=true
            build=true
            merge=true
            install=true
            ;;
        mkdirs)
            mkdirs=true
            ;;
        mklinks)
            mklinks=true
            ;;
        configure)
           configure=true
            ;;
        clean)
            clean=true
            ;;
        build)
            build=true
            ;;
        merge)
            merge=true
            ;;
        install)
            install=true
            ;;
        *)
            echo Usage: $0 ffmpeg-src-dir '[all|mkdirs|mklinks|configure|clean|build|merge|install] ...'
            exit
            ;;
    esac
done

if $mkdirs; then
    echo $0: mkdirs
    if $I386; then mkdir build-i386; fi
    if $PPC; then mkdir build-ppc; fi
    if $X86_64; then mkdir build-x86_64; fi
    if $PPC64; then mkdir build-ppc64; fi
    mkdir libavformat
    mkdir libavcodec
    mkdir libavutil
    mkdir libavdevice
    mkdir libswscale
else
    echo $0: skipping mkdirs
fi

if $mklinks; then
    echo $0: mklinks
    ( cd libavformat ; ln -s $srcdir/libavformat/*.h .)
    ( cd libavcodec ; ln -s $srcdir/libavcodec/*.h .)
    ( cd libavutil ; ln -s $srcdir/libavutil/*.h .)
    ( cd libavdevice ; ln -s $srcdir/libavdevice/*.h .)
    ( cd libswscale ; ln -s $srcdir/libswscale/*.h .)
else
    echo $0: skipping mklinks
fi

if $configure; then
	if $I386; then
		echo $0: configure i386
		(
			cd build-i386
			$srcdir/configure \
				--extra-cflags="-arch i386 $SYSROOT" \
				--extra-ldflags="-arch i386 $SYSROOT" \
				$I386_CONFIGOPTS \
				$CONFIGOPTS
		)
	else
		echo $0: skipping configure for i386
	fi
	if $PPC; then
		echo $0: configure ppc
		(
			cd build-ppc
			$srcdir/configure \
				--extra-cflags="-arch ppc $SYSROOT" \
				--extra-ldflags="-arch ppc $SYSROOT" \
				$PPC_CONFIGOPTS \
				$CONFIGOPTS
		)
	else
		echo $0: skipping configure for ppc
	fi
	if $PPC64; then
		echo $0: configure ppc64
		(
			cd build-ppc64
			$srcdir/configure \
				--extra-cflags="-arch ppc64 $SYSROOT" \
				--extra-ldflags="-arch ppc64 $SYSROOT" \
				$PPC64_CONFIGOPTS \
				$CONFIGOPTS
		)
	else
		echo $0: skipping configure for ppc64
	fi
	if $X86_64; then
		echo $0: configure x86_64
		(
			cd build-x86_64
			$srcdir/configure \
				--extra-cflags="-arch x86_64 $SYSROOT" \
				--extra-ldflags="-arch x86_64 $SYSROOT" \
				$X86_64_CONFIGOPTS \
				$CONFIGOPTS
		)
	else
		echo $0: skipping configure for x86_64
	fi
else
    echo $0: skipping configure
fi

if $clean; then
	if $I386; then
		echo $0: clean i386
		( cd build-i386; make clean)
	fi
	if $PPC; then
		echo $0: clean ppc
		( cd build-ppc; make clean)
	fi
	if $PPC64; then
		echo $0: clean ppc64
		( cd build-ppc64; make clean)
	fi
	if $X86_64; then
		echo $0: clean x86_64
		( cd build-x86_64; make clean)
	fi
else
    echo $0: skipping clean
fi

if $build; then
	if $I386; then
		echo $0: build i386
		( cd build-i386; make $MAKEFLAGS)
	else
		echo $0: skipping build for i386
	fi
	if $PPC; then
		echo $0: build ppc
		( cd build-ppc; make $MAKEFLAGS)
	else
		echo $0: skipping build for ppc
	fi
	if $PPC64; then
		echo $0: build ppc64
		( cd build-ppc64; make $MAKEFLAGS)
	else
		echo $0: skipping build for ppc64
	fi
	if $X86_64; then
		echo $0: build x86_64
		( cd build-x86_64; make $MAKEFLAGS)
	else
		echo $0: skipping build for x86_64
	fi
else
    echo $0: skipping build
fi

if $merge; then
    echo $0: merge
    for i in \
        libavformat/libavformat.a \
        libavformat/libavformat.dylib \
        libavcodec/libavcodec.a \
        libavcodec/libavcodec.dylib \
        libavutil/libavutil.a \
        libavutil/libavutil.dylib \
        libswscale/libswscale.a \
        libswscale/libswscale.dylib
    do
        lipo -create -output $i build-*/$i
    done
    for i in avformat avutil avcodec swscale; do
    	cp build-$ANY_RANDOM_ARCH/lib$i/*.pc lib$i/
    done
    if [ -f build-$ANY_RANDOM_ARCH/libavutil/avconfig.h ]; then
        cp build-$ANY_RANDOM_ARCH/libavutil/avconfig.h libavutil/avconfig.h
    fi
else
    echo $0: skipping merge
fi

if $install; then
    echo $0: install
    (cd build-$ANY_RANDOM_ARCH ; make -n install) | sed -e 's/^.*$/(&)/' > install.sh
    sh install.sh
else
    echo $0: skipping install
fi
