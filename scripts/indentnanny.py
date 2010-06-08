#
# Check indentation of C/C++ sourcecode
#

import re
import os
import sys

class IndentNanny:
    
    GOOD_PATTERNS=[
        r"^[\t ]*$",        # Lines with only whitespace, including empty lines
        r"^[^\t ].*$",      # Lines not starting with space or tab
        r"^\t+[^\t ].*$",   # Lines starting with one or more tabs, no spaces
        r"^  [^\t ].*$",    # Lines sttarting with 2 spaces
        r"^ \*",            # Assume this is a comment continuation
    ]
    
    REPLACE_PATTERNS=[
        (r"^    ", r"\t"),        # 4 initial spaces can be replaced by tab
        (r"^(\t+)    ", r"\1\t"),     # 4 spaces trailing a tab are replaced by two tabs
    ]
    
    def __init__(self):
        self.init_good_patterns()
        self.init_replace_patterns()
        
    def init_good_patterns(self):
        self.good_patterns = []
        for p in self.GOOD_PATTERNS:
            self.good_patterns.append(re.compile(p))
            
    def init_replace_patterns(self):
        self.replace_patterns = []
        for pat, rep in self.REPLACE_PATTERNS:
            self.replace_patterns.append((re.compile(pat), rep))
    
    def check_line_good(self, line):
        for p in self.good_patterns:
            if p.match(line):
                return True
        return False
        
    def check_line(self, line):
        while True:
            if self.check_line_good(line):
                return line
            for pat, rep in self.replace_patterns:
                if pat.match(line):
                    line = pat.sub(rep, line)
                    break
            else:
                return None
        
    def fix_file(self, filename):
        # XXX Incomplete
        rv = True
        fp = open(filename)
        lino = 0
        for line in fp.readlines():
            lino += 1
            new_line =  self.check_line(line)
            if new_line is None:
                print '\t%d: %s' % (lino, line),
                rv = False
        return rv

    def check_file(self, filename):
        # XXX Incomplete
        rv = True
        fp = open(filename)
        lino = 0
        for line in fp.readlines():
            lino += 1
            ok =  self.check_line_good(line)
            if rv and not ok:
                new_line = self.check_line(line)
                if new_line:
                    print '%s: tab/space mix (first at line %d)' % (filename, lino)
                    break
                print '%s: bad indentation (first at line %d)' % (filename, lino)
                break
        return rv

verbose = True  
        
def main():
    allok = True
    nanny = IndentNanny()
    for fn in sys.argv[1:]:
        ok = nanny.check_file(fn)
        if not ok:
            allok = False
        elif verbose:
            print '%s: ok' % fn
            
    if allok: sys.exit(0)
    sys.exit(1)
    
if __name__ == '__main__':
    main()
    