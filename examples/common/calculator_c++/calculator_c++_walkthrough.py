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

unsupported_target_info = []
target_info = target_list()

#******************************************************************************
# Parser for cmd line options
#******************************************************************************
parser = argparse.ArgumentParser(prog='calculator_walkthrough.py', description=__doc__, formatter_class=RawTextHelpFormatter)
call_parser(parser)

options = parser.parse_args()


# run calculator_c++
def run_calc():
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
	
	HEXAGON_TOOLS_ROOT=os.getenv('HEXAGON_TOOLS_ROOT')
	if HEXAGON_TOOLS_ROOT == None:
		DEFAULT_HEXAGON_TOOLS_ROOT=os.getenv('DEFAULT_HEXAGON_TOOLS_ROOT');
		HEXAGON_TOOLS_ROOT=DEFAULT_HEXAGON_TOOLS_ROOT

	#parsing the subsystem Falg 
	object_new = get_config() #creating an object for get_config class in Common_Walkthrough script
	hex_variant, Flag, variant = object_new.get_parameters()
	
	if options.target == "sdm820":
		hex_variant="hexagon_Debug_dynamic_toolv72_v60"

	#parsing the subsystem Falg 
	if "True" != options.hex_version:
		hex_variant = check_for_user_supplied_hex_variant(options.hex_version)
	var = hex_variant.rsplit('_',1)[1]
	print "hex_variant = "+hex_variant

	ANDROID_ROOT_DIR=os.getenv('ANDROID_ROOT_DIR')
	if ANDROID_ROOT_DIR == None:
		ANDROID_ROOT_DIR=HEXAGON_SDK_ROOT+'/tools/android-ndk-r10d'

	calculator_exe='{}/examples/common/calculator_c++/{}/ship/calculator_plus'.format(HEXAGON_SDK_ROOT,variant)
	libcalculator_skel='{}/examples/common/calculator_c++/{}/ship/libcalculator_plus_skel.so'.format(HEXAGON_SDK_ROOT,hex_variant)
	libstdcppso=HEXAGON_TOOLS_ROOT+'/Tools/target/hexagon/lib/'+var+'/G0/pic/libstdc++.so'
	calculator_data=HEXAGON_SDK_ROOT+'/examples/common/calculator_c++/calculator.input'

	if _platform == "win32":
		clean_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_c++ tree_clean V='+variant+' '+Flag+' VERBOSE=1 || exit /b'
		build_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_c++ tree V='+variant+' '+Flag+' VERBOSE=1  || exit /b'
		clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_c++ tree_clean V='+hex_variant+' VERBOSE=1 || exit /b'
		build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_c++ tree V='+hex_variant+' VERBOSE=1 || exit /b'
	else:
		clean_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_c++ tree_clean V='+variant+' '+Flag+' VERBOSE=1  || exit 1'
		build_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_c++ tree V='+variant+' '+Flag+' VERBOSE=1  || exit 1'
		clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_c++ tree_clean V='+hex_variant+' VERBOSE=1 || exit 1'
		build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/calculator_c++ tree V='+hex_variant+' VERBOSE=1 || exit 1'

	call_test_sig , APPS_DST, DSP_DST, LIB_DST, DSP_LIB_PATH, DSP_LIB_SEARCH_PATH = get_DST_PARAMS(HEXAGON_SDK_ROOT)

	if not options.no_rebuild:
		print "---- Build calculator_plus example for both Android and Hexagon ----"
		print_and_run_cmd(clean_variant)
		print_and_run_cmd(build_variant)
		print_and_run_cmd(clean_hexagon_variant)
		print_and_run_cmd(build_hexagon_variant)
	else:
		print "---- Skip rebuilding calculator_plus example for both Android and Hexagon ----"

	print_and_run_cmd('adb -s '+device_number+'  logcat -c')
	if not options.no_signing :
		os.system(call_test_sig)

	print "---- root/remount device ----"
	mount_device(device_number, parser)

	print "---- Push Android components ----"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell mkdir -p '+APPS_DST)	
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device push '+calculator_exe+' '+APPS_DST)
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell chmod 777 '+APPS_DST+'/calculator_plus')
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device push '+calculator_data+' '+DSP_DST)

	print "---- Push Hexagon Components ----"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell mkdir -p '+DSP_DST)
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device push '+libcalculator_skel+' '+DSP_DST)
	print_and_run_cmd('adb -s '+device_number+'  push '+libstdcppso+' '+DSP_DST)

	print "---- Direct dsp messages to logcat ---"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell "echo 0x1f > '+DSP_DST+'calculator_plus.farf"')
	# print_and_run_cmd('adb -s '+device_number+'  wait-for-device reboot')	
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device')

	if _platform == "win32":
		print "---- Launch logcat window to see aDSP diagnostic messages"
		print_and_run_cmd('start cmd.exe /c adb -s '+device_number+'  logcat -s adsprpc')
		print_and_run_cmd('sleep 2')

	print "---- Run calculator_plus Example ----"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell export LD_LIBRARY_PATH='+LIB_DST+' '+DSP_LIB_SEARCH_PATH+'='+DSP_LIB_PATH+' '+APPS_DST+'/calculator_plus 10')
	print "Done"

run_calc()												# call function to initialize debug_agent
sys.stdout.flush()										# show output immediately in command prompt

