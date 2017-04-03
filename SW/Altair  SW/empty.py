#! /usr/bin/python
# -*- coding: UTF-8 -*-
# formatted for tab-stops 4
#
#	By Peter Schorn, peter.schorn@acm.org, September 2006

import sys

def	usage():
	print 'Usage %s filename (altair | cpm)\n' % sys.argv[0]
	sys.exit(1)

if	len(sys.argv) <> 3:
	usage()

suffix = '.dsk'
name = sys.argv[1]
if	name.find(suffix) <> len(name) - len(suffix):
	name += suffix

if	sys.argv[2].lower() == 'cpm':
	n = 1113966
elif sys.argv[2].lower() == 'altair':
	n = 337664
else:
	usage()

f, c = file(name, 'wb+'), n
while c:
	f.write(chr(0xE5))
	c -= 1
f.close

print '%i characters 0xE5 written to %s\n' % (n, name)
