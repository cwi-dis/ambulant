#!/bin/sh
set -e -x

VERSION=2.4.1
if [ ! -f include/ambulant/version.h ]; then
	echo "Please run only in a toplevel ambulant directory"
	exit 1
fi
echo '+ May need: sudo sudo apt-get install dpkg-dev debhelper devscripts fakeroot'
if [ -d ambulant-debiandist-tmp ]; then
	rm -rf ambulant-debiandist-tmp
fi
mkdir ambulant-debiandist-tmp
cd ambulant-debiandist-tmp
hg clone .. ambulant-$VERSION
hg up $1
rm -rf ambulant-$VERSION/.hg
rm -rf ambulant-$VERSION/sandbox
tar cfz ambulant_$VERSION.orig.tar.gz ambulant-$VERSION
cd ambulant-$VERSION
debuild -kC75B80BC
cd ..
rm -rf ambulant-$VERSION
dpkg-scanpackages . /dev/null | gzip -9c >Packages.gz
dpkg-scansources . /dev/null | gzip -9c > Sources.gz


