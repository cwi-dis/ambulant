#!/bin/sh
# Clean out old nightly builds on the ambulantplayer.org website.
BUILDDIR=/var/www/AmbulantPlayerOrg/nightlybuilds
set -x
set -x
cd $BUILDDIR
subdirs=`echo */*/`
for dir in $subdirs; do
	cd $BUILDDIR
	cd $dir
	ls -t | grep '20[0-9][0-9][0-9][0-9][0-9][0-9]' | tail -n +6 | xargs rm
done

	