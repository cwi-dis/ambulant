#!/bin/sh
#
# Script to do a clean build of a full gstambulantsrc Debian package for Ubuntu.
#
# Derived from: .../ambulant/scripts/nightlybiuild/nightlybuild.debian.sh
#
# NOTE: a number of things must be installed (and working) for this
# script to run successfully, a.o:
#
#  postfix mailutils mercurial curl ssh devscripts chrpath debhelper pkg-config
#  libgstreamer1.0-dev (and its plugins, tools etc.)
#
# NOTE 2: the key used for signing (in debuild) must have no passphrase.
# I think this can only be done with the gpg --edit-key command line
# interface, then use the "passwd" command and set the passphrase to empty.
# The key currently used is ambulant-private.
set -e
set -x

MODULE_VERSION=1.0
MODULE=gstambulantsrc
BUILD_INDEX=1

# An optional parameter is the branch name, which also sets destination directory
BRANCH=
case x$1 in
x)	
	BRANCH=default
	;;
*)	BRANCH=$1
esac

# Tunable parameters, to some extent
## AMBULANTVERSION=2.5
ARCH=`uname -p`
## HGARGS=""
## HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
## DESTINATION=sen5@ambulantplayer.org:/var/www/AmbulantPlayerOrg/nightlybuilds
BUILDHOME=$HOME/tmp/$MODULE
TODAY=`date +%Y%m%d`

# The rest should be automatic
case x$BRANCH in
x)	
	;;
xrelease*)
	DESTINATION=$DESTINATION/$BRANCH
	VERSIONSUFFIX=
	UBUNTUPPA=ppa:ambulant/ambulant
	release=yes
	;;
*)
	DESTINATION=$DESTINATION/$BRANCH
	VERSIONSUFFIX=.$TODAY
	UBUNTUPPA=ppa:ambulant/ambulant-nightly
	release=no
esac
CLDATE=`date --rfc-2822`
BUILDDIR=$MODULE-debian-$TODAY

DISTRIB_RELEASE=unknown
DISTRIB_CODENAME=unknown
. /etc/lsb-release
DEBARCH=`dpkg-architecture -qDEB_HOST_ARCH`
DESTINATION_DEBIAN=$DESTINATION/deb/
DESTINATION_WORKAREA=dists
RELPATH_SRC=$DESTINATION_WORKAREA/$DISTRIB_RELEASE/$MODULE/source
RELPATH_BIN=$DESTINATION_WORKAREA/$DISTRIB_RELEASE/$MODULE/binary-$DEBARCH
## echo
## echo ==========================================================
## echo Ambulant nightly build for Debian, $ARCH, $USER@`hostname`, `date`
## echo ==========================================================
## echo

##
## TBD Check out a fresh copy of $MODULE
## TBD Copy the source
#
if [ ! -e ./autogen.sh ] 
then   
     echo "autogen.sh not found in $PWD"
     echo "This script is intended to run from toplevel $MODULE"
     exit -1
fi
SOURCE_DIR=$PWD
MICRO_VERSION=`cat debian/next_micro_version`
MODULE_VERSION=$MODULE_VERSION.$MICRO_VERSION
FULLVERSION=$MODULE_VERSION-$BUILD_INDEX ##$VERSIONSUFFIX~$DISTRIB_CODENAME
# Update version in configure.ac to match this script
sed -e "/AC_INIT/s/AC_INIT(\[$MODULE\],\[.*\])/AC_INIT([$MODULE],[$MODULE_VERSION])/"  configure.ac >  configure.ac.tmp; mv  configure.ac.tmp  configure.ac 

rm -fr $BUILDHOME
BUILDHOME=$BUILDHOME/$MODULE-$MODULE_VERSION
mkdir -p $BUILDHOME
cp -r . $BUILDHOME
cd $BUILDHOME
rm -rf $BUILDDIR
rm -rf $DESTDIR
## touch .empty
## echo If the following command fails you have no SSH key that matches the destination
## scp .empty $DESTINATION/.empty

## ls -t | tail -n +6 | grep debian- | xargs chmod -R a+w .empty
## ls -t | tail -n +6 | grep debian- | xargs rm -rf

#
# Create staging area
#
rm -rf $DESTINATION_WORKAREA
mkdir -p $RELPATH_SRC/debian-$TODAY
mkdir -p $RELPATH_BIN/debian-$TODAY

## #
## # Prepare the tree
## #
## hg $HGARGS clone $HGCLONEARGS $BUILDDIR
## cd $BUILDDIR
## case x$BRANCH in
## x)	;;
## *)
## 	hg up -r $BRANCH
## esac

## # Get rid of mercurial administration
## rm -r .hg
## # Get rid of Sandbox
## rm -r sandbox

sh autogen.sh
make dist ## check
mv $MODULE-$MODULE_VERSION.tar.gz ../$MODULE\_$MODULE_VERSION.orig.tar.gz
# Replace debian changelog for nightly build distributions
case x$release in
xno)
cat > debian/changelog << xyzzy
$MODULE ($FULLVERSION) $DISTRIB_CODENAME; urgency=low

  * Nightly build, for testing only

 -- CWI $MODULE Team <ambulant@cwi.nl>  $CLDATE
xyzzy
;;
esac
mkdir -p debian/source
cat > debian/source/format <<EOF
3.0 (quilt)
EOF
#
# Build debian packages, first binary then source
#

cd debian
echo "Parent directory: $PWD/.."
debuild -kC75B80BC 
cd ..
cd ..
echo $PWD;ls
TMPDIR=$HOME/tmp/tmp-$TODAY
mkdir -p $TMPDIR
mv *.deb *.dsc *.changes $TMPDIR

cd $BUILDHOME
cd debian
debuild -S -sa -kC75B80BC 
cd ..
cd ..
mv *.tar.gz *.dsc *.changes *.build $BUILDHOME/$RELPATH_SRC/debian-$TODAY/
mv $TMPDIR/* $BUILDHOME/$RELPATH_BIN/debian-$TODAY/
rm -fr $TMPDIR
cd $BUILDHOME
dpkg-scanpackages $RELPATH_BIN/debian-$TODAY | gzip -9c > $RELPATH_BIN/Packages.gz
dpkg-scansources $RELPATH_SRC/debian-$TODAY | gzip -9c > $RELPATH_SRC/Sources.gz

## #
## # Upload to our repository
## #

## rsync -r $DESTINATION_WORKAREA $DESTINATION_DEBIAN

#
# Upload to PPA
#
case x$UBUNTUPPA in
x)
	;;
*)
	dput -d $UBUNTUPPA $RELPATH_SRC/debian-$TODAY/${MODULE}_${FULLVERSION}_source.changes
	;;
esac
cd ..
# Increment micro version for next ppa build and remember
cd $SOURCE_DIR
let MICRO_VERSION=MICRO_VERSION+1
echo $MICRO_VERSION > ./debian/next_micro_version
#
# Delete old installers, remember current
#
# XXX TODO
