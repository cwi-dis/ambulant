#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib/lua/5.1/socket:/usr/local/lib/ginga:/usr/local/lib/ginga/adapters:/usr/local/lib/ginga/cm:/usr/local/lib/ginga/converters:/usr/local/lib/ginga/dp:/usr/local/lib/ginga/ic:/usr/local/lib/ginga/mb:/usr/local/lib/ginga/mb/dec:/usr/local/lib/ginga/iocontents:/usr/local/lib/ginga/players:/usr/local/lib64:/usr/local/lib:/usr/lib64:/usr/lib:/lib64:/lib:/usr/kerberos/lib

export BROWSER=firefox

AmbulantPlayer $srcdir/test1.smil &
pid=$!
sleep 15
kill -9 $pid

