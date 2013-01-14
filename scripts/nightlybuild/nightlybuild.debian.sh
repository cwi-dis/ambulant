#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Debian packages, built on Ubuntu version
#
# NOTE: a number of things must be installed (and working) for this
# script to run successfully:
#  devscripts
#  mercurial
#  chrpath
#  autoconf
#  automake
#  libtool
#  postfix
#  mailutils
#  curl
#  ssh
# libxml2-dev 
# libqt3-mt-dev 
# liblivemedia-dev 
# libsdl1.2-dev 
# libavformat-dev 
# libavcodec-dev 
# libswscale-dev 
# libxerces-c-dev 
# python-dev 
# python-gtk2-dev 
# python-gobject-dev
#
# NOTE 2: the key used for signing (in debuild) must have no passphrase.
# I think this can only be done with the gpg --edit-key command line
# interface, then use the "passwd" command and set the passphrase to empty.
#
set -e
set -x

# An optional parameter is the branch name, which also sets destination directory
BRANCH=
case x$1 in
x)	
	BRANCH=default
	;;
*)	BRANCH=$1
esac

# Tunable parameters, to some extent
<<<<<<< local
AMBULANTVERSION=2.5
=======
AMBULANTVERSION=2.4.1
UBUNTUVERSION=precise
>>>>>>> other
ARCH=`uname -p`
HGARGS=""
HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
DESTINATION=sen5@ambulantplayer.org:/var/www/AmbulantPlayerOrg/nightlybuilds
BUILDHOME=$HOME/tmp/ambulant-nightly
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
BUILDDIR=ambulant-debian-$TODAY

DISTRIB_RELEASE=unknown
. /etc/lsb-release
DEBARCH=`dpkg-architecture -qDEB_HOST_ARCH`
DESTINATION_DEBIAN=$DESTINATION/deb/
DESTINATION_STAGING=dists
RELPATH_SRC=$DESTINATION_STAGING/$DISTRIB_RELEASE/ambulant/source
RELPATH_BIN=$DESTINATION_STAGING/$DISTRIB_RELEASE/ambulant/binary-$DEBARCH

echo
echo ==========================================================
echo Ambulant nightly build for Debian, $ARCH, $USER@`hostname`, `date`
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

ls -t | tail -n +6 | grep debian- | xargs chmod -R a+w .empty
ls -t | tail -n +6 | grep debian- | xargs rm -rf
hg $HGARGS clone $HGCLONEARGS $BUILDDIR
#
# Prepare the tree
#
cd $BUILDDIR
case x$BRANCH in
x)	;;
*)
	hg up -r $BRANCH
esac
# Get rid of mercurial administration
rm -r .hg
# Get rid of Sandbox
rm -r sandbox

sh autogen.sh
case x$release in
xno)
cat > debian/changelog << xyzzy
ambulant ($AMBULANTVERSION$VERSIONSUFFIX) $UBUNTUVERSION; urgency=low

  * Nightly build, for testing only

 -- CWI Ambulant Team <ambulant@cwi.nl>  $CLDATE
xyzzy
;;
esac

#
# Build debian packages, first binary then source
#
rm -rf $DESTINATION_STAGING
mkdir -p $RELPATH_SRC/debian-$TODAY
mkdir -p $RELPATH_BIN/debian-$TODAY

cd debian
debuild -kC75B80BC 
cd ..
mv *.deb *.dsc *.changes $RELPATH_BIN/debian-$TODAY/

cd debian
debuild -S -sa -kC75B80BC 
cd ..
mv *.tar.gz *.dsc *.changes *.build $RELPATH_SRC/debian-$TODAY/

dpkg-scanpackages $RELPATH_BIN/debian-$TODAY | gzip -9c > $RELPATH_BIN/Packages.gz
dpkg-scansources $RELPATH_SRC/debian-$TODAY | gzip -9c > $RELPATH_SRC/Sources.gz

#
# Upload to our repository
#

rsync -r $DESTINATION_STAGING $DESTINATION_DEBIAN

#
# Upload to PPA
#
dput $UBUNTUPPA $RELPATH_SRC/debian-$TODAY/ambulant_${AMBULANTVERSION}${VERSIONSUFFIX}_source.changes
cd ..

#
# Delete old installers, remember current
#
# XXX TODO
