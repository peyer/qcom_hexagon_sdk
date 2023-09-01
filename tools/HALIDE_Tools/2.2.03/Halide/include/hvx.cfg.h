/*=====================================================================
               Copyright (c) 2013 QUALCOMM Incorporated.
                          All Rights Reserved.
                 QUALCOMM Confidential and Proprietary
======================================================================*/
#pragma once

#if !defined(LOG2VLEN)
#define         LOG2VLEN    7
#endif
#define         VLEN        (1<<LOG2VLEN)
#if LOG2VLEN == 6
    #define FUNCTION_NAME(name) name##_hvx64_i
#else
    #define FUNCTION_NAME(name) name##_hvx128_i
#endif
