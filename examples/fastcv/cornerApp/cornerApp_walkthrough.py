
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

example_name="cornerApp"
target_info = target_list()
unsupported_target_info =[]
#******************************************************************************
# Parser for cmd line options
#******************************************************************************
parser = argparse.ArgumentParser(prog='cornerApp_walkthrough.py', description=__doc__, formatter_class=RawTextHelpFormatter)
call_parser(parser)

options = parser.parse_args()

def getQCCompliance():
	global QUALCOMM_COMPLIANCE
	retValue = False
	QUALCOMM_COMPLIANCE = os.environ.get('QUALCOMM_COMPLIANCE', None)
	if QUALCOMM_COMPLIANCE is not None :
		retValue = True
	return retValue

def disclaimer_approval():
	qccompliance = getQCCompliance()
	if qccompliance is False:
		disclaimer = """
		Attention:
		By default all the examples will run one after the other, do you want to run ??

		Agree? [y/n]:
		"""

		while True:
			sys.stdout.write(disclaimer)
			choice = raw_input().lower()
			if choice in ['yes', 'no', 'y', 'n']:
				if choice in ['no', 'n']:
					sys.exit(2)
				else:
					return
	else:
		return
	
# run walkthrough
def run_example():
	target_name=options.target
	if options.target not in target_info : 
		print "Error! Target name is not in list \nPlease pass -T with below supported targets :"
		for target_name in target_info: print "\t"+target_name
		sys.exit()

	EXAMPLE_PATH='/examples/fastcv/'
	device_number=dev_conf.dev_dict(target_name)
	if device_number=="":
		print "Error! device not connected!"
		sys.exit()
	#parsing the subsystem Falg 
	object_new = get_config() #creating an object for get_config class in Common_Walkthrough script
	hex_variant, Flag, variant = object_new.get_parameters()

	example_name_exe=HEXAGON_SDK_ROOT +EXAMPLE_PATH+example_name+'/'+variant+'/ship/'+example_name
	libexample_name=HEXAGON_SDK_ROOT +EXAMPLE_PATH+example_name+'/'+variant+'/ship/lib'+example_name+'.so'
	libexample_name_skel=HEXAGON_SDK_ROOT +EXAMPLE_PATH+example_name+'/'+hex_variant+'/ship/lib'+example_name+'_skel.so'
	libdspCV_skel=HEXAGON_SDK_ROOT+'/libs/fastcv/dspCV/'+hex_variant+'/ship/libdspCV_skel.so'

	if _platform == "win32":
		clean_variant = 'make -C ' + HEXAGON_SDK_ROOT +EXAMPLE_PATH+example_name+' tree_clean V='+variant+' '+Flag+'  VERBOSE=1 || exit /b'
		build_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree V='+variant+' '+Flag+'  VERBOSE=1  || exit /b'
		clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree_clean V='+hex_variant+' VERBOSE=1  || exit /b'
		build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree V='+hex_variant+' VERBOSE=1  || exit /b'
	else:
		clean_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree_clean V='+variant+' '+Flag+'  VERBOSE=1  || exit 1'
		build_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree V='+variant+' '+Flag+'  VERBOSE=1  || exit 1'
		clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree_clean V='+hex_variant+' VERBOSE=1  || exit 1'
		build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+example_name+' tree V='+hex_variant+' VERBOSE=1  || exit 1'

	call_test_sig , APPS_DST, DSP_DST, LIB_DST, DSP_LIB_PATH, DSP_LIB_SEARCH_PATH = get_DST_PARAMS(HEXAGON_SDK_ROOT)

	if not options.no_rebuild:
		print "---- Build "+example_name+" example for both Android and Hexagon ----"
		print_and_run_cmd(clean_variant)
		print_and_run_cmd(build_variant)
		print_and_run_cmd(clean_hexagon_variant)
		print_and_run_cmd(build_hexagon_variant)
	else:
		print "---- Skip rebuilding "+example_name+" example for both Android and Hexagon ----"

	print_and_run_cmd('adb -s '+device_number+'  logcat -c')
	if not options.no_signing :
		os.system(call_test_sig)
	
	print "---- root/remount device ----"
	mount_device(device_number, parser)
	
	print "---- Push Android components ----"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell mkdir -p '+APPS_DST)	
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device push '+example_name_exe+' '+APPS_DST)
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell chmod 777 '+APPS_DST+'/'+example_name)

	print " ---- Push Hexagon Components ----"
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell mkdir -p '+DSP_DST)
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device push '+libexample_name_skel+' '+DSP_DST)
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device push '+libdspCV_skel+' '+DSP_DST)

	print_and_run_cmd('adb -s '+device_number+'  wait-for-device push '+libexample_name+' '+LIB_DST)
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device push '+ HEXAGON_SDK_ROOT +'/libs/fastcv/fastcv/'+hex_variant+'/libfastcvadsp.so '+DSP_DST)

	print "---- Direct dsp messages to logcat ---"
	#print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell "echo 0x1f > '+DSP_DST+example_name+'.farf"')
	# print_and_run_cmd('adb -s '+device_number+'  wait-for-device reboot')
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device')

	if _platform == "win32":
		print "---- Launch logcat window to see aDSP diagnostic messages"
		print_and_run_cmd('start cmd.exe /c adb -s '+device_number+'  logcat -s adsprpc')
		print_and_run_cmd('sleep 2')

	print("---- Run CornerApp Example on " +Flag[:4] +" ----")
	print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell export LD_LIBRARY_PATH='+LIB_DST+':$LD_LIBRARY_PATH '+DSP_LIB_SEARCH_PATH+'='+DSP_LIB_PATH+' '+APPS_DST+'/'+example_name)

	print "Done"

run_example()										# call function to initialize debug_agent
sys.stdout.flush()										# show output immediately in command prompt

