#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# iPhone version
#
set -e
set -x
export PATH=/usr/local/bin:`xcode-select -print-path`/usr/bin:$PATH

# Unlock the nightly build keychain
security unlock-keychain -p ambulant $HOME/Library/Keychains/nightlybuilds.keychain
security default-keychain -s $HOME/Library/Keychains/nightlybuilds.keychain

# An optional parameter is the branch name, which also sets destination directory
BRANCH=
case x$1 in
x)	
	BRANCH=default
	;;
*)	BRANCH=$1
esac

# Tunable parameters, to some extent
AMBULANTVERSION=2.5
HGARGS=""
HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
DESTINATION_HOST=sen5@ambulantplayer.org
DESTINATION_DIR=/scratch/www/vhosts/ambulantplayer.org/public_html/nightlybuilds
BUILDHOME=$HOME/tmp/ambulant-nightly
TODAY=`date +%Y%m%d`

if [ -f $HOME/.bashrc ]; then
	. $HOME/.bashrc
fi

# The rest should be automatic
case x$BRANCH in
x)	
	;;
xrelease*)
	TODAY=$TODAY-$BRANCH
	DESTINATION_DIR=$DESTINATION_DIR/$BRANCH
	VERSIONSUFFIX=
	;;
*)
	TODAY=$TODAY-$BRANCH
	DESTINATION_DIR=$DESTINATION_DIR/$BRANCH
	VERSIONSUFFIX=.$TODAY
esac
BUILDDIR=ambulant-iphone-build-$TODAY
DESTDIR=ambulant-iphone-install-$TODAY
DESTINATION=$DESTINATION_HOST:$DESTINATION_DIR
DESTINATION_IPHONE_DIR=$DESTINATION_DIR/iphone/
DESTINATION_IPHONE=$DESTINATION_HOST:$DESTINATION_IPHONE_DIR

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
touch .empty
echo If the following commands fails you have no SSH key that matches the destination
ssh -n $DESTINATION_HOST mkdir -p $DESTINATION_DIR
scp .empty $DESTINATION/.empty

ls -t | tail -n +6 | grep ambulant- | xargs chmod -R a+w .empty
ls -t | tail -n +6 | grep ambulant- | xargs rm -rf
hg $HGARGS clone $HGCLONEARGS $BUILDDIR
#
# We are building a binary distribution, so we want to completely ignore any
# library installed system-wide (in /usr/local, basically)
#
export PKG_CONFIG_LIBDIR=$BUILDHOME/$BUILDDIR/build-iOS/third_party_packages/installed/lib/pkgconfig
#
# Prepare the tree
#
cd $BUILDDIR
case x$BRANCH in
x)	;;
*)
hg up -r $BRANCH
esac
if [ ! -e libltdl/m4 ] ; then mkdir -p libltdl/m4 ; fi
sh autogen.sh
#
# Build CG player
#
# export MAKEFLAGS=-j`sysctl -a|grep core_count|awk '{print $2}'`
cd projects/xcode43
xcodebuild -project libambulant.xcodeproj \
	-target libambulantiPhone \
	-configuration Release \
	-sdk iphoneos6.0 \
	build
#
# The keychain may have been locked again in the mean time
#
security unlock-keychain -p ambulant $HOME/Library/Keychains/nightlybuilds.keychain
security default-keychain -s $HOME/Library/Keychains/nightlybuilds.keychain
xcodebuild -project iAmbulant.xcodeproj \
	-target iAmbulant \
	-configuration Distribution \
	-sdk iphoneos6.0 \
	build
## DSTROOT=$BUILDHOME/$DESTDIR \
## INSTALL_PATH=/Applications \
cd ../..
#
# Create installer IPA file and upload
#
sh installers/mkiphonedist.sh iAmbulant-$AMBULANTVERSION.$TODAY.ipa projects/xcode43/build/Distribution-iphoneos/iAmbulant.app
ssh -n $DESTINATION_HOST mkdir -p $DESTINATION_IPHONE_DIR
scp iAmbulant-$AMBULANTVERSION.$TODAY.ipa $DESTINATION_IPHONE
#
# Delete old installers, remember current
#
# XXX TODO
