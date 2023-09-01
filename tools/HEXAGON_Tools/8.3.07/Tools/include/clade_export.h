#ifndef CLADE_EXPORT_H
#define CLADE_EXPORT_H
/*==========================================================================
 * FILE:         clade_export.h
 *
 * DESCRIPTION:  Shared library export values
 *
 * Copyright (c) 2016 Qualcomm Technologies Incorporated.
 * All Rights Reserved. QUALCOMM Proprietary and Confidential.
 ===========================================================================*/

#ifdef _MSC_VER
#define DDRCLADE_API_EXPORT __declspec(dllexport)
#else
#define DDRCLADE_API_EXPORT
#endif

#endif
