#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Linux version
#
# If you are on Ubuntu, you will need to 'apt-get install' at least the following packages:
#  mercurial
#  chrpath
#  autogen
#  autoconf
#  automake
#  libtool
#  g++
#  yasm
#  gettext
#  libpango1.0-dev
#  libgtk2.0-dev
#  libqt3-mt-dev
#  postfix
#  mailutils
#  curl
#  ssh
#  libxt-dev
#  libxext-dev
#  libsdl1.2-dev
#
set -e
set -x

# An optional parameter is the branch name, which also sets destination directory
BRANCH=
case x$1 in
x)	
	BRANCH=default
	;;
*)	BRANCH=$1
esac

# Tunable parameters, to some extent
AMBULANTVERSION=2.7
ARCH=`uname -m`
HGARGS=""
HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
DESTINATION_HOST=sen5@ambulantplayer.org
DESTINATION_DIR=/scratch/www/vhosts/ambulantplayer.org/public_html/nightlybuilds
BUILDHOME=$HOME/tmp/ambulant-nightly
TODAY=`date +%Y%m%d`

# The rest should be automatic
case x$BRANCH in
x)	
	;;
xrelease*)
	TODAY=$TODAY-$BRANCH
	DESTINATION_DIR=$DESTINATION_DIR/$BRANCH
	VERSIONSUFFIX=$VERSIONSUFFIXADDITION
	;;
*)
	TODAY=$TODAY-$BRANCH
	DESTINATION_DIR=$DESTINATION_DIR/$BRANCH
	VERSIONSUFFIX=.$TODAY$VERSIONSUFFIXADDITION
esac
BUILDDIR=ambulant-build-$TODAY
DESTDIR=ambulant-install-$TODAY
BUILD3PPARGS=linux
CONFIGOPTS="--with-sdl2 --with-xerces --with-xerces-plugin CFLAGS=-fPIC CXXFLAGS=-fPIC"
MAKEOPTS=

DESTINATION=$DESTINATION_HOST:$DESTINATION_DIR
DESTINATION_SRC_DIR=$DESTINATION_DIR/src/
DESTINATION_NPAMBULANT_DIR=$DESTINATION_DIR/linux-$ARCH-firefoxplugin/
DESTINATION_SRC=$DESTINATION_HOST:$DESTINATION_SRC_DIR
DESTINATION_NPAMBULANT=$DESTINATION_HOST:$DESTINATION_NPAMBULANT_DIR

echo
echo ==========================================================
echo Ambulant nightly build for Linux, $ARCH, $USER@`hostname`, `date`
echo LogLocation=linux-$ARCH-$TODAY.txt
echo ==========================================================
echo

#
# Check that chrpath exists (which it often doesn't)
#
chrpath -v >/dev/null 2>&1
case $? in
0)
	;;
*)
	echo $0: chrpath not found, therefore nightly build will fail. Please install it first.
	exit 1
	;;
esac
#
# Check out a fresh copy of Ambulant
#
mkdir -p $BUILDHOME
cd $BUILDHOME
rm -rf $BUILDDIR
rm -rf $DESTDIR
touch .empty
echo If the following commands fails you have no SSH key that matches the destination
ssh -n $DESTINATION_HOST mkdir -p $DESTINATION_DIR
scp .empty $DESTINATION/.empty

ls -t | tail -n +6 | grep ambulant- | xargs chmod -R a+w .empty
ls -t | tail -n +6 | grep ambulant- | xargs rm -rf
hg $HGARGS clone $HGCLONEARGS $BUILDDIR
###
### We are building a binary distribution, so we want to completely ignore any
### library installed system-wide (in /usr/local, basically)
###
##export PKG_CONFIG_LIBDIR=$BUILDHOME/$BUILDDIR/third_party_packages/installed/lib/pkgconfig

#
# Prepare the tree
#
cd $BUILDDIR
case x$BRANCH in
x)	;;
*)
	hg up -r $BRANCH
esac
sh autogen.sh
cd third_party_packages
# Remember the standard pkg-config library search directories, we need it for npambulant
PKG_CONFIG_DEFAULT_LIB_PATH=`pkg-config --variable pc_path pkg-config`
# We are building for distribution, so we want to force local copies of packages.
# But we do want to obtain packages we don't buil ourselves (such as gtk).
PKG_CONFIG_LIBDIR=`pwd`/installed/lib/pkgconfig python ../scripts/build-third-party-packages.py -x $BUILD3PPARGS
PATH=`pwd`/installed/bin:$PATH
cd ..

#
# Build source distribution, upload
#
./configure $CONFIGOPTS
make $MAKEOPTS distcheck
make $MAKEOPTS dist
case x$VERSIONSUFFIX in
x)	
	;;
*)
	mv ambulant-$AMBULANTVERSION.tar.gz ambulant-$AMBULANTVERSION$VERSIONSUFFIX.tar.gz
	;;
esac
ssh -n $DESTINATION_HOST mkdir -p $DESTINATION_SRC_DIR
scp ambulant-$AMBULANTVERSION$VERSIONSUFFIX.tar.gz $DESTINATION_SRC

#
# configure, make, make install AmvulantPlayer_gtk
#
./configure $CONFIGOPTS --with-gtk=gtk2  --without-npambulant
make $MAKEOPTS
make $MAKEOPTS DESTDIR=$BUILDHOME/$DESTDIR install
#
# Create installer package, upload
#
# XXX TODO

#
# Build plugin installer, upload
#
# npambulant cannot yet use gtk+-3.0, therefore fall back to gtk+-3`3.0
make uninstall
make distclean
# turn off pkg-config standard lib search path and use our versions before the default ones
PKG_CONFIG_LIBDIR= PKG_CONFIG_PATH=$BUILDDIR/third_party_packages/installed/lib/pkgconfig:$PKG_CONFIG_DEFAULT_LIB_PATH ./configure $CONFIGOPTS --with-gtk=gtk2  --with-npambulant --without-gcd-event-processor
make $MAKEOPTS
make $MAKEOPTS DESTDIR=$BUILDHOME/$DESTDIR install

cd src/npambulant
make installer
case x$VERSIONSUFFIX in
x)	
	;;
*)
	mv npambulant-$AMBULANTVERSION-linux-$ARCH.xpi npambulant-$AMBULANTVERSION$VERSIONSUFFIX-linux-$ARCH.xpi
	;;
esac
ssh -n $DESTINATION_HOST mkdir -p $DESTINATION_NPAMBULANT_DIR
scp npambulant-$AMBULANTVERSION$VERSIONSUFFIX-linux-$ARCH.xpi $DESTINATION_NPAMBULANT
cd ../..
#
# Delete old installers, remember current
#
# XXX TODO
