#!/bin/python
#
# This script turns latency log files (produced by Ambulant if LOGGER_VIDEOLATENCY
# defined during build, and AMBULANT_LOGFILE_LATENCY points to an output log file
# during runtime) into csv files which can be read into Numbers or Excel or somesuch.
import sys
import os

def parseline(line):
    line = line.strip()
    [timestamp, level, vl, stage, dts, pts, url] = line.split()
    [wallclock, us] = timestamp.split('.')
    hh, mm, ss = wallclock.split(':')
    timestamp = int(ss) + 60 * (int(mm) + 60 * int(hh))
    timestamp = timestamp + int(us) / 1000000.0
    return timestamp, url, int(pts), int(dts), stage
    
def process(input, output):
    epoch = None
    output.write('timestamp,url,pts,dts,stage\n')
    for line in input:
        timestamp, url, pts, dts, stage = parseline(line)
        if not epoch:
            epoch = timestamp
        timestamp -= epoch
        output.write('%f,"%s",%d,%d,"%s"\n' % (timestamp, url, pts, dts, stage))
            
def main():
    if len(sys.argv) <= 1:
        print >> sys.stderr, 'Processing stdin to stdout'
        process(sys.stdin, sys.stdout)
    else:
        for filename in sys.argv[1:]:
            basename, _ = os.path.splitext(filename)
            outputfilename = basename + '.csv'
            ifp = open(filename)
            ofp = open(outputfilename, 'w')
            print >> sys.stderr, 'Processing %s to %s' % (filename, outputfilename)
            process(ifp, ofp)
            
if __name__ == '__main__':
    main()
