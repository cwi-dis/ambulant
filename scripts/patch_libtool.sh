#!/bin/bash
# libtool patch currently needed on fedora 8 64 bit systems (Jun 19, 2008)
# test if architecture is 64 bit and patch not yet applied
#DBG set -x
if  which arch 2>&1 >> /dev/null  \
	&& [ `arch` =  "x86_64" -o  `arch` =  "X86_64" ]\
	&& grep -q "file_magic ELF" libtool
then 
	echo "apply $0" 
	mv libtool libtool.orig
# The  -Xlinker -Bsymbolic flag was suggested as 1 out of 5 options to solve
# linkage problems on 64 bit Linux machines in:
# http://lists.mplayerhq.hu/pipermail/ffmpeg-devel/2007-November/038835.html
	cat libtool.orig | sed -e "s/deplibs_check_method=.*/deplibs_check_method=pass_all/g;s/ -shared/ -shared -Xlinker -Bsymbolic -nostartfiles/g;s/export_dynamic_flag_spec=\"\"/export_dynamic_flag_spec=\"\${wl}--export-dynamic\"/" > libtool
	chmod 755 libtool
fi
