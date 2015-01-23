#!/bin/bash
set -x
case x$1 in
x)
	echo Usage: $0 branchname
	exit
	;;
*)
	BRANCHNAME=$1
	;;
esac

boot() {
	# Usage: boot parallelsname
	prlctl start $1
}

unboot() {
	# Usage: boot parallelsname
	prlctl stop $1
}

waitboot() {
	# Usage: waitboot hostname
	ssh $1 sleep 1
	ssh $1 sleep 1 
}

mkdistribution() {
	# Usage is: mkdistribution  hostname distribution
	hostname=$1
	distribution=$2
	scriptname=nightlybuild.${distribution}.sh
	curl  -o /tmp/${scriptname} http://ambulantplayer.org/cgi-bin/hgweb.cgi/hg/ambulant/raw-file/${BRANCHNAME}/scripts/nightlybuild/${scriptname}
	scp /tmp/${scriptname} ${hostname}:/tmp/${scriptname}
	ssh ${hostname} /tmp/${scriptname} ${BRANCHNAME}
}

PHOST=Mac-1010-Nightly
IHOST=maunakea-mac1010-nightly
boot ${PHOST} 
waitboot ${IHOST}
mkdistribution ${IHOST} mac
mkdistribution ${IHOST} iphone
unboot ${PHOST}

PHOST=Ubuntu-1404-32bit
IHOST=maunakea-ubuntu1404-32
boot ${PHOST} 
waitboot ${IHOST}
mkdistribution ${IHOST} linux
mkdistribution ${IHOST} debian
unboot ${PHOST}

PHOST=Ubuntu-1404-64bit
IHOST=maunakea-ubuntu1404-64
boot ${PHOST} 
waitboot ${IHOST}
mkdistribution ${IHOST} linux
mkdistribution ${IHOST} debian
unboot ${PHOST}

PHOST=Win7-VS1010-Nightly
echo "Ready to stop normal Windows nightlybuild and run yours?"
read a
boot ${PHOST}
