#------------------------------------------------------------------------------
# Copyright (c) 2014-2017 Qualcomm Technologies, Inc.  All Rights Reserved
#------------------------------------------------------------------------------
import struct
import sys
import pprint
import pickle

GMON_TAG_TIME_HIST = 0
GMON_TAG_CG_ARC = 1
GMON_TAG_BB_COUNT = 2

RANGE_THRESH = 4096

def get_byte(string,idx):
	val = struct.unpack("B",string[idx:idx+1])
	idx += 1
	return (val[0],idx)

def fput_byte(f,val):
	f.write(struct.pack("B",val))

def get_half(string,idx):
	val = struct.unpack("H",string[idx:idx+2])
	idx += 2
	return (val[0],idx)

def fput_half(f,val):
	f.write(struct.pack("H",val))

def get_word(string,idx):
	val = struct.unpack("I",string[idx:idx+4])
	idx += 4
	return (val[0],idx)

def fput_word(f,val):
	f.write(struct.pack("I",val))

def get_longlong(string,idx):
	val = struct.unpack("L",string[idx:idx+8])
	idx += 8
	return (val[0],idx)

def fput_longlong(string,idx):
	f.write(struct.pack("L",val))

def get_compressed(string,idx):
	(thislen,idx) = get_byte(string,idx)
	if ((thislen & 0x80) == 0):
		return (thislen,idx)
	val = 0
	for i in range(thislen & 0x7f):
		(nextbyte,idx) = get_byte(string,idx)
		val |= nextbyte << (8*i) # little endian
		#val |= nextbyte << (8*(thislen-i-1)) # big endian
	return (val,idx)

def fput_compressed(f,val):
	if (val < 0x80):
		fput_byte(f,val)
		return
	maxbyte = 7
	while ((val & (0xFF << (8*maxbyte))) == 0):
		maxbyte -= 1
	fput_byte(f,0x80 | (maxbyte+1))
	for i in range(maxbyte+1):
		fput_byte(f,(val >> (8*i)) & 0xFF)

def do_time_hist(string,idx,histinfo):
	(low_pc,idx) = get_word(string,idx)
	(high_pc,idx) = get_word(string,idx)
	(histsize,idx) = get_word(string,idx)
	(profrate,idx) = get_word(string,idx)
	print ('low=%08x high=%08x size=%08x rate=%x' % (low_pc, high_pc, histsize, profrate))
	for i in range(15):
		(unused,idx) = get_byte(string,idx)
		print (' unused dimension byte: <<%s>>' % chr(unused))
	(dim_abbr,idx) = get_byte(string,idx)
	print (' dimension abbr: <<%s>>' % chr(dim_abbr))
	for i in range(histsize):
		(val,idx) = get_compressed(string,idx)
		if not val: continue
		if not (low_pc+i*4) in histinfo: histinfo[low_pc+i*4] = 0
		print ('%08x: %d' % (low_pc+i*4,val))
		histinfo[low_pc+i*4] += val
	return idx

def write_time_hist(f,lowpc,highpc,histinfo):
	fput_byte(f,GMON_TAG_TIME_HIST)
	histsize = ((highpc-lowpc)/4)
	fput_word(f,lowpc)
	fput_word(f,highpc)
	fput_word(f,histsize)
	profrate = 1
	fput_word(f,profrate)
	fput_byte(f,ord('1'))
	fput_byte(f,ord('.'))
	fput_byte(f,ord('0'))
	for i in range(15-3):
		fput_byte(f,0)
	fput_byte(f,ord('1'))
	for i in range(histsize):
		pc = lowpc + i*4
		cycles = histinfo.get(pc,0)
		fput_compressed(f,cycles)

def do_arc(string,idx):
	(from_pc,idx) = get_word(string,idx)
	(self_pc,idx) = get_word(string,idx)
	(count,idx) = get_compressed(string,idx)
	return idx

def do_bb_counts(string,idx):
	(numblocks,idx) = get_word(string,idx)
	for i in range(numblocks):
		(addr,idx) = get_word(string,idx)
		(count,idx) = get_compressed(string,idx)
	return idx

def parse_gmon(string,histinfo):
	idx = 0
	(cookie,idx) = get_word(string,idx)
	(version,idx) = get_word(string,idx)
	(spare0,idx) = get_word(string,idx)
	(spare1,idx) = get_word(string,idx)
	(spare2,idx) = get_word(string,idx)
	print ('cookie=%x version=%d spare=%d,%d,%d.' % (cookie, version, spare0, spare1, spare2))
	while string[idx:]:
		(tag,idx) = get_byte(string,idx)
		if (tag == GMON_TAG_TIME_HIST):
			print ('Processing time hist')
			idx = do_time_hist(string,idx,histinfo)
		elif (tag == GMON_TAG_CG_ARC):
			#print ('Processing arcs')
			idx = do_arc(string,idx)
		elif (tag == GMON_TAG_BB_COUNT):
			#print ('Processing bbs')
			idx = do_bb_counts(string,idx)
		else:
			raise Exception("Don't know that tag: %d" % tag)


def read_gmon(filename,histinfo):
	data = open(filename).read()
	parse_gmon(data,histinfo)

def find_ranges(histinfo):
	ret = []
	pcs = sorted(histinfo.keys())
	range_start = pcs[0]
	for i in range(len(pcs[:-1])):
		if ((pcs[i+1] - pcs[i]) > RANGE_THRESH):
			range_end = pcs[i]
			ret.append( (range_start, range_end) )
			range_start = pcs[i+1]
	ret.append( ( range_start, pcs[-1]) )
	return ret



def write_gmon(filename,histinfo):
	f = open(filename,"w")
	cookie = 0x6e6f6d67
	fput_word(f,cookie)
	version = 2
	fput_word(f,version)
	spare0 = 0
	fput_word(f,spare0)
	spare1 = 0
	fput_word(f,spare1)
	spare2 = 0
	fput_word(f,spare2)
	ranges = find_ranges(histinfo)
	for lowpc,highpc in ranges:
		write_time_hist(f,lowpc,highpc+4,histinfo)
	f.close()

if __name__=='__main__':
	histinfo = {}
	for fn in sys.argv[1:]:
		read_gmon(fn,histinfo)
	f = open("gmon_pickle","w")
	pickle.dump(histinfo,f)
	#pprint.pprint(histinfo)
	f.close()
