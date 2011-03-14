#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant, indirectly
# Debian package version

curl http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant/raw-file/default/scripts/nightlybuild.debian.sh | sh
