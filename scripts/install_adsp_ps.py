
# child python script

### Imports ###
import os		# provide output to command prompt
import signal	# allow communication between child and parent processes
import time		# delay functions
import sys		# flush command prompt output
from sys import platform as _platform
import argparse
from argparse import RawTextHelpFormatter

### Global vars
pid = os.getpid()	# return the current process ID
received = False	# initialize signal received to false

### signal handler
def signal_usr1(signum, frame):	# signum is signal used to call handler 'signal_usr1', frame is current stack frame
	print "Exiting..."			# print appropriate message
	sys.stdout.flush()			# flush output to command prompt
	sys.exit(0)					# exit child process

### push adsp_ps binary and libraries to target
def run_adsp_info():
	if not os.getenv('SDK_SETUP_ENV'):
		sys.exit("\nSDK Environment not set up -> please run setup_sdk_env script from SDK's root directory.")
		
	HEXAGON_SDK_ROOT=os.getenv('HEXAGON_SDK_ROOT')
	adsp_info_exe=HEXAGON_SDK_ROOT+'/libs/common/adsp_ps/ship/android_Release/adsp_ps'
	libadsp_info_skel=HEXAGON_SDK_ROOT+'/libs/common/adsp_ps/ship/hexagon_Debug_dynamic/libadsp_ps_skel.so'

	parser = argparse.ArgumentParser(formatter_class=RawTextHelpFormatter)
	parser.add_argument("-N", dest="no_signing", action="store_true", help="skips test signature installation")
	options = parser.parse_args()
	if not options.no_signing :
		call_test_sig='python '+ HEXAGON_SDK_ROOT+'/scripts/testsig.py'	
		os.system(call_test_sig)

	print "---- root/remount device ----"
        os.system('adb wait-for-device')
	os.system('adb root')
	os.system('adb wait-for-device')
	os.system('adb remount')
	
	print "---- Push Android components ----"
	os.system('adb shell mkdir -p /vendor/bin')	
	os.system('adb push '+adsp_info_exe+' /vendor/bin')
	os.system('adb shell chmod 777 /vendor/bin/adsp_ps')

	print " ---- Push Hexagon Components ----"
	os.system('adb shell mkdir -p /vendor/lib/rfsa/adsp/')
	os.system('adb push '+libadsp_info_skel+' /vendor/lib/rfsa/adsp')
	os.system('adb reboot')	
	os.system('adb wait-for-device')

### main entry point for child process
if __name__ == '__main__':
	signal.signal(signal.SIGINT, signal_usr1)				# register signal handler 'signal.SIGINT' to function handler 'signal_usr1'
	run_adsp_info()											# call function to push adsp_ps binary and libraries
	sys.stdout.flush()										# show output immediately in command prompt

