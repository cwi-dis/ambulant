#!/bin/sh
#
# Script to do a nightly (actually, eary=ly-in-the-morning dayly) check that everything has built
# corectly
set -e
set -x 
curl -o nightlycheck.py http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant/raw-file/default/scripts/nightlybuild/nightlycheck.py
if python nightlycheck.py > /tmp/nightlycheck$$; then
	:
else
	mail -s "Ambulant nightly builds failed" Jack.Jansen@cwi.nl Kees.Blom@cwi.nl < /tmp/nightlycheck$$
fi


