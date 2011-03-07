#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Mac 10.6 version
#
set -e
set -x
AMBULANTVERSION=2.3
HGARGS=""
HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
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
DESTINATION=ssh.cwi.nl:public_html/ambulant/nightly
DESTINATION_DESKTOP=$DESTINATION/mac-intel-desktop-cocoa/
DESTINATION_PLUGIN=$DESTINATION/mac-intel-webkitplugin/
DESTINATION_CG=$DESTINATION/mac-intel-desktop-cg/

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
rm -rf $BUILDDIR
hg $HGARGS clone $HGCLONEARGS $BUILDDIR
#
# We are building a binary distribution, so we want to completely ignore any
# library installed system-wide (in /usr/local, basically)
#
export PKG_CONFIG_LIBDIR=$BUILDHOME/$BUILDDIR/build-3264/third_party_packages/installed/lib/pkgconfig
#
# Prepare the tree
#
cd $BUILDDIR
sh autogen.sh
rm -rf build-3264
mkdir build-3264
cd build-3264
mkdir third_party_packages
cd third_party_packages
python ../../third_party_packages/build-third-party-packages.py $BUILD3PPARGS
cd ..
#
# configure, make, make install
#
../configure $CONFIGOPTS
make $MAKEOPTS
cd src/player_macosx
make $MAKEOPTS DESTDIR=$BUILDHOME/$DESTDIR install
cd ../.. # Back to build dir
cd .. # Back to source dir
#
# Create installer dmg, upload
#
cd installers/sh-macos
sh mkmacdist.sh $DMGNAME $BUILDHOME/$DESTDIR
scp $DMGNAME.dmg $DESTINATION_DESKTOP
cd ../..
#
# Build CG player
#
cd projects/xcode32
xcodebuild -project AmbulantPlayer.xcodeproj \
	-target AmbulantPlayerCG \
	-configuration Release \
	AMBULANT_BUILDDIR=$BUILDHOME/$BUILDDIR \
	AMBULANT_3PP=$BUILDHOME/$BUILDDIR/build-3264/third_party_packages \
	DSTROOT=$BUILDHOME/$DESTDIR \
	install
cd ../..
#
# Create installer dmg, upload
#
cd installers/sh-macos
sh mkmacdist.sh -a AmbulantPlayerCG.app $DMGNAME-CG $BUILDHOME/$DESTDIR
scp $DMGNAME-CG.dmg $DESTINATION_CG
cd ../..
#
# Build webkit plugin
#
cd projects/xcode32
rm -rf "$HOME/Library/Internet Plug-Ins/AmbulantWebKitPlugin.plugin"
mkdir -p "$HOME/Library/Internet Plug-Ins"
xcodebuild -project AmbulantWebKitPlugin.xcodeproj -target AmbulantWebKitPlugin -configuration Release -sdk macosx10.6
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
scp $PLUGINDMGNAME.zip $DESTINATION_PLUGIN
#
# Delete old installers, remember current
#
# XXX TODO
