CDSP Trace collection and PyETM Instructions:

Prerequisites: Python 2.7, pyETM (found in Hexagon SDK), adb drivers

1. Push sysMonApp to device
	> pushd <Hexagon_SDK_Root>\tools\utils\sysmon\
	> adb root
	> adb push sysMonApp /data/sysMonApp
	> adb shell chmod 777 /data/sysMonApp
	> adb reboot

2. Obtain Root on device
	> adb wait-for-device root

3. Echo internal values for disableing ftrace logs and STM, then set trace buffer size and reset trace sinks:
	> adb shell "echo 0 > /sys/kernel/debug/tracing/events/enable"
	> adb shell "echo 0 > /sys/bus/coresight/devices/coresight-stm/enable_source"
	> adb shell "echo 0x2000000 > /sys/bus/coresight/devices/coresight-tmc-etr/mem_size" 
	> adb shell "echo 1 > /sys/bus/coresight/reset_source_sink"
	> adb shell "echo 1 > /sys/bus/coresight/devices/coresight-tmc-etr/enable_sink"

4. Enable CDSP ETM and Funnels
	> adb shell "echo 1 > /sys/bus/coresight/devices/coresight-turing-etm0/enable_source"
	
5. Enable SysMon to grab the dll lists
    > adb shell setprop vendor.fastrpc.process.attrs 1

6. Enable trace collection (Will begin filling the trace buffer):
	> adb shell /data/sysMonApp etmTrace --command etm --q6 CDSP
	
7. Wait several seconds allowing the use case to run, then pull trace from device:
	> adb shell cat /dev/coresight-tmc-etr > /data/<my_file>.bin
	> adb pull /data/<my_file>.bin
	
8. Collect DLL values (run command then copy output to a .txt to save)
	> adb shell /data/sysMonApp etmTrace --command dll --q6 CDSP

9. Copy all relevant ELF or .so files listed by sysmon from the device or the build to trace location
	
10. Setup Config.py:
	- Copy config.py from pyETM/example folder. Modify according to use case, important changes below:
		~ include absolute path to each copied elf in the elflist in double quotes, separated by commas
		~ include load address of each elf (given by sysmon) in elfoffset list, separated by commas


		
Parsing the pulled trace files using pyETM:
 
	Pyetm runs under Linux, it can run standalone, an example Dockerfile and configuration are included.
    Docker is used to provide a consistent virtualized linux environment, and installs all the tools required to run pyetm
    To run pyetm under Docker use 
    docker build -t pyetm_env .  
    docker run -v "/pkg/qct/software/hexagon":"/app/hexagon" -v $WORKSPACE/results:"/app/results" pyetm_env /bin/bash -c './pyetm ./config.py ./results ./trace.bin '

    To run pyetm standalone, without using docker, use the following command.
    ./pyetm ./config.py ./results ./trace.bin
	
	


