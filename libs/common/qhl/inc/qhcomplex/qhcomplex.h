/**=============================================================================
@file
    qhcomplex.h

@brief
    Header file of QHCOMPLEX library.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#ifndef _QHCOMPLEX_H
#define _QHCOMPLEX_H

#ifdef __cplusplus
extern "C" {
#endif

#define complex _Complex /**< complex type */
#define _Complex_I (0.0f+1.0fi) /**< the complex unit i */
#define I _Complex_I /**< the complex or imaginary unit i */

/** @defgroup qhcomplex_functions QHCOMPLEX functions
 *  @{
 */

/** @defgroup qhcomplex_manipulation_functions Manipulation functions
 *  @{
 */

/**
 * @brief       Returns the real part of z
 * @param[in]   z   Complex number
 * @return      The real part of z
 */
float qhcomplex_creal_f(float complex z);

/**
 * @brief       Returns the imaginary part of z
 * @param[in]   z   Complex number
 * @return      The imaginary part of z
 */
float qhcomplex_cimag_f(float complex z);

/**
 * @brief       Computes the complex absolute value of z
 * @param[in]   z   Complex number
 * @return      The absolute value of z
 */
float qhcomplex_cabs_f(float complex z);

/**
 * @brief       Computes the argument of z
 * @param[in]   z   Complex number for which argument is calculated
 * @return      Argument (phase angle) of z
 */
float qhcomplex_carg_f(float complex z);

/**
 * @brief       Computes the complex conjugate of z
 * @param[in]   z   Complex number
 * @return      The conjugate of z
 */
float complex qhcomplex_conj_f(float complex z);

/**
 * @brief       Computes projection of z on the Riemann sphere
 * @param[in]   z Complex number
 * @return      The projection of z on the Riemann sphere
 */
float complex qhcomplex_cproj_f(float complex z);

/**
 * @} //qhcomplex_manipulation_functions
 */

/** @defgroup qhcomplex_exponential_functions Exponential functions
 *  @{
 */

/**
 * @brief       Computes the complex base-e exponential of z
 * @param[in]   z   Complex number
 * @return      The exponent power of input z
 */
float complex qhcomplex_cexp_f(float complex z);

/**
 * @brief       Computes the complex natural (base-e) logarithm of z
 * @param[in]   z   Complex number
 * @return      The logarithm of input z
 */
float complex qhcomplex_clog_f(float complex z);

/**
 * @} //qhcomplex_exponential_functions
 */

/** @defgroup qhcomplex_power_functions Power functions
 *  @{
 */

/**
 * @brief       Computes the complex power function z^(c)
 * @param[in]   z   Complex number
 * @param[in]   c   Complex number
 * @return      The complex input z to the power of complex input c
 */
float complex qhcomplex_cpow_f(float complex z, float complex c);

/**
 * @brief       Computes the complex square root of z
 * @param[in]   z   Complex number
 * @return      The complex sqrt root of z
 */
float complex qhcomplex_csqrt_f(float complex z);

/**
 * @} //qhcomplex_power_functions
 */

/** @defgroup qhcomplex_trigonometric_functions Trigonometric functions
 *  @{
 */

/**
 * @brief       Computes the complex sine of z
 * @param[in]   z   Complex number
 * @return      The sine of input z
 */
float complex qhcomplex_csin_f(float complex z);

/**
 * @brief       Computes the complex cosine of z
 * @param[in]   z   Complex number
 * @return      The cosine of input z
 */
float complex qhcomplex_ccos_f(float complex z);

/**
 * @brief       Computes the complex tangent of z
 * @param[in]   z   Complex number
 * @return      The complex tangent of z
 */
float complex qhcomplex_ctan_f(float complex z);

/**
 * @brief       Computes the complex arcsin of z
 * @param[in]   z   Complex number
 * @return      The arcsin of input z
 */
float complex qhcomplex_casin_f(float complex z);

/**
 * @brief       Computes the complex arccos of z
 * @param[in]   z   Complex number
 * @return      The arccos of input z
 */
float complex qhcomplex_cacos_f(float complex z);

/**
 * @brief       Computes the complex arctan of z
 * @param[in]   z   Complex number
 * @return      The complex arctan of z
 */
float complex qhcomplex_catan_f(float complex z);

/**
 * @} //qhcomplex_trigonometric_functions
 */

/** @defgroup qhcomplex_hyperbolic_functions Hyperbolic functions
 *  @{
 */

/**
 * @brief       Computes the complex hyperbolic sine of z
 * @param[in]   x   Floating point complex value representing argument.
 * @return      Complex sine hyperbolic of x.
 */
float complex qhcomplex_csinh_f(float complex x);

/**
 * @brief       Computes the complex hyperbolic cosine of z
 * @param[in]   x   Floating point complex value representing argument.
 * @return      Complex cosine hyperbolic of x.
 */
float complex qhcomplex_ccosh_f(float complex x);

/**
 * @brief       Computes the complex hyperbolic tangent of z
 * @param[in]   z   Complex number
 * @return      The complex hyperbolic tangent of z
 */
float complex qhcomplex_ctanh_f(float complex z);

/**
 * @brief       Computes the complex arcsin hyperbolic of z
 * @param[in]   z   Complex number
 * @return      The arcsin hyperbolic of input z
 */
float complex qhcomplex_casinh_f(float complex z);

/**
 * @brief       Computes the complex arccos hyperbolic of z
 * @param[in]   z   Complex number
 * @return      The arccos hyperbolic of input z
 */
float complex qhcomplex_cacosh_f(float complex z);

/**
 * @brief       Computes the complex arctan hyperbolic of z
 * @param[in]   z   Complex number
 * @return      The complex arctan hyperbolic of z
 */
float complex qhcomplex_catanh_f(float complex z);

/**
 * @} //qhcomplex_hyperbolic_functions
 */

/**
 * @} //qhcomplex_functions
 */

#ifdef __cplusplus
}
#endif

#endif /* _QHCOMPLEX_H */
