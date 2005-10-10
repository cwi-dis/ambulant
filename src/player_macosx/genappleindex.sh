#!/bin/sh
case x$1 in
x)
	helpdir=`pwd`
	;;
*)
	helpdir=$1
	;;
esac
"/Developer/Applications/Utilities/Help Indexer.app/Contents/MacOS/Help Indexer" $helpdir -IndexAnchors -TigerIndexing
