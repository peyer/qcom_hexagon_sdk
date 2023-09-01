/**=============================================================================
Copyright (c) 2015 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#ifndef CONV3X3A32_ASM_H
#define CONV3X3A32_ASM_H

#ifdef __cplusplus
extern "C"
{
#endif

void conv3x3Per2Row(
    unsigned char *inp,
    int            stride_i,
    int            width,
    signed char   *mask,
    int            shift,
    unsigned char *outp,
    int            stride_o
    );


#ifdef __cplusplus
}
#endif

#endif    // CONV3X3A32_ASM_H
