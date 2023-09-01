/*========================================================================*/
/**
@file adsp_media_fmt.h

This file contains the structure definitions for the media format
blocks used in client-ASM communications.
*/

/*===========================================================================
NOTE: The @brief description above does not appear in the PDF.
      The description that displays in the PDF is located in the
      audio_mainpage.dox file. Contact Tech Pubs for assistance.
===========================================================================*/

/*===========================================================================
Copyright (c) 2010-2016, 2018 Qualcomm Technologies, Inc.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
======================================================================== */

/*========================================================================
Edit History

$Header: //source/qcom/qct/platform/adsp/rel/Hexagon_SDK/3.5.4/Linux/SYNC/qualcomm_hexagon_sdk_3_5_4/incs/audio/adsp_media_fmt.h#1 $

when       who     what, where, why
--------   ---      -------------------------------------------------------
09/06/13   sw      (Tech Pubs) Merged Doxygen comments/markup from 2.0; edited
                    new Doxygen comments for 2.2.
08/06/12   sw      (Tech Pubs) Edited comments; updated Doxygen markup to
                   Rev D.2 templates; updated legal statements for QTI.
05/30/11   sw      (Tech Pubs) Updated Doxygen comments for Interface Spec doc.
10/12/10   leo     (Tech Pubs) Edited/added Doxygen comments and markup.
04/15/10   rkc      Created file.

========================================================================== */

#ifndef _ADSP_MEDIA_FMT_H_
#define _ADSP_MEDIA_FMT_H_

#include "mmdefs.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*
 * Include file to add new media format ID and media format block. This file is present at adsp_proc\hap\integration\audencdec
 * To include this file, set OEM_ROOT from command prompt
 */
#if defined(OEM_ROOT)
#include "custom_adsp_media_fmt.h"
#endif

/** @ingroup asmstrm_mediafmt_eg
    Media format example for clients to try out a dummy encoder or decoder.
    This is for illustration purposes only.

    @par Example format block
    No format block is required.

    @par Example encode configuration block
    No format block is required.
*/
#define ASM_MEDIA_FMT_EXAMPLE                   0x00010C4D

/** @addtogroup asmstrm_mediafmt_pcm_ch_defs
@{ */

/** Front left channel. */
#define PCM_CHANNEL_L    1

/** Front right channel. */
#define PCM_CHANNEL_R    2

/** Front center channel. */
#define PCM_CHANNEL_C    3

/** Left surround channel.

    This speaker position enumeration is maintained for backward compatibility.
    Currently, there are no devices that support this type. The client is
    responsible for ensuring that valid and existing channel mapping
    configurations corresponding to real speaker positions are used to ensure
    proper user experience without audio/data losses.
*/
#define PCM_CHANNEL_LS   4

/** Right surround channel.

    This speaker position enumeration is maintained for backward compatibility.
    Currently, there are no devices that support this type. The client is
    responsible for ensuring that valid and existing channel mapping
    configurations corresponding to real speaker positions are used to ensure
    proper user experience without audio/data losses.
*/
#define PCM_CHANNEL_RS   5

/** Low frequency effect channel. */
#define PCM_CHANNEL_LFE  6

/** Center surround channel; rear center channel. */
#define PCM_CHANNEL_CS   7

/** Center back channel. */
#define PCM_CHANNEL_CB  PCM_CHANNEL_CS

/** Left back channel; rear left channel. */
#define PCM_CHANNEL_LB   8

/** Right back channel; rear right channel. */
#define PCM_CHANNEL_RB   9

/** Top surround channel.

    This speaker position enumeration is maintained for backward compatibility.
    Currently, there are no devices that support this type. The client is
    responsible for ensuring that valid and existing channel mapping
    configurations corresponding to real speaker positions are used to ensure
    proper user experience without audio/data losses.
*/
#define PCM_CHANNEL_TS   10

/** Center vertical height channel.

    This speaker position enumeration is maintained for backward compatibility.
    Currently, there are no devices that support this type. The client is
    responsible for ensuring that valid and existing channel mapping
    configurations corresponding to real speaker positions are used to ensure
    proper user experience without audio/data losses.
*/
#define PCM_CHANNEL_CVH  11

/** Top front center channel.

    Currently, there are no devices that support this type. The client is
    responsible for ensuring that valid and existing channel mapping
    configurations corresponding to real speaker positions are used to ensure
    proper user experience without audio/data losses.
*/
#define PCM_CHANNEL_TFC  PCM_CHANNEL_CVH

/** Mono surround channel.

    This speaker position enumeration is maintained for backward compatibility.
    Currently, there are no devices that support this type. The client is
    responsible for ensuring that valid and existing channel mapping
    configurations corresponding to real speaker positions are used to ensure
    proper user experience without audio/data losses.
*/
#define PCM_CHANNEL_MS   12

/** Front left of center channel. */
#define PCM_CHANNEL_FLC  13

/** Front right of center channel. */
#define PCM_CHANNEL_FRC  14

/** Rear left of center channel. */
#define PCM_CHANNEL_RLC  15

/** Rear right of center channel. */
#define PCM_CHANNEL_RRC  16

/** Secondary low frequency effect channel. */
#define PCM_CHANNEL_LFE2  17

/** Side left channel. */
#define PCM_CHANNEL_SL  18

/** Side right channel. */
#define PCM_CHANNEL_SR  19

/** Top front left channel. */
#define PCM_CHANNEL_TFL  20

/** Left vertical height channel. */
#define PCM_CHANNEL_LVH  PCM_CHANNEL_TFL

/** Top front right channel. */
#define PCM_CHANNEL_TFR 21

/** Right vertical height channel. */
#define PCM_CHANNEL_RVH PCM_CHANNEL_TFR

/** Top center channel. */
#define PCM_CHANNEL_TC  22

/** Top back left channel. */
#define PCM_CHANNEL_TBL  23

/** Top back right channel. */
#define PCM_CHANNEL_TBR  24

/** Top side left channel. */
#define PCM_CHANNEL_TSL  25

/** Top side right channel. */
#define PCM_CHANNEL_TSR  26

/** Top back center channel. */
#define PCM_CHANNEL_TBC  27

/** Bottom front center channel. */
#define PCM_CHANNEL_BFC  28

/** Bottom front left channel. */
#define PCM_CHANNEL_BFL  29

/** Bottom front right channel. */
#define PCM_CHANNEL_BFR  30

/** Left wide channel. */
#define PCM_CHANNEL_LW  31

/** Right wide channel. */
#define PCM_CHANNEL_RW  32

/** Left side direct channel. */
#define PCM_CHANNEL_LSD  33

/** Right side direct channel. */
#define PCM_CHANNEL_RSD  34

/** @} */ /* end_addtogroup asmstrm_mediafmt_pcm_ch_defs */

/* Reserved for custom channel maps */
/** @addtogroup asmstrm_mediafmt_pcm_ch_maps
@{ */
/** @name
@{ */
#define PCM_CUSTOM_CHANNEL_MAP_1   48
#define PCM_CUSTOM_CHANNEL_MAP_2   49
#define PCM_CUSTOM_CHANNEL_MAP_3   50
#define PCM_CUSTOM_CHANNEL_MAP_4   51
#define PCM_CUSTOM_CHANNEL_MAP_5   52
#define PCM_CUSTOM_CHANNEL_MAP_6   53
#define PCM_CUSTOM_CHANNEL_MAP_7   54
#define PCM_CUSTOM_CHANNEL_MAP_8   55
#define PCM_CUSTOM_CHANNEL_MAP_9   56
#define PCM_CUSTOM_CHANNEL_MAP_10  57
#define PCM_CUSTOM_CHANNEL_MAP_11  58
#define PCM_CUSTOM_CHANNEL_MAP_12  59
#define PCM_CUSTOM_CHANNEL_MAP_13  60
#define PCM_CUSTOM_CHANNEL_MAP_14  61
#define PCM_CUSTOM_CHANNEL_MAP_15  62
#define PCM_CUSTOM_CHANNEL_MAP_16  63
/** @} *//* end_name */

/** Enumeration for the maximum number of channel maps. */
#define PCM_MAX_CHANNEL_MAP   63

/** @} *//* end_addtogroup asmstrm_mediafmt_pcm_ch_maps */

/** @ingroup asmstrm_mediafmt_mchan_pcm_v2
    Media format ID for multiple channel linear PCM.

    @par Multiple Channel PCM format block (asm_multi_channel_pcm_fmt_blk_v2_t}
    @tablens{weak__asm__multi__channel__pcm__fmt__blk__v2__t}

    @par Multiple Channel PCM encode configuration block (asm_multi_channel_pcm_enc_cfg_v2_t)
    @table{weak__asm__multi__channel__pcm__enc__cfg__v2__t}
*/
#define ASM_MEDIA_FMT_MULTI_CHANNEL_PCM_V2                0x00010DA5

#define ASM_MEDIA_FMT_ADPCM                     0x00010BE7

;


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_ADSP_MEDIA_FMT_H_*/
