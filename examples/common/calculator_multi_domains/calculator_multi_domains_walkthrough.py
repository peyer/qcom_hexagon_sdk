#!/usr/bin/env python
# child python script

### Imports ###
import os		# provide output to command prompt
import sys		# flush command prompt output
from sys import platform as _platform

HEXAGON_SDK_ROOT=os.getenv('HEXAGON_SDK_ROOT')
script_dir = HEXAGON_SDK_ROOT + '/scripts/'
sys.path.append(script_dir)
import Common_Walkthrough
from Common_Walkthrough import *
import Device_configuration as dev_conf

pid = os.getpid()	# return the current process ID
received = False	# initialize signal received to false

unsupported_target_info = ["8996"]
target_info = target_list()
#******************************************************************************
# Parser for cmd line options
#******************************************************************************
parser = argparse.ArgumentParser(prog='calculator_multi_domains_walkthrough.py', description=__doc__, formatter_class=RawTextHelpFormatter)
call_parser(parser)

options = parser.parse_args()

# run walk-through
def run_example():
	target_name=""
	if options.target in unsupported_target_info:
		print "Error! "+ options.target+" is not supported."
		sys.exit()
	if options.target in target_info:
		target_name=options.target 
	else :
		target_name=""
	if target_name=="" : 
		print "Error! Target name is not in list \nPlease pass -T with below supported targets :"
		for target_name in target_info: print "\t"+target_name
		sys.exit()
	device_number=dev_conf.dev_dict(target_name)
	if device_number=="":
		print "Error! device not connected!"
		sys.exit()
	#parsing the subsystem Falg 
	object_new = get_config() #creating an object for get_config class in Common_Walkthrough script
	hex_variant, Flag, variant = object_new.get_parameters()

	#parsing the subsystem Falg 
	if "True" != options.hex_version:
		hex_variant = check_for_user_supplied_hex_variant(options.hex_version)

	print "hex_variant = "+hex_variant


	HEXAGON_SDK_ROOT=os.getenv('HEXAGON_SDK_ROOT')
	calculator_multi_domains_exe='{}/examples/common/calculator_multi_domains/{}/ship/calculator_multi_domains'.format(HEXAGON_SDK_ROOT,variant)
	libcalculator_multi_domains='{}/examples/common/calculator_multi_domains/{}/ship/libcalculator_multi_domains.so'.format(HEXAGON_SDK_ROOT,variant)
	libcalculator_multi_domains_skel='{}/examples/common/calculator_multi_domains/{}/ship/libcalculator_multi_domains_skel.so'.format(HEXAGON_SDK_ROOT,hex_variant)
	
	
	if _platform == "win32":
		clean_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_multi_domains tree_clean V='+variant+' '+Flag+' VERBOSE=1 || exit /b'
		build_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_multi_domains tree V='+variant+' '+Flag+' VERBOSE=1 || exit /b'
		clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_multi_domains tree_clean V='+hex_variant+' VERBOSE=1 || exit /b'
		build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_multi_domains tree V='+hex_variant+' VERBOSE=1 || exit /b'
	else:
		clean_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_multi_domains tree_clean V='+variant+' '+Flag+' VERBOSE=1 || exit 1'
		build_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_multi_domains tree V='+variant+' '+Flag+' VERBOSE=1 || exit 1'
		clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_multi_domains tree_clean V='+hex_variant+' VERBOSE=1 || exit 1'
		build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_multi_domains tree V='+hex_variant+' VERBOSE=1 || exit 1'

	call_test_sig , APPS_DST, DSP_DST, LIB_DST, DSP_LIB_PATH, DSP_LIB_SEARCH_PATH = get_DST_PARAMS(HEXAGON_SDK_ROOT)

	if not options.no_rebuild:
		print "---- Build calculator_multi_domains example for both Android and Hexagon ----"
		print_and_run_cmd(clean_variant)
		print_and_run_cmd(build_variant)
		print_and_run_cmd(clean_hexagon_variant)
		print_and_run_cmd(build_hexagon_variant)
	else:
		print "---- Skip rebuilding calculator_multi_domains example for both Android and Hexagon ----"

	print_and_run_cmd('adb -s '+device_number+'  logcat -c')
	
	if not options.no_signing:
		os.system(call_test_sig)
	
	print "---- root/remount device ----"
	mount_device(device_number, parser)
	
	print "---- Push Android components ----"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell mkdir -p '+APPS_DST)
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device push '+calculator_multi_domains_exe+' '+APPS_DST)
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell chmod 777 '+APPS_DST+'/calculator_multi_domains')
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device push '+libcalculator_multi_domains+' '+LIB_DST)

	print " ---- Push Hexagon Components ----"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell mkdir -p '+DSP_DST)
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device push '+libcalculator_multi_domains_skel+' '+DSP_DST)

	print "---- Direct dsp messages to logcat ---"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell "echo 0x1f > '+DSP_DST+'calculator_multi_domains.farf"')

	# print_and_run_cmd('adb -s '+device_number+'  wait-for-device reboot')
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device')

	if _platform == "win32":
		print "---- Launch logcat window to see aDSP diagnostic messages"
		print_and_run_cmd('start cmd.exe /c adb -s '+device_number+'  logcat -s adsprpc')
		print_and_run_cmd('sleep 2')

	print "---- Run Calculator Multi Domains Example Locally on Android ----"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell export LD_LIBRARY_PATH=\"'+LIB_DST+';\" '+APPS_DST+'/calculator_multi_domains 1 0 1000')
	print "---- Run Calculator Multi Domains Example on aDSP ----"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell export LD_LIBRARY_PATH='+LIB_DST+':$LD_LIBRARY_PATH '+DSP_LIB_SEARCH_PATH+'='+DSP_LIB_PATH+' '+APPS_DST+'/calculator_multi_domains 0 0 1000')
	print "---- Run Calculator Multi Domains Example on cDSP ----"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell export LD_LIBRARY_PATH='+LIB_DST+':$LD_LIBRARY_PATH '+DSP_LIB_SEARCH_PATH+'='+DSP_LIB_PATH+' '+APPS_DST+'/calculator_multi_domains 0 3 1000')
	print "---- Run Calculator Multi Domains Example on mDSP ----"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell export LD_LIBRARY_PATH='+LIB_DST+':$LD_LIBRARY_PATH '+DSP_LIB_SEARCH_PATH+'='+DSP_LIB_PATH+' '+APPS_DST+'/calculator_multi_domains 0 1 1000')
	if options.target == "sdm845":
		print "---- Calculator Multi Domains Example is not supported on SLPI because SLPI does not have enough memory to support fastrpc user mode invocation"
		print "---- So walk-through script does not run this example on SLPI"
	if options.target == "sm8150":
		print "---- Run Calculator Multi Domains Example on sDSP ----"
		print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell export LD_LIBRARY_PATH='+LIB_DST+':$LD_LIBRARY_PATH '+DSP_LIB_SEARCH_PATH+'='+DSP_LIB_PATH+' '+APPS_DST+'/calculator_multi_domains 0 2 1000')
	print "Done"

run_example()												# call function to initialize debug_agent
sys.stdout.flush()										# show output immediately in command prompt

