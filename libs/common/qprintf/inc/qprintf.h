/**=============================================================================

@file
   qprintf.h

@brief
   API, macros and struct definitions for qprintf utilities available from C.
   
Copyright (c) 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

#ifndef qprintf_H
#define qprintf_H


#ifdef __cplusplus
extern "C"
{
#endif

#include "hexagon_protos.h"
#include "hexagon_types.h"
#include "stdlib.h"

#ifdef BUILDING_SO
/// MACRO enables function to be visible in shared-library case.
#define qprintf_API __attribute__ ((visibility ("default")))
#else
/// MACRO empty for non-shared-library case.
#define qprintf_API
#endif

#define QPRINTF_MASK_ALL -1ull
#define QPRINTF_MASK_NONE 0ull
#define QPRINTF_MASK_EVEN_8  0x5555555555555555ull
#define QPRINTF_MASK_ODD_8   0xaaaaaaaaaaaaaaaaull
#define QPRINTF_MASK_EVEN_16 0x3333333333333333ull
#define QPRINTF_MASK_ODD_16  0xccccccccccccccccull
#define QPRINTF_MASK_EVEN_32   0x0f0f0f0f0f0f0f0full
#define QPRINTF_MASK_ODD_32    0xf0f0f0f0f0f0f0f0ull


//---------------------------------------------------------------------------
/// @brief
///   Set the mask controling which bytes to display when printing out an HVX
///   register.
///
/// @detailed
///   If the nth bit of mask is set, the nth byte of HVX will be displayed.
///   When printing HVX as 16-bit or 32-bit elements, only the bit corresponding
///   to the lowest byte of the element controls whether the element will be
///   printed out or not.
///
/// @param high
///   Mask for upper 64 bytes of HVX vector.
///
/// @param low
///   Mask for lower 64 bytes of HVX vector.
///
/// @return
///   None.
//---------------------------------------------------------------------------
qprintf_API void qprintf_set_mask(unsigned long long high, unsigned long long low);

#if (__HEXAGON_ARCH__ >= 60)

//---------------------------------------------------------------------------
/// @brief
///   Print a V register. see documentation for details on supported format.
///
/// @param msg
///   Character string used to display V register.
///
/// @param V
///   HVX vector register to display.
///
/// @return
///   None.
//---------------------------------------------------------------------------
void qprintf_V(char* msg, HVX_Vector V);

//---------------------------------------------------------------------------
/// @brief
///   Print a Q register. see documentation for details on supported format.
///
/// @param msg
///   Character string used to display Q register.
///
/// @param Q
///   HVX predicate register to display.
///
/// @return
///   None.
//---------------------------------------------------------------------------
void qprintf_Q(char* msg, HVX_VectorPred Q);

//---------------------------------------------------------------------------
/// @brief
///   Display all HVX registers.
///
/// @detailed
///    TBD.
///
/// @return
///   None.
//---------------------------------------------------------------------------
extern qprintf_API void qprintf_V_all();

#endif

//---------------------------------------------------------------------------
/// @brief
///   Display all scalar registers.
///
/// @detailed
///    TBD.
///
/// @return
///   None.
//---------------------------------------------------------------------------
extern qprintf_API void qprintf_R_all();

#ifdef __cplusplus
}
#endif

#endif  // #ifndef qprintf_H
