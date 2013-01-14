#!/bin/sh
set -e -x
VERSION=2.4.1
SUFFIX=a
UVERSION=precise
PPA=ppa:ambulant/ambulant
if [ ! -f include/ambulant/version.h ]; then
	echo "Please run only in a toplevel ambulant directory"
	exit 1
fi
case x$1 in
xrelease)
	;;
xnightly)
	VERSION=$VERSION.`date +%Y%m%d`$SUFFIX
	fixchangelog=yes
	;;
*)
	echo Usage: $0 "[release|nightly] [branch]"
	exit 1
esac

if [ -d ambulant-debiandist-tmp ]; then
	rm -rf ambulant-debiandist-tmp
fi
mkdir ambulant-debiandist-tmp
cd ambulant-debiandist-tmp
case x$2 in
x)	branch=default
	;;
*)	branch=$2
	;;
esac
echo Building debian packages for ambulant-$VERSION from branch $branch
hg clone -u $branch .. ambulant-$VERSION
rm -rf ambulant-$VERSION/.hg
rm -rf ambulant-$VERSION/sandbox
case x$fixchangelog in
xyes)
	CLDATE=`date --rfc-2822`
	cat > ambulant-$VERSION/debian/changelog << xyzzy
ambulant ($VERSION) $UVERSION; urgency=low

  * Nightly build, for testing only

 -- CWI Ambulant Team <ambulant@cwi.nl>  $CLDATE
xyzzy
	;;
esac
tar cfz ambulant_$VERSION.orig.tar.gz ambulant-$VERSION
cd ambulant-$VERSION
debuild -S -sa -kC75B80BC
cd ..
rm -rf ambulant-$VERSION
dput $PPA ambulant_${VERSION}_source.changes

