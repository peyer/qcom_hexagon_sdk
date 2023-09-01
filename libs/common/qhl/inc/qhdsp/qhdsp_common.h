/**=============================================================================
@file
    qhdsp_common.h

@brief
    Header file for common routines of QHDSP library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#ifndef _QHDSP_COMMON_H
#define _QHDSP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup qhdsp_fft_routines QHDSP FFT routines
 *  @{
 */

/**
 * @brief           Generate twiddle factors for [float] complex FFT/IFFT functions.
 * @param[in,out]   twiddle - output buffer for storing generated twiddle factors
 * @param[in]       NP - number of samples on which FFT/IFFT is performed
 * @param[in]       log2NP - logarithm of power 2 of number of samples
 * @return
 * @note
 *                  - Generated twiddle factors are used for both FFT and IFFT.
 *                  - Assumptions:
 *                                  1. twiddle - buffer aligned by 8bytes
 *                                  2. twiddle buffer size is 3*NP/4 * [8bytes]
 */
void qhdsp_fft_gen_twiddles_complex_f(float complex* twiddle, int32_t NP, int32_t log2NP);

/**
 * @brief           Generate twiddle factors for [float] real FFT/IFFT functions.
 * @param[in,out]   twiddle1 - output buffer for storing generated twiddle factors - for N/2-point complex FFT
 * @param[in,out]   twiddle2 - output buffer for storing generated twiddle factors - for last stage
 * @param[in]       np - number of samples on which FFT/IFFT is performed
 * @param[in]       log2np - logarithm of power 2 of number of samples
 * @note
 *                  - Generated twiddle factors are used for both FFT and IFFT.
 *                  - Assumptions:
 *                                  1. twiddle1 & twiddle2 - buffers aligned by 8bytes
 *                                  2. twiddle1 buffer size is 3*np/8 * [8bytes]
 *                                  3. twiddle2 buffer size is np/2 * [8bytes]
 */
void qhdsp_fft_gen_twiddles_real_f(float complex *twiddle1, float complex *twiddle2, int32_t np, int32_t log2np);

/**
 * @brief           Generate twiddle factors for [fixed-point] complex FFT/IFFT functions.
 * @param[in,out]   twiddle - output buffer for storing generated twiddle factors
 * @param[in]       NP - number of samples on which FFT/IFFT is performed
 * @param[in]       log2NP - logarithm of power 2 of number of samples
 * @return
 * @note            
 *                  - Generated twiddle factors are used for both FFT and IFFT.
 *                  - twiddle factors format: Q15 
 *                  - Assumptions:
 *                                  1. twiddle - buffer aligned by 4bytes
 *                                  2. twiddle buffer size is 3*NP/4 * [4bytes]
 */
void qhdsp_fft_gen_twiddles_complex_h(int32_t* twiddle, int32_t NP, int32_t log2NP);

/**
 * @brief           Generate twiddle factors for [fixed-point] real FFT/IFFT functions.
 * @param[in,out]   twiddle1 - output buffer for storing generated twiddle factors - for N/2-point complex FFT
 * @param[in,out]   twiddle2 - output buffer for storing generated twiddle factors - for last stage
 * @param[in]       NP - number of samples on which FFT/IFFT is performed
 * @param[in]       log2NP - logarithm of power 2 of number of samples
 * @note            
 *                  - Generated twiddle factors are used for both FFT and IFFT.
 *                  - twiddle factors format: Q15 
 *                  - Assumptions:
 *                                  1. twiddle1 & twiddle2 - buffers aligned by 4bytes
 *                                  2. twiddle1 buffer size is 3*NP/8 * [4bytes]
 *                                  3. twiddle2 buffer size is NP/2 * [4bytes]
 */
void qhdsp_fft_gen_twiddles_real_h(int32_t* twiddle1, int32_t* twiddle2, int32_t NP, int32_t log2NP);

/**
  * @} //qhdsp_fft_routines
  */

#ifdef __cplusplus
}
#endif

#endif /* _QHDSP_COMMON_H */
