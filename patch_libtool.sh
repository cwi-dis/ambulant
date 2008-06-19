#! /bin/bash
# libtool patch currently needed on fedora 8 64 bit systems (Jun 19, 2008)
# test if architecture is 64 bit and patch not yet applied
if  which arch 2>&1 >> /dev/null  \
	&& [ `arch` =  "x86_64" ]\
	&& grep -q "file_magic ELF" libtool
then 
	echo "apply $0" 
	mv libtool libtool.orig
	cat libtool.orig | sed -e "s/deplibs_check_method=.*/deplibs_check_method=pass_all/g;s/ -shared/ -shared -Xlinker -Bsymbolic -nostartfiles/g" > libtool
	chmod 755 libtool
fi
