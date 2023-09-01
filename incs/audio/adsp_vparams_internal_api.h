#ifndef __ADSP_VPARAMS_INTERNAL_API_H__
#define __ADSP_VPARAMS_INTERNAL_API_H__

/**
  @file adsp_vparams_api.h
  @brief This file contains module and parameter IDs for calibration purposes.
*/
/*===========================================================================
NOTE: The @brief description and any detailed descriptions above do not appear
      in the PDF.

      The Voice_mainpage.dox file contains all file/group
      descriptions that are in the output PDF generated using Doxygen and
      Latex. To edit or update any of the file/group text in the PDF, edit
      the Voice_mainpage.dox file or contact Tech Pubs.
===========================================================================*/

/*===========================================================================

  Copyright (c) 2010, 2012-2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*===========================================================================
                        EDIT HISTORY FOR MODULE


when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/17/14   sa      Created this file for internal GUIDs
===========================================================================*/


/** Identifies the Soft Mute algorithm that is part of voice postprocessing on
    the Stream and Device Rx/Tx path.
    This module supports the following parameter IDs:

    - #VOICE_PARAM_SOFT_MUTE
*/
#define VOICE_MODULE_SOFT_MUTE             (0x00010F1C)

/** Soft mute param Id. The maximum array size for this parameter is 4 bytes.

    @inputtable{Calibration_ID_VOICE_PARAM_SOFTMUTE.tex}
*/
#define VOICE_PARAM_SOFT_MUTE                 (0x00010E45)


/** facing theshold param Id. payload provided the facing angle threshold set in the algorithm.
    This is read-only paramId and FW need to register to sensor's with this threshold.
    The maximum array size for this parameter is 4 bytes.
    facing angle is used for registering with the sensors for getting evets when the
    holding angle crosses this threshold

    @inputtable{Calibration_ID_VOICE_PARAM_FACING_THRESHOLD.tex}
*/
#define VOICE_PARAM_FACING_THRESHOLD         (0x00010E52)

/** ParamId to set holding patter reported by accelerometer.
    This is write-only paramId and FW set the value to algorithm when it is changed.
    The maximum array size for this parameter is 4 bytes.
    facing angle is used for registering with the sensors for getting evets when the
    holding angle crosses this threshold

    @inputtable{Calibration_ID_VOICE_PARAM_HOLDING_PATTERN.tex}
*/
#define VOICE_PARAM_HOLDING_PATTERN         (0x00010E53)

/** Identifies FNS module in TX path. This module is invoked from SMECNS for non stationary noise
 * suppression
 */

#define VOICE_MODULE_TX_EANS            (0x00010F22)

/** Identifies the Sample Slip module which is part of voice processing on Tx and Rx
    path. This module supports following paramids :

    SS_PARAM_MODE_OPER
    SS_PARAM_INST_DRIFT
    SS_PARAM_DRIFT_SAMPLES
    SS_PARAM_RESET

 */
#define VOICE_MODULE_SAMPLE_SLIP             (0x00010F1D)

/** param-id to set the operating mode of Sample slip capi
 */
#define SS_PARAM_MODE_OPER (0x00010E4E)



/** Identifies the HPCM framework that is part of voice postprocessing on
    the Stream and Device Rx/Tx path.
    This module does not support any param Ids:
*/
#define VOICE_MODULE_HPCM                    (0x00010F1E)

/** Identifies IIR filter based voice resampler module.
    This module does not support any param Ids:
*/
#define VOICE_MODULE_IIR_RESAMPLER          (0x00010F21)

/** Channel mapping used in VPTx */

/** Offset corresponding to the number of public channel
    definations supported by Audio. leaving headroom for 20 new definations by Audio
*/
#define PCM_CHANNEL_OFFSET (34+20)

/** primary mic input channel*/
#define PCM_CHANNEL_PRI_MIC (PCM_CHANNEL_OFFSET+1)

/** secondary mic 1 input channel */
#define PCM_CHANNEL_SEC_MIC_1 (PCM_CHANNEL_OFFSET+2)

/** secondary mic 2 input channel */
#define PCM_CHANNEL_SEC_MIC_2 (PCM_CHANNEL_OFFSET+3)

/** secondary mic 3 input channel*/
#define PCM_CHANNEL_SEC_MIC_3 (PCM_CHANNEL_OFFSET+4)

/** Far-end reference input channel*/
#define PCM_CHANNEL_FAR_REF (PCM_CHANNEL_OFFSET+5)

/** Linear echo canceller output channel*/
#define PCM_CHANNEL_LEC_OUT (PCM_CHANNEL_OFFSET+6)

/** Linear noise reference output channel*/
#define PCM_CHANNEL_NREF_OUT (PCM_CHANNEL_OFFSET+7)

/** @endcond */

/** @} */ /* end_addtogroup cal_module_ids */

#endif /* __ADSP_VPARAMS_INTERNAL_API_H__ */
