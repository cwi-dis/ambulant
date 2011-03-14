#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant
# Mac 10.6 version
#
set -e
set -x
AMBULANTVERSION=2.3
ARCH=`uname -p`
HGARGS=""
HGCLONEARGS="http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant"
BUILDHOME=$HOME/tmp/ambulant-nightly
TODAY=`date +%Y%m%d`
CLDATE=`date --rfc-2822`
BUILDDIR=ambulant-debian-$TODAY
VERSIONSUFFIX=.$TODAY
DESTINATION=ssh.cwi.nl:public_html/ambulant/nightly/debian/

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
ls -t | tail -n +6 | grep ambulant- | xargs rm -rf
hg $HGARGS clone $HGCLONEARGS $BUILDDIR
#
# Prepare the tree
#
cd $BUILDDIR
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
debuild -kC75B80BC << xyzzy
ambulant
ambulant
xyzzy
cd ..

#
# Upload
#

cd ..
mkdir debian-$TODAY
mv *.tar.gz *.deb *.dsc *.changes *.build debian-$TODAY/
scp -r debian-$TODAY $DESTINATION

#
# Delete old installers, remember current
#
# XXX TODO
