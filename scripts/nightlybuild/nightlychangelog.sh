#!/bin/sh
#
# Script to do an automatic update of the 'ChangeLog' file for Ambulant
#
set -e
set -x
export PATH=/usr/local/bin:/Developer/usr/bin:$PATH
MSG="Automated ChangeLog Update"
SRCDIR=$HOME/src/ambulant # for user 'nightlybuilds' on 'moes'
#SRCDIR=$HOME/Work/Ambulant/ambulant # for user 'kees' on 'wroclaw'
TEMPFILE=/tmp/ChangeLog

cd $SRCDIR
hg pull -u
# produce ChangeLog since yesterday
hg --quiet log --style changelog -d -1 >$TEMPFILE
# Remove log entry produced by this action
if grep "$MSG" $TEMPFILE  >/dev/null;
then ed $TEMPFILE <<EOD
/$MSG/
.-2,.+1d
w
q
EOD
fi
mv ChangeLog ChangeLog.orig
cat $TEMPFILE ./ChangeLog.orig > ChangeLog
hg commit -m"$MSG" ./ChangeLog
hg push
rm $TEMPFILE ./ChangeLog.orig


