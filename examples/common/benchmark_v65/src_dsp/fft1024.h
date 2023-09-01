/**=============================================================================
Copyright (c) 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

//---------------------------------------------------------------------------
/// @brief
///   Perform 1024x1024 FFT. 8-bit unsigned real input, 32-bit fixed-point
///     Q29.3 output. Each row presented as 1024 real values, followed by
///     1024 imaginary values.
///
/// @param srcImg
///   source. 1024 x 1024 8-bit real values.
///   \n\b NOTE: array assumed to be 128-byte aligned
///
/// @param srcImg
///   destination. 2048 x 1024 signed 32-bit values. First 1024 of each row
///     are real components of FFT output, followed by the 1024 imaginary components.
///     Numeric format is Q29.3, i.e. the last 3 bits are fractional. 
///   \n\b NOTE: array assumed to be 128-byte aligned
///
/// @return
///   0 for success, any other value indicates failure to complete FFT.

int fft1024_hvx(const uint8* imgSrc, int32_t* imgDst);

