# This program parsers and prints the header lines of all frames
# in file 'aap' produced by ambulant-recorder plugin.
# Useful for debugging.

import string 

fp = open('aap')
count  = 0
while True:
	time = fp.readline()
	if not time:
		break
	count = count + 1
	sizeln = fp.readline()
	size = string.atoi(sizeln.strip().split()[-1].strip())
	w = fp.readline()
	h = fp.readline()
	sum = fp.readline()
	data = fp.read(size)
	print count, time.strip(), sizeln.strip(), w.strip(), h.strip(), size, len(data), repr(data[:4])
