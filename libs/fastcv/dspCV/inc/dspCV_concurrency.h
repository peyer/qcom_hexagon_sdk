#ifndef DSPCV_CONCURRENCY_H
#define DSPCV_CONCURRENCY_H

/**=============================================================================

@file
   dspCV_concurrency.h

@brief
   Utility providing concurrency related API's for compute use cases.

Copyright (c) 2015 Qualcomm Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary

Export of this technology or software is regulated by the U.S.
Government. Diversion contrary to U.S. law prohibited.

All ideas, data and information contained in or disclosed by
this document are confidential and proprietary information of
Qualcomm Technologies Incorporated and all rights therein are expressly reserved.
By accepting this material the recipient agrees that this material
and the information contained therein are held in confidence and in
trust and will not be used, copied, reproduced in whole or in part,
nor its contents revealed in any manner to others without the express
written permission of Qualcomm Technologies Incorporated.

=============================================================================**/
//==============================================================================
// Defines
//==============================================================================
#ifdef BUILDING_SO
/// MACRO enables function to be visible in shared-library case.
#define DSPCV_CONCURRENCY_API __attribute__ ((visibility ("default")))
#else
/// MACRO empty for non-shared-library case.
#define DSPCV_CONCURRENCY_API
#endif

//==============================================================================
// Include Files
//==============================================================================

#ifdef __cplusplus
extern "C" {
#endif

#include "dspCV.h"

/*===========================================================================
    TYPEDEF
===========================================================================*/

/*===========================================================================
    CONSTANTS
===========================================================================*/

//==============================================================================
// Declarations
//==============================================================================

//---------------------------------------------------------------------------
/// @brief
///   This function queries for DSP concurrency information helpful to determine 
///   whether it is safe to run a compute use case. See dspCV.idl for attribute 
///   details.  
///
/// @detailed
///    TBD.
///
/// @param attribs
///   This is a list of requested attributes, to be filled with values by this function.
//---------------------------------------------------------------------------
DSPCV_CONCURRENCY_API void
dspCV_concurrency_query(dspCV_ConcurrencyAttribute* attrib, int attribLen);

//---------------------------------------------------------------------------
/// @brief
///   This utility function sets the threshold for Audio MPPS which, under 
///   concurrency with a single-HVX camera streaming use case, should be 
///   considered too heavy to advise CV use cases to continue running on the 
///   DSP. 
///
/// @param threshold indicates the minimum audio MPPS concurrent with a 
///   single-HVX camera streaming use case that should advise against running
///   additional CV (or other compute) processing on the DSP.
///
/// @detailed
///    TBD.
//---------------------------------------------------------------------------
DSPCV_CONCURRENCY_API void
dspCV_concurrency_set_audio_mpps_1_hvx_threshold(int threshold);

//---------------------------------------------------------------------------
/// @brief
///   This utility function sets the threshold for Audio MPPS which, under 
///   concurrency with a dual-HVX camera streaming use case, should be 
///   considered too heavy to advise CV use cases to continue running on the 
///   DSP. 
///
/// @param threshold indicates the minimum audio MPPS concurrent with a 
///   dual-HVX camera streaming use case that should advise against running
///   additional CV (or other compute) processing on the DSP.
///
/// @detailed
///    TBD.
//---------------------------------------------------------------------------
DSPCV_CONCURRENCY_API void
dspCV_concurrency_set_audio_mpps_2_hvx_threshold(int threshold);

#ifdef __cplusplus
}
#endif

#endif  // #ifndef DSPCV_CONCURRENCY_H
