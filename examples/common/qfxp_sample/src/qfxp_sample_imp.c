/**=============================================================================

@file
   qfxp_sample_imp.cpp

@brief
   implementation file for testing qfxp.

Copyright (c) 2017 QUALCOMM Technologies Incorporated.
_all Rights Reserved Qualcomm Proprietary
=============================================================================**/

//==============================================================================
// Include Files
//==============================================================================

// enable message outputs for profiling by defining _DEBUG and including HAP_farf.h
#ifndef _DEBUG
#define _DEBUG
#endif

#include "HAP_farf.h"
#undef FARF_HIGH
#define FARF_HIGH 1

#include "qfxp_sample.h"
#include "dspCV_hvx.h"
#include "dspCV_worker.h"

#include "AEEStdErr.h"

// includes
#include "qfxp.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "hexagon_protos.h"
#include "example.h"

#ifdef __cplusplus
extern "C" {
#endif


/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
AEEResult qfxp_sample_open(const char *uri, remote_handle64 *h) 
{
    *h = 0x00DEAD00;
    return 0;
}

AEEResult qfxp_sample_close(remote_handle64 h) 
{
    return 0;
}
AEEResult qfxp_sample_run(remote_handle64 h)
{
#if defined(__HEXAGON_ARCH__) && (__HEXAGON_ARCH__ < 62)
    // only supporting HVX version in this example.
    return AEE_EUNSUPPORTED;
#endif

	int retVal = 0;
	
    // power on HVX (this may be redundant on recent targets, but harmless).
    (void) dspCV_hvx_power_on();

    if (128 != dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 0))
    {
        FARF(ERROR,"Could not acquire HVX in 128B mode!!");
		retVal=AEE_EFAILED;
        goto bail;
    }
    
	example();
	
    FARF(HIGH, "SUCCESS");

bail: 
    dspCV_hvx_unlock();	
    dspCV_worker_pool_deinit();
	
    return AEE_SUCCESS;
}

#ifdef __cplusplus
}
#endif

