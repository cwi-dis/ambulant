#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Mac 10.6 version
#
set -e
set -x

# An optional parameter is the branch name, which also sets destination directory
BRANCH=
case x$1 in
x)	;;
*)	BRANCH=$1
esac

# Tunable parameters, to some extent
AMBULANTVERSION=2.3
HGARGS=""
HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
DESTINATION=sen5@ambulantplayer.org:/var/www/AmbulantPlayerOrg/nightlybuilds
BUILDHOME=$HOME/tmp/ambulant-nightly
MAKEOPTS=-j2
TODAY=`date +%Y%m%d`

# The rest should be automatic
case x$BRANCH in
x)	;;
*)
	TODAY=$TODAY-$BRANCH
	DESTINATION=$DESTINATION/$BRANCH
esac
BUILDDIR=ambulant-build-$TODAY
DESTDIR=ambulant-install-$TODAY
BUILD3PPARGS=mac10.6
CONFIGOPTS="--with-macfat --disable-dependency-tracking --with-xerces-plugin --with-python --with-python-plugin"
VERSIONSUFFIX=.$TODAY
DMGNAME=Ambulant-$AMBULANTVERSION$VERSIONSUFFIX-mac
PLUGINDMGNAME=AmbulantWebKitPlugin-$AMBULANTVERSION$VERSIONSUFFIX-mac
DESTINATION_DESKTOP=$DESTINATION/mac-intel-desktop-cocoa/
DESTINATION_PLUGIN=$DESTINATION/mac-intel-webkitplugin/
DESTINATION_CG=$DESTINATION/mac-intel-desktop-cg/

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
echo If the following command fails you have no SSH key that matches the destination
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
	INSTALL_PATH=/Applications \
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
