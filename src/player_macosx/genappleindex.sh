#!/bin/sh
case x$1 in
x)
	helpdir=`pwd`
	;;
*)
	helpdir=$1
	;;
esac
case `uname -r` in
[789]*)
	"/Developer/Applications/Utilities/Help Indexer.app/Contents/MacOS/Help Indexer" $helpdir -IndexAnchors -TigerIndexing
	;;
*)
	hiutil -Cf $helpdir/user.helpindex $helpdir
	;;
esac