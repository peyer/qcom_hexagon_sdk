/**=============================================================================
Copyright (c) 2014-2016 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#ifndef DILATE5x5_ASM_H
#define DILATE5x5_ASM_H

#ifdef __cplusplus
extern "C"
{
#endif

void dilate5x5Per2Row(
    unsigned char   *src,
    int              stride_i,
    int              width,
    unsigned char   *dst,
    int              stride_o
    );

#ifdef __cplusplus
}
#endif

#endif    // DILATE5x5_ASM_H
