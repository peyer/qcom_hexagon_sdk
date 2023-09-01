#!/usr/bin/env python
# child python script

### Imports ###
import os		# provide output to command prompt
import signal	# allow communication between child and parent processes
import time		# delay functions
import sys		# flush command prompt output
import subprocess
import argparse
import platform
from argparse import RawTextHelpFormatter
import Device_configuration as dev_conf
dev_num=""
pid = os.getpid()	# return the current process ID
received = False	# initialize signal received to false

#******************************************************************************
# Parser for cmd line options
#******************************************************************************
parser = argparse.ArgumentParser(prog='testsig.py', description=__doc__, formatter_class=RawTextHelpFormatter)
parser.add_argument("-LE", dest="linux_env", action="store_true", help="Build if your device HLOS is Linux embedded (e.g APQ8096)  Don't choose this option if device HLOS is Linux Android") 
parser.add_argument("-T", dest="target", help="specify target name  <sdm835, sdm820, sdm660, sdm845, sdm670, sdm710, qcs605, sm8150, sm6150, qcs405, sxr1130, sm7150, sm6125, sm8250, rennell, saipan, bitra>")
options = parser.parse_args()
adb_ouput=""
LE_DEVICE = 0
target_name = ""
if options.linux_env:
	LE_DEVICE = 1

# signal handler
def signal_usr1(signum, frame):	# signum is signal used to call handler 'signal_usr1', frame is current stack frame
	print "Exiting..."			# print appropriate message
	sys.stdout.flush()			# flush output to command prompt
	sys.exit(0)					# exit child process

def print_and_run_cmd(cmd):
	print cmd
	return os.system(cmd)

def disable_verity_check():
	print "checking disable-verity..."
	verity_output = subprocess.check_output("adb -s "+dev_num+" wait-for-device disable-verity", shell=True)
	if "already" not in verity_output and "disable-verity" not in verity_output and verity_output != "":
		print "\n---- Successfully disabled verity ----\nrebooting..."
		print_and_run_cmd("adb -s "+dev_num+" wait-for-device reboot")
		print_and_run_cmd("adb -s "+dev_num+" wait-for-device root")
	else :
		print "verity is already disabled"

def get_dev_num():
	global options
	global dev_num
	if options.target:
		dev_num=dev_conf.dev_dict(options.target)
	else:
		if platform.system()=="Linux":
			adb_ouput=dev_conf.adb_devices_output_linux()
			devices=adb_ouput.split('\n')
			count =0
			for i in devices:
				if '               ' in i:
					count+=1
					if count>1:
						print("More than one target connected, please specify the target name!")
						sys.exit()
					i=str(i)
					dev_num=dev_conf.get_dev_no(i)
		if platform.system()=="Windows":
			adb_ouput=dev_conf.adb_devices_output()
			devices=adb_ouput.split('\r\n')
			count=0
			for i in devices:
				if '\t' in i:
					count+=1
					if count>1:
						print("More than one target connected, please specify the target name!")
						sys.exit()
					i=str(i)
					dev_num=dev_conf.get_dev_no(i)
	return dev_num

def get_serial_num():
	HEXAGON_SDK_ROOT=os.getenv('HEXAGON_SDK_ROOT')
	if not HEXAGON_SDK_ROOT:
		sys.exit("\nSDK Environment not set up -> please run setup_sdk_env script from SDK's root directory.")

	print  "\n---- root/remount device ----"
	print_and_run_cmd('adb -s '+dev_num+' wait-for-device root')		# restart adb as root
	adb_details = subprocess.check_output('adb -s '+dev_num+' wait-for-device version', shell=True)
	adb_version_number = adb_details[29:35]  			#getting the adb version number
	if adb_version_number >= "1.0.33" and LE_DEVICE != 1:			# disable_verity_check is applicable from 1.0.33 and above versions
		disable_verity_check()
	if LE_DEVICE:
		os.system('adb -s '+dev_num+' wait-for-device shell mount -o,remount rw /')
	else:
		print_and_run_cmd('adb -s '+dev_num+' wait-for-device remount')

	print "\n--- Read serial number from device ---"
	read_serial_num='adb -s '+dev_num+' wait-for-device shell cat /sys/devices/soc0/serial_number'
	serialNum= subprocess.check_output(read_serial_num, shell=True)
	if serialNum >= "0":
		serialNum=hex(int(serialNum)).rstrip('L')
		return serialNum
	else :
		# if -LE is passed as a argument - testsig is used for Ubuntu target otherwise Android targets
		dest = "/vendor/bin/"
		build_varient="android_Release"
		if LE_DEVICE:
			build_varient="UbuntuARM_Release"
			dest="/usr/bin/"

		dsp_list = ["CDSP", "ADSP"]
		for dsp in dsp_list: 
			print "\n---- Push Get Serial on {} ----".format(dsp)
			print_and_run_cmd('adb -s '+dev_num+' wait-for-device push {}/tools/elfsigner/getserial/{}/{}/getserial {}'.format(HEXAGON_SDK_ROOT,dsp,build_varient,dest))
			print_and_run_cmd('adb -s '+dev_num+' wait-for-device shell chmod 777 {}/getserial'.format(dest))

			getSerialNum='adb -s '+dev_num+' wait-for-device shell {}/getserial | grep -o \"0.*\" | tr -d "\\n\\r"'.format(dest)
			serialNum = subprocess.check_output(getSerialNum, shell=True)
			if serialNum >= "0":
				return serialNum
	return "-1"

# initialize debugger
def generate_testsig(arg):
	HEXAGON_SDK_ROOT=os.getenv('HEXAGON_SDK_ROOT')
	if serialNum == "0": 
		print "\n--- Warning: serialNum = 0 ---"
	elif serialNum == "-1":
		sys.exit("\n--- Failed to retrieve serial number, please check logcat logs whether DSP is up or not. ---")
		
	print "\n--- Generate testsig ---"
	generate_testsig_cmd = 'python {}/tools/elfsigner/elfsigner.py -t {} -o {}/tools/elfsigner/testsigs'.format(HEXAGON_SDK_ROOT,serialNum,HEXAGON_SDK_ROOT)

	# if testsig generation failed exit the script with 2 exit status
	if print_and_run_cmd(generate_testsig_cmd) != 0 : sys.exit(2)

	testsig='{}/tools/elfsigner/testsigs/testsig-{}.so'.format(HEXAGON_SDK_ROOT,serialNum)
	if os.path.exists(testsig): print "\n--- Testsig generated sucessfully ---"
	else : print "\n--- Error while generating testsig - {} ---".format(testsig)

	sys.stdout.flush()           #flush output to command prompt
	dsp_path="/vendor/lib/rfsa/dsp/testsig"
	if LE_DEVICE: 
		dsp_path="/usr/lib/rfsa/dsp/testsig"
		if target_name.lower() == "qcs405":
			dsp_path="/data/lib/rfsa/dsp/testsig"

	print "\n--- Push Test Signature ---"
	print_and_run_cmd('adb -s '+dev_num+' wait-for-device shell mkdir -p '+dsp_path)
	print_and_run_cmd('adb -s '+dev_num+' wait-for-device push {} {}'.format(testsig,dsp_path))
	print_and_run_cmd('adb -s '+dev_num+' wait-for-device reboot')
	print_and_run_cmd('adb -s '+dev_num+' wait-for-device')
	print "Done"


# main entry point for child process
if __name__ == '__main__':
	signal.signal(signal.SIGINT, signal_usr1)						# register signal handler 'signal.SIGINT' to function handler 'signal_usr1'
	dev_num =get_dev_num()											# get device serial number
	target_name=dev_conf.get_target_name_using_dev_num(dev_num)		# get target chipset name using the device serial number
	serialNum = get_serial_num()									# get serial number from the device
	generate_testsig(serialNum)										# call function to initialize debug_agent
	sys.stdout.flush()												# show output immediately in command prompt