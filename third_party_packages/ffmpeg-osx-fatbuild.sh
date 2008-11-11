#!/bin/sh
#
#CONFIGOPTS="--enable-swscale --disable-mmx --disable-encoders --enable-gpl --disable-vhook --disable-ffmpeg"
#CONFIGOPTS="--disable-optimizations --disable-stripping --disable-mmx --enable-gpl --disable-vhook --disable-ffserver --disable-ffmpeg --enable-static --enable-shared --enable-libfaad --disable-libfaac --extra-cflags=-I/usr/local/include --disable-altivec"
CONFIGOPTS="--disable-mmx --disable-encoders --enable-gpl --disable-vhook --disable-ffserver --disable-ffmpeg --enable-static --enable-shared --enable-libfaad --disable-libfaac --extra-cflags=-I/usr/local/include --disable-altivec"
srcdir=$1
# XXXX Enable these for doing 10.4-compatible distributions
SYSROOT=" -isysroot /Developer/SDKs/MacOSX10.4u.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.4
# XXXX Enable these for native 10.5 distributions
#SYSROOT=""
#export MACOSX_DEPLOYMENT_TARGET=10.5
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
    mkdir build-i386
    mkdir build-ppc
    mkdir libavformat
    mkdir libavcodec
    mkdir libavutil
    mkdir libswscale
else
    echo $0: skipping mkdirs
fi

if $mklinks; then
    echo $0: mklinks
    ( cd libavformat ; ln -s $srcdir/libavformat/*.h .)
    ( cd libavcodec ; ln -s $srcdir/libavcodec/*.h .)
    ( cd libavutil ; ln -s $srcdir/libavutil/*.h .)
    ( cd libswscale ; ln -s $srcdir/libswscale/*.h .)
else
    echo $0: skipping mklinks
fi

if $configure; then
    echo $0: configure i386
    (
        cd build-i386
        $srcdir/configure \
            --extra-cflags="-arch i386 $SYSROOT" \
            --extra-ldflags="-arch i386 $SYSROOT" \
            --arch=i686 \
            --cpu=i686 \
            --enable-shared \
            $CONFIGOPTS
    )
    echo $0: configure ppc
    (
        cd build-ppc
        $srcdir/configure \
            --extra-cflags="-arch ppc $SYSROOT" \
            --extra-ldflags="-arch ppc $SYSROOT" \
            --arch=powerpc \
            --cpu=g4 \
            --enable-altivec \
            --enable-shared \
            $CONFIGOPTS
    )
else
    echo $0: skipping configure
fi

if $clean; then
    echo $0: clean i386
    ( cd build-i386; make clean)
    echo $0: clean ppc
    ( cd build-ppc; make clean)
else
    echo $0: skipping clean
fi

if $build; then
    echo $0: build i386
    ( cd build-i386; make)
    echo $0: build ppc
    ( cd build-ppc; make)
else
    echo $0: skipping build
fi

if $merge; then
    echo $0: merge
    lipo -create -output libavformat/libavformat.a build-{i386,ppc}/libavformat/libavformat.a
    lipo -create -output libavcodec/libavcodec.a build-{i386,ppc}/libavcodec/libavcodec.a
    lipo -create -output libavutil/libavutil.a build-{i386,ppc}/libavutil/libavutil.a
    lipo -create -output libswscale/libswscale.a build-{i386,ppc}/libswscale/libswscale.a
    # XXXX Should also do the dynamic libraries?
    # Also need to copy .pc files. Do it here.
    # NOTE: this is correct as of ffmpeg 15-jun-08, expected to change soon (ffmpeg are
    # moving their .pc files around).
    cp build-i386/*.pc .    
    # Here's the newer(?) version:
    #cp build-i386/libav{format,util,codec}/*.pc .
else
    echo $0: skipping merge
fi

if $install; then
    echo $0: install
    echo Sorry, cannot do yet.
    exit 1
else
    echo $0: skipping install
fi