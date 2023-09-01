import time
import sys
import re
import os
import glob
import ntpath
import inspect
import argparse
from argparse import RawTextHelpFormatter
import posixpath
from subprocess import Popen, PIPE, STDOUT

RES_ERR = -1
RES_OK  = 0
VERBOSE = 1
adsp_image_info = [['8994', '/obj/qdsp6v5_ReleaseG/signed/LA/system/etc/firmware', '/etc/firmware'],
                  ['8084', '/obj/qdsp6v5_ReleaseG/signed/LA/system/etc/firmware', '/etc/firmware'], 
                  ['8074', '/obj/qdsp6v5_ReleaseG/signed/LA/system/etc/firmware', '/etc/firmware'], 
                  ['8996', '/obj/qdsp6v5_ReleaseG/signed/LA/system/etc/firmware', '/firmware/image'], 
                  ['8096', '/obj/qdsp6v5_ReleaseG/signed/LA/system/etc/firmware', '/firmware/image'],
                 ]
adspso_rel_path = '/build/dynamic_signed/adspso.bin'

parser = argparse.ArgumentParser(prog='push_adsp.py', formatter_class=RawTextHelpFormatter)
parser.add_argument("-i", action="store", dest="load_dsp", 
		help="load dsp image to the device. Example: C:\QCOM\adsp_proc") 
parser.add_argument("-t", action="store", dest="target", 
                    default=None, help="target/chip id") 
parser.add_argument("-nodep", action="store_true", dest="nodep", 
                    help="Disable loading dependencies") 
parser.add_argument("-b", action="store", dest="boot", 
                    help="-b <boot.img>: fastboot boot <boot.img>") 
options = parser.parse_args()

def ErrMsg(msg):
    print ('\n\t ==> ERROR - {}\n\n'.format(msg))

def get_env(env_name):
    env = os.environ.get(env_name)
    if not env:
        return None
    return env

def run_cmds(cmds, verbose = False, output = None, FilterFunction = None, RunOutputVerify = True):
    for cmd in cmds:
        if verbose:
            print cmd

        proc = Popen(cmd, shell=True, stdout=PIPE, stderr=STDOUT)
        while True:
            nextline = proc.stdout.readline()
            if nextline == '' and proc.poll() != None:
                break

            if FilterFunction is not None:
                if not FilterFunction(nextline):
                    return RES_ERR

            sys.stdout.write(nextline)
            sys.stdout.flush()

            if output is not None:
                nextline = nextline.strip()
                nextline =  nextline.rstrip("\n\r")
                if len(nextline): 
                    output.append(nextline)
    return RES_OK

if __name__ == '__main__':
    parser.parse_args()
   
    if options.target == None:
	    target = '8996'
	    print 'Selected Default target'
    else:
	    target = options.target
            print 'Selected target: '+target

    if options.load_dsp:
    	adsp_path = options.load_dsp
    else:
        ErrMsg('Failed to find ADSP image path. Use -i <adsp_path>')
        sys.exit()
    
    adsp_src_path=None;
    adsp_dst_path=None;
    for index, item in enumerate(adsp_image_info):
         if item[0] == target:
            adsp_src_path = adsp_path + adsp_image_info[index][1]
            adsp_dst_path = adsp_image_info[index][2]
            break
    if adsp_src_path == None or adsp_dst_path == None:
    	print 'ADSP image paths are incorrect'
    	sys.exit() 
    print '>>> ADSP image:'
    print '    src: '+adsp_src_path
    print '    dst: '+adsp_dst_path

    err = os.path.exists(adsp_src_path);
    if err == False:
    	ErrMsg('ADSP image path not found. check '+adsp_src_path);
    	sys.exit()

    dsp_load_cmds = [
        'adb wait-for-device',
        'adb root', 
        'adb wait-for-device',
        'adb shell mount -o remount, rw {}'.format('/firmware'),
        'adb shell rm {}/adsp.* '.format(adsp_dst_path),
        'adb push  {} {}'.format(adsp_src_path, adsp_dst_path),
        'adb shell sync',
    ]

    if not options.nodep:
	dsp_load_cmds.append('adb reboot bootloader')
    else:
	dsp_load_cmds.append('adb reboot')
     
    #print dsp_load_cmds
    run_cmds(dsp_load_cmds, VERBOSE)

    # Flash adspso.bin by default
    # fastboot utility dont have wait-for-device. So used to wait and re-try 5 times
    if not options.nodep:
	adsp_so_file = adsp_path + adspso_rel_path
	print '\n\n>>> ADSP dependencies:'
	print '    adspso: '+adsp_so_file
	cmd = 'fastboot devices'
	count = 1;
	flash_dsp = 0;
	while True:
		#print 'Check device status for fastboot'
	        print cmd
		proc = Popen('fastboot devices', shell=True, stdout=PIPE, stderr=STDOUT)
		nextline = proc.stdout.readline()
		print nextline
		if nextline and 'fastboot' in nextline:
			flash_dsp = 1;
			break;
		else:
			print 'Retry device not in fastboot: '+str(count)+'(5)'
			count = count + 1;
			if count > 5:
		            break;
			time.sleep(5)

	if flash_dsp == 1:
            print 'device found in fastboot. Flash dependencies adspso.bin to /dsp partition'
            boot = 'fastboot continue'
            if options.boot:
               boot = 'fastboot boot ' + options.boot
            dep_load_cmds = [
	  		    'fastboot flash dsp {}'.format(adsp_so_file),
			    boot,
			    ]
            #print dep_load_cmds
	    run_cmds(dep_load_cmds, VERBOSE)
	else:
	    print 'device not found in fastboot. Check device and adspso.bin NOT flashed'

