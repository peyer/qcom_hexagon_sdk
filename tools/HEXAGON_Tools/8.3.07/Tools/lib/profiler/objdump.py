#------------------------------------------------------------------------------
# Copyright (c) 2014-2017 Qualcomm Technologies, Inc.  All Rights Reserved
#------------------------------------------------------------------------------
from subprocess import *
import re
import bisect
import os
import shlex
import sys

try:
    from profiler import addl_opts
    get_addl_objdump_opts = addl_opts.get_addl_objdump_opts
except ImportError:
    def get_addl_objdump_opts(): return ''

def get_elf_args(text):
    drive, suffix = os.path.splitdrive(text)
    result = suffix.rsplit(':', 1)
    path = drive + result[0]

    if len(result) == 2:
        addr = result[1]
        try:
            addr_val = int(addr, 0)
        except ValueError:
            addr_val = addr

        return path, addr_val
    else:
        assert(len(result) == 1)
        return path, None


objdump_command = "hexagon-llvm-objdump"

funcs = []
extrasyms = []
sortsyms = []
sortsym_addrs = []

symtab = {}
rsymtab = {}

# Form PC to function dictionary
pc_to_func = {}

def lookup(name):
    return symtab.get(name,None)

def find_altfunc(pc):
    try:
        return rsymtab[sortsym_addrs[bisect_right(sortsym_addrs,pc)-1]]
    except:
        return '<unknown>'

def pc_to_funcs(all_pcs):
    def findfunc(pc,idx):
        if (idx >= len(funcs)):
            pc_to_func[pc] = find_altfunc(pc)
            return True
        elif (pc < funcs[idx][0]):
            pc_to_func[pc] = find_altfunc(pc)
            return True
        elif (pc < funcs[idx][1]):
            pc_to_func[pc] = funcs[funcidx][2]
            return True
        return False
    funcidx = 0
    for pc in all_pcs:
        if findfunc(pc,funcidx): continue
        while (pc >= funcs[funcidx][1]):
            funcidx += 1
            if (funcidx >= len(funcs)): break
        if findfunc(pc,funcidx): continue
        pc_to_func[pc] = find_altfunc(pc)
    return pc_to_func

# read symbols
# "0000b9c0 g     F .text       0000002c unlockMutex"
# "0000b9c0 g       .text       00000000 globallabel"
# "000060c8 g     F .text       00000178 .hidden compute_fractal"

# Read functions, return list of tuples (start, end, 'name') or None if bad parse

func_re = re.compile(r"(?P<addr>[0-9a-f]+)\s+(?:[gwl]+\s+)?(?P<isfunc>F\s+)?(?P<section>[.]\S+)\s+(?P<size>[0-9a-f]+)\s+(.hidden\s+)?(?P<name>\S+)$")

def read_funcs(cmd, fn_elf, offset):
    global sortsyms
    global sortsym_addrs
    try:
        p = Popen([cmd, '-t', fn_elf],stdout=PIPE,stderr=STDOUT,bufsize=1)
    except:
        # Couldn't execute function
        return None
    for line in p.stdout:
        m = func_re.match(line.strip().decode('utf-8'))
        if not m:
            #print ('Could not parse: %s' % line)
            continue
        d = m.groupdict()
        if d['isfunc']:
            start = int(d['addr'],16) + offset
            length = int(d['size'],16)
            funcs.append( (start,start+length,d['name']) )
            symtab[d['name']] = start
            rsymtab[start] = d['name']
        elif "text" in d['section']:
            start = int(d['addr'],16) + offset
            extrasyms.append( (start,d['name']) )
            symtab[d['name']] = start
            rsymtab[start] = d['name']
    ret = p.wait()
    if ret != 0: return None

    sortsyms = sorted (extrasyms, key=lambda x: x[0])

    sortsym_addrs = [ x[0] for x in sortsyms]
    funcs.sort(key=lambda x: x[0])
    return funcs

disdict  = {}
hlldict  = {}
funcdict = {}

fn2idx = {}
filenames = ['']

funcname_re = re.compile(r'^(?P<name>[\.]*[_a-zA-Z0-9]+):$')
disas_re = re.compile(r'\s*(?P<addr>[0-9a-f]+):\s+(?:[0-9a-f]{2}\s+){4}\s*[0-9a-f]{8}\s+(?P<disas>\S.*)\s*$')
win_linenum_re = re.compile(r'(?P<drive>[a-zA-Z]:)(?P<filename>[-\.\\\/_0-9a-zA-Z]+):(?P<linenumber>[0-9]+)')
win_compilepath_linenum_re = re.compile(r'(?P<cp_drive>[a-zA-Z]:)(?P<cp_path>[-\.\\\/_0-9a-zA-Z]+)(?P<drive>[a-zA-Z]:)(?P<filename>[-\.\\\/_0-9a-zA-Z]+):(?P<linenumber>[0-9]+)')
lnx_linenum_re = re.compile(r'(?P<filename>[-\.\\\/_0-9a-zA-Z]+):(?P<linenumber>[0-9]+)')

def read_disassembly(cmd, fn_elf, offset):
    packet_pc = 0
    packet = []
    hll_linenum = ''
    addl_opts = get_addl_objdump_opts()
    cmd = '{cmd} -line-numbers -print-imm-hex {addl_opts} {fn_elf}'.format(**locals())
    args = shlex.split(cmd) if sys.platform != 'win32' else cmd
    try:
        p = Popen(args, stdout=PIPE,stderr=STDOUT,bufsize=1)
    except:
        # couldn't execute function
        return None
    for line in p.stdout:
        line = line.strip().decode('utf-8')
        dm = disas_re.match(line)
        if not dm:
            lm = win_linenum_re.match(line)
            if lm:
                filename = re.sub(r'\\',r'\\\\', os.path.normpath(lm.group('drive') + lm.group('filename')))
                idx = fn2idx.get(filename)

                if not idx:
                    idx = len(filenames)
                    fn2idx[filename] = idx
                    filenames.append(filename)

                hll_linenum = lm.group('linenumber')
                hll_ref     = str(idx) + ':' + hll_linenum
                continue

            lm = lnx_linenum_re.match(line)
            if lm:
                filename = re.sub(r'\\',r'/',os.path.normpath(lm.group('filename')))
                idx = fn2idx.get(filename)

                if not idx:
                    idx = len(filenames)
                    fn2idx[filename] = idx
                    filenames.append(filename)

                hll_linenum = lm.group('linenumber')
                hll_ref     = str(idx) + ':' + hll_linenum
                continue

            fm = funcname_re.match(line)
            if fm:
                hll_linenum = ''
                func_ref    = line
                continue

            #print ('Could not parse: %s' % line)
            continue

        pc = int(dm.group('addr'),16) + offset
        disas = dm.group('disas')

        if "{" in disas:
            if packet: disdict[packet_pc] = ";\n".join(packet)
            packet = [ disas ]
            packet_pc = pc
            if (hll_linenum != ''):
                hlldict[pc]  = hll_ref
                funcdict[pc] = func_ref
        elif "}" in disas:
            packet.append(disas)
            disdict[packet_pc] = ";\n".join(packet)
            packet = []
        elif '***warn' in disas:
            packet = []
            continue
        elif packet:
            packet.append(disas)
        else:
            disdict[pc] = disas
            if (hll_linenum != ''):
                hlldict[pc] = hll_ref
    if packet: disdict[packet_pc] = ";\n".join(packet)


def get_info(fn_elf, tools_dir, verbose):
    import os.path
    cmd = os.path.join(tools_dir, objdump_command)

    for arg in fn_elf.split(','):
        fn, offset_ = get_elf_args(arg)

        offset = offset_ if offset_ else 0

        if not read_funcs (cmd, fn, offset):
            if os.path.isfile(cmd):
                print ("objdump.py: Warning: '%s -t %s' failed" % (cmd, fn))

            if not read_funcs (objdump_command, fn, offset):
                print ("objdump.py: Warning: '%s -t %s' failed" % (objdump_command, fn))
            else:
                if verbose:
                    disasm = ""
                    for l in Popen(['bash', '-c', 'type ' + objdump_command], stdout=PIPE).stdout:
                        disasm = disasm + l.decode('utf-8')
                    print ('Disassembler: %s' % disasm)
                read_disassembly (objdump_command, fn, offset)
        else:
            if verbose:
                print ('Disassembler: %s' % cmd)
            read_disassembly (cmd, fn, offset)


def die_usage(cmd):
    # only used when running as a standalone python script
    import os.path
    print ('Usage: python %s <hexagon.elf> [--hll] [<tools_dir>]' % os.path.basename(cmd))
    sys.exit(1)


if __name__ == "__main__":
    import sys
    argv = sys.argv[0:]
    argc = len(argv)
    display_hll = False
    tools_dir = ''

    if ((argc < 2) or (argc > 4)): die_usage(argv[0])

    fn_elf = argv[1]

    if (argc == 3):
        if (argv[2] == '--hll'):
            display_hll = True
        else:
            tools_dir = argv[2]
    elif (argc == 4):
        if (argv[2] == '--hll'):
            display_hll = True
            tools_dir = argv[3]
        else:
            die_usage(argv[0])

    get_info(fn_elf, tools_dir, True)

    items = sorted (disdict.items(), key=lambda x: x[0])

    prev_hll = ''
    prev_func = ''

    for pc,disas in items:
        if (display_hll == True):
            func = funcdict.get(pc,'')
            if ((func == '') and (prev_func != '')):
                print ('')
            elif (func != prev_func):
                print ('\n\n%s' % func)
            prev_func = func

            hll = hlldict.get(pc,'')
            if (hll == None):
                if (prev_hll != ''):
                    print ('')
                    prev_hll = hll
            elif (hll != prev_hll):
                parts = hll.split(':')
                if len(parts) == 2:
                    print ('\n%s:%s' % (filenames[int(parts[0],10)], parts[1]))
                else:
                    print ('Error: %s' % hll)
                prev_hll = hll

        if (pc < 0x100): tab = "\t\t"
        else:            tab = "\t"
        print ('    %x:%s%s' % (pc, tab, re.sub('\n',' ',disas)))
