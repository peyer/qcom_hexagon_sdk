#ifndef GAUSSIAN7X7_ASM_H
#define GAUSSIAN7X7_ASM_H

#ifdef __cplusplus
extern "C"
{
#endif

//---------------------------------------------------------------------------
/// @brief
///   Compute 1 row of gaussian 7x7.
///
/// @detailed
///    TBD.
///
/// @param psrc
///   list of 7 pointers to input rows
///
/// @param width
///   Image width.
///   \n\b NOTE: must be multiple of VLEN
///
/// @param VLEN
///   HVX vector length. 
///   \n\b NOTE: Must accurately match the current HVX mode configured in HW.
//---------------------------------------------------------------------------

void Gaussian7x7u8PerRow(
    unsigned char **psrc,
    int            width,
    unsigned char *dst,
    int VLEN
    );


#ifdef __cplusplus
}
#endif

#endif    // GAUSSIAN7X7_ASM_H
