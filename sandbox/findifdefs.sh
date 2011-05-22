#!/bin/sh
sources=`find src -print | grep -P '.c$|.cpp$|.m$|.h$|.mm$'`
includes=`find include -print | grep '.h$'`
if [ x$1 = x ]; then
	echo $sources $includes |
		xargs grep '^[:space:]*#[:space:]*if' |
		sed -e 's/.*#[ \t]*if[a-z]*[ \t]*//' |
		sort |
		uniq
else
	echo $sources $includes |
		xargs grep -l $1
fi
