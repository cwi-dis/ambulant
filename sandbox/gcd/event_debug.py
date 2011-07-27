# Debug GCD event handling. Feed ambulant logfile to stdin.
#
import sys

outstanding={}
errors = 0

for line in sys.stdin.readline():
    fields = line.split()
    if len(fields) < 7:
        continue
    if fields[2] == 'add_event':
        id = fields[6]
        if id in outstanding:
            print '* Duplicate event:', id
            errors += 1
            print line
        else:
            outstanding[id] = line
    if field[2] == 'serve_event':
        if not id in outstanding:
            print '* Event served that was not added:', id
            errors += 1
        else:
            del outstanding[event]

for ev, line in outstanding:
    print '* Event that was never served:', line
    errors += 1

print errors, 'errors detected'    
            