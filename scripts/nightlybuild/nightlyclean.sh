#!/bin/sh
# Clean out old nightly builds on the ambulantplayer.org website.
BUILDDIR=/scratch/www/vhosts/ambulantplayer.org/public_html/nightlybuilds
set -e
set -x
cd $BUILDDIR
subdirs=`echo */*/ */deb/dists/*/ambulant/*`
for dir in $subdirs; do
	cd $BUILDDIR
	cd $dir
	ls -t | grep '20[0-9][0-9][0-9][0-9][0-9][0-9]' | tail -n +6 | xargs --no-run-if-empty rm -r
done
