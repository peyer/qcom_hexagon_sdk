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
parser = argparse.ArgumentParser(prog='farf_runtime_test_walkthrough.py', description=__doc__, formatter_class=RawTextHelpFormatter)
call_parser(parser)

options = parser.parse_args()

# run farf_runtime_test
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
	object_new = get_config() #creating an object for get_config class in Common_Walkthrough script
	hex_variant, Flag, variant = object_new.get_parameters()

	
	#parsing the subsystem Falg 
	if "True" != options.hex_version:
		hex_variant = check_for_user_supplied_hex_variant(options.hex_version)

	print "hex_variant = "+hex_variant

	farf_runtime_test_exe='{}/examples/common/farf_runtime_test/{}/ship/farf_runtime_test'.format(HEXAGON_SDK_ROOT,variant)
	#libfarf_runtime_test='{}/examples/common/farf_runtime_test/{}/ship/libfarf_runtime_test.so'.format(HEXAGON_SDK_ROOT,variant)
	libfarf_runtime_test_skel='{}/examples/common/farf_runtime_test/{}/ship/libfarf_runtime_test_skel.so'.format(HEXAGON_SDK_ROOT,hex_variant)
	
	print "---- Build farf_runtime_test example for both Android and Hexagon ----"
	if _platform == "win32":
		clean_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/farf_runtime_test tree_clean V='+variant+' '+Flag+' VERBOSE=1 || exit /b'
		build_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/farf_runtime_test tree V='+variant+' '+Flag+' VERBOSE=1 || exit /b'
		clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/farf_runtime_test tree_clean V='+hex_variant+' VERBOSE=1 || exit /b'
		build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/farf_runtime_test tree V='+hex_variant+' VERBOSE=1 || exit /b'
	else:
		clean_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/farf_runtime_test tree_clean V='+variant+' '+Flag+' VERBOSE=1 || exit 1'
		build_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/farf_runtime_test tree V='+variant+' '+Flag+' VERBOSE=1 || exit 1'
		clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/farf_runtime_test tree_clean V='+hex_variant+' VERBOSE=1 || exit 1'
		build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/farf_runtime_test tree V='+hex_variant+' VERBOSE=1 || exit 1'

	call_test_sig , APPS_DST, DSP_DST, LIB_DST, DSP_LIB_PATH, DSP_LIB_SEARCH_PATH = get_DST_PARAMS(HEXAGON_SDK_ROOT)

	if not options.no_rebuild:
		print "---- Build farf_runtime_test example for both Android and Hexagon ----"
		print_and_run_cmd(clean_variant)
		print_and_run_cmd(build_variant)
		print_and_run_cmd(clean_hexagon_variant)
		print_and_run_cmd(build_hexagon_variant)
		print_and_run_cmd('adb logcat -c')
	else: 
		print "---- Skip rebuilding farf_runtime_test example for both Android and Hexagon ----"
	
	if not options.no_signing :
                os.system(call_test_sig)
	
	print "---- root/remount device ----"
	mount_device(device_number, parser)
	
	print "---- Push Android components ----"
	print_and_run_cmd('adb wait-for-device shell mkdir -p '+APPS_DST)
	print_and_run_cmd('adb wait-for-device push '+farf_runtime_test_exe+' '+APPS_DST)
	print_and_run_cmd('adb wait-for-device shell chmod 777 '+APPS_DST+'/farf_runtime_test')
	#print_and_run_cmd('adb wait-for-device push '+libfarf_runtime_test+' '+LIB_DST)

	print " ---- Push Hexagon Components ----"
	print_and_run_cmd('adb wait-for-device shell mkdir -p '+DSP_DST)
	print_and_run_cmd('adb wait-for-device push '+libfarf_runtime_test_skel+' '+DSP_DST)

    # print_and_run_cmd('adb wait-for-device reboot')
	print_and_run_cmd('adb wait-for-device')

	if _platform == "win32":
		print "---- Launch logcat window to see aDSP diagnostic messages"
		print_and_run_cmd('start cmd.exe /c adb logcat -s adsprpc')
		print_and_run_cmd('sleep 2')

	print "---- Run farf_runtime_test Example ----"
	if options.linux_env :
		print "---- When the adb shell opens, do this: cd /usr/bin; ./farf_runtime_test"
	else:
		print "---- When the adb shell opens, do this: cd /vendor/bin; ./farf_runtime_test"
	print_and_run_cmd('adb wait-for-device shell ')

#    print_and_run_cmd('adb wait-for-device shell ADSP_LIBRARY_PATH='+ADSP_LIB_PATH+' '+APPS_DST+'/farf_runtime_test ')
	
	print "Done"

run_example()												# call function to initialize debug_agent
sys.stdout.flush()										# show output immediately in command prompt

