The libqmath.a static library can be built in the SDK environment with the following command:

    make tree VERBOSE=1 V=<flavor> 

For example, building for Hexagon v65, 
    
    make tree VERBOSE=1 V=hexagon_Release_dynamic_toolv81_v65 (if it will be statically linked to a dynamic .so)
        - or -
    make tree VERBOSE=1 V=hexagon_Release_toolv81_v65 (if it will be linked to a simulator executable)
    
For more information on the library contents, please see docs/qmath_lib.pdf.

