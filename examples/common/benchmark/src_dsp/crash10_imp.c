/**=============================================================================

@file
   crash10_imp.cpp

@brief
   implementation file for dummy memcpy with a forced crash on 10th iteration.

Copyright (c) 2016 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

//==============================================================================
// Include Files
//==============================================================================

// enable message outputs for profiling by defining _DEBUG and including HAP_farf.h
#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"

// profile DSP execution time (without RPC overhead) via HAP_perf api's.
#include "HAP_perf.h"

#include "AEEStdErr.h"

// bilateral9x9 includes
#include "benchmark.h"

#include "hexagon_types.h"
#include "hexagon_protos.h"

/*===========================================================================
    DEFINITIONS
===========================================================================*/
#define PROFILING_ON

/*===========================================================================
    DECLARATIONS
===========================================================================*/

/*===========================================================================
    TYPEDEF
===========================================================================*/

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/

AEEResult benchmark_crash10(remote_handle64 handle,const uint8* src, 
    int srcLen, int32 stride, int32 width, int32 height, uint8* dst, int dstLen, 
    int32 LOOPS, int32 wakeupOnly, int32 *dspUsec, int32 *dspCyc) 
{
    static int count = 0;
    
    // trigger a crash on the 10th invocation
    if (10 == ++count)
    {
        volatile int* crash=NULL;
        *crash = 0xdead;
    }
    
    *dspUsec = 0, *dspCyc = 0;
    if (wakeupOnly)
    {
        return AEE_SUCCESS;
    }
// only supporting HVX version in this example.
#if (__HEXAGON_ARCH__ < 60)
    return AEE_EUNSUPPORTED;
#endif

    // record start time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 startTime = HAP_perf_get_time_us();
    uint64 startCycles = HAP_perf_get_pcycles();
#endif

    for (int loops = 0; loops < LOOPS; loops++)
    {
        for(int y=0; y<height; y++)
        {
            for(int x=0; x<width; x++)
            {
                dst[y*stride+x] = src[y*stride+x];
            }
        }
    }

    // record end time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 endCycles = HAP_perf_get_pcycles();
    uint64 endTime = HAP_perf_get_time_us();
    *dspUsec = (int32)(endTime - startTime);
    *dspCyc = (int32)(endCycles - startCycles);
    FARF(HIGH,"crash10 profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
        (int)((endCycles - startCycles) / (endTime - startTime)));
#endif

	return AEE_SUCCESS;
}


