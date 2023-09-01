The qmath_sample application demonstrates how to build & link an application that uses
qmath API's. It can be built and run as a Hexagon simulator executable, or as an Android
application and DSP shared library.

Build/run in simulation:
------------------------

    make tree VERBOSE=1 V=<static build flavor> 
    
For example, building for Hexagon v65, 
    make tree VERBOSE=1 V=hexagon_Release_toolv81_v65

Build/run in Android:
---------------------

    make tree VERBOSE=1 V=android_Release
    make tree VERBOSE=1 V=<dynamic build flavor>

For example, building for Hexagon v65, 
    make tree VERBOSE=1 V=hexagon_Release_dynamic_toolv81_v65
    
    adb push android_Release\ship\qmath_sample /data/local
    adb push hexagon_Release_dynamic_toolv81_v65\ship\libqmath_sample_skel.so /system/lib/rfsa/adsp
    adb shell chmod 755 /data/local/qmath_sample
    adb shell /data/local/qmath_sample
    
    

