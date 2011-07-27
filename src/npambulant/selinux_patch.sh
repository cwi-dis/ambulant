#!/bin/sh
# with seliunxenabled, set appropriate security context
PATH=${PATH}:/sbin:/usr/sbin ;
which chcon selinuxenabled 2>1 1>/dev/null && selinuxenabled;
if [ $? == 0 -a $1 ];then chcon -t texrel_shlib_t $1; fi
