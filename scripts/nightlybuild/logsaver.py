#!/usr/bin/python
#
# This script should receive all the mail that contains cron error reports
# from nightly builds.
# It will scan those mails for a line that starts with "LogLocation=" and
# use the rest of the line to determine where to save to logfile that is the
# body of the message.
# This script works together with nightlycheck.py, which knows where the
# log files are deposited (although it knows the URLs and this script knows
# the filenames) so pleople that receive the nightlycheck mail are one click
# away from getting the log of the failing build.
#
import os
import sys
import rfc822

SAVE_PATHNAME="/scratch/www/vhosts/ambulantplayer.org/public_html/nightlybuilds/logs/%(logLocation)s"

def parseAndSave(fp):
    msg = rfc822.Message(fp)
    lines = msg.fp.readlines()
    logLocation = None
    for line in lines:
        if line.startswith('LogLocation='):
            logLocation = line[len('LogLocation='):].strip()
            break
    if logLocation:
        pathname = SAVE_PATHNAME % dict(logLocation=logLocation)
        fp = open(pathname, 'w')
        fp.writelines(lines)
        
if __name__ == '__main__':
    parseAndSave(sys.stdin)
    
    
