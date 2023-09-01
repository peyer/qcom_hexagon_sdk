#!/usr/bin/env python
import sys
import os
from sys import platform as _platform

HEXAGON_SDK_ROOT=os.getenv('HEXAGON_SDK_ROOT')
script_dir = HEXAGON_SDK_ROOT + '/scripts/'
rpcperf_dir  = HEXAGON_SDK_ROOT + '/examples/common/rpcperf/'
sys.path.append(script_dir)
import Common_Walkthrough
from Common_Walkthrough import *
import Device_configuration as dev_conf

DBG = 0
VERBOSE = 1
RES_ERR = -1
RES_OK  = 0


CPU_CLOCK_ROOT =  '/sys/devices/system/cpu/'
CPU_NUM_OF_CLOCKS = 10
HEAP_ID = 25
unsupported_target_info = []
target_info = target_list()

tests = [
    [ 'noop',                   0,   '0K'],
    [ 'inbuf',          32 * 1024,  '32K'],
    [ 'routbuf',        32 * 1024,  '32K'],
    [ 'inbuf',          64 * 1024,  '64K'],
    [ 'routbuf',        64 * 1024,  '64K'],
    [ 'inbuf',         128 * 1024, '128K'],
    [ 'routbuf',       128 * 1024, '128K'],
    [ 'inbuf',        1024 * 1024,   '1M'],
    [ 'routbuf',      1024 * 1024,   '1M'],
    [ 'inbuf',    4 * 1024 * 1024,   '4M'],
    [ 'routbuf',  4 * 1024 * 1024,   '4M'],
    [ 'inbuf',    8 * 1024 * 1024,   '8M'],
    [ 'routbuf',  8 * 1024 * 1024,   '8M'],
    [ 'inbuf',   16 * 1024 * 1024,  '16M'],
    [ 'routbuf', 16 * 1024 * 1024,  '16M'],
]

def set_cmd_options():
    parser = argparse.ArgumentParser(prog='rpcperf.py')
    parser.add_argument("-i", action="store_true", dest="ion_enable",
                        default=False, help="Turn ION memory on. Default setting is OFF")
    parser.add_argument("-m", action="store_true", dest="check_mem",
                        default=False, help="Verify the memory operations.  Default setting is OFF")
    parser.add_argument("-z", action="store_true", dest="cpz",
                        default=False, help="Migrate to CPZ. Default setting is OFF")
    parser.add_argument("-x", action="store_true", dest="coherency_on", 
                        default=False, help="Enable coherency mode. Default setting is OFF")
    parser.add_argument("-u", action="store_true", dest="uncached", 
                        default=False, help="Use uncached buffers on the HLOS. Default setting is OFF")
    parser.add_argument("-c", action="store_true", dest="apps_clk_enable", help="Turn APPS clocks up")

    parser.add_argument("-n", action="store", dest="iters", nargs="?",
                        help="Buffer iterations to run. Default setting is 10000. noop iterations will be (buffer iterations * 10)") 
    call_parser(parser)
    return parser.parse_args()

def dbgprint(msg):
    global DBG
    if 1 == DBG and msg is not None:
        print ('{} = {}'.format(inspect.stack()[1][3], msg))

def vprint(msg):
    global VERBOSE
    if 1 == VERBOSE and msg is not None:
        print msg

def output_error_filter(outputLine):
    outputFilter = {
        'not executable: magic 7F45':'ERROR ==> Attempting to run a 64 bit app !',
        'No such file or directory':'ERROR ==> File or directory not found !',
        'device not found':'Check Device Connection !',
        'not found':'ERROR ==> Application not found !',
        'only position independent executables':'ERROR ==> Attempting to run a 32 bit app in 64 bit build !',
        "'nm' is not recognized":'ERROR ==> nm not found. Is cygwin installed?!',
        "can't execute: Permission denied":'ERROR ==> Execute Permission Denied',
        "Usage: mount [-r] [-w] [-o options]":'ERROR ==> Maybe non existant parition?',
    }

    if None ==  outputFilter:
        return True

    if None ==  outputLine:
        return True

    for k in outputFilter:
        if k in outputLine:
            print '{} - {}'.format(outputLine.rstrip('\n\r'), outputFilter[k])
            return False

    return True

def output_filter_skip(outputLine):
    global VERBOSE

    dbgprint('Start')
    outputFilter = {
        "adb is already running as root":'Device rooted',
        "* daemon started successfully *":'adb is running',
        "remount succeeded":'adb is running',
    }

    for k in outputFilter:
        if k in outputLine:
            vprint('{} - {}'.format(outputLine.rstrip('\n\r'), outputFilter[k]))
            return True

    return False

def run_cmds(cmds, verbose = False, capture_output = None):
    for cmd in cmds:
        if verbose:
            print cmd

        proc = Popen(cmd, shell=True, stdout=PIPE, stderr=STDOUT)
        while True:
            dbgprint('waiting to read a line')
            nextline = proc.stdout.readline()
            dbgprint('line read')
            if nextline == '' and proc.poll() != None:
                break

            nextline = nextline.lstrip('\n\r')
            if output_filter_skip(nextline):
                continue

            if not output_error_filter(nextline):
                return RES_ERR

            if -1 != nextline.find('WARNING: linker:'):
                continue

            if len(nextline) == 0:
                continue

            # save log to process times
            if capture_output is not None:
                capture_output.append(nextline)

            sys.stdout.write(nextline)
            sys.stdout.flush()

    return proc.returncode

def set_apps_clocks(device_number):
    clock_cmds = [
        'adb -s '+device_number+' root',
        'adb -s '+device_number+' wait-for-device',
        'adb -s '+device_number+' remount',
    ]
    run_cmds(clock_cmds, True)
    print "value of device_number is : " + device_number
    clock_cmds = [
        'adb -s '+device_number+' wait-for-device shell "echo performance > /sys/class/devfreq/soc:qcom,m4m/governor',
    ]
    run_cmds(clock_cmds, True)

    clock_cmds = [
        'adb -s '+device_number+' wait-for-device shell "echo performance > /sys/class/devfreq/soc:qcom,cpubw/governor',
    ]
    run_cmds(clock_cmds, True)

    clock_cmds = [
        'adb -s '+device_number+' wait-for-device shell "echo Y > /sys/module/lpm_levels/parameters/sleep_disabled',
    ]
    run_cmds(clock_cmds, True)

    clock_cmds = []
    for num in range(0, CPU_NUM_OF_CLOCKS):
        clock_cmds.append('adb -s '+device_number+' wait-for-device shell "echo 1 > {}/cpu{}/{}"'.format(CPU_CLOCK_ROOT, int(num), 'online'))
    run_cmds(clock_cmds, True)

    clock_cmds = []
    for num in range(0, CPU_NUM_OF_CLOCKS):
        clock_cmds.append('adb -s '+device_number+' wait-for-device shell "echo performance > {}/cpu{}/{}"'.format(CPU_CLOCK_ROOT, int(num), 'cpufreq/scaling_governor'))
    return run_cmds(clock_cmds, True)


def run_performance_tests(opt, output_capture, device_number):
    global VERBOSE
    global tests
    envion = ""
    test_suite = []
    APP_INSTALL_DIR = '/vendor/bin/'
    if opt.linux_env :
       APP_INSTALL_DIR = '/usr/bin/'
    full_app_path = os.path.join(APP_INSTALL_DIR, 'rpcperf')
    for test in tests:
        if 'noop' == test[0]:
            test_suite.append("adb -s "+device_number+" wait-for-device shell {} time {} power_boost_on {} noop {}; echo $?".
                            format(envion, full_app_path, opt.cpz, opt.noop_iters))
            test.append(opt.noop_iters)
        else:
            #test_suite.append("adb -s '+device_number+' wait-for-device shell {} time {} power_boost_on coherency_on {} {} {} {} {}".
            if opt.linux_env :
                test_suite.append("adb -s "+device_number+" wait-for-device shell {} RPCPERF_HEAP_ID={} RPCPERF_HEAP_FLAGS=1 time {} {} power_boost_on {} {} {} {} {} {} {}; echo $?".
                            format(envion, HEAP_ID, full_app_path, opt.coherency_on, opt.uncached, opt.cpz, 
                                   test[0], opt.iters, test[1],
                                   opt.ion_enable, opt.check_mem))
            else:
                test_suite.append("adb -s "+device_number+" wait-for-device shell time {} RPCPERF_HEAP_ID={} RPCPERF_HEAP_FLAGS=1 {} {} power_boost_on {} {} {} {} {} {} {}; echo $?".
                            format(envion, HEAP_ID, full_app_path, opt.coherency_on, opt.uncached, opt.cpz, 
                                   test[0], opt.iters, test[1],
                                   opt.ion_enable, opt.check_mem))
            test.append(opt.iters)
        test_suite.append("sleep 1")

    return run_cmds(test_suite, True, output_capture)

def time_str_to_usec(timeStr, bufIter):
    usecs = 0
    secs = 0
    mins = 0

    dbgprint('reged match {}'.format(timeStr))
    dbgprint('reged match {}'.format(timeStr))
    dbgprint('bufIter {}'.format(bufIter))
    if 'h' in timeStr:
        h = timeStr.split('h')
        m = timeStr.split('m')
        s = m[1].split('s')
        m = m[0].split('h')

        hours = int(h[0])
        dbgprint('hours founds {}'.format(hours))
        mins = int(m[1])
        dbgprint('mins founds {}'.format(mins))
        secs = float(s[0])
        dbgprint('sec founds {}'.format(secs))
        mins = (hours * 60) + mins
    elif 'm' in timeStr:
        m = timeStr.split('m')
        s = m[1].split('s')

        mins = int(m[0])
        dbgprint('mins founds {}'.format(mins))

        secs = float(s[0])
        dbgprint('secs found {}'.format(secs))
        secs = (mins * 60) + secs
        dbgprint('total secs {}'.format(secs))
    else :
        s = timeStr.split('s')
        secs = float(s[0])
        dbgprint('secs found {}'.format(secs))

    secs = float(secs) / float(bufIter)
    dbgprint('usecs after iter div {}'.format(secs))
    usecs = int(float(secs) * float(1000 * 1000))
    dbgprint('Total microsecs {}'.format(usecs))
    return usecs


def print_full_summary(opt, appsClk, output_summary):
    global tests
    test_index = 0

    print '\n\nTest Settings:'
    print '\tNoop Iterations    - %10d' % opt.noop_iters
    print '\tBuffer Iterations  - %10d' % opt.iters

    if opt.ion_enable:   
        print '\tION MEMORY         [ENABLED]'
    else:
        print '\tION MEMORY         [NOT ENABLED]'

    if appsClk:   
        print '\tAPPS CLOCKS        [ON]'
    else:
        print '\tAPPS CLOCKS        [OFF]'

    test_usecs = []
    print '\n\n%-10s %13s\n' % ('Test Name', 'Time')
    for line in output_summary:
        line = line.strip('\n\r')
        line = line.strip(' ')
        fields = line.split(' ')
        a = []
        for f in fields:
            f = f.strip(' ')
            if f is not None and len(f) > 0:
                a.append(f)
        if opt.linux_env :
            if "real" not in a[0] :
                continue
            if "user" in a[0]: continue
            if "sys" in a[0] : continue
            print '[%-8s %4s]  %10s %s ' % (tests[test_index][0],tests[test_index][2], a[0], a[1])
            test_index = test_index + 1
        else:
            if(len(a) == 1 and str(int(line)) == line):
                continue
    #           if(int(line) != 0):
    #               print ("test %s failed" % test_index)
    #           test_index = test_index + 1

            print '[%-8s %4s] %12s %s %10s %s %10s %s' % (tests[test_index][0], 
                                                          tests[test_index][2], 
                                                          a[0], a[1], a[2], a[3], a[4], a[5])
            test_index = test_index + 1

    test_index = 0
    print '\n\n%-10s          %s\n' % ('Test Name', 'Time Per Iteration (usecs)')
    for line in output_summary:
        line = line.strip('\n\r')
        line = line.strip(' ')
        times = line.split(' ')

        if not opt.linux_env :
            if(len(times) == 1 and str(int(line)) == line):
                continue
            usecs = time_str_to_usec(times[0], tests[test_index][3])
            print '[%-8s %4s] %12d usecs' % (tests[test_index][0], tests[test_index][2], usecs)
            test_index = test_index + 1
        else:
            if "real" not in times[0] : #and ("user" not in times[0]) and ("sys" not in times[0]) :
                dbgprint(dbgprint)
                continue
            else :
                times[0] = times[0].rsplit('\t', 1)[1]
                times[0]+=times[1] #if it fails to print properly comment this
                dbgprint(times) 
                # print times[0]

            usecs = time_str_to_usec(times[0], tests[test_index][3])
            print '[%-8s %4s] %12d usecs' % (tests[test_index][0], tests[test_index][2], usecs)
            test_index = test_index + 1


def print_open_summary(testName, BUF_ITERATIONS, appsClk, output_summary):
    test_index = 0

    print '\n\nTest Settings:'
    print '\tIterations         %d' % BUF_ITERATIONS

    print '\tION MEMORY         {}'.format(('[NOT ENABLED]', '[ENABLED]')[ION_ENABLE])
    print '\tAPPS CLOCKS        {}'.format(('[OFF]', '[ON]')[appsClk])

    print '\n\n{0:<16}{1:>14}{2:>30}{3:>30}\n'.format('Test Name', 'Total Time', 'Per Iteration (usecs)', 'Per Iteration (msecs)')
    for line in output_summary:
        time_fields = line.strip('\n\r').strip(' ').split(' ')

        # Remove empty elements
        a = [field for field in time_fields if field]

        t = time_utils(a[0], BUF_ITERATIONS)
        usecs = t.time_per_iteration_in_usecs()
        msecs = t.time_per_iteration_in_msecs()
        print '{0:<16} {1:>12}{2:>19}{3:>27}'.format('[{}]'.format(testName), a[0], int(usecs), int(msecs))

        print '[%-8s %4s] %12d usecs' % (tests[test_index][0], tests[test_index][2], usecs)


def VariantExists(variant):
   dbgprint('folderPath is {}'.format(variant))
   folderPath = os.path.join(variant, 'ship')

   if os.path.exists(folderPath):
      return folderPath
   else:
      print '{} does not exist'.format(folderPath)
      return None
    

def setup_device(variant, hexagon_variant):
   print "variant =",variant
   print "hexagon_tool_variant=",hexagon_variant
   variantPath = VariantExists(rpcperf_dir + variant)
   ADSP_LIB_PATH = '/vendor/lib/rfsa/adsp'
   APP_INSTALL_DIR = '/vendor/bin/'
   if opt.linux_env :
       ADSP_LIB_PATH = '/usr/lib/rfsa/adsp'
       APP_INSTALL_DIR = '/usr/bin/'

   if not variantPath:
       return RES_ERR

   hexagonPath = VariantExists(rpcperf_dir + hexagon_variant)
   print "hexagonPath =",format(hexagonPath)
   if not hexagonPath:
      return RES_ERR
   hexagonPath = os.path.join(hexagonPath, '.')
   variantPath = os.path.join(variantPath, 'rpcperf')
   if not os.path.isfile(variantPath):
      print 'rpcperf not found in {}'.format(variantPath)
      return RES_ERR

  
   setup = [
       'adb -s '+device_number+' wait-for-device root',
       'adb -s '+device_number+' wait-for-device remount',
       'adb -s '+device_number+' push {} {}'.format(variantPath,APP_INSTALL_DIR),
       'adb -s '+device_number+' wait-for-device shell chmod 777 {}/rpcperf'.format(APP_INSTALL_DIR),
       'adb -s '+device_number+' push {}/. {}'.format(hexagonPath,ADSP_LIB_PATH), 
   ]

   return run_cmds(setup)

if __name__ == '__main__':
    RES_OK = 0
    global HEAP_ID;
    opt = set_cmd_options()
    #print (opt)

    target_name=""
    if opt.target in unsupported_target_info:
        print "Error! "+ opt.target+" is not supported."
        sys.exit()
    if opt.target in target_info:
        target_name=opt.target 
    else :
        target_name=""
    if target_name=="" : 
        print "Error! Target name is not in list \nPlease pass -T with below supported targets :"
        for target_name in target_info: print "\t"+target_name
        sys.exit()


    #If -L specified, UbuntuARM_Debug_aarch64
    object_new = get_config() #creating an object for get_config class in Common_Walkthrough script
    hex_variant, Flag, variant = object_new.get_parameters()
    device_number=dev_conf.dev_dict(target_name)
    if device_number=="":
        print "Error! device not connected!"
        sys.exit()
    #parsing the subsystem Falg 
    if "True" != opt.hex_version:
        hex_variant = check_for_user_supplied_hex_variant(options.hex_version)

    if _platform == "win32":
        clean_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/rpcperf tree_clean V='+variant+' '+Flag+' VERBOSE=1 || exit /b'
        build_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/rpcperf tree V='+variant+' '+Flag+' VERBOSE=1 || exit /b'
        clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/rpcperf tree_clean V='+hex_variant+' VERBOSE=1 || exit /b'
        build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/rpcperf tree V='+hex_variant+' VERBOSE=1 || exit /b'
    else:
        clean_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/rpcperf tree_clean V='+variant+' '+Flag+' VERBOSE=1 || exit 1'
        build_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/rpcperf tree V='+variant+' '+Flag+' VERBOSE=1 || exit 1'
        clean_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/rpcperf tree_clean V='+hex_variant+' VERBOSE=1 || exit 1'
        build_hexagon_variant = 'make -C ' + HEXAGON_SDK_ROOT + '/examples/common/rpcperf tree V='+hex_variant+' VERBOSE=1 || exit 1'

    call_test_sig='python '+ HEXAGON_SDK_ROOT+'/scripts/testsig.py'
    if opt.linux_env :
        call_test_sig='python '+ HEXAGON_SDK_ROOT+'/scripts/testsig.py -LE'
    if not opt.no_rebuild:
        print "---- Build rpcperf example for both Android and Hexagon ----"
        print_and_run_cmd(clean_variant)
        print_and_run_cmd(build_variant)
        print_and_run_cmd(clean_hexagon_variant)
        print_and_run_cmd(build_hexagon_variant)
    else: 
        print "---- Skip rebuilding rpcperf example for both Android and Hexagon ----"
    if not opt.no_signing :
                os.system(call_test_sig)

    if RES_OK != setup_device(variant, hex_variant):
        print 'Failed copying binaries to device'
        sys.exit()

    if opt.apps_clk_enable:
        set_apps_clocks(device_number)

    if opt.ion_enable:
        opt.ion_enable = 1
    else:
        opt.ion_enable = 0

    if opt.check_mem:
        opt.check_mem = 1
    else:
        opt.check_mem = 0

    if opt.coherency_on:
        opt.coherency_on = "coherency_on"
    else:
        opt.coherency_on = ""

    if opt.uncached:
        opt.uncached = "uncached"
    else:
        opt.uncached = ""

    if opt.cpz:
        opt.cpz = "migrate_cpz"
    else:
        opt.cpz = ""

    if None != opt.iters:
        opt.iters = int(opt.iters)
    else:
        opt.iters = 1000

    if None != opt.iters:
         opt.noop_iters = 10 * opt.iters

    full_test_summary = []
    if RES_OK != run_performance_tests(opt, full_test_summary, device_number):
       print 'run_performance_tests failed'
       sys.exit()
    print_full_summary(opt, opt.apps_clk_enable, full_test_summary)
