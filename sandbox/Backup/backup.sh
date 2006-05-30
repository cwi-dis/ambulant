#!/bin/bash -
# script to be run as user ambulant on apps.cwi.nl
PATH=/cwi/bin:$PATH

# Debug level: 0=nothing 1=mail errors 2=mail succes 3=trace this script
#
debug=2
if [ $debug -ge 3 ]
then
	set -x
fi

# Installable settings
#
dir="/ufs/ambulant/mirror"
url="rsync://keesblom@ambulant.cvs.sourceforge.net/cvsroot/ambulant/"
mailto="kees@cwi.nl"
ccto="jack@cwi.nl"
sleeptime=60 # seconds between retries
retries=60

# Derived and initial settings
day=`date | awk '{print $1}'`
month=`date | awk '{print $2}'`
daynr=`date | awk '{print $3}'`
RESULT=1
EXIT=1
COUNT=0

cd $dir

while [ $EXIT != 0 ]
do	
	rsync -avz --delete rsync://ambulant.cvs.sourceforge.net/cvsroot/ambulant/ 
	RESULT=$?
	if [ $COUNT -ne 0 ]
	then
		sleep $sleeptime
	fi
	EXIT=$RESULT
	if [ $COUNT -ge $retries ]
	then
		EXIT=0
	fi
	let "COUNT+=1"
done

if [ $RESULT -eq 0 ]
then 
	if [ $debug -ge 2 ]
	then
		MSG="MESSAGE [$month $daynr]: Ambulant CVS backup successful from $url to $dir"
		SUB="MESSAGE [$month $daynr]: Ambulant CV backup successful"
		echo $MSG | /usr/bin/mailx -s "$SUB" -c "$ccto" $mailto
	fi
else
	if [ $debug -ge 1 ]; then
		MSG="ERROR [$month $daynr]: Ambulant CVS backup from $url failed, $dir may be CORRUPTED. Return code from rsync was: $RESULT"
		SUB="ERROR [$month $daynr]: Ambulant CVS backup error"
		echo $MSG | /usr/bin/mailx -s "$SUB" -c "$ccto" $mailto
	fi
fi
    

