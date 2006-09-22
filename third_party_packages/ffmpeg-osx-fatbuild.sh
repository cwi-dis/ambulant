#!/bin/sh
#
srcdir=$1
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
fi

if $mklinks; then
    echo $0: mklinks
    ( cd libavformat ; ln -s $srcdir/libavformat/*.h .)
    ( cd libavcodec ; ln -s $srcdir/libavcodec/*.h .)
    ( cd libavutil ; ln -s $srcdir/libavutil/*.h .)
fi

if $configure; then
    echo $0: configure i386
    (
        cd build-i386
        $srcdir/configure \
            --extra-cflags="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386" \
            --extra-ldflags="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386" \
            --cpu=i386
    )
    echo $0: configure ppc
    (
        cd build-ppc
        $srcdir/configure \
            --extra-cflags="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc" \
            --extra-ldflags="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc" \
            --cpu=ppc --enable-shared
    )
fi

if $clean; then
    echo $0: clean i386
    ( cd build-i386; make clean)
    echo $0: clean ppc
    ( cd build-ppc; make clean)
fi

if $build; then
    echo $0: build i386
    ( cd build-i386; make)
    echo $0: build ppc
    ( cd build-ppc; make)
fi

if $merge; then
    echo $0: merge
    lipo -create -output libavformat/libavformat.a build-{i386,ppc}/libavformat/libavformat.a
    lipo -create -output libavcodec/libavcodec.a build-{i386,ppc}/libavcodec/libavcodec.a
    lipo -create -output libavutil/libavutil.a build-{i386,ppc}/libavutil/libavutil.a
    # XXXX Should also do the dynamic libraries?
fi

if $install; then
    echo $0: install
    echo Sorry, cannot do yet.
    exit 1
fi