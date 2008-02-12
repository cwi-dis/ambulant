#! /bin/bash
mv libtool libtool.orig
cat libtool.orig | sed -e "s/deplibs_check_method=.*/deplibs_check_method=pass_all/g;s/-shared/-shared -nostartfiles/g" > libtool
chmod 755 libtool
