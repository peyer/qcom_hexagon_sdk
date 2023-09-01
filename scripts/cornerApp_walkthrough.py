#!/usr/bin/env python
# child python script

### Imports ###
import os		# provide output to command prompt
import signal	# allow communication between child and parent processes
import time		# delay functions
import sys		# flush command prompt output
from sys import platform as _platform

pid = os.getpid()	# return the current process ID
received = False	# initialize signal received to false


# signal handler
def signal_usr1(signum, frame):	# signum is signal used to call handler 'signal_usr1', frame is current stack frame
	print "Exiting..."	# print appropriate message
	sys.stdout.flush()			# flush output to command prompt
	sys.exit(0)					# exit child process

def print_and_run_cmd(cmd):
    print cmd
    os.system(cmd)

def cornerApp_walkthrough():
	if not os.getenv('SDK_SETUP_ENV'):
		sys.exit("\nSDK Environment not set up -> please run setup_sdk_env script from SDK's root directory.")
		
	HEXAGON_SDK_ROOT=os.getenv('HEXAGON_SDK_ROOT')
	cornerpApp_exe=HEXAGON_SDK_ROOT+'/examples/fastcv/cornerApp/android_Release/ship/cornerApp'
	libcornerApp=HEXAGON_SDK_ROOT+'/examples/fastcv/cornerApp/android_Release/ship/libcornerApp.so'
	libdspCV_skel=HEXAGON_SDK_ROOT+'/libs/fastcv/dspCV/hexagon_Release_dynamic/ship/libdspCV_skel.so'
	libapps_mem_heap=HEXAGON_SDK_ROOT+'/libs/common/apps_mem_heap/ship/hexagon_Release_dynamic/libapps_mem_heap.so'
	libcornerApp_skel=HEXAGON_SDK_ROOT+'/examples/fastcv/cornerApp/hexagon_Release_dynamic/ship/libcornerApp_skel.so'
	libfastcvadsp=HEXAGON_SDK_ROOT+'/libs/fastcv/fastcv/hexagon_Release_dynamic/libfastcvadsp.so'
	
	print "---- Build calculator example for both Android and Hexagon ----"
	if _platform == "win32":
		clean_android_Release = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/fastcv/cornerApp tree_clean V=android_Release || exit /b'
		android_Release = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/fastcv/cornerApp tree V=android_Release || exit /b'
		clean_hexagon_Release_dynamic = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/fastcv/cornerApp tree_clean V=hexagon_Release_dynamic || exit /b'
		hexagon_Release_dynamic = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/fastcv/cornerApp tree V=hexagon_Release_dynamic || exit /b'
	else:
		clean_android_Release = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/fastcv/cornerApp tree_clean V=android_Release || exit 1'
		android_Release = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/fastcv/cornerApp tree V=android_Release || exit 1'
		clean_hexagon_Release_dynamic = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/fastcv/cornerApp tree_clean V=hexagon_Release_dynamic || exit 1'
		hexagon_Release_dynamic = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/fastcv/cornerApp tree V=hexagon_Release_dynamic || exit 1'
	call_test_sig='python '+ HEXAGON_SDK_ROOT+'/scripts/testsig.py'
	
	print_and_run_cmd(clean_android_Release)				
	print_and_run_cmd(android_Release)
	print_and_run_cmd(clean_hexagon_Release_dynamic)
	print_and_run_cmd(hexagon_Release_dynamic)
	print_and_run_cmd(call_test_sig)
	
	print "---- root/remount device ----"
        print_and_run_cmd('adb wait-for-device')				
	print_and_run_cmd('adb root')
	print_and_run_cmd('adb wait-for-device')	
	print_and_run_cmd('adb remount')
	
	print "---- Push Android components ----"
	print_and_run_cmd('adb shell mkdir -p /vendor/bin')	
	print_and_run_cmd('adb push '+cornerpApp_exe+' /vendor/bin')
	print_and_run_cmd('adb shell chmod 777 /vendor/bin/cornerApp')
	print_and_run_cmd('adb push '+libcornerApp+' /system/lib')

	print " ---- Push Hexagon Components ----"
	print_and_run_cmd('adb shell mkdir -p /system/lib/rfsa/adsp/')				
	print_and_run_cmd('adb push '+libcornerApp_skel+' /system/lib/rfsa/adsp')
	print_and_run_cmd('adb push '+libdspCV_skel+' /system/lib/rfsa/adsp')
	print_and_run_cmd('adb push '+libfastcvadsp+' /system/lib/rfsa/adsp')
	print_and_run_cmd('adb push '+libapps_mem_heap+' /system/lib/rfsa/adsp')

	print "---- Run cornerApp Example on aDSP ----"
	print_and_run_cmd('adb shell ADSP_LIBRARY_PATH=/system/lib/rfsa/adsp /vendor/bin/cornerApp')
	print "Done"


# main entry point for child process
if __name__ == '__main__':
	signal.signal(signal.SIGINT, signal_usr1)				# register signal handler 'signal.SIGINT' to function handler 'signal_usr1'
	cornerApp_walkthrough()												# call function to initialize debug_agent
	sys.stdout.flush()										# show output immediately in command prompt

