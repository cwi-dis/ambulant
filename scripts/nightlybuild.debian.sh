#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Debian packages, built on Ubuntu version
#
# NOTE: a number of things must be installed (and working) for this
# script to run successfully:
# - development tools (gcc, make, auto*)
# - debuild
# - chrpath
# - mail and configuration (actually for the cron job)
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
AMBULANTVERSION=2.3
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
release*)
	DESTINATION=$DESTINATION/$BRANCH
	VERSIONSUFFIX=
	;;
*)
	DESTINATION=$DESTINATION/$BRANCH
	VERSIONSUFFIX=.$TODAY
esac
CLDATE=`date --rfc-2822`
BUILDDIR=ambulant-debian-$TODAY
DESTINATION_DEBIAN=$DESTINATION/debian/

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
sh autogen.sh
cat > debian/changelog << xyzzy
ambulant ($AMBULANTVERSION.$TODAY) unstable; urgency=low

  * Nightly build, for testing only

 -- CWI Ambulant Team <ambulant@cwi.nl>  $CLDATE
xyzzy

#
# Build debian package (incomplete)
#
cd debian
debuild -kC75B80BC
cd ..

#
# Upload
#

cd ..
mkdir debian-$TODAY
mv *.tar.gz *.deb *.dsc *.changes *.build debian-$TODAY/
dpkg-scanpackages debian-$TODAY | gzip -9c > Packages.gz
dpkg-scansources debian-$TODAY | gzip -9c > Sources.gz
scp -r debian-$TODAY $DESTINATION_DEBIAN
scp Packages.gz Sources.gz $DESTINATION_DEBIAN

#
# Delete old installers, remember current
#
# XXX TODO
