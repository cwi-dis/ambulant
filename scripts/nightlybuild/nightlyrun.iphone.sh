#!/bin/sh
#
# Script to do a nightly clean build of a full Ambulant, indirectly
# iPhone version
#
curl http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant/raw-file/default/scripts/nightlybuild.iphone.sh | sh
