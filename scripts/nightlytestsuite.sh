#!/bin/sh
# Test suite for nightly builds
# Syntax: sh nightlytestsuite.sh <program> [<testfile> ...]
# <program> is started repeatedly with each <testfile> as argument.
# After that, <program> is started repeatly to execute all <URLs> in the
# TESTSUITE list defined here.
COMPARESCRIPT=testoutputcompare.py
TESTSUITE=http://ambulantplayer.org/Demos/VideoTests/VideoTests.smil
set -x
if [ $# -gt 0 ]
then program=$1; shift;
fi
for file in $@; do $program $file; if [ -e `basename $file .smil`-reference.txt ]
then python $COMPARERSCRIPT `basename $file .smil`-reference.txt AM_TEST-output.txt; done
for url in $TESTSUITE; do $program $url; done
   