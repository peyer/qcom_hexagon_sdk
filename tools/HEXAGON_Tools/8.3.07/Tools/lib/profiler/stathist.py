#------------------------------------------------------------------------------
# Copyright (c) 2014-2017 Qualcomm Technologies, Inc.  All Rights Reserved
#------------------------------------------------------------------------------
import sys
import json

data = None
majorVersion = 0
minorVersion = 0

#
# Old format:
#{
#	"0x00000420": {
#		"commits": 63,
#		"stalls": {
#			"ICACHE_DEMAND_MISS_CYCLES": 9999,
#			"IU_BUF_CYCLES": 126,
#			"FE_MISPREDICT_TIME_CYCLES": 372,
#			"TOTAL_STALLS": 10497 <-- maybe goes away
#		}
#	},
#	"0x00000b7c": {
#		"commits": 1,
#		"stalls": {
#			"ICACHE_DEMAND_MISS_CYCLES": 324,
#			"IU_BUF_CYCLES": 2,
#			"FE_MISPREDICT_TIME_CYCLES": 6,
#			"TOTAL_STALLS": 332 <-- maybe goes away
#		}
#	},
#
# New format:
#{
#        "version":"2.1",
#        "siminfo": {
#                "revid":"4062",
#                "core":"V62A_512",
#                "cache_config":"L1-I$ = 16 Kb, L1-D$ = 32 Kb, L2-$ = 256 Kb",
#                "command_line":"--packet_analyze out.json --timing hello.elf"
#         },
#        "core_packet_profile": {
#                "0x0000064c": {
#                        "commits":127,
#                        "stalls": {
#                                "IU_BUF_CYCLES":254,
#                                "IU_UTLB_MISS_CYCLES":3,
#                                "FE_MISPREDICT_TIME_CYCLES":756,
#                                "TOTAL_STALLS":1013
#                         },
#                        "events": {
#                                "AXI_READ_REQUEST":126,
#                                "AXI2_READ_REQUEST":0,
#                                "AHB_READ_REQUEST":0
#                         }
#                 },


def oldformat_convert(data):
	return {
		"version": "1.0",
		"siminfo": {
			"revid":"unknown",
			"core":"unknown",
			"cache_config":"unknown",
			"command_line":"unknown",
		},
		"core_packet_profile": data,
		"core_packet_profile_help":{},
		"stats":{}
	}

def read_pa(f):
	global data
	global majorVersion
	global minorVersion
	if not data:
		data = json.load(f)
	if "version" not in data:
		data = oldformat_convert(data)
	v = data['version'].split('.')
	majorVersion = int(v[0])
	minorVersion = int(v[1])
	return data


def parse(packet_profile='core_packet_profile',callfn=None):
	"""
	Returns a tuple of dictionaries. One maps each PC to its stall information. The stall information is itself a dictionary that
	maps each stall type name to its number of cycles. It also includes entries for "commits", "stall_total", and
	"cycles". The other dictionary maps each PC to its events information.
	"""

	pc_stats = {}
	pc_stats_events = {}
	for pcstr,valdict in data[packet_profile].items():
		# find the stall stats, put in pc_stats
		pc = int(pcstr,16)
		newstats = pc_stats.setdefault(pc,{})
		commits = valdict.get("commits",0)
		stalldict = valdict.get("stalls",{})
		stall_total = 0
		cycles = commits

		for stallname,val in stalldict.items():
			stallname = stallname.encode("ascii","ignore")
			stallname = stallname.decode('utf-8')
			if stallname == "TOTAL_STALLS": continue
			newstats[stallname] = val + newstats.get(stallname,0)
			stall_total += val

		cycles += stall_total
		newstats['commits'] = commits
		newstats['stall_total'] = stall_total
		newstats['cycles'] = cycles

		# find the event stats, put in pc_stats_events
		newstats_events = pc_stats_events.setdefault(pc, {})
		eventdict = valdict.get("events",{})

		for (eventname, val) in eventdict.items():
			eventname = eventname.encode("ascii","ignore")
			eventname = eventname.decode('utf-8')
			newstats_events[eventname] = val

		pc_stats_events[pc] = newstats_events

	return (pc_stats, pc_stats_events)


if __name__ == '__main__':
	f = open(sys.argv[1],"r")
	read_pa(f)

	for (packet_profile, processor) in data['packet_profiles'].items():
		(pc_stats, pc_stats_events) = parse(packet_profile)

	print (pc_stats)
	print (pc_stats_events)
