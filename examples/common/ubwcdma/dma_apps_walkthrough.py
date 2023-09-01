#!/usr/bin/env python
# child python script

### Imports ###
import os       # provide output to command prompt
import sys      # flush command prompt output
from sys import platform as _platform

HEXAGON_SDK_ROOT=os.getenv('HEXAGON_SDK_ROOT')
script_dir = HEXAGON_SDK_ROOT + '/scripts/'
sys.path.append(script_dir)
import Common_Walkthrough
from Common_Walkthrough import *
import Device_configuration as dev_conf

pid = os.getpid()   # return the current process ID
received = False    # initialize signal received to false

example_name = "dma_apps"
unsupported_target_info = ["8998","8996","sdm660","sm6150","qcs405"]
legacy_driver_target = ["sdm845", "sdm710", "sdm610", "sdm670"]

target_info = target_list()
#******************************************************************************
# Parser for cmd line options
#******************************************************************************
parser = argparse.ArgumentParser(prog='dma_apps_walkthrough.py', description=__doc__, formatter_class=RawTextHelpFormatter)
call_parser(parser)
parser.add_argument("app_args", nargs=5, help="[app] [height] [width] [fmt] [ubwc_en]\n\tapp = [memcpy|blend|sum_hvx]\n\theight = frame height\n\twidth= frame width\n\tfmt = [0-3]\t\t-- 0=NV12,1=NV21,2=NV124R,3=P010\n\tubwc_en = [0-1]\t\t-- 0=disable,1=enable\n")

options = parser.parse_args()

# run walkthrough
def run_example():
    if len(options.app_args) != 5:
        parser.print_help()
        sys.exit()

    target_name=""
    if options.target in unsupported_target_info:
        print "Error! "+ options.target+" is not supported."
        sys.exit()
    if options.target in target_info:
        target_name=options.target 
    else :
        target_name=""
    if target_name == "" : 
        print "Target name is not in list - supported targets :"
        for target_name in target_info: print "\t"+target_name
        sys.exit()

    EXAMPLE_PATH='/examples/common/ubwcdma'
    device_number=dev_conf.dev_dict(target_name)
    if device_number=="":
        print "Error! device not connected!"
        sys.exit()
    # parsing the subsystem Flag 
    object_new = get_config() #creating an object for get_config class in Common_Walkthrough script
    hex_variant, Flag, variant = object_new.get_parameters()

    example_name_exe=HEXAGON_SDK_ROOT +EXAMPLE_PATH+'/'+variant+'/ship/'+example_name+'_test'
    libexample_name_skel=HEXAGON_SDK_ROOT +EXAMPLE_PATH+'/'+hex_variant+'/ship/lib'+example_name+'_skel.so'

    opt = ""
    if target_name.lower() in legacy_driver_target:
        opt = "UBWCDMA_LEGACY_DRIVER=1"

    if _platform == "win32":
        clean_variant = 'make -C ' + HEXAGON_SDK_ROOT +EXAMPLE_PATH+' tree_clean V='+variant+' '+Flag+' || exit /b'
        build_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+' tree V='+variant+' '+Flag+'  || exit /b'
        clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+' tree_clean V='+hex_variant+' || exit /b'
        build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+' tree V='+hex_variant+' '+opt+' || exit /b'
    else:
        clean_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+' tree_clean V='+variant+' '+Flag+'  || exit 1'
        build_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+' tree V='+variant+' '+Flag+'  || exit 1'
        clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+' tree_clean V='+hex_variant+' || exit 1'
        build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + EXAMPLE_PATH+' tree V='+hex_variant+' '+opt+' || exit 1'

    call_test_sig , APPS_DST, DSP_DST, LIB_DST, DSP_LIB_PATH, DSP_LIB_SEARCH_PATH = get_DST_PARAMS(HEXAGON_SDK_ROOT)

    if not options.no_rebuild:
        print "---- Build ubwcdma example for both Android and Hexagon ----"
        print_and_run_cmd(clean_variant)
        print_and_run_cmd(build_variant)
        print_and_run_cmd(clean_hexagon_variant)
        print_and_run_cmd(build_hexagon_variant)
    else: 
        print "---- Skip rebuilding ubwcdma example for both Android and Hexagon ----"
    if not options.no_signing :
        os.system(call_test_sig)
    
    print "---- root/remount device ----"
    mount_device(device_number, parser)
    
    print "---- Push Android components ----"
    print_and_run_cmd('adb -s '+device_number+' wait-for-device shell mkdir -p '+APPS_DST)   
    print_and_run_cmd('adb -s '+device_number+' wait-for-device push '+example_name_exe+' '+APPS_DST)
    print_and_run_cmd('adb -s '+device_number+'  wait-for-device shell chmod 777 '+APPS_DST+'/dma_apps_test')
    print "---- Push Hexagon Components ----"
    print_and_run_cmd('adb -s '+device_number+' wait-for-device shell mkdir -p '+DSP_DST)
    print_and_run_cmd('adb -s '+device_number+' wait-for-device push '+libexample_name_skel+' '+DSP_DST)

    print "---- Run Example on ---- " +Flag[:4] +" ----"
    args_list = ""
    for arg in options.app_args:
        args_list = args_list + arg + " "
    print_and_run_cmd('adb -s '+device_number+' wait-for-device shell export LD_LIBRARY_PATH='+LIB_DST+' '+DSP_LIB_SEARCH_PATH+'='+DSP_LIB_PATH+' '+APPS_DST+'/'+example_name+'_test '+args_list)

    print "Done"

run_example()                                       # call function to initialize debug_agent
sys.stdout.flush()                                      # show output immediately in command prompt

