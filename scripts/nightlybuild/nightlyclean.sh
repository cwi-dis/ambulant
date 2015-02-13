#!/bin/sh
# Clean out old nightly builds on the ambulantplayer.org website.
BUILDDIR=/scratch/www/vhosts/ambulantplayer.org/public_html/nightlybuilds
TODAY=`date +%Y%m%d`
echo
echo ==========================================================
echo Ambulant nightly clean, $USER@`hostname`, `date`
echo LogLocation=clean-$TODAY.txt
echo ==========================================================
echo

set -e
set -x
cd $BUILDDIR
subdirs=`echo */*/ */deb/dists/*/ambulant/*`
for dir in $subdirs; do
	cd $BUILDDIR
	cd $dir
	ls -t | grep '20[0-9][0-9][0-9][0-9][0-9][0-9]' | tail -n +6 | xargs --no-run-if-empty rm -r
done
cd $BUILDDIR
cd logs
ls -t | tail -n +20 | xargs --no-run-if-empty rm 
