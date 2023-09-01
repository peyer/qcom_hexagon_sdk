/**=============================================================================
@file
    qhdsp.h

@brief
    Header file of QHDSP library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#ifndef _QHDSP_H
#define _QHDSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "qhcomplex.h"
#include "qhdsp_common.h"
#include "qhmath.h"

/** @defgroup qhdsp_fft_routines QHDSP FFT routines
 *  @{
 */

/**
 * @brief           Real 2^N FFT for single precision float point
 * @param[in]       input - input samples in time domain (real)
 * @param[in]       nr_points - number of samples on which FFT is performed
 * @param[in]       twiddles1 - twiddle factors - for N/2-point complex FFT
 * @param[in]       twiddles2 - twiddle factors - for last stage
 * @param[out]      output - FFT output buffer
 * @note
 *                  - Assumptions:
 *                                  1. input buffer - aligned by the FFT size
 *                                  2. twiddles1 & twiddles2 - aligned by 8 bytes
 *                                  3. twiddles1 & twiddles2 - generated with qhdsp_fft_gen_twiddles_real_f() function
 *                                  4. output buffer - aligned by 8 bytes
 *                  - Constraints:
 *                                  1. nr_points has to be >= 8 (and power of 2)
 */
void qhdsp_r1dfft_f(const float *input, int32_t nr_points, const float complex *twiddles1, const float complex *twiddles2, float complex *output);

/**
 * @brief           Real 2^N IFFT for single precision float point
 * @param[in]       input - input samples in frequency domain
 * @param[in]       nr_points - number of samples on which IFFT is performed
 * @param[in]       Wt1 - twiddle factors - for N/2-point complex IFFT
 * @param[in]       Wt2 - twiddle factors - for last stage
 * @param[out]      output - IFFT output buffer
 * @note
 *                  - Assumptions:
 *                                  1. input buffer - aligned by 8 bytes
 *                                  2. Wt1 & Wt2 - aligned by 8 bytes
 *                                  3. Wt1 & Wt2 - generated with qhdsp_fft_gen_twiddles_real_f() function
 *                                  4. output buffer - aligned by 8 bytes
 *                  - Constraints:
 *                                  1. nr_points has to be >= 8 (and power of 2)
 */
void qhdsp_r1difft_f(const float complex* input, int32_t nr_points, const float complex* Wt1, const float complex* Wt2, float complex* output);

/**
 * @brief           Complex 2^N FFT for single precision float point
 * @param[in]       input - input samples in time domain (complex)
 * @param[in]       N - number of samples on which FFT is performed
 * @param[in]       w - twiddle factors
 * @param[out]      output - FFT output buffer
 * @note
 *                  - Assumptions:
 *                                  1. input buffer - aligned by the FFT size
 *                                  2. w - aligned by 8 bytes
 *                                  3. w - generated with qhdsp_fft_gen_twiddles_complex_f() function
 *                                  4. output buffer - aligned by 8 bytes
 *                  - Constraints:
 *                                  1. nr_points has to be >= 4 (and power of 2)
 */
void qhdsp_c1dfft_f(const float complex *input, int32_t N, const float complex *w, float complex *output);

/**
 * @brief           Complex 2^N IFFT for single precision float point
 * @param[in]       input - input samples in frequency domain
 * @param[in]       N - number of samples on which IFFT is performed
 * @param[in]       w - twiddle factors
 * @param[out]      output - IFFT output buffer
 * @note
 *                  - Assumptions:
 *                                  1. input buffer - aligned by the FFT size
 *                                  2. w - aligned by 8 bytes
 *                                  3. w - generated with qhdsp_fft_gen_twiddles_complex_f() function
 *                                  4. output buffer - aligned by 8 bytes
 *                  - Constraints:
 *                                  1. nr_points has to be >= 4 (and power of 2)
 */
void qhdsp_c1difft_f(const float complex* input, int32_t N, const float complex* w, float complex* output);

/**
 * @brief           Real 2^N FFT for 16x16 (complex number: bits 0:15-real part, bits 16:31-imag part) fixed-point
 * @param[in]       input - input samples in time domain (real)
 * @param[in]       N - number of samples on which FFT is performed
 * @param[in]       Wt1 - twiddle factors - for N/2-point complex FFT
 * @param[in]       Wt2 - twiddle factors - for last stage
 * @param[out]      output - FFT output buffer
 * @note
 *                  - input format Q15, output format Q<log2(N)>.<15-log2(N)>; example: N=16 -> log2(N) = 4 -> output format Q4.11
 *                  - Assumptions:
 *                                  1. input buffer - aligned by the FFT size
 *                                  2. Wt1 & Wt2 - aligned by 4 bytes
 *                                  3. Wt1 & Wt2 - generated with qhdsp_fft_gen_twiddles_complex_h() function
 *                                  4. output buffer - aligned by 8 bytes
 *                  - Constraints:
 *                                  1. N has to be >= 16 (and power of 2)
 */
void qhdsp_r1dfft_h(const int16_t* input, int32_t N, const int32_t* Wt1, const int32_t* Wt2, int32_t* output);

/**
 * @brief           Real 2^N IFFT for 16x16 (complex number: bits 0:15-real part, bits 16:31-imag part) fixed-point
 * @param[in]       input - input samples in frequency domain
 * @param[in]       N - number of samples on which FFT is performed
 * @param[in]       Wt1 - twiddle factors - for N/2-point complex FFT
 * @param[in]       Wt2 - twiddle factors - for last stage
 * @param[out]      output - IFFT output buffer
 * @note
 *                  - Scale factor [1/N] absent since scaling was done in FFT function
 *                  - input format Q<log2(N)>.<15-log2(N)>; example: N=16 -> log2(N) = 4 -> input format Q4.11, output format Q15
 *                  - Assumptions:
 *                                  1. input buffer - aligned by the IFFT size
 *                                  2. Wt1 & Wt2 - aligned by 4 bytes
 *                                  3. Wt1 & Wt2 - generated with qhdsp_fft_gen_twiddles_complex_h() function
 *                                  4. output buffer - aligned by 8 bytes
 *                  - Constraints:
 *                                  1. N has to be >= 16 (and power of 2)
 */
void qhdsp_r1difft_h(const int32_t *input, int32_t N, const int32_t *Wt1, const int32_t *Wt2, int16_t *output);

/**
 * @brief           Complex 2^N FFT for 16x16 (complex number: bits 0:15-real part, bits 16:31-imag part) fixed-point
 * @param[in]       input - input samples in time domain (complex)
 * @param[in]       N - number of samples on which FFT is performed
 * @param[in]       w - twiddle factors
 * @param[out]      output - FFT output buffer
 * @note
 *                  - Scale factor: 1/N
 *                  - input format Q15, output format Q<log2(N)>.<15-log2(N)>; example: N=16 -> log2(N) = 4 -> output format Q4.11
 *                  - Assumptions:
 *                                  1. input buffer - aligned by the FFT size
 *                                  2. w - aligned by 4 bytes
 *                                  3. w - generated with qhdsp_fft_gen_twiddles_complex_h() function
 *                                  4. output buffer - aligned by 8 bytes
 *                  - Constraints:
 *                                  1. nr_points has to be >= 8 (and power of 2)
 */
void qhdsp_c1dfft_h(const int32_t *input, int32_t N, const int32_t *w, int32_t *output);

/**
 * @brief           Complex 2^N IFFT for 16x16 (complex number: bits 0:15-real part, bits 16:31-imag part) fixed-point
 * @param[in]       input - input samples in frequency domain
 * @param[in]       N - number of samples on which IFFT is performed
 * @param[in]       w - twiddle factors
 * @param[out]      output - IFFT output buffer
 * @note
 *                  - Scale factor [1/N] absent since scaling was done in FFT function
 *                  - input format Q<log2(N)>.<15-log2(N)>; example: N=16 -> log2(N) = 4 -> input format Q4.11, output format Q15
 *                  - Assumptions:
 *                                  1. input buffer - aligned by the FFT size
 *                                  2. w - aligned by 4 bytes
 *                                  3. w - generated with qhdsp_fft_gen_twiddles_complex_h() function
 *                                  4. output buffer - aligned by 8 bytes
 *                  - Constraints:
 *                                  1. nr_points has to be >= 8 (and power of 2)
 */
void qhdsp_c1difft_h(const int32_t *input, int32_t N, const int32_t *w, int32_t *output);

/**
  * @} //qhdsp_fft_routines
  */

/** @defgroup qhdsp_fir_routines QHDSP FIR routines
 *  @{
 */

/**
 * @brief           FIR filtering on a bulk of data - 16b fixed point.
 * @param[in]       in_samples - input samples in time domain <Q15>
 * @param[in]       coefs - FIR filter coefficients <Q15>
 * @param[in]       taps - number of filter taps
 * @param[in]       length - length of input/output data (number of samples)
 * @param[out]      out_samples - output buffer <Q15>
 * @note
 *                  - Assumptions:
 *                                  1. in_samples, coefs and out_samples aligned by 8 bytes
 *                                  2. coefficients arranged in reversed order
 *                                  3. in_samples buffer has to be of (taps + length) size
 *                                  4. new incoming samples stored at [taps-1] offset in in_samples buffer
 */
int32_t qhdsp_fir_h(int16_t *in_samples, int16_t *coefs, uint32_t taps, uint32_t length, int16_t *out_samples);

/**
 * @brief           FIR filtering on a bulk of data - float point.
 * @param[in]       in_samples - input samples in time domain
 * @param[in]       delay_line - delay buffer (size is taps samples)
 * @param[in]       coefs - FIR filter coefficients
 * @param[in]       taps - number of filter taps
 * @param[in]       length - length of input/output data (number of samples)
 * @param[out]      out_samples - output buffer
 * @note
 *                  - Assumptions:
 *                                  1. out_samples aligned by 8 bytes
 */
int32_t qhdsp_fir_f(const float *in_samples, float *delay_line, const float *coefs, uint32_t taps, uint32_t length, float_a8_t *out_samples);

/**
  * @} //qhdsp_fir_routines
  */


/** @defgroup qhdsp_iir_routines QHDSP IIR routines
 *  @{
 */

/**
 * @brief           IIR (cascaded biquad) filtering on a bulk of data - float point.
 * @param[in]       in_samples - input samples in time domain
 * @param[in]       coefs - IIR filter coefficients - arranged as b0, b1, a1, b2, a2
 * @param[in/out]   states - pointer to filter states buffer
 * @param[in]       num_biquads - number of consecutive IIR filter calls (with different coeffs)
 * @param[in]       length - length of input/output data (number of samples)
 * @param[out]      out_samples - output buffer
 * @note
 *                  - Assumptions:
 *                                  1. states buffer should be aligned to 8 bytes
 */
int32_t qhdsp_iir_f(const float *in_samples, const float *coefs, float *states, uint32_t num_biquads, uint32_t length, float *out_samples);

/**
 * @brief           IIR (cascaded biquad) filtering on a bulk of data - 16-bit fixed point.
 * @param[in]       in_samples - input samples in time domain <Q15>
 * @param[in]       coefs - IIR filter coefficients - arranged as b0, b1, a1, b2, a2 <Q15> and shift_amt <int16>
 * @param[in/out]   states - pointer to filter states buffer <Q47>
 * @param[in]       num_biquads - number of consecutive IIR filter calls (with different coeffs) - valid range 1-5
 * @param[in]       length - length of input/output data (number of samples)
 * @param[out]      out_samples - output buffer <Q15>
 * @note
                    Cascade of biquads, i.e. transfer function of:

                               b0 + b1*z^-1 + b2*z^-2
                       H(z) = -----------------------
                               a0 + a1*z^-1 + a2*z^-2

                    for each biquad in the filter. This implementation uses 5 instead
                    of 6 coefficients (all coefficients divided with a0 coefficient).
                    Also, a1 and a2 coeffs are stored with - sign and divided by 2.0,
                    so transfer function looks like this:

                               (b0/a0) +        (b1/a0)*z^-1 +        (b2/a0)*z^-2
                       H(z) = -----------------------------------------------------
                                  1    + 2*(-a1/(2*a0))*z^-1 + 2*(-a2/(2*a0))*z^-2

                    If any of b0/a0, b1/a0 or b2/a0 coefficients are greater than 1.0 (abs value),
                    all three coefficients are divided with next larger power of 2.
                    Additionally, at the end of coefficients buffer, shift amount is
                    stored to compensate division of these three coefficients. If these
                    were not larger than 1.0, shift amount should be set to 0. Shift amount
                    is always >=0.
                    Transfer function if any of b0/a0, b1/a0 or b2/a0 coefficients are greater than
                    1.0 (abs value):

                                           (b0/a0)/2^shift + (b1/a0)/2^shift*z^-1 + (b2/a0)/2^shift*z^-2
                       H(z) = (2^shift) * ----------------------------------------------------------------
                                            1              + 2*(-a1/(2*a0))*z^-1  + 2*(-a2/(2*a0))*z^-2


                    Coefficients are stored in following order:
                        b0/a0 <Q15>, b1/a0 <Q15>, -a1/(2*a0) <Q15>
                        b2/a0 <Q15>, -a2/(2*a0) <Q15>, shift_amount <int16>

                    Example 1:

                        Lowpass filter @2kHz, sample rate 48kHz, Q=0.707

                        b0/a0 = 0.01440110
                        b1/a0 = 0.02880221
                        b2/a0 = 0.01440110
                        a1/a0 = -1.63295501
                        a2/a0 = 0.69055942

                        Coeffs:
                            0x01d7,    // b0/a0 = 0.01440110
                            0x03af,    // b1/a0 = 0.02880221
                            0x6882,    // -(a1/(2*a0)) = 0.816477505
                            0x01d7,    // b2/a0 = 0.01440110
                            0xd3ce,    // -(a2/(2*a0)) = -0.34527971
                            0x0000     // shift amount

                    Example 2:

                        Lowshelf filter @2kHz, sample rate 48kHz, Q=0.707

                        b0/a0 = 1.01062062
                        b1/a0 = -1.63980490
                        b2/a0 = 0.69052203
                        a1/a0 = -1.64314011
                        a2/a0 = 0.69780744

                        max(abs(b0/a0), abs(b1/a0), abs(b2/a0)) = 1.63980490
                        all three "b" coefficients are divided by div=2.0
                        shift amount is log2(div) = 1

                        Coeffs:
                            0x40ae,    // b0/(2*a0) = 0.50531031
                            0x970e,    // b1/(2*a0) = -0.81990245
                            0x6929,    // -(a1/2*a0) = 0.821570055
                            0x2c31,    // b2/(2*a0) = 0.345261015
                            0xd358,    // -(a2/2*a0) = -0.34890372
                            0x0001     // shift amount

 *
 */
int32_t qhdsp_iir_h(const int16_t *in_samples, const int16_t *coefs, int64_t *states, uint32_t num_biquads, uint32_t length, int16_t *out_samples);


/**
  * @} //qhdsp_iir_routines
  */

 /** @defgroup qhdsp_decim_routines QHDSP decimation routines
 *  @{
 */

/**
 * @brief           Decimation FIR - floating-point.
 * @param[in]       in_samples - input samples in time domain
 * @param[in]       delay_line - delay buffer (size is taps samples)
 * @param[in]       coefs - FIR filter coefficients
 * @param[in]       taps - number of filter taps
 * @param[in]       length - length of input data (number of samples); maximum supported is 512
 * @param[in]       decimator - downscalling factor; minimum 1
 * @param[out]      out_samples - output buffer; with length (input length / decimator)
 * @note
 *                  - Assumptions:
 *                      1. coefficients arranged in reversed order
 *                      2. in_samples buffer has to be of (taps + length) size
 *                      3. new incoming samples stored at [taps-1] offset in in_samples buffer
 */
int32_t qhdsp_firdecim_f(const float *in_samples, float *delay_line, const float *coefs, uint32_t taps,
                       uint32_t length, uint32_t decimator, float *out_samples);

/**
 * @brief           Decimation FIR - 16-bit fixed-point.
 * @param[in]       in_samples - input samples in time domain <Q15>
 * @param[in]       coefs - FIR filter coefficients <Q15>
 * @param[in]       taps - number of filter taps
 * @param[in]       length - length of input data (number of samples); maximum supported is 1024
 * @param[in]       decimator - downscalling factor, minimum 1
 * @param[out]      out_samples - output buffer <Q15>; with length (input length / decimator)
 * @note
 *                  - Assumptions:
 *                                  1. in_samples, coefs and out_samples aligned by 8 bytes
 *                                  2. coefficients arranged in reversed order
 *                                  3. in_samples buffer has to be of (taps + length) size
 *                                  4. new incoming samples stored at [taps-1] offset in in_samples buffer
 */
int32_t qhdsp_firdecim_h(int16_t *in_samples, int16_t *coefs, uint32_t taps,
                       uint32_t length, uint32_t decimator, int16_t *out_samples);

/**
 * @brief           Decimation IIR - floating-point.
 * @param[in]       in_samples - input samples in time domain
 * @param[in]       coefs - IIR filter coefficients - arranged as b0, b1, a1, b2, a2
 * @param[in/out]   states - pointer to filter states buffer
 * @param[in]       num_biquads - number of consecutive IIR filter calls (with different coeffs)
 * @param[in]       length - length of input data (number of samples); maximum supported is 512
 * @param[in]       decimator - downscalling factor, minimum 1
 * @param[out]      out_samples - output buffer; with length (input length / decimator)
 * @note
 *                  - Assumptions:
 *                                  1. states buffer should be aligned to 8 bytes
 */
int32_t qhdsp_iirdecim_f(float *in_samples, float *coefs, float *states, uint32_t num_biquads,
                       uint32_t length, uint32_t decimator, float *out_samples);

/**
 * @brief           Decimation IIR - 16-bit fixed-point.
 * @param[in]       in_samples - input samples in time domain <Q15>
 * @param[in]       coefs - IIR filter coefficients - arranged as b0, b1, a1, b2, a2 <Q15> and shift_amt <int16>
 * @param[in/out]   states - pointer to filter states buffer <Q47>
 * @param[in]       num_biquads - number of consecutive IIR filter calls (with different coeffs) - valid range 1-5
 * @param[in]       length - length of input data (number of samples); maximum supported is 1024
 * @param[in]       decimator - downscalling factor, minimum 1
 * @param[out]      out_samples - output buffer <Q15>; with length (input length / decimator)
 *
 */
int32_t qhdsp_iirdecim_h(int16_t *in_samples, int16_t *coefs, int64_t *states, uint32_t num_biquads,
                       uint32_t length, uint32_t decimator, int16_t *out_samples);

/**
  * @} //qhdsp_decim_routines
  */

 /** @defgroup qhdsp_interp_routines QHDSP interpolation routines
 *  @{
 */

/**
 * @brief           Interpolation IIR - floating-point.
 * @param[in]       in_samples - input samples in time domain
 * @param[in]       coefs - IIR filter coefficients - arranged as b0, b1, a1, b2, a2
 * @param[in/out]   states - pointer to filter states buffer
 * @param[in]       num_biquads - number of consecutive IIR filter calls (with different coeffs)
 * @param[in]       length - length of input data (number of samples); maximum supported is (512 / r)
 * @param[in]       r - upscalling factor, minimum 1
 * @param[out]      out_samples - output buffer; with length (input length * r)
 * @note
 *                  - Assumptions:
 *                                  1. states buffer should be aligned to 8 bytes
 */
int32_t qhdsp_iirinterp_f(float *in_samples, float *coefs, float *states, uint32_t num_biquads,
                       uint32_t length, uint32_t r, float *out_samples);

/**
 * @brief           Interpolation IIR - 16-bit fixed-point.
 * @param[in]       in_samples - input samples in time domain <Q15>
 * @param[in]       coefs - IIR filter coefficients - arranged as b0, b1, a1, b2, a2 <Q15> and shift_amt <int16>
 * @param[in/out]   states - pointer to filter states buffer <Q47>
 * @param[in]       num_biquads - number of consecutive IIR filter calls (with different coeffs) - valid range 1-5
 * @param[in]       length - length of input data (number of samples); maximum supported is (1024 / r)
 * @param[in]       r - upscalling factor, minimum 1
 * @param[out]      out_samples - output buffer <Q15>; with length (input length * r)
 *
 */
int32_t qhdsp_iirinterp_h(int16_t *in_samples, int16_t *coefs, int64_t *states, uint32_t num_biquads,
                       uint32_t length, uint32_t r, int16_t *out_samples);

/**
  * @} //qhdsp_interp_routines
  */

#ifdef __cplusplus
}
#endif

#endif /* _QHDSP_H */
