#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant, indirectly
# Debian package version
set -e
set -x 
curl http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant/raw-file/default/scripts/nightlybuild/nightlybuild.debian.sh | sh

