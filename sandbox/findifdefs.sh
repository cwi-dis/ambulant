#!/bin/sh
sources=`find src -print | grep '.c$|.cpp$|.m$|.h$|.mm$'`
includes=`find include -print | grep '.h$'`
echo $sources $includes |
	xargs grep '^[:space:]*#[:space:]*if' |
	sed -e 's/.*#[ \t]*if[a-z]*[ \t]*//' |
	sort |
	uniq