# Converts from jpb to dat file format consumed by NN graph
#
# Usage:
# python3 imagedump.py <format> input output
#
# format: 'byte' or 'float'
# input: input image in jpg format
# output: output image in dat format

#!/usr/bin/env python

from PIL import Image
import sys
import struct

if (len(sys.argv) != 4):
    raise Exception("Usage: %s [format:'byte' or 'float'] <input_image> <output_dump>" % sys.argv[0])

im = Image.open(sys.argv[2])
outf = open(sys.argv[3],"wb")

print "Image size: %dx%d" % im.size

data = im.getdata()

if (sys.argv[1] == 'byte'):
    for (r,g,b) in data:
	outf.write(struct.pack('BBB',r,g,b))
elif (sys.argv[1] == 'float'):
    for (r,g,b) in data:
	outf.write(struct.pack('fff',float(r)/128-1,float(g)/128-1,float(b)/128-1))

outf.close()
