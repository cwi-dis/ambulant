#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Mac 10.6 version
#
set -e
set -x
CVSUSER="jackjansen"
CVSARGS="-d $CVSUSER@ambulant.cvs.sourceforge.net:/cvsroot/ambulant"
CHECKOUTARGS=
BUILDHOME=$HOME/tmp/ambulant-nightly
BUILDDIR=ambulant-build-`date +%Y%m%d`
DESTDIR=ambulant-install-`date +%Y%m%d`
BUILD3PPARGS=mac10.6
CONFIGOPTS="--with-macfat --disable-dependency-tracking --with-xerces-plugin"
MAKEOPTS=-j2
#
# Check out a fresh copy of Ambulant
#
mkdir -p $BUILDHOME
cd $BUILDHOME
##cvs $CVSARGS checkout $CHECKOUTARGS -d "$BUILDDIR" ambulant
#
# Prepare the tree
#
cd $BUILDDIR
sh autogen.sh
cd third_party_packages
python build-third-party-packages.py $BUILD3PPARGS
#
# configure, make, make install
#
cd ..
./configure $CONFIGOPTS
make $MAKEOPTS
cd src/player_macosx
make $MAKEOPTS DESTDIR=$BUILDHOME/$DESTDIR install
#
# Create installer dmg
#
# XXX TODO
#
# Build webkit plugin
#
# XXX TODO
#
# Build plugin installer
#
# XXX TODO
#
# Upload installers
#
# XXX TODO
#
# Delete old installers, remember current
#
# XXX TODO
