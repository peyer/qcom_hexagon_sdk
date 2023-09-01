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

example_name = "hvx_add_constant"
target_info = target_list()
unsupported_target_info=[]
#******************************************************************************
# Parser for cmd line options
#******************************************************************************
parser = argparse.ArgumentParser(prog='hvx_add_constant_walkthrough.py', description=__doc__, formatter_class=RawTextHelpFormatter)
call_parser(parser)

options = parser.parse_args()

# run walkthrough
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
		print "Target name is not in list - supported targets :"
		for target_name in target_info: print "\t"+target_name
		sys.exit()

	EXAMPLE_PATH='/examples/camera_streaming/'
	device_number=dev_conf.dev_dict(target_name)
	if device_number=="":
		print "Error! device not connected!"
		sys.exit()
	object_new = get_config() #creating an object for get_config class in Common_Walkthrough script
	hex_variant, Flag, variant = object_new.get_parameters()

	# change the hex tool and variant here

	if options.target == "sm8250":
		print "---- root/remount device ----"
		print_and_run_cmd('adb -s '+device_number+' wait-for-device root')
		print_and_run_cmd('adb -s '+device_number+' wait-for-device remount')
		read_crm_version='adb -s '+device_number+' wait-for-device shell "cd /sys/devices/soc0;echo 16 > select_image && echo "`cat image_version`"'
		crm_version= subprocess.check_output(read_crm_version, shell=True)
		print ('CRM Version = '+ crm_version)
		print ('CRM Version num = '+ crm_version[15:20])
		if crm_version[15:20] <= "00353":
			opt ="_legacy=1"
		else:
			opt ="=1"
	else:
		opt ="=1"
	
	print ('opt : '+ opt)



	if options.target == "sdm835" or options.target == "sdm820" or options.target == "sdm660" or options.target == "sm6125":
		example_name_exe=HEXAGON_SDK_ROOT +EXAMPLE_PATH+example_name+'_stub/'+variant+'/ship/'+'libmmcamera_hvx_add_constant.so'
		libexample_name_skel=HEXAGON_SDK_ROOT +EXAMPLE_PATH+example_name+'/'+hex_variant+'/ship/'+'libadsp_hvx_add_constant.so'
	else:
		example_name_exe=HEXAGON_SDK_ROOT +EXAMPLE_PATH+example_name+'_stub/'+variant+'/ship/'+'libmmcamera_hvx_add_constant.so'
		libexample_name_skel=HEXAGON_SDK_ROOT +EXAMPLE_PATH+example_name+'/'+hex_variant+'/ship/'+'libdsp_streamer_add_constant.so'
	
	print "---- Build example_name example for both Android and Hexagon ----"
	if _platform == "win32":
		if options.target == "sdm835" or options.target == "sdm820" or options.target == "sdm660" or options.target == "sm6125":
			clean_variant = 'make -C ' + HEXAGON_SDK_ROOT +EXAMPLE_PATH+example_name+'_stub tree_clean V='+variant+' '+Flag+' || exit /b'
			build_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+'_stub tree V='+variant+' '+Flag+'  || exit /b'
		clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree_clean V='+hex_variant+' || exit /b'
		if options.target == "sm6125" or options.target == "sm8250" or options.target == "sm7150":
			build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree V='+hex_variant+' '+options.target+''+opt+' || exit 1'
		else:
			build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree V='+hex_variant+' || exit /b'
	else:
		if options.target == "sdm835" or options.target == "sdm820" or options.target == "sdm660" or options.target == "sm6125":
			clean_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+'_stub tree_clean V='+variant+' '+Flag+'  || exit 1'
			build_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+'_stub tree V='+variant+' '+Flag+'  || exit 1'
		clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree_clean V='+hex_variant+' || exit 1'
		if options.target == "sm6125" or options.target == "sm8250" or options.target == "sm7150":
			build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree V='+hex_variant+' '+options.target+''+opt+' || exit 1'
		else:
			build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree V='+hex_variant+' || exit 1'
	call_test_sig , APPS_DST, DSP_DST, LIB_DST, DSP_LIB_PATH, DSP_LIB_SEARCH_PATH = get_DST_PARAMS(HEXAGON_SDK_ROOT)

	if not options.no_rebuild:
		print "---- Build example for both Android and Hexagon ----"
		if options.target == "sdm835" or options.target == "sdm820" or options.target == "sdm660" or options.target == "sm6125":
			print_and_run_cmd(clean_variant)
			print_and_run_cmd(build_variant)
		print_and_run_cmd(clean_hexagon_variant)
		print_and_run_cmd(build_hexagon_variant)
	else: 
		print "---- Skip rebuilding example for both Android and Hexagon ----"
		
	if not options.no_signing :
		os.system(call_test_sig)
	
	mount_device(device_number,parser)
	
	print "---- Push Android components ----"
	if options.target == "sdm835" or options.target == "sdm820" or options.target == "sdm660" or options.target == "sm6125":
		print_and_run_cmd('adb -s '+device_number+' wait-for-device push '+example_name_exe+' '+LIB_DST)
		print_and_run_cmd('adb -s '+device_number+' wait-for-device shell chmod 777 '+APPS_DST)
	print " ---- Push Hexagon Components ----"
	print_and_run_cmd('adb -s '+device_number+' wait-for-device shell mkdir -p '+DSP_DST)
	print_and_run_cmd('adb -s '+device_number+' wait-for-device push '+libexample_name_skel+' '+DSP_DST)

	if _platform == "win32":
		print "---- Launch logcat window to see aDSP diagnostic messages"
		print_and_run_cmd('start cmd.exe /c adb -s '+device_number+' logcat -s adsprpc')
		print_and_run_cmd('sleep 2')

	print("---- Run Example on " +Flag[:4]+ " ----")
	api_level = subprocess.check_output('adb -s '+device_number+' shell getprop ro.build.version.sdk', shell=True)
	if options.target == "sdm835" or options.target == "sdm820" or options.target == "sdm660" or options.target == "sm6125":
		if api_level >= "28":
			print_and_run_cmd('adb -s '+device_number+' shell setprop persist.vendor.camera.hvx_lib_1 "libmmcamera_hvx_add_constant.so"')
		else:
			print_and_run_cmd('adb -s '+device_number+' shell setprop persist.camera.hvx_lib_1 "libmmcamera_hvx_add_constant.so"')
		print "Now Turn on Camera App to see the add constant effect"
	else:
		print_and_run_cmd('adb -s '+device_number+' wait-for-device shell mkdir -p vendor/etc/camera')
#		if options.target == "sm8150":
		print_and_run_cmd('adb -s '+device_number+' shell " echo enableDualIFE=FALSE >> /vendor/etc/camera/camxoverridesettings.txt"')
		print_and_run_cmd('adb -s '+device_number+' shell " echo enableHVXStreaming=1 >> /vendor/etc/camera/camxoverridesettings.txt"')
		print_and_run_cmd('adb -s '+device_number+' reboot')
		print_and_run_cmd('adb -s '+device_number+' wait-for-device')
		print_and_run_cmd('adb -s '+device_number+' wait-for-device root')
		print_and_run_cmd('adb -s '+device_number+' wait-for-device remount')

	print "|=======================================================|"
	print "| Now Turn on Camera App to see the add constant effect |"
	print "|=======================================================|"
	print " "	
	print "|=======================================================|"
	print "|        To Disable the Example please call:            |"
	print "|=======================================================|"
	if options.target == "sdm835" or options.target == "sdm820" or options.target == "sdm660" or options.target == "sm6125":
		if api_level >= "28":
			print "adb -s "+device_number+" shell setprop persist.vendor.camera.hvx_lib_1 0"
		else:
			print "adb -s "+device_number+" shell setprop persist.camera.hvx_lib_1 0"
		
	else:
		print "adb -s "+device_number+" shell rm /vendor/etc/camera/camxoverridesettings.txt"

	print " "	
	print "Done"

run_example()										# call function to initialize debug_agent
sys.stdout.flush()										# show output immediately in command prompt

