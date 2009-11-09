#!/bin/sh
case x$1 in
x)
	helpdir=`pwd`
	;;
*)
	helpdir=$1
	;;
esac
hiutil -Cf $helpdir/user.helpindex $helpdir
