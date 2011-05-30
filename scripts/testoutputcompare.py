#
# Script to compare the ambulant test output ('make check') produced by
# pyamplugin_test.py to an instance of that output that is known to be correct
#

import sys
DEBUG=False

# Tunable parameters, should be passed on the command line eventually

MAX_ABS_DELTA=1.0
MAX_REL_DELTA=0.1

def parse(filename):
    rv = {}
    fp = open(filename)
    for line in fp:
        if line[0] == '#':
            continue
        fields = line.strip().split(' ')
        if DEBUG:
            print fields
        if fields[0] == 'TEST':
            value = float(fields[1])
            key = ' '.join(fields[2:])
            if key in rv:
                print 'Duplicate event in %s: %s' % (filename, key)
                sys.exit(1)
            rv[key] = value
    return rv
    
def main():
    if len(sys.argv) != 3:
        print 'Usage: %s rferenceoutput testoutput' % sys.argv[0]
        sys.exit(2)
    gooddata = parse(sys.argv[1])
    testdata = parse(sys.argv[2])
    ok = True
    # Test that all keys occurr in both dictionaries
    for k in gooddata.keys():
        if not k in testdata:
            ok = False
            print 'Missing event:', k
    for k in testdata.keys():
        if not k in gooddata:
            ok = False
            print 'Unexpected event:', k
    if not ok:
        sys.exit(1)
    # Compare times
    for k in testdata.keys():
        testts = testdata[k]
        goodts = gooddata[k]
        deltats = abs(testts-goodts)
        if DEBUG:
            print k, goodts, testts, deltats 
        if deltats > MAX_ABS_DELTA + MAX_REL_DELTA*goodts:
            print 'Bad timestamp:', k
            print '\tExpected:', goodts
            print '\tGot:', testts
            ok = False
    if not ok:
        sys.exit(1)
    sys.exit(0)

if __name__ == '__main__':
    main()
    
