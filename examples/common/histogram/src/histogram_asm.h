/**=============================================================================
Copyright (c) 2015 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
#ifndef HISTOGRAM_ASM_H
#define HISTOGRAM_ASM_H

#ifdef __cplusplus
extern "C"
{
#endif

void histogramPernRow(
    unsigned char   *src,
    int              stride,
    int              width,
    int              height,
    int             *hist
    );

void clearHistogram(
    int		*hist
    );

#ifdef __cplusplus
}
#endif

#endif    // HISTOGRAM_ASM_H
