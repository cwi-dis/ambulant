#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Mac 10.6 version
#
set -e
set -x
export CVS_RSH=ssh
AMBULANTVERSION=2.3
CVSUSER="jackjansen"
CVSARGS="-d $CVSUSER@ambulant.cvs.sourceforge.net:/cvsroot/ambulant"
CHECKOUTARGS=-P
BUILDHOME=$HOME/tmp/ambulant-nightly
TODAY=`date +%Y%m%d`
BUILDDIR=ambulant-build-$TODAY
DESTDIR=ambulant-install-$TODAY
BUILD3PPARGS=mac10.6
CONFIGOPTS="--with-macfat --disable-dependency-tracking --with-xerces-plugin"
MAKEOPTS=-j2
VERSIONSUFFIX=.$TODAY
DMGNAME=Ambulant-$AMBULANTVERSION$VERSIONSUFFIX-mac
PLUGINDMGNAME=AmbulantWebKitPlugin-$AMBULANTVERSION$VERSIONSUFFIX-mac
DESTINATION=ssh.cwi.nl:public_html/ambulant/

echo nightly to stderr >&2
echo nightly to stdout
echo nightly to stderr again >2&

echo
echo ==========================================================
echo Ambulant nightly build for MacOSX, $USER@`hostname`, `date`
echo ==========================================================
echo

#
# Check out a fresh copy of Ambulant
#
mkdir -p $BUILDHOME
cd $BUILDHOME
cvs $CVSARGS checkout $CHECKOUTARGS -d "$BUILDDIR" ambulant
#
# We are building a binary distribution, so we want to completely ignore any
# library installed system-wide (in /usr/local, basically)
#
export PKG_CONFIG_LIBDIR=$BUILDHOME/$BUILDDIR/third_party_packages/installed/lib/pkgconfig
#
# Prepare the tree
#
cd $BUILDDIR
sh autogen.sh
cd third_party_packages
python build-third-party-packages.py $BUILD3PPARGS
cd ..
#
# configure, make, make install
#
./configure $CONFIGOPTS
make $MAKEOPTS
cd src/player_macosx
make $MAKEOPTS DESTDIR=$BUILDHOME/$DESTDIR install
cd ../..
#
# Create installer dmg, upload
#
cd installers/sh-macos
sh mkmacdist.sh $DMGNAME $BUILDHOME/$DESTDIR
scp $DMGNAME.dmg $DESTINATION
cd ../..
#
# Build webkit plugin
#
cd src/webkit_plugin
rm -rf "$HOME/Library/Internet Plug-Ins/AmbulantWebKitPlugin.plugin"
mkdir -p "$HOME/Library/Internet Plug-Ins"
xcodebuild -project AmbulantWebKitPlugin.xcodeproj -target AmbulantWebKitPlugin -configuration Release-10.6 -sdk macosx10.6
cd ../..
#
# Build plugin installer, upload
#
mkdir -p "$BUILDHOME/$DESTDIR/Library/Internet Plug-Ins"
cd "$BUILDHOME/$DESTDIR/Library/Internet Plug-Ins"
rm -rf $PLUGINDMGNAME
mkdir $PLUGINDMGNAME
mv "$HOME/Library/Internet Plug-Ins/AmbulantWebKitPlugin.plugin" $PLUGINDMGNAME
cp $BUILDHOME/$BUILDDIR/src/webkit_plugin/README $PLUGINDMGNAME
zip -r $PLUGINDMGNAME.zip $PLUGINDMGNAME
scp $PLUGINDMGNAME.zip $DESTINATION
#
# Delete old installers, remember current
#
# XXX TODO
