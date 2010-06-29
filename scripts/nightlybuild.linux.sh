#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Mac 10.6 version
#
set -e
set -x
AMBULANTVERSION=2.3
ARCH=i686
CVSUSER="jackjansen"
CVSARGS="-d $CVSUSER@ambulant.cvs.sourceforge.net:/cvsroot/ambulant"
CHECKOUTARGS=
BUILDHOME=$HOME/tmp/ambulant-nightly
TODAY=`date +%Y%m%d`
BUILDDIR=ambulant-build-$TODAY
DESTDIR=ambulant-install-$TODAY
BUILD3PPARGS=linux
CONFIGOPTS="--with-qt --with-gtk --with-xerces --with-xerces-plugin --with-npambulant"
MAKEOPTS=
VERSIONSUFFIX=.$TODAY
##PLUGINDMGNAME=AmbulantWebKitPlugin-$AMBULANTVERSION$VERSIONSUFFIX-mac
DESTINATION=ssh.cwi.nl:public_html/ambulant/

#
# Check out a fresh copy of Ambulant
#
mkdir -p $BUILDHOME
cd $BUILDHOME
cvs $CVSARGS checkout $CHECKOUTARGS -d "$BUILDDIR" ambulant
###
### We are building a binary distribution, so we want to completely ignore any
### library installed system-wide (in /usr/local, basically)
###
##export PKG_CONFIG_LIBDIR=$BUILDHOME/$BUILDDIR/third_party_packages/installed/lib/pkgconfig

#
# Prepare the tree
#
cd $BUILDDIR
sh autogen.sh
cd third_party_packages
python build-third-party-packages.py $BUILD3PPARGS
cd ..


#
# Build source distribution, upload
#
./configure $CONFIGOPTS
make $MAKEOPTS distcheck
make $MAKEOPTS dist
mv ambulant-$AMBULANTVERSION.tar.gz ambulant-$AMBULANTVERSION$VERSIONSUFFIX.tar.gz
scp ambulant-$AMBULANTVERSION$VERSIONSUFFIX.tar.gz $DESTINATION

#
# configure, make, make install
#
./configure $CONFIGOPTS
make $MAKEOPTS
make $MAKEOPTS DESTDIR=$BUILDHOME/$DESTDIR install

#
# Create installer package, upload
#
# XXX TODO

#
# Build plugin installer, upload
#
cd src/npambulant
make installer
mv npambulant-$AMBULANTVERSION-linux-$ARCH.xpi npambulant-$AMBULANTVERSION$VERSIONSUFFIX-linux-$ARCH.xpi
scp npambulant-$AMBULANTVERSION$VERSIONSUFFIX-linux-$ARCH.xpi $DESTINATION
cd ../..
#
# Delete old installers, remember current
#
# XXX TODO
