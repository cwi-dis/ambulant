#!/bin/sh
#
# Script to do an automatic update of the 'ChangeLog' file for Ambulant
#
set -e
set -x
export PATH=/usr/local/bin:/Developer/usr/bin:$PATH
MSG="Automated ChangeLog Update"
#SRCDIR=$HOME/src/ambulant # for user 'nightlybuilds' on 'moes'
SRCDIR=$HOME/Work/Ambulant/ambulant # for user 'kees' on 'wroclaw'
TEMPFILE=/tmp/ChangeLog

cd $SRCDIR
hg pull -u
hg --quiet log --style changelog -d -1 >$TMPFILE
ed $TMPFILE <EOD
/$MSG/
.,.-3d
w
q
EOD
mv $TMPFILE ./ChangeLog
hg commit -m\"$MSG\" ./ChangeLog
hg push


