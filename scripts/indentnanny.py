#
# Check indentation of C/C++ sourcecode
#

import re
import os
import sys

class IndentNanny:

    tab_only = False
    open_editor = None
    
    foreign_pattern=re.compile(r"/\*AMBULANT_FOREIGN_INDENT_RULES\*/")
    skip_patterns=[
    ]
    
    GOOD_PATTERNS=[
        r"^[\t ]*$",        # Lines with only whitespace, including empty lines
        r"^[^\t ].*$",      # Lines not starting with space or tab
        r"^\t+[^\t ].*$",   # Lines starting with one or more tabs, no spaces
        r"^  [^\t ].*$",    # Lines sttarting with 2 spaces
        r"^ \*",            # Assume this is a comment continuation
    ]
    
    REPLACE_PATTERNS=[
        (r"^    ", r"\t"),          # 4 initial spaces can be replaced by tab
        (r"^(\t+)    ", r"\1\t"),   # 4 spaces trailing a tab are replaced by two tabs
        (r"^ public", r"  public"), # Errors in some source files
        (r"^ private", r"  private"),
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
    
    def check_indentation(self):
        pass
        
    def check_line_good(self, line):
        for p in self.good_patterns:
            if p.match(line):
                self.check_indentation()
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
            if self.foreign_pattern.match(line):
                return True
            new_line =  self.check_line(line)
            if new_line is None:
                print '\t%d: %s' % (lino, line),
                rv = False
        return rv

    def check_file(self, filename):
        self.level = 0
        self.indents = [None]
        # XXX Incomplete
        rv = True
        fp = open(filename)
        lino = 0
        for line in fp.readlines():
            lino += 1
            if self.foreign_pattern.match(line):
                return True
            ok =  self.check_line_good(line)
            if rv and not ok:
                rv = False
                new_line = self.check_line(line)
                if new_line:
                    if not self.tab_only:
                        continue
                    print '%s: space indentation (first at line %d)' % (filename, lino)
                else:
                    print '%s: unknown indentation (first at line %d)' % (filename, lino)
                if self.open_editor:
                    os.system("%s +%d '%s'" % (self.open_editor, lino, filename))
                break
        return rv
        
    def match_skip_pattern(self, fn):
        for pat in self.skip_patterns:
            if pat in fn:
                return True
        return False
        
    def check(self, filename):
        rv = True
        if os.path.isdir(filename):
            for root, dirs, files in os.walk(filename):
                if self.match_skip_pattern(root):
                    continue
                for fn in files:
                    if self.match_skip_pattern(fn):
                        continue
                    _, ext = os.path.splitext(fn)
                    if ext in ('.c', '.h', '.cpp', '.cc', '.C', '.hh', '.H', '.m', '.mm'):
                        ok = self.check_file(os.path.join(root, fn))
                        if not ok:
                            rv = False
                if '.svn' in dirs:
                    dirs.remove('.svn')
                if 'CVS' in dirs:
                    dirs.remove('CVS')
                if '.hg' in dirs:
                    dirs.remove('.hg')
            return rv
        else:
            return self.check_file(filename)

verbose = True  

def main():
    from optparse import OptionParser
    parser = OptionParser(usage="usage: %prog [options] filename-or-dir [...]")
    parser.add_option("-v", "--verbose", dest="verbose", action="store_true",
        help="print detailed progress")
    parser.add_option("-e", "--edit", dest="open_editor", metavar="CMD",
        help='run "CMD +line file" for each error')
    parser.add_option("-t", "--tabs", dest="tab_only", action="store_true",
        help="allow only tab-based indentation (except for 2 spaces)" )
    parser.add_option("-s", "--skip", dest="skip_patterns", action="append", metavar="STR",
        help="skip filenames or dirs that contain STR. CVS, .svn and .hg are automatically skipped")
    options, args = parser.parse_args()
    
    allok = True
    nanny = IndentNanny()
    
    if options.skip_patterns: nanny.skip_patterns = options.skip_patterns
    if options.open_editor: nanny.open_editor = options.open_editor
    if options.tab_only: nanny.tab_only = options.tab_only
    for fn in args:
        ok = nanny.check(fn)
        if not ok:
            allok = False
        elif options.verbose:
            print '%s: ok' % fn
            
    if allok: sys.exit(0)
    sys.exit(1)
    
if __name__ == '__main__':
    main()
    