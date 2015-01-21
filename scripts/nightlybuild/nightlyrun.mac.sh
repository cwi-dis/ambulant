#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant, indirectly
# Mac 10.6 version
#
curl http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant/raw-file/default/scripts/nightlybuild/nightlybuild.mac.sh | ssh localhost
