#ifndef DSPCV_HVX_H
#define DSPCV_HVX_H

/**=============================================================================

@file
   dspCV_hvx.h

@brief
   Utility providing functions for accessing the Hexagon Vector Extensions (HVX)
   hardware.

Copyright (c) 2014 Qualcomm Technologies Incorporated.
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
#define DSPCV_HVX_API __attribute__ ((visibility ("default")))
#else
/// MACRO empty for non-shared-library case.
#define DSPCV_HVX_API
#endif

//==============================================================================
// Include Files
//==============================================================================

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
    TYPEDEF
===========================================================================*/
typedef enum 
{
    DSPCV_HVX_MODE_DONT_CARE = 0,  // don't-care, just use whatever is current mode
    DSPCV_HVX_MODE_64B,            // 64 byte HVX vector width
    DSPCV_HVX_MODE_128B            // 128 byte HVX vector width
} dspCV_hvx_mode_t;

// HVX configuration struct
typedef struct
{
    int numUnits;           // number of HVX units
    int tempReserve;        // indicates HVX pool reservation is temporary and needs to be released after use
    dspCV_hvx_mode_t mode;  // configured HVX mode
    int VLEN;               // configured HVX vector width (64 or 128 bytes)
    int numThreads;         // number of threads that can lock HVX units
} dspCV_hvx_config_t;

/*===========================================================================
    CONSTANTS
===========================================================================*/

//==============================================================================
// Declarations
//==============================================================================

//---------------------------------------------------------------------------
/// @brief
///   This function resets HVX configuration (such as enable/disable, preferred 
///   mode, reservations, etc.) to defaults. 
///
/// @detailed
///    TBD.
//---------------------------------------------------------------------------
DSPCV_HVX_API void
dspCV_hvx_reset_config(void);

//---------------------------------------------------------------------------
/// @brief
///   This function reserves HVX units for the protection domain to which 
///   the caller belongs. Reservation is optional before locking HVX units.
///   Typically it would be called by applications that want to guarantee 
///   up front that the requested number of HVX units will be available 
///   for the duration of the application. Each call to this function 
///   over-rides any previous calls from within the same protection domain. 
///
/// @detailed
///    TBD.
///
/// @param num_units
///   Number of HVX units to reserve. 0 indicates to reserve all the units
///   present in the given target. > 0 indicates the number of single HVX units
///   to reserve. Mode (64 byte vs. 128 byte) is not specified. 
///
/// @return the number of HVX units (in terms of 64 byte single units)
///   successfully reserved. -1 indicates no HVX HW is available on the 
///   target.
//---------------------------------------------------------------------------
DSPCV_HVX_API int
dspCV_hvx_reserve(unsigned int num_units);

//---------------------------------------------------------------------------
/// @brief
///   This function releases all HVX unit from reservation. A call to this 
///   function nullifies all previous calls to reserve HVX units from within
///   this worker pool's protection domain.
///
/// @detailed
///    TBD.
//---------------------------------------------------------------------------
DSPCV_HVX_API void
dspCV_hvx_unreserve(void);

//---------------------------------------------------------------------------
/// @brief
///   This function returns the number of HVX units (single 64 byte units)
///   currently reserved for this worker pool's protection domain.
///
/// @detailed
///    TBD.
///
/// @return the number of HVX units (in terms of 64 byte single units)
///   currently reserved.
//---------------------------------------------------------------------------
DSPCV_HVX_API int
dspCV_hvx_num_reserved(void);

//---------------------------------------------------------------------------
/// @brief
///   This function turns on the HVX hw. It must be called sometime before 
///   (possibly multiple) SW threads lock HVX units.
///
/// @detailed
///    TBD.
///
/// @return 0 = success, -1 indicates HVX could not be powered on.
//---------------------------------------------------------------------------
DSPCV_HVX_API int
dspCV_hvx_power_on(void);

//---------------------------------------------------------------------------
/// @brief
///   This function turns off the HVX hw. It must be called sometime after all
///   threads have unlocked their HVX units.
///
/// @detailed
///    TBD.
//---------------------------------------------------------------------------
DSPCV_HVX_API void
dspCV_hvx_power_off(void);

//---------------------------------------------------------------------------
/// @brief
///   This function locks an HVX unit for the calling SW thread.
///
/// @detailed
///    TBD.
///
/// @param mode
///   inidcates the desired HVX vector width. 
///
/// @param block
///   inidcates whether or not to issue a blocking call to lock the
///   HVX unit. 0 indicates to attempt a lock, but not block. 1 indicates
///   to issue a blocking call to lock the HVX unit. In this case, function
///   will not return until an HVX unit is available in the desired mode and
///   successfully locked. 
///
/// @return -1 indicates the HVX unit could not be locked. Otherwise, return
///   value is the resultant HVX vector length (VLEN) 
//---------------------------------------------------------------------------
DSPCV_HVX_API int
dspCV_hvx_lock(dspCV_hvx_mode_t mode, unsigned int block);

//---------------------------------------------------------------------------
/// @brief
///   This function unlocks an HVX unit from the calling SW thread.
///
/// @detailed
///    TBD.
///
//---------------------------------------------------------------------------
DSPCV_HVX_API void
dspCV_hvx_unlock(void);

//---------------------------------------------------------------------------
/// @brief
///   This utility function helps a typical HVX application gather the 
///   current HVX configuration and prepare for a multi-threaded HVX job.
///   It is optional to use this function. The application may explicitly 
///   perform the similar operations if it requires a specific HVX mode
///   or configuration.
///
/// @detailed
///    TBD.
///
/// @param hvx_config
///   The caller should zero-initialize this structure, but populate the 
///   mode field to non-zero if a specific mode is required. The function
///   will populate the rest of the fields with the HVX configuration that 
///   the multi-threaded job should use.
//---------------------------------------------------------------------------
DSPCV_HVX_API void
dspCV_hvx_prepare_mt_job(dspCV_hvx_config_t *hvx_config);

//---------------------------------------------------------------------------
/// @brief
///   This utility function should be called after the conclusion of a 
///   multi-threaded job that was prepared via a call to  
///   dspCV_hvx_prepare_mt_job. 
///
/// @detailed
///    TBD.
///
/// @param hvx_config
///   This struct should have been configured via call to dspCV_hvx_prepare_mt_job.
//---------------------------------------------------------------------------
DSPCV_HVX_API void
dspCV_hvx_cleanup_mt_job(dspCV_hvx_config_t *hvx_config);

//---------------------------------------------------------------------------
/// @brief
///   This utility function sets this instance of dspCV to respond to HVX requests
///   as though there were no HVX hardware present. It is useful for forcing
///   the same conditions for the caller as when there is no HVX HW present 
///   (e.g. for ensuring the non-HVX code paths will be followed and no HVX HW
///   will be used). 
///
/// @detailed
///    TBD.
//---------------------------------------------------------------------------
DSPCV_HVX_API void
dspCV_hvx_disable(void);

//---------------------------------------------------------------------------
/// @brief
///   This utility function re-enables HVX access, if it has been previously
///   explicitly disabled. By default, HVX access is enabled. 
///
/// @detailed
///    TBD.
//---------------------------------------------------------------------------
DSPCV_HVX_API void
dspCV_hvx_enable(void);

//---------------------------------------------------------------------------
/// @brief
///   This utility function sets the default HVX vector width mode, to be used
///   (only) when a dspCV client locks HVX context with DSPCV_HVX_MODE_DONT_CARE
///   mode. 
///
/// @param mode indicates the desired vector width.
///
/// @detailed
///    TBD.
//---------------------------------------------------------------------------
DSPCV_HVX_API void
dspCV_hvx_set_default_mode(dspCV_hvx_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif  // #ifndef DSPCV_HVX_H
