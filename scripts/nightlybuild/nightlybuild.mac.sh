#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Mac 10.7 version
#
set -e
set -x
export PATH=/usr/local/bin:`xcode-select -print-path`/usr/bin:$PATH

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
export MACOSX_DEPLOYMENT_TARGET=10.7
export SDKROOT=/Developer/SDKs/MacOSX$MACOSX_DEPLOYMENT_TARGET.sdk
HGARGS=""
HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
DESTINATION_HOST=sen5@ambulantplayer.org
DESTINATION_DIR=/scratch/www/vhosts/ambulantplayer.org/public_html/nightlybuilds
BUILDHOME=$HOME/tmp/ambulant-nightly
MAKEOPTS=-j2
TODAY=`date +%Y%m%d`

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
BUILDDIR=ambulant-build-$TODAY
DESTDIR=ambulant-install-$TODAY
BUILD3PPARGS=macosx$MACOSX_DEPLOYMENT_TARGET
CONFIGOPTS="--with-macfat --disable-dependency-tracking --with-xerces-plugin --with-python=/usr/bin/python --with-python-plugin --with-included-ltdl"
DMGNAME=Ambulant-$AMBULANTVERSION$VERSIONSUFFIX-mac
PLUGINNAME=npambulant-$AMBULANTVERSION$VERSIONSUFFIX-mac
PLUGINDMGNAME=$PLUGINNAME.dmg
DESTINATION=$DESTINATION_HOST:$DESTINATION_DIR
DESTINATION_DESKTOP_DIR=$DESTINATION_DIR/mac-intel-desktop-cg/
DESTINATION_PLUGIN_DIR=$DESTINATION_DIR/mac-intel-firefoxplugin/
DESTINATION_DESKTOP=$DESTINATION_HOST:$DESTINATION_DESKTOP_DIR
DESTINATION_PLUGIN=$DESTINATION_HOST:$DESTINATION_PLUGIN_DIR

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
export PKG_CONFIG_LIBDIR=$BUILDHOME/$BUILDDIR/build-3264/third_party_packages/installed/lib/pkgconfig
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
rm -rf build-3264
mkdir build-3264
cd build-3264
mkdir third_party_packages
cd third_party_packages
python ../../scripts/build-third-party-packages.py $BUILD3PPARGS
PATH=`pwd`/installed/bin:$PATH
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
sh make-dmg-installer.sh -n 'Ambulant Player' -t AmbulantPlayer-template.dmg -s "$BUILDHOME/$DESTDIR/Applications/Ambulant Player.app/." -d "Ambulant Player.app/." -s ../../README -d ./README -s ../../COPYING  -d ./COPYING -s ../../Extras/DemoPresentation/. -d DemoPresentation/.
mv "Ambulant Player.dmg" $DMGNAME.dmg
ssh -n $DESTINATION_HOST mkdir -p $DESTINATION_DESKTOP_DIR
scp $DMGNAME.dmg $DESTINATION_DESKTOP
cd ../..
#
# Build npambulant (Internet Plugin).
#
cd $BUILDHOME/$BUILDDIR
cd projects/xcode43
rm -rf "$HOME/Library/Internet Plug-Ins/npambulant.plugin"
mkdir -p "$HOME/Library/Internet Plug-Ins"
xcodebuild -project npambulant.xcodeproj \
	-target npambulant \
	-configuration Release -sdk macosx10.7 \
	AMBULANT_BUILDDIR=$BUILDHOME/$BUILDDIR \
	AMBULANT_3PP=$BUILDHOME/$BUILDDIR/build-3264/third_party_packages \
	DSTROOT=$BUILDHOME/$DESTDIR \
	INSTALL_PATH="/Library/Internet Plug-ins" \
	install
cd ../..
#
# Build plugin installer, upload
#
cd installers/sh-macos
sh make-dmg-installer.sh -n 'Ambulant Web Plugin' -t npambulant-template.dmg -s "$BUILDHOME/$DESTDIR/Library/Internet Plug-ins/npambulant.plugin/." -d "npambulant.plugin/." -s npambulant-installer-README -d ./README -s ../../COPYING  -d ./COPYING
mv "Ambulant Web Plugin.dmg" $PLUGINDMGNAME
ssh -n $DESTINATION_HOST mkdir -p $DESTINATION_PLUGIN_DIR
scp $PLUGINDMGNAME $DESTINATION_PLUGIN
#
# Delete old installers, remember current
#
# XXX TODO
