#!/bin/python
#
# This script turns latency log files (produced by Ambulant if LOGGER_VIDEOLATENCY
# defined during build, and AMBULANT_LOGFILE_LATENCY points to an output log file
# during runtime) into csv files which can be read into Numbers or Excel or somesuch.
import sys
import os

class LatencyData:
    def __init__(self):
        self.data = {}
        self.allstages = {}
        
    def addline(self, (timestamp, url, pts, dts, stage)):
        if not stage in self.allstages:
            self.allstages[stage] = True
        if not (url, pts) in self.data:
            self.data[(url, pts)] = {}
        perpts = self.data[(url, pts)]
        if dts and 'dts' in perpts:
            print >>sys.stderr, 'Duplicate DTS for PTS=%d: %d and %d' % (pts, dts, perpts['dts'])
        if dts:
            perpts['dts'] = dts
        if stage in perpts:
            print >>sys.stderr, 'Duplicate %s for PTS=%d: %d and %d' % (stage, pts, timestamp, perpts[stage])
        else:
            perpts[stage] = timestamp
            
    def parseline(self, line):
        line = line.strip()
        [timestamp, level, vl, stage, dts, pts, url] = line.split()
        [wallclock, us] = timestamp.split('.')
        hh, mm, ss = wallclock.split(':')
        timestamp = int(ss) + 60 * (int(mm) + 60 * int(hh))
        timestamp = timestamp + int(us) / 1000000.0
        return timestamp, url, int(pts), int(dts), stage
    
    def process(self, input, output):
        epoch = None
        for line in input:
            timestamp, url, pts, dts, stage = self.parseline(line)
            if not epoch:
                epoch = timestamp
            timestamp -= epoch
            self.addline((timestamp, url, pts, dts, stage))
        self.genoutput(output)

    def genoutput(self, output):
        allstagenames = self.allstages.keys()
        allstagenames.sort()
        output.write('url,pts,dts,%s\n' % ','.join(allstagenames))
        allkeys = self.data.keys()
        allkeys.sort()
        for url, pts in allkeys:
            perpts = self.data[(url, pts)]
            if 'dts' in perpts:
                dts = str(perpts['dts'])
            else:
                dts = ''
            stagedata = []
            for stage in allstagenames:
                if stage in perpts:
                    stagedata.append(str(perpts[stage]))
                else:
                    stagedata.append('')
            output.write('"%s",%d,%s,%s\n' % (url, pts, dts, ','.join(stagedata)))
            del self.data[(url, pts)]
        assert not self.data
            
def main():
    if len(sys.argv) <= 1:
        print >> sys.stderr, 'Processing stdin to stdout'
        LatencyData().process(sys.stdin, sys.stdout)
    else:
        for filename in sys.argv[1:]:
            basename, _ = os.path.splitext(filename)
            outputfilename = basename + '.csv'
            ifp = open(filename)
            ofp = open(outputfilename, 'w')
            print >> sys.stderr, 'Processing %s to %s' % (filename, outputfilename)
            LatencyData().process(ifp, ofp)
            
if __name__ == '__main__':
    main()
