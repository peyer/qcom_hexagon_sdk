/**=============================================================================
Copyright (c) 2014-2015 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#ifndef SIGMA3x3_ASM_H
#define SIGMA3x3_ASM_H

#ifdef __cplusplus
extern "C"
{
#endif

void sigma3x3PerRow(
    unsigned char   *src,
    int              stride_i,
    int              width,
    unsigned char    threshold,
    unsigned char   *dst
    );

#ifdef __cplusplus
}
#endif

#endif    // SIGMA3x3_ASM_H
