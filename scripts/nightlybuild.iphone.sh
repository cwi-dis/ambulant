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
VERSIONSUFFIX=.$TODAY
DESTINATION=sen5@ambulantplayer.org:/var/www/AmbulantPlayerOrg/nightlybuilds/iPhone/

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
export PKG_CONFIG_LIBDIR=$BUILDHOME/$BUILDDIR/build-iOS-Fat/third_party_packages/installed/lib/pkgconfig
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
##	DSTROOT=$BUILDHOME/$DESTDIR \
##	INSTALL_PATH=/Applications \
cd ../..
#
# Create installer IPA file and upload
#
sh installers/mkiphonedist.sh iAmbulant-$AMBULANTVERSION.$TODAY.ipa
scp iAmbulant-$AMBULANTVERSION.$TODAY.ipa $DESTINATION
#
# Delete old installers, remember current
#
# XXX TODO
