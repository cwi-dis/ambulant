#!/bin/sh
# Test suite for nightly builds
#
# Syntax: sh nightlytestsuite.sh [<program> [<testfile>.smil ...]]
#
# <program> is started repeatedly with each <testfile>.smil as argument.
# If it exists, then the test output is compared with the reference output
# in the file <testfile>-reference.txt, if that file exits.
#
# After that, <program> is started repeatly to execute all files or
# <URLs> in the  TESTSUITE array defined here and compare the output
# from ./AM_TEST-output.txt with the corresponding file (if it exists) 
# in the TESTRESULTS array
if [ x$DEBUG != x ]; then set -x; fi
if [ x$AMBULANT_TOP_DIR != x ]; then AMBULANT_TOP_DIR=../..;fi
COMPARESCRIPT=$AMBULANT_TOP_DIR/scripts/testoutputcompare.py
# Note: when adding an entry in TESTSUITE, a corresponding entry is to be
# added in TESTRESULTS, use 'void' if not available
TESTSUITE=(
	$AMBULANT_TOP_DIR/Extras/Welcome/Welcome.smil
	http://ambulantplayer.org/Demos/VideoTests/VideoTests.smil
)
TESTRESULTS=(\
	$AMBULANT_TOP_DIR/tests/nightly/Welcome-reference.txt \
	$AMBULANT_TOP_DIR/tests/nightly/VideoTests-http-reference.txt \
)
program=AmbulantPlayer
#set -x
# get program filename then skip it
if [ $# -gt 0 ]
then program=$1; shift;
fi
# loop over remaining arguments given
for file in $@; do $program $file; if [ -e `basename $file .smil`-reference.txt ]
then python $COMPARESCRIPT `basename $file .smil`-reference.txt AM_TEST-output.txt; fi;
done

let i=0
# loop over all elements in array TESTSUITE
for url in ${TESTSUITE[@]}
# execute the program with the current element
do $program $url;
# find the corresponding filename with reference output and, if present,
# compare with the output produced 
	ref=${TESTRESULTS[$i]}
	if [ -e $ref ]
	then python $COMPARESCRIPT $ref AM_TEST-output.txt;
# bail out if the compare failed
		if [ $? -ne 0 ]
		then exit -15
		fi
	fi
	let i=$i+1
done
   