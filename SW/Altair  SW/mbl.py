#! /usr/bin/python
# -*- coding: UTF-8 -*-
# formatted for tab-stops 4
#
#	By Peter Schorn, peter.schorn@acm.org, October 2009
#
#
# Transform an MBL file to a binary file suitable for loading with SIMH
#
# Structure of MBL files is as follows:
# <byte>+ 0x00 0x00
# (checkSum<byte> 0x3c count<byte> loadLow<byte> loadHigh<byte>
#  <byte> * count)+
# where the lower 8 bit of the load address are determined by loadLow
# and the upper 8 bit of the load address are determined by loadHigh
# For checkSum the following rules hold:
#	For the first load record: 0
#	For the second load record: (sum of all load bytes of the first
#		load record) mod 256
#	For the third and higher load records: (sum of all load bytes of
#		the preceding load record - 1) mod 256
# A header with count = 0 or second position is unequal to 0x3c denotes
# end of file.

import sys

if	len(sys.argv) <> 3:
	print 'Usage %s <infile>(MBL or TAP) <outfile>(BIN)\n' % sys.argv[0]
	sys.exit(1)

f = file(sys.argv[1], 'rb')
b = f.read()
f.close()

CHR0 = 2		# see if first header is prefixed by 2 chr(0)
i = b.find(chr(0) * CHR0 + chr(0) + chr(0x3c))
if	i == -1:
	CHR0 = 1	# try again with 1
	i = b.find(chr(0) * CHR0 + chr(0) + chr(0x3c))
	if	i == -1:
		print 'Prefix not found - cannot process this file.'
		sys.exit(1)
i += CHR0 + 2
result = [chr(0)] * len(b)

k = 0
count = ord(b[i])
while count and (ord(b[i - 1]) == 0x3c):
	l = ord(b[i + 1]) + (ord(b[i + 2]) << 8)
	checkSum = 0
	for j in range(count):
		result[l + j] = b[i + 3 + j]
		checkSum += ord(b[i + 3 + j])
	expectedCheckSum = ord(b[i-2])
	receivedCheckSum = expectedCheckSum
	if	k == 1:
		receivedCheckSum = previousCheckSum & 255
	elif k > 1:
		receivedCheckSum = (previousCheckSum - 1) & 255
	if	receivedCheckSum <> expectedCheckSum:
		print 'Checksum error in record %i. Got 0x%02x and expected 0x%02x.' % (
				k, receivedCheckSum, expectedCheckSum)
	i += count + 5
	count = ord(b[i])
	k += 1
	previousCheckSum = checkSum

i = len(result)
while result[i - 1] == chr(0):
	i -= 1

f = file(sys.argv[2], 'wb+')
for c in result[:i]:
	f.write(c)
f.close()
print '%i load records processed and %i bytes written to %s' % (k, i,
	sys.argv[2])
