#------------------------------------------------------------------------------
# Copyright (c) 2014-2017 Qualcomm Technologies, Inc.  All Rights Reserved
#------------------------------------------------------------------------------
import sys
import re

# uarch trace for: commit
#
# PCYC=%d:T%d:PC=%08x:COMMIT
# PCYC=%d:T%d:    insn
# PCYC=%d:T%d:    insn
# PCYC=%d:T%d:    insn
# PCYC=%d:T%d:    insn
#
# for stall:
# PCYC=%d:T%d:PC=%08x:STALL:type
#
# Others:
# PCYC=%d:T%d:PC=%08x:LOCKWAIT
# PCYC=%d:T%d:PC=%08x:BUSREQ:TYPE=%s:ID=%d:PA=%016x:WIDTH=%d
# PCYC=%d:T%d:PC=%08x:BUSRSP:TYPE=%s:ID=%d:PA=%016x:WIDTH=%d:DELAY=%d
# PCYC=%d:T%d:PC=%08x:IMISS:VA=%x:PA=%x
# PCYC=%d:T%d:PC=%08x:DMISS:TYPE=%s:VA=%x:PA=%x
# PCYC=%d:T%d:PC=%08x:DCACHE:REPLACE:(CLEAN|DIRTY):OLDPA=%x
# PCYC=%d:T%d:PC=%08x:ICACHE:REPLACE:OLDPA=%x
#

# EJP: we're using RE's here.  Maybe we just use ":".split() and look at known locations?


#generic_re = re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):PC=(?P<pc>[0-9a-f]+):(?P<type>[^:]+)')

#type_res = {
#  'COMMIT' : re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):PC=(?P<pc>[0-9a-f]+):COMMIT'),
#  'EXCEPTION' : re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):PC=(?P<pc>[0-9a-f]+):EXCEPTION'),
#  'STALL' : re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):PC=(?P<pc>[0-9a-f]+):STALL:(?P<stalltype>[^:]+([:][^:]+)?)'),
#  'LOCKWAIT' : re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):PC=(?P<pc>[0-9a-f]+):LOCKWAIT'),
#  'BUSREQ' : re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):PC=(?P<pc>[0-9a-f]+):BUSREQ:TYPE=(?P<reqtype>[^:]+):ID=(?P<id>[0-9a-f]+):PA=(?P<pa>[0-9a-f]+):WIDTH=(?P<width>[0-9]+)'),
#  'BUSRSP' : re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):PC=(?P<pc>[0-9a-f]+):BUSRSP:TYPE=(?P<reqtype>[^:]+):ID=(?P<id>[0-9a-f]+):PA=(?P<pa>[0-9a-f]+):WIDTH=(?P<width>[0-9]+):DELAY=(?P<delay>[0-9]+)'),
#  'DCACHE' : re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):PC=(?P<pc>[0-9a-f]+):DCACHE:(?P<dcop>[A-Z0-9_]+):(?P<l2data>.*)'),
#  'ICACHE' : re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):PC=(?P<pc>[0-9a-f]+):ICACHE:(?P<dcop>[A-Z0-9_]+):(?P<l2data>.*)'),
#  'L2CACHE' : re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):PC=(?P<pc>[0-9a-f]+):L2CACHE:(?P<l2op>[A-Z0-9_]+):(?P<l2data>.*)')
#}

#insn_re = re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):\s+(?P<insn_info>\S.+)$')
#insn_info_re = re.compile(r'(?P<insn>([^/]|[/](?![/]))+)(?P<extra_info>[/][/].+)?$')

# cu_be_info = re.compile(r'PCYC=(?P<pcycle>[0-9]+):T(?P<tnum>[0-9]):PC=(?P<pc>[0-9a-f]+):STALL:cu_reg_interlock:cu_be:piclass=(?P<piclass>[^:]+):ciclass=(?P<ciclass>[^:]+):ptclass=(?P<ptclass>[^:]+):ctclass=(?P<ctclass>[^:]+)')

# EJP: FIXME: separate into normal pc_stats and coproc pc_stats
pc_stats = {}
#perstall_stats = {}
#topstalls = {}
#insns = {}
stallhooks = None

def record_commit(pc,tnum_str,cycle_str,rest):
	#tnum = int(info['tnum'])
	#cycle = int(info['pcycle'])
	pcdict = pc_stats.setdefault(pc,{})
	pcdict['commits'] = pcdict.get('commits',0) + 1

def record_stall(pc,tnum_str,pcycle_str,rest):
	#stalltype = rest[0] + '_' + rest[1]
	stalltype = rest[1]
	pcdict = pc_stats.setdefault(pc,{})
	pcdict[stalltype] = pcdict.get(stalltype,0) + 1
	if stallhooks:
		if stalltype in stallhooks:
			stallhooks[stalltype](pc,tnum_str,pcycle_str,rest)

def record_coproc(pc,tnum_str,pcycle_str,rest):
	if rest[1] == 'STALL':
		# EJP: FIXME: record in coproc stall dictionary
		record_stall(pc,tnum_str,pcycle_str,rest[2:])

def record_off(pc,tnum_str,pcycle_str,rest):
        record_stall(pc,tnum_str,pcycle_str,['','OFF_CYCLES'])
def record_wait(pc,tnum_str,pcycle_str,rest):
        record_stall(pc,tnum_str,pcycle_str,['','WAIT_CYCLES'])
def record_lockwait(pc,tnum_str,pcycle_str,rest):
        record_stall(pc,tnum_str,pcycle_str,['','LOCKWAIT_CYCLES'])

type_handlers = {
	'COMMIT': record_commit,
	'STALL': record_stall,
	'COPROC': record_coproc,
	'OFF': record_off,
	'WAIT': record_wait,
	'LOCKWAIT': record_lockwait
}

def record_insn(pc,m):
	insn_info = m.group('insn_info')
	mycycle = int(m.group('pcycle'))
	if not pc in insns:
		insns[pc] = {
			'pc' : pc,
			'first_seen_cycle' : mycycle,
			'insns' : []
		}
	if insns[pc]['first_seen_cycle'] != mycycle: return
	im = insn_info_re.match(insn_info)
	if not im: raise Exception("Didn't match insn_info_re: <<%s>>" % insn_info)
	insns[pc]['insns'].append(im.group('insn').strip())



def calc_cycles():
	for pc,data in pc_stats.items():
		cycles = sum(data.values())
		stall_total = cycles - data.get('commits',0)
		data['cycles'] = cycles
		data['stall_total'] = stall_total

def parse_line(line,last_pc,callfn):
	parts = line.split(":")
	if not parts[2].startswith("PC"):
		return last_pc
		#if ":\t" in line:
		#	record_insn(last_pc,insn_re.match(line))
		#	return last_pc
		#else:
		#	raise Exception("Bad parse: <<%s>>" % line)
	pc = int(parts[2][3:],16)
	tnum_str = parts[1][1:]
	cycle_str = parts[0][5:]
	fn = type_handlers.get(parts[3])
	if fn: fn(pc,tnum_str,cycle_str,parts[4:])
	if callfn: callfn(pc,tnum_str,cycle_str,parts[3],parts[4:])
	return pc

def find_startpc(f,startpc,last_pc,callfn):
	while True:
		line = f.readline()
		parts = line.split(":")
		if not parts[2].startswith("PC"): continue
		pc = int(parts[2][3:],16)
		if (pc != startpc): continue
		return parse_line(line,last_pc,callfn)


def parse(f,startpc,stoppc,callfn=None):
	last_pc = 0
	state = True
	if (startpc != None):
		last_pc = find_startpc(f,startpc,last_pc,callfn)
	for line in f:
		if not state:
			last_pc = find_start(startpc,startcycle)
		last_pc = parse_line(line.rstrip(),last_pc,callfn)
		if last_pc == stoppc: break
	calc_cycles()
	return pc_stats

if __name__ == '__main__':
	f = open(sys.argv[1])
	parse(f)
