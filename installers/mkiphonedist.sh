#!/bin/sh
provision=`dirname $0`/iAmbulantBuiltByJack.mobileprovision
if [ $# -ne 2 ]; then
	echo Usage: $0 ipafilename app
	echo ipafilename is something like iiAmbulant-2.4.ipa
	echo app is the pathname of the iphone application
	exit 1
fi
set -e
set -x
mkdir Payload
cp -PRp $2 Payload
zip -r $1 Payload
rm -rf Payload


