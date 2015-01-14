#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Debian packages, built on Ubuntu version
#
# NOTE: a number of things must be installed (and working) for this
# script to run successfully:
#
#  postfix mailutils mercurial curl ssh devscripts chrpath dh-autoreconf autopoint
#
# PLUS: 
#
#  sudo apt-add-repository ppa:zoogie/sdl2-snapshots
#  sudo apt-add-repository ppa:samrog131/ppa
#  sudo apt-get update
#
# PLUS:
#
#  cd ambulant/third_party_packages
#  sudo python build-third-party-packages.py debian
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
AMBULANTVERSION=2.5
ARCH=`uname -p`
HGARGS=""
HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
DESTINATION=sen5@ambulantplayer.org:/scratch/www/vhosts/ambulantplayer.org/public_html/nightlybuilds
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
DISTRIB_CODENAME=unknown
. /etc/lsb-release
DEBARCH=`dpkg-architecture -qDEB_HOST_ARCH`
DESTINATION_DEBIAN=$DESTINATION/deb/
DESTINATION_STAGING=dists
RELPATH_SRC=$DESTINATION_STAGING/$DISTRIB_RELEASE/ambulant/source
RELPATH_BIN=$DESTINATION_STAGING/$DISTRIB_RELEASE/ambulant/binary-$DEBARCH
FULLAMBULANTVERSION=$AMBULANTVERSION$VERSIONSUFFIX~$DISTRIB_CODENAME
echo
echo ==========================================================
echo Ambulant nightly build for Debian, $ARCH, $USER@`hostname`, `date`
echo LogLocation=Ubuntu-$DISTRIB_RELEASE-$DEBARCH-$TODAY.txt
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

#
# Create staging area
#
rm -rf $DESTINATION_STAGING
mkdir -p $RELPATH_SRC/debian-$TODAY
mkdir -p $RELPATH_BIN/debian-$TODAY

#
# Prepare the tree
#
hg $HGARGS clone $HGCLONEARGS $BUILDDIR
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

# Replace debian changelog for nightly build distributions
case x$release in
xno)
cat > debian/changelog << xyzzy
ambulant ($FULLAMBULANTVERSION) $DISTRIB_CODENAME; urgency=low

  * Nightly build, for testing only

 -- CWI Ambulant Team <ambulant@cwi.nl>  $CLDATE
xyzzy
;;
esac

#
# Build debian packages, first binary then source
#

cd debian
debuild -kC75B80BC 
cd ..
cd ..
mv *.deb *.dsc *.changes $RELPATH_BIN/debian-$TODAY/

cd $BUILDDIR
cd debian
debuild -S -sa -kC75B80BC 
cd ..
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
case x$UBUNTUPPA in
x)
	;;
*)
	dput $UBUNTUPPA $RELPATH_SRC/debian-$TODAY/ambulant_${FULLAMBULANTVERSION}_source.changes
	;;
esac
cd ..

#
# Delete old installers, remember current
#
# XXX TODO
