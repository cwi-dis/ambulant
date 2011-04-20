#!/bin/sh
for i in libtool libltdl/libtool; do
ed $i << FOOBAR
/^libext=/
c
libext=a
.
/The mapping between symbol names and symbols/
a
#ifndef lt_ptr
#define lt_ptr void *
#endif
.
w
q
FOOBAR
done
