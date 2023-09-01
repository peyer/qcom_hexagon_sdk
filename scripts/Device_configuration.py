import os		# provide output to command prompt
import signal	# allow communication between child and parent processes
import time		# delay functions
import sys		# flush command prompt output
from sys import platform as _platform
import subprocess
from subprocess import check_output,CalledProcessError,Popen, PIPE, STDOUT
import platform
import argparse
from argparse import RawTextHelpFormatter
import re
import inspect

adb_ouput=""
adb_ouput_l =""

if not os.getenv('SDK_SETUP_ENV'):
		sys.exit("\nSDK Environment not set up -> please run setup_sdk_env script from SDK's root directory.")

# signal handler
def signal_usr1(signum, frame):	# signum is signal used to call handler 'signal_usr1', frame is current stack frame
	print "Exiting..."			# print appropriate message
	sys.stdout.flush()			# flush output to command prompt
	sys.exit(0)					# exit child process

#print command and execute with the error check
def print_and_run_cmd(cmd):
	print cmd
	if os.system(cmd) != 0 : sys.exit(2) # in error stop execution and exit

# This function return the output from "adb devices" command
def adb_devices_output():
	try:
		adb_ouput = check_output(["adb", "devices"])
		adb_ouput=adb_ouput.decode()
		print(adb_ouput)
	except CalledProcessError as e:
		print("Error occured while calling adb devices!")
		print(e.returncode)
		sys.exit()
	return adb_ouput

def adb_devices_output_linux():
	try:
		adb_ouput_l = check_output(["adb", "devices","-l"])
		adb_ouput_l=adb_ouput_l.decode()
		print(adb_ouput_l)
	except CalledProcessError as e:
		print("Error occured while calling adb devices -l!")
		print(e.returncode)
		sys.exit()
	return adb_ouput_l

def mount_device(device_number, Lparser):
      LocalParser = Lparser.parse_args()
      cmd1='adb -s '+device_number+' wait-for-device root'
      print_and_run_cmd(cmd1)
      cmd2='adb -s '+device_number+' wait-for-device remount'
      print_and_run_cmd(cmd2)
      if LocalParser.linux_env:
        cmd3='adb -s '+device_number+' wait-for-device shell mount -o remount,rw,exec /'
        print_and_run_cmd(cmd3)

def get_target_name_using_dev_num(dev_no):
	cmd1 = "adb -s " +dev_no+" wait-for-device root"
	print_and_run_cmd(cmd1)

	path="/firmware/verinfo/Ver_Info.txt"
	path2="/vendor/firmware/verinfo/ver_info.txt"
	path3="/vendor/firmware_mnt/verinfo/ver_info.txt"
	path4="/vendor/firmware_mnt/verinfo/Ver_info.txt"
	path5="/firmware/verinfo/ver_info.txt"
	x=4
	while x!=0:
		try:
			cmd = check_output(["adb","-s",dev_no,"wait-for-device","shell","ls",path])
			break
		except:
			if x==4:
				path=path2
			if x==3:
				path=path3
			if x==2:
				path=path4
			if x==1:
				path=path5
		x-=1
	print path
	verinfo = check_output(["adb","-s",dev_no, "wait-for-device", "shell", "cat ",path])
	lines=verinfo.split('\n')
	for j in lines:
		if "Meta_Build_ID" in j:
			target=j.split(":")[1].strip()
			target=target.split('"')[1].split(".")[0]
			if target == "SM8250_SDX55":
				target = "sm8250"
			if target == "MSM8996" or target == "APQ8096AU" or target == "APQ8096":
				target = "sdm820"
			if target == "MSM8998":
				target = "sdm835"
			if target == "Nicobar":
				target = "sm6125"
			if target == "QCS610":
				target = "sm6150"
	return target

def get_dev_no(i):
	if platform.system()=="Windows":
		dev_no=i.split('\t')[0]
		dev_no=str(dev_no)
	if platform.system()=="Linux":
		dev_no=i.split('               ')[0]
	return dev_no

# This creates a mapping of device name with its serial number
def create_dict():
	devices_dict={}
	if platform.system()=="Windows":
		devices=adb_ouput.split('\r\n')
		print devices
		for i in devices:
			if '\t' in i:
				i=str(i)
				dev_no=get_dev_no(i)
				target = get_target_name_using_dev_num(dev_no)
				if target in devices_dict.keys():
					devices_dict[target.lower()].append(dev_no)
				else:
					devices_dict[target.lower()]=[dev_no]
	elif platform.system()=="Linux":
		devices=adb_ouput_l.split('\n')
		for i in devices:
			if '               ' in i:
				i=str(i)
				dev_no=get_dev_no(i)
				target = get_target_name_using_dev_num(dev_no)
				if target in devices_dict.keys():
					devices_dict[target.lower()].append(dev_no)
				else:
					devices_dict[target.lower()]=[dev_no]
	return devices_dict

#it gives the device number for given target name
def dev_dict(target_name):
	devices_dict=create_dict()
	if target_name not in devices_dict.keys():
		print("Device with target name ",target_name," not connected")
		device_number=""
		return device_number
	device_list=devices_dict[target_name]
	if len(device_list)>1:
		print("More than one device is connected with same target name!")
		print("target name: ",target_name)
		print("devices connected: ",device_list)
		sys.exit()
	device_number=device_list[0]
	print "returned device_number is" + device_number
	return device_number
	
if platform.system()=="Windows" and adb_ouput=="":
	adb_ouput=adb_devices_output()

	
if platform.system()=="Linux" and adb_ouput_l=="":
	adb_ouput_l=adb_devices_output_linux()