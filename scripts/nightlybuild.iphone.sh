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
BUILDDIR=ambulant-iphone-build-$TODAY
DESTDIR=ambulant-iphone-install-$TODAY
BUILD3PPARGS=mac10.6 xyzzy
CONFIGOPTS="--with-macfat --disable-dependency-tracking --with-xerces-plugin --with-python --with-python-plugin"
MAKEOPTS=-j2
VERSIONSUFFIX=.$TODAY
DMGNAME=Ambulant-$AMBULANTVERSION$VERSIONSUFFIX-mac
PLUGINDMGNAME=AmbulantWebKitPlugin-$AMBULANTVERSION$VERSIONSUFFIX-mac
DESTINATION=sen5@ambulantplayer.org:/var/www/AmbulantPlayerOrg/nightlybuilds
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
rm -rf $DESTDIR
ls -t | tail +6 | grep ambulant- | xargs rm -rf
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
#
# Build CG player
#
cd projects/xcode32
xcodebuild -project iAmbulant.xcodeproj \
	-target iAmbulant \
	-configuration Distribution \
	-sdk iphoneos4.2 \
	AMBULANT_BUILDDIR=$BUILDHOME/$BUILDDIR \
	build
##	AMBULANT_3PP=$BUILDHOME/$BUILDDIR/build-3264/third_party_packages \
##	DSTROOT=$BUILDHOME/$DESTDIR \
##	INSTALL_PATH=/Applications \
cd ../..
echo That went well!
exit
#
# Create installer dmg, upload
#
cd installers/sh-macos
sh mkmacdist.sh -a AmbulantPlayerCG.app $DMGNAME-CG $BUILDHOME/$DESTDIR
scp $DMGNAME-CG.dmg $DESTINATION_CG
cd ../..
#
# Build webkit plugin.
#
cd projects/xcode32
rm -rf "$HOME/Library/Internet Plug-Ins/AmbulantWebKitPlugin.plugin"
mkdir -p "$HOME/Library/Internet Plug-Ins"
xcodebuild -project AmbulantWebKitPlugin.xcodeproj \
	-target AmbulantWebKitPlugin \
	-configuration Release -sdk macosx10.6 \
	AMBULANT_BUILDDIR=$BUILDHOME/$BUILDDIR \
	AMBULANT_3PP=$BUILDHOME/$BUILDDIR/build-3264/third_party_packages \
	DSTROOT=$BUILDHOME/$DESTDIR \
	INSTALL_PATH="/Library/Internet Plug-ins" \
	install
cd ../..
#
# Build plugin installer, upload
#
mkdir -p "$BUILDHOME/$DESTDIR/Library/Internet Plug-Ins"
cd "$BUILDHOME/$DESTDIR/Library/Internet Plug-Ins"
rm -rf $PLUGINDMGNAME
mkdir $PLUGINDMGNAME
mv "AmbulantWebKitPlugin.webplugin" $PLUGINDMGNAME
cp $BUILDHOME/$BUILDDIR/src/webkit_plugin/README $PLUGINDMGNAME
zip -r $PLUGINDMGNAME.zip $PLUGINDMGNAME
scp $PLUGINDMGNAME.zip $DESTINATION_PLUGIN
#
# Delete old installers, remember current
#
# XXX TODO
