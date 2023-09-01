/**=============================================================================
Copyright (c) 2014-2015 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#ifndef DILATE3x3_ASM_H
#define DILATE3x3_ASM_H

#ifdef __cplusplus
extern "C"
{
#endif

void dilate3x3Per2Row(
    unsigned char   *src,
    int              stride_i,
    int              width,
    unsigned char   *dst,
    int              stride_o
    );

#ifdef __cplusplus
}
#endif

#endif    // DILATE3x3_ASM_H
