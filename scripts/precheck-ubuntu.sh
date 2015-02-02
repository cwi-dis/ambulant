#!/bin/sh
#
# Check that all required development packages are
# available, or install them if the optional "install"
# argument is given
packages="
  mercurial
  chrpath
  autogen
  autoconf
  automake
  dh-autoreconf
  libtool
  g++
  yasm
  gettext
  libpango1.0-dev
  libgtk2.0-dev
  libgtk-3-dev
  postfix
  mailutils
  curl
  ssh
  libxt-dev
  libxext-dev
"
debpackages="
  devscripts
  mercurial
  postfix
  mailutils
  python-dev 
  python-gtk2-dev 
  python-gobject-dev
  libdispatch-dev
  libsdl2-image-dev
"

case x$1 in
x)
	command="dpkg --get-selections"
	;;
xcheck)
	command="dpkg --get-selections"
	;;
xinstall)
	command="apt-get install"
	;;
xdebcheck)
	command="dpkg --get-selections"
	packages="$packages $debpackages"
	;;
xdebinstall)
	command="apt-get install"
	packages="$packages $debpackages"
	;;
*)
	echo $0: Command can be check, install, debcheck or debinstall.
	exit 1
esac
set -x
for pkg in $packages; do
	$command $pkg
done
