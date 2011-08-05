#!/bin/sh
#
# Script to do a nightly clean of nightly runs, indirectly
#
curl http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant/raw-file/default/scripts/nightlybuild/nightlyclean.sh | ssh sen5@ambulantplayer.org
