#!/usr/bin/env python
# child python script

### Imports ###
import os       # provide output to command prompt
import signal   # allow communication between child and parent processes
import sys      # flush command prompt output
import subprocess

# signal handler
def signal_usr1(signum, frame): # signum is signal used to call handler 'signal_usr1', frame is current stack frame
    print ("Exiting...")    # print appropriate message
    sys.stdout.flush()          # flush output to command prompt
    sys.exit(0)                 # exit child process


# initialize debugger
def setup_and_start_remote_debug_agent():

    rdbg_system_loc = '/system/lib/modules/rdbg.ko'
    rdbg_system_ls = 'adb shell ls ' + rdbg_system_loc
    rdbg_system_insmod = 'adb shell insmod ' + rdbg_system_loc
    rdbg_vendor_loc = '/vendor/lib/modules/rdbg.ko'
    rdbg_vendor_ls = 'adb shell ls ' + rdbg_vendor_loc
    rdbg_vendor_insmod = 'adb shell insmod ' + rdbg_vendor_loc
    lsmod = 'adb shell lsmod' # adb command to list all running modules on device

    # make sure SDK environment has been set up properly
    if not os.getenv('SDK_SETUP_ENV'):
        sys.exit("\nSDK Environment not set up -> please run setup_sdk_env script from SDK's root directory.")
    if len(sys.argv) == 1:
        sys.exit("\nPlease pass the DSP as an argument. e.g: -ADSP")

    if sys.argv[1] == '-ADSP':
        push_rda = 'adb push ' + os.getenv('HEXAGON_SDK_ROOT') + '/tools/debug/remote_debug_agent/android/remote_debug_agent_adsp /vendor/bin/remote_debug_agent'
    elif sys.argv[1] == '-CDSP':
        push_rda = 'adb push ' + os.getenv('HEXAGON_SDK_ROOT') + '/tools/debug/remote_debug_agent/android/remote_debug_agent_cdsp /vendor/bin/remote_debug_agent'
    else:
        print("Error: Unknown DSP")
        return

    print("Waiting for device ...")
    os.system('adb wait-for-device')
    os.system('adb root')
    os.system('adb wait-for-device')
    os.system('adb remount')
    os.system('adb wait-for-device')
    os.system(push_rda)

    found_system = 1
    found_vendor = 1

    try:
        output1 = subprocess.check_output(rdbg_system_ls, stderr = subprocess.STDOUT, shell = True)
    except Exception, e:
        found_system = 0
        output1 = str(e)
    else:
        if ('No such file or directory' in output1):
            found_system = 0

    try:
        output2 = subprocess.check_output(rdbg_vendor_ls, stderr = subprocess.STDOUT, shell = True)
    except Exception, e:
        found_vendor = 0
        output2 = str(e)
    else:
        if ('No such file or directory' in output2):
            found_vendor = 0

    if (not (found_system or found_vendor)):
        print '\nERROR: failed to find ' + rdbg_system_loc + ' or ' + rdbg_vendor_loc
        sys.exit(1)

    try:
        output1 = subprocess.check_output(rdbg_system_insmod, stderr = subprocess.STDOUT, shell = True)
    except Exception, e:
        output1 = str(e)

    try:
        output2 = subprocess.check_output(rdbg_vendor_insmod, stderr = subprocess.STDOUT, shell = True)
    except Exception, e:
        output2 = str(e)

    # check to see if rdbg.ko module is live -> if not then rdbg.ko not insantiated properly
    if (found_system and ('failed' in output1) and not ('rdbg' in subprocess.check_output(lsmod, shell = True))):
        print '\nERROR: rdbg.ko module could not be installed/instantiated properly. Expected file location on device: ' + rdbg_system_loc
        sys.exit(1)
    elif (found_vendor and ('failed' in output2) and not ('rdbg' in subprocess.check_output(lsmod, shell = True))):
        print '\nERROR: rdbg.ko module could not be installed/instantiated properly. Expected file location on device:  ' + rdbg_vendor_loc
        sys.exit(1)

    os.system('adb forward tcp:5555 tcp:56283')
    os.system('adb shell chmod 755 /vendor/bin/remote_debug_agent')
    os.system('adb shell /vendor/bin/remote_debug_agent')
    sys.stdout.flush()


# main entry point for child process
if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_usr1) # register signal handler 'signal.SIGINT' to function handler 'signal_usr1'
    setup_and_start_remote_debug_agent()
    sys.stdout.flush()
