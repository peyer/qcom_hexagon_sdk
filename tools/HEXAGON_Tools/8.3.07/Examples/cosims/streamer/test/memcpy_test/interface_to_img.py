#! /usr/bin/env python

import sys, re, getopt
import struct

re_comment = re.compile(r'\#.*$')
re_hex = re.compile(r"0x([0-9a-fA-F]+)L?")

d_decode = {
	'sof':(2,0x1),
	'sol':(3,0x1),
	'pixel_valid':(6,0x1),
	'pixel':(7,0x3fff),
}

raw = 1;

mask = 0x3fff
def main():
	global raw
	global mask
	infile = ""
	outfile=""
	raw = 0
	opts, args = getopt.getopt(sys.argv[1:], "", ['raw', 'mask='])
	if len(args) < 2:
		sys.stdout.write('Usage: infile outfile\n')
	for o, a in opts:
		if o == "--raw":
			raw = 1;
		if o == "--mask":
			mask = mask & ~int(a,0)
	infile = args[-2]
	outfile = args[-1]
	infp = open(infile, 'r')
	if raw:
		outfp = open(outfile, 'wb')
	else:
		outfp = open(outfile, 'w')
	process(infp, outfp)
	outfp.close()
	infp.close()

def decode_tx(hexval, key):
	(shift,mask)= d_decode[key]
	return (hexval >> shift) & mask

def output_pixel_raw(fp, pixel):
	output_pixel_raw.pix_buf = (output_pixel_raw.pix_buf << 10) | pixel
	output_pixel_raw.pix_buf_occupied += 1
	if (output_pixel_raw.pix_buf_occupied  == 5):
		pix_raw = struct.pack('q', output_pixel_raw.pix_buf)
		fp.write(pix_raw)
		output_pixel_raw.pix_buf_occupied = 0
		output_pixel_raw.pix_buf = 0

output_pixel_raw.pix_buf = 0
output_pixel_raw.pix_buf_occupied = 0

def output_pixel_pretty(fp, pixel):
	fp.write(hex(pixel) + " \n")

def output_pixel(fp, pixel):
	global raw
	if raw:
		output_pixel_raw(fp, pixel)
	else:
		output_pixel_pretty(fp, pixel)

def process(infp, outfp, env = {}):
	global mask
	sofs  = 0
	sols  = 0
	pixels = 0
	for line in infp:
		hexresult = re.search('[\s]*(0x[0-9a-fA-F]+)[\s]*.*$', line)
		hexin = int(hexresult.group(1), 0)
		if decode_tx(hexin, 'pixel_valid'):
			pixel = decode_tx(hexin, 'pixel')
			pixel = pixel & mask
			output_pixel(outfp, pixel)
			pixels += 1
		elif (decode_tx(hexin, 'sol')):
			sols+=1
		elif (decode_tx(hexin, 'sof')):
			sofs+=1

	print "Wrote " + str(sofs) + " frames " + str(sols) + " lines and " + \
		str(pixels) + " pixels."

if __name__ == '__main__':
    main()

