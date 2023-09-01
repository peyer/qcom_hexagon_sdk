/**=============================================================================

 @file
 qmath.inl

 @brief
 Inline versions of math functions. These may perform better than their binary
 equivalents, due to avoiding function call overhead, but should be used carefully. 
 If too many inline functions using HVX are called from the same function, register
 spills may start to degrade performance, and could even lead to stack overflow in
 extreme cases.

 Copyright (c) 2017 Qualcomm Technologies Incorporated.
 All Rights Reserved Qualcomm Proprietary

 Export of this technology or software is regulated by the U.S.
 Government. Diversion contrary to U.S. law prohibited.

 All ideas, data and information contained in or disclosed by
 this document are confidential and proprietary information of
 Qualcomm Technologies Incorporated and all rights therein are expressly reserved.
 By accepting this material the recipient agrees that this material
 and the information contained therein are held in confidence and in
 trust and will not be used, copied, reproduced in whole or in part,
 nor its contents revealed in any manner to others without the express
 written permission of Qualcomm Technologies Incorporated.

 =============================================================================**/

//==============================================================================
// Include Files
//==============================================================================

/*===========================================================================
 DEFINITIONS
 ===========================================================================*/

/*===========================================================================
 TYPEDEF
 ===========================================================================*/

typedef long HEXAGON_Vect_UN __attribute__((__vector_size__(QMATH_VLEN)))__attribute__((aligned(4)));
#define VMEMU(A) *((HEXAGON_Vect_UN*)(A))

//==============================================================================
// CONSTANTS
//==============================================================================

static const HEXAGON_Vect64 MAXINT_NEGMAXINT = 0x7FFFFFFF80000000ULL;
static const HEXAGON_Vect64 ONES_THIRTYONES_32 = 0x000000010000001FULL;
static const HEXAGON_Vect64 MAXEXP_NEGMAXEXP_32 = 0x20000000e0000000ULL;
static const HEXAGON_Vect64 IMPLIED1_MANTMASK_32 = 0x00800000007FFFFFULL;

//==============================================================================
// INLINE FUNCTIONS
//==============================================================================
static inline qm_vqf32_t qm_vqf32_from_int_inl(int* src)
{
    HVX_Vector const0 = Q6_V_vzero();
    HVX_Vector const31 = Q6_V_vsplat_R(31);
    HVX_Vector constNEGMAXEXP = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(MAXEXP_NEGMAXEXP_32));
    HVX_Vector mant = VMEMU(src);
    HVX_VectorPred Q0 = Q6_Q_vcmp_eq_VwVw(const0, mant);
    HVX_Vector exp = Q6_Vw_vnormamt_Vw(mant);
    mant = Q6_Vw_vasl_VwVw(mant, exp);
    exp = Q6_Vw_vsub_VwVw(const31, exp);
    mant = Q6_V_vmux_QVV(Q0, const0, mant);
    exp = Q6_V_vmux_QVV(Q0, constNEGMAXEXP, exp);
    return (Q6_W_vcombine_VV(exp, mant));
}

static inline qm_vqf32_t qm_vqf32_from_float_inl(float* src)
{
    HVX_Vector raw = VMEMU(src);
    HVX_Vector constIMPLIED1 = Q6_V_vsplat_R(0x00800000);
    HVX_Vector constMANTMASK = Q6_V_vsplat_R(0x7FFFFF);
    HVX_Vector tempExp = Q6_Vuw_vlsr_VuwR(raw, 23);
    HVX_Vector tempMant = Q6_V_vand_VV(raw, constMANTMASK);

    HVX_VectorPair tempPair = Q6_Wuh_vzxt_Vub(tempExp);
    tempExp = Q6_V_lo_W(tempPair);
    tempMant = Q6_V_vor_VV(tempMant, constIMPLIED1);

    HVX_Vector constNEG126 = Q6_V_vsplat_R(-126);
    HVX_Vector const255 = Q6_V_vsplat_R(255);
    HVX_Vector constMAXINT = Q6_V_vsplat_R(HEXAGON_V64_GET_W1(MAXINT_NEGMAXINT));
    HVX_Vector const0 = Q6_V_vzero();

    HVX_VectorPred Q0 = Q6_Q_vcmp_gt_VwVw(const0, raw);
    HVX_VectorPred Q1 = Q6_Q_vcmp_eq_VwVw(tempExp, const255);
    HVX_VectorPred Q2 = Q6_Q_vcmp_eq_VwVw(tempExp, const0);

    tempExp = Q6_Vw_vadd_VwVw(tempExp, constNEG126);
    tempMant = Q6_Vw_vasl_VwR(tempMant, 7);
    tempMant = Q6_V_vmux_QVV(Q1, constMAXINT, tempMant);

    HVX_Vector constNEGMAXEXP = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(MAXEXP_NEGMAXEXP_32));
    HVX_Vector constMAXEXP = Q6_V_vsplat_R(HEXAGON_V64_GET_W1(MAXEXP_NEGMAXEXP_32));
    tempExp = Q6_V_vmux_QVV(Q1, constMAXEXP, tempExp);
    tempExp = Q6_V_vmux_QVV(Q2, constNEGMAXEXP, tempExp);

    HVX_Vector negMant = Q6_Vw_vsub_VwVw(const0, tempMant);
    tempMant = Q6_V_vmux_QVV(Q0, negMant, tempMant);
    tempMant = Q6_V_vmux_QVV(Q2, const0, tempMant);

    return (Q6_W_vcombine_VV(tempExp, tempMant));
}

#define EVEN_ROUNDING
static inline void qm_vqf32_to_float_inl(qm_vqf32_t src, float* dst)
{
    HVX_Vector mant = Q6_V_lo_W(src);
    HVX_Vector exp = Q6_V_hi_W(src);

    HVX_Vector const0 = Q6_V_vzero();
    HVX_Vector const1 = Q6_V_vsplat_R(1);
    HVX_Vector constRound = Q6_V_vsplat_R(64);
    HVX_VectorPred Q3 = Q6_Q_vcmp_gt_VwVw(const0, mant);
#ifdef EVEN_ROUNDING
    HVX_VectorPred odd = Q6_Q_vand_VR(mant, 0x80);
    constRound = Q6_Vw_condnac_QnVwVw(odd, constRound, const1);
#endif
    mant = Q6_Vw_vadd_VwVw_sat(mant, constRound);   // rounding constant

    HVX_Vector const126 = Q6_V_vsplat_R(126);
    HVX_Vector const255 = Q6_V_vsplat_R(255);
    exp = Q6_Vw_vadd_VwVw_sat(exp, const126);
    HVX_VectorPred Q0 = Q6_Q_vcmp_gt_VwVw(const255, exp);
    Q0 = Q6_Q_vcmp_gtand_QVwVw(Q0, exp, const0);
    exp = Q6_Vw_vmin_VwVw(exp, const255);
    exp = Q6_Vw_vmax_VwVw(exp, const0);

    HVX_Vector constMANTMASK = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(IMPLIED1_MANTMASK_32));
    mant = Q6_Vw_vasr_VwR(mant, 7);
    HVX_VectorPred Q1 = Q6_Q_vcmp_eq_VwVw(mant,Q6_V_vsplat_R(0xFF000000));
    mant = Q6_Vw_vabs_Vw_sat(mant);
    mant = Q6_V_vand_VV(mant, constMANTMASK);
    exp = Q6_Vw_condacc_QVwVw(Q1,exp,const1);

    HVX_Vector sign = Q6_V_vand_QR(Q3, 0x80000000);
    exp = Q6_Vw_vasl_VwR(exp, 23);
    exp = Q6_V_vor_VV(sign, exp);
    exp = Q6_Vw_condacc_QVwVw(Q0, exp, mant);

    VMEMU(dst) = exp;

    return;
}

static inline qm_vqf32_t qm_vqf32_from_double_inl(double* src)
{
    HVX_Vector srcLo = VMEMU(src);
    HVX_Vector srcHi = VMEMU(src + QMATH_VLEN/sizeof(src[0]));
    HVX_VectorPair tmp = Q6_W_vdeal_VVR(srcHi, srcLo, -4);
    srcLo = Q6_V_lo_W(tmp);
    srcHi = Q6_V_hi_W(tmp);

    HVX_Vector const0 = Q6_V_vzero();
    HVX_Vector constIMPLIED1 = Q6_V_vsplat_R(0x40000000);
    HVX_Vector constMANTMASK = Q6_V_vsplat_R(0x000FFFFF);
    HVX_VectorPred Q0 = Q6_Q_vcmp_gt_VwVw(const0, srcHi);
    HVX_Vector tempMant = Q6_V_vand_VV(srcHi, constMANTMASK);

    HVX_Vector constExpMask = Q6_V_vsplat_R(0x7FF00000);
    HVX_Vector tempExp = Q6_V_vand_VV(srcHi, constExpMask);
    HVX_Vector mantLo = Q6_Vuw_vlsr_VuwR(srcLo, 22);
    HVX_Vector const2047 = Q6_V_vsplat_R(2047);
    HVX_Vector constNEG1022 = Q6_V_vsplat_R(-1022);
    tempMant = Q6_Vw_vasl_VwR(tempMant, 10);
    tempMant = Q6_V_vor_VV(tempMant, Q6_V_vor_VV(constIMPLIED1, mantLo));

    HVX_Vector constNEGMAXEXP = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(MAXEXP_NEGMAXEXP_32));
    HVX_Vector constMAXEXP = Q6_V_vsplat_R(HEXAGON_V64_GET_W1(MAXEXP_NEGMAXEXP_32));
    tempExp = Q6_Vw_vasr_VwR(tempExp, 20);
    HVX_Vector constMAXINT = Q6_V_vsplat_R(HEXAGON_V64_GET_W1(MAXINT_NEGMAXINT));

    HVX_VectorPred Q1 = Q6_Q_vcmp_eq_VwVw(tempExp, const2047);
    HVX_VectorPred Q2 = Q6_Q_vcmp_eq_VwVw(tempExp, const0);
    tempExp = Q6_Vw_vadd_VwVw(tempExp, constNEG1022);
    tempMant = Q6_V_vmux_QVV(Q1, constMAXINT, tempMant);
    HVX_Vector negMant = Q6_Vw_vsub_VwVw(const0, tempMant);
    tempExp = Q6_V_vmux_QVV(Q1, constMAXEXP, tempExp);
    tempMant = Q6_V_vmux_QVV(Q0, negMant, tempMant);
    tempExp = Q6_V_vmux_QVV(Q2, constNEGMAXEXP, tempExp);
    tempMant = Q6_V_vmux_QVV(Q2, const0, tempMant);

    return (Q6_W_vcombine_VV(tempExp, tempMant));
}

static inline void qm_vqf32_to_double_inl(qm_vqf32_t src, double* dst)
{
    HVX_Vector mant = Q6_V_lo_W(src);
    HVX_Vector exp = Q6_V_hi_W(src);

    HVX_Vector const0 = Q6_V_vzero();
    HVX_VectorPred Q3 = Q6_Q_vcmp_gt_VwVw(const0, mant);
    HVX_Vector absmant = Q6_Vw_vabs_Vw(mant);
    HVX_VectorPred Q0 = Q6_Q_vcmp_eq_VwVw(mant, Q6_V_vsplat_R(0x80000000)); // special handling for saturation case
    exp = Q6_Vw_condacc_QVwVw(Q0,exp,Q6_V_vsplat_R(1));

    HVX_Vector const1022 = Q6_V_vsplat_R(1022);
    HVX_Vector const2047 = Q6_V_vsplat_R(2047);
    exp = Q6_Vw_vadd_VwVw_sat(exp, const1022);
    Q0 = Q6_Q_vcmp_gt_VwVw(const2047, exp);
    Q0 = Q6_Q_vcmp_gtand_QVwVw(Q0, exp, const0);
    exp = Q6_Vw_vmin_VwVw(exp, const2047);
    exp = Q6_Vw_vmax_VwVw(exp, const0);

    HVX_Vector mantMask = Q6_V_vsplat_R(0x000FFFFF);
    HVX_Vector upperMant = Q6_Vuw_vlsr_VuwR(absmant, 10);
    upperMant = Q6_V_vand_VV(upperMant, mantMask);
    HVX_Vector lowerMant = Q6_Vw_vasl_VwR(absmant, 22);

    HVX_Vector sign = Q6_V_vand_QR(Q3, 0x80000000);
    exp = Q6_Vw_vasl_VwR(exp, 20);
    lowerMant = Q6_V_vmux_QVV(Q0, lowerMant, const0);
    sign = Q6_Vw_condacc_QVwVw(Q0, sign, upperMant);

    exp = Q6_V_vor_VV(sign, exp);

    HVX_VectorPair result = Q6_W_vshuff_VVR(exp, lowerMant, -4);

    VMEMU(dst) = Q6_V_lo_W(result);
    VMEMU(dst+(QMATH_VLEN/sizeof(dst[0]))) = Q6_V_hi_W(result);

    return;
}

static inline qm_vqf32_t qm_vqf32_mpy_inl(qm_vqf32_t a, qm_vqf32_t b)
{
    HVX_Vector c_mant, c_exp;
    HVX_Vector a_mant = Q6_V_lo_W(a);
    HVX_Vector a_exp = Q6_V_hi_W(a);
    HVX_Vector b_mant = Q6_V_lo_W(b);
    HVX_Vector b_exp = Q6_V_hi_W(b);

    HVX_Vector const0 = Q6_V_vzero();
    HVX_Vector constNEGMAXEXP = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(MAXEXP_NEGMAXEXP_32));
    HVX_Vector constMAXEXP = Q6_V_vsplat_R(HEXAGON_V64_GET_W1(MAXEXP_NEGMAXEXP_32));
    c_mant = Q6_Vw_vmpye_VwVuh(a_mant, b_mant);
    c_mant = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(c_mant, a_mant, b_mant);
    HVX_VectorPred Q0 = Q6_Q_vcmp_eq_VwVw(c_mant, const0);

    a_exp = Q6_Vw_vadd_VwVw_sat(a_exp, b_exp);
    HVX_Vector k = Q6_Vw_vnormamt_Vw(c_mant);
    c_exp = Q6_Vw_vsub_VwVw(a_exp, k);
    c_exp = Q6_V_vmux_QVV(Q0, constNEGMAXEXP, c_exp);
    c_exp = Q6_Vw_vmin_VwVw(c_exp, constMAXEXP);
    c_mant = Q6_Vw_vasl_VwVw(c_mant, k);
    return (Q6_W_vcombine_VV(c_exp, c_mant));
}

static inline qm_vqf32_t qm_vqf32_add_inl(qm_vqf32_t a, qm_vqf32_t b)
{
    HVX_Vector a_mant = Q6_V_lo_W(a);
    HVX_Vector a_exp = Q6_V_hi_W(a);
    HVX_Vector b_mant = Q6_V_lo_W(b);
    HVX_Vector b_exp = Q6_V_hi_W(b);
    HVX_Vector const31 = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(ONES_THIRTYONES_32));
    HVX_Vector const1 = Q6_V_vsplat_R(HEXAGON_V64_GET_W1(ONES_THIRTYONES_32));
    HVX_Vector constNEGMAXEXP = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(MAXEXP_NEGMAXEXP_32));

    HVX_VectorPred Q0 = Q6_Q_vcmp_gt_VwVw(b_exp, a_exp);
    HVX_Vector const0 = Q6_V_vzero();
    HVX_VectorPair sortedExp = Q6_W_vswap_QVV(Q0, b_exp, a_exp);
    HVX_VectorPair sortedMant = Q6_W_vswap_QVV(Q0, b_mant, a_mant);
    a_mant = Q6_V_lo_W(sortedMant);
    a_exp = Q6_V_lo_W(sortedExp);
    b_mant = Q6_V_hi_W(sortedMant);
    b_exp = Q6_V_hi_W(sortedExp);

    HVX_Vector expDelta = Q6_Vw_vsub_VwVw(a_exp, b_exp);
    a_exp = Q6_Vw_vadd_VwVw(a_exp, const1);
    Q0 = Q6_Q_vcmp_gt_VwVw(expDelta, const31);
    b_mant = Q6_Vw_vasr_VwVw(b_mant, expDelta);
    b_mant = Q6_V_vmux_QVV(Q0, const0, b_mant);

    HVX_Vector c_mant, c_exp;
    c_mant = Q6_Vw_vavg_VwVw_rnd(a_mant, b_mant);
    Q0 = Q6_Q_vcmp_eq_VwVw(c_mant, const0);
    HVX_Vector k = Q6_Vw_vnormamt_Vw(c_mant);
    c_exp = Q6_Vw_vsub_VwVw(a_exp, k);
    c_exp = Q6_V_vmux_QVV(Q0, constNEGMAXEXP, c_exp);
    c_mant = Q6_Vw_vasl_VwVw(c_mant, k);

    return (Q6_W_vcombine_VV(c_exp, c_mant));
}

static inline qm_vqf32_t qm_vqf32_sub_inl(qm_vqf32_t a, qm_vqf32_t b)
{
    HVX_Vector a_mant = Q6_V_lo_W(a);
    HVX_Vector a_exp = Q6_V_hi_W(a);
    HVX_Vector b_mant = Q6_V_lo_W(b);
    HVX_Vector b_exp = Q6_V_hi_W(b);
    HVX_Vector const0 = Q6_V_vzero();
    b_mant = Q6_Vw_vsub_VwVw_sat(const0, b_mant);
    HVX_Vector const31 = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(ONES_THIRTYONES_32));
    HVX_Vector const1 = Q6_V_vsplat_R(HEXAGON_V64_GET_W1(ONES_THIRTYONES_32));
    HVX_Vector constNEGMAXEXP = Q6_V_vsplat_R( HEXAGON_V64_GET_W0(MAXEXP_NEGMAXEXP_32));

    HVX_VectorPred Q0 = Q6_Q_vcmp_gt_VwVw(b_exp, a_exp);
    HVX_VectorPair sortedExp = Q6_W_vswap_QVV(Q0, b_exp, a_exp);
    HVX_VectorPair sortedMant = Q6_W_vswap_QVV(Q0, b_mant, a_mant);
    a_mant = Q6_V_lo_W(sortedMant);
    a_exp = Q6_V_lo_W(sortedExp);
    b_mant = Q6_V_hi_W(sortedMant);
    b_exp = Q6_V_hi_W(sortedExp);

    HVX_Vector expDelta = Q6_Vw_vsub_VwVw(a_exp, b_exp);
    a_exp = Q6_Vw_vadd_VwVw(a_exp, const1);
    Q0 = Q6_Q_vcmp_gt_VwVw(expDelta, const31);
    b_mant = Q6_Vw_vasr_VwVw(b_mant, expDelta);
    b_mant = Q6_V_vmux_QVV(Q0, const0, b_mant);

    HVX_Vector c_mant, c_exp;
    c_mant = Q6_Vw_vavg_VwVw_rnd(a_mant, b_mant);
    Q0 = Q6_Q_vcmp_eq_VwVw(c_mant, const0);
    HVX_Vector k = Q6_Vw_vnormamt_Vw(c_mant);
    c_exp = Q6_Vw_vsub_VwVw(a_exp, k);
    c_exp = Q6_V_vmux_QVV(Q0, constNEGMAXEXP, c_exp);
    c_mant = Q6_Vw_vasl_VwVw(c_mant, k);

    return (Q6_W_vcombine_VV(c_exp, c_mant));
}

static inline qm_vqf32_t qm_vqf32_mpyadd_inl(qm_vqf32_t acc, qm_vqf32_t a, qm_vqf32_t b)
{
    return qm_vqf32_add_inl(acc, qm_vqf32_mpy_inl(a, b));
}

static inline qm_vqf32_t qm_vqf32_mpysub_inl(qm_vqf32_t acc, qm_vqf32_t a, qm_vqf32_t b)
{
    return qm_vqf32_sub_inl(acc, qm_vqf32_mpy_inl(a, b));
}

static inline qm_vqf32_t qm_vqf32_negate_inl(qm_vqf32_t a)
{
    HVX_Vector const40000000 = Q6_V_vsplat_R(0x40000000);
    HVX_Vector const80000000 = Q6_Vw_vadd_VwVw(const40000000, const40000000);
    HVX_Vector const1 = Q6_V_vsplat_R(1);
    HVX_Vector a_mant = Q6_V_lo_W(a);
    HVX_Vector a_exp = Q6_V_hi_W(a);
    HVX_VectorPred Q0 = Q6_Q_vcmp_eq_VwVw(a_mant, const80000000); // handle saturation case
    a_exp = Q6_Vw_condacc_QVwVw(Q0, a_exp, const1);
    HVX_Vector const0 = Q6_V_vzero();
    a_mant = Q6_Vw_vsub_VwVw(const0, a_mant);
    a_mant = Q6_V_vmux_QVV(Q0, const40000000, a_mant);
    HVX_Vector k = Q6_Vw_vnormamt_Vw(a_mant);
    a_exp = Q6_Vw_vsub_VwVw(a_exp, k);
    a_mant = Q6_Vw_vasl_VwVw(a_mant, k);
    return (Q6_W_vcombine_VV(a_exp, a_mant));
}

static inline qm_vqf32_t qm_vqf32_abs_inl(qm_vqf32_t a)
{
    HVX_Vector const40000000 = Q6_V_vsplat_R(0x40000000);
    HVX_Vector const80000000 = Q6_Vw_vadd_VwVw(const40000000, const40000000);
    HVX_Vector const1 = Q6_V_vsplat_R(1);
    HVX_Vector a_mant = Q6_V_lo_W(a);
    HVX_Vector a_exp = Q6_V_hi_W(a);
    HVX_VectorPred Q0 = Q6_Q_vcmp_eq_VwVw(a_mant, const80000000); // handle saturation case
    a_mant = Q6_V_vmux_QVV(Q0, const40000000, a_mant);
    a_exp = Q6_Vw_condacc_QVwVw(Q0, a_exp, const1);
    a_mant = Q6_Vw_vabs_Vw(a_mant);
    HVX_Vector k = Q6_Vw_vnormamt_Vw(a_mant);
    a_exp = Q6_Vw_vsub_VwVw(a_exp, k);
    a_mant = Q6_Vw_vasl_VwVw(a_mant, k);
    return (Q6_W_vcombine_VV(a_exp, a_mant));
}

static inline qm_vqf32_t qm_vqf32_min_inl(qm_vqf32_t a, qm_vqf32_t b)
{
    HVX_Vector a_mant = Q6_V_lo_W(a);
    HVX_Vector a_exp = Q6_V_hi_W(a);
    HVX_Vector b_mant = Q6_V_lo_W(b);
    HVX_Vector b_exp = Q6_V_hi_W(b);
    HVX_Vector const31 = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(ONES_THIRTYONES_32));

    HVX_VectorPred q0;
    HVX_VectorPair dy_dx;
    HVX_Vector diff, x_mant, y_mant;
    HVX_Vector zero = Q6_V_vzero();
    HVX_Vector c_exp, c_mant;

    q0 = Q6_Q_vcmp_gt_VwVw(b_exp, a_exp);

    diff = Q6_Vuw_vabsdiff_VwVw(a_exp, b_exp);

    dy_dx = Q6_W_vswap_QVV(q0, diff, zero);
    HVX_VectorPred q1 = Q6_Q_vcmp_gt_VwVw(Q6_V_lo_W(dy_dx), const31);
    HVX_VectorPred q2 = Q6_Q_vcmp_gt_VwVw(Q6_V_hi_W(dy_dx), const31);

    x_mant = Q6_Vw_vasr_VwVw(a_mant, Q6_V_lo_W(dy_dx));
    y_mant = Q6_Vw_vasr_VwVw(b_mant, Q6_V_hi_W(dy_dx));

    x_mant =  Q6_V_vmux_QVV(q1, zero, x_mant);
    y_mant =  Q6_V_vmux_QVV(q2, zero, y_mant);

    q1 = Q6_Q_vcmp_gt_VwVw(y_mant, x_mant);
    c_mant = Q6_V_vmux_QVV(q1, a_mant, b_mant);
    c_exp = Q6_V_vmux_QVV(q1, a_exp, b_exp);

    return (Q6_W_vcombine_VV(c_exp, c_mant));
}

static inline qm_vqf32_t qm_vqf32_max_inl(qm_vqf32_t a, qm_vqf32_t b)
{
    HVX_Vector a_mant = Q6_V_lo_W(a);
    HVX_Vector a_exp = Q6_V_hi_W(a);
    HVX_Vector b_mant = Q6_V_lo_W(b);
    HVX_Vector b_exp = Q6_V_hi_W(b);
    HVX_Vector const31 = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(ONES_THIRTYONES_32));

    HVX_VectorPred q0;
    HVX_VectorPair dy_dx;
    HVX_Vector diff, x_mant, y_mant;
    HVX_Vector zero = Q6_V_vzero();
    HVX_Vector c_exp, c_mant;

    q0 = Q6_Q_vcmp_gt_VwVw(b_exp, a_exp);

    diff = Q6_Vuw_vabsdiff_VwVw(a_exp, b_exp);

    dy_dx = Q6_W_vswap_QVV(q0, diff, zero);
    HVX_VectorPred q1 = Q6_Q_vcmp_gt_VwVw(Q6_V_lo_W(dy_dx), const31);
    HVX_VectorPred q2 = Q6_Q_vcmp_gt_VwVw(Q6_V_hi_W(dy_dx), const31);

    x_mant = Q6_Vw_vasr_VwVw(a_mant, Q6_V_lo_W(dy_dx));
    y_mant = Q6_Vw_vasr_VwVw(b_mant, Q6_V_hi_W(dy_dx));

    x_mant =  Q6_V_vmux_QVV(q1, zero, x_mant);
    y_mant =  Q6_V_vmux_QVV(q2, zero, y_mant);

    q1 = Q6_Q_vcmp_gt_VwVw(x_mant, y_mant);
    c_mant = Q6_V_vmux_QVV(q1, a_mant, b_mant);
    c_exp = Q6_V_vmux_QVV(q1, a_exp, b_exp);

    return (Q6_W_vcombine_VV(c_exp, c_mant));
}

static inline HVX_VectorPred qm_vqf32_lessthan_inl(qm_vqf32_t a, qm_vqf32_t b)
{
    HVX_Vector a_mant = Q6_V_lo_W(a);
    HVX_Vector a_exp = Q6_V_hi_W(a);
    HVX_Vector b_mant = Q6_V_lo_W(b);
    HVX_Vector b_exp = Q6_V_hi_W(b);
    HVX_Vector const31 = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(ONES_THIRTYONES_32));

    HVX_VectorPred q0;
    HVX_VectorPair dy_dx;
    HVX_Vector diff, x_mant, y_mant;
    HVX_Vector zero = Q6_V_vzero();

    q0 = Q6_Q_vcmp_gt_VwVw(b_exp, a_exp);

    diff = Q6_Vuw_vabsdiff_VwVw(a_exp, b_exp);

    dy_dx = Q6_W_vswap_QVV(q0, diff, zero);
    HVX_VectorPred q1 = Q6_Q_vcmp_gt_VwVw(Q6_V_lo_W(dy_dx), const31);
    HVX_VectorPred q2 = Q6_Q_vcmp_gt_VwVw(Q6_V_hi_W(dy_dx), const31);

    x_mant = Q6_Vw_vasr_VwVw(a_mant, Q6_V_lo_W(dy_dx));
    y_mant = Q6_Vw_vasr_VwVw(b_mant, Q6_V_hi_W(dy_dx));

    x_mant =  Q6_V_vmux_QVV(q1, zero, x_mant);
    y_mant =  Q6_V_vmux_QVV(q2, zero, y_mant);

    q1 = Q6_Q_vcmp_gt_VwVw(y_mant, x_mant);

    return (q1);
}

static inline HVX_VectorPred qm_vqf32_greaterthan_inl(qm_vqf32_t a, qm_vqf32_t b)
{
    HVX_Vector a_mant = Q6_V_lo_W(a);
    HVX_Vector a_exp = Q6_V_hi_W(a);
    HVX_Vector b_mant = Q6_V_lo_W(b);
    HVX_Vector b_exp = Q6_V_hi_W(b);
    HVX_Vector const31 = Q6_V_vsplat_R(HEXAGON_V64_GET_W0(ONES_THIRTYONES_32));

    HVX_VectorPred q0;
    HVX_VectorPair dy_dx;
    HVX_Vector diff, x_mant, y_mant;
    HVX_Vector zero = Q6_V_vzero();

    q0 = Q6_Q_vcmp_gt_VwVw(b_exp, a_exp);

    diff = Q6_Vuw_vabsdiff_VwVw(a_exp, b_exp);

    dy_dx = Q6_W_vswap_QVV(q0, diff, zero);
    HVX_VectorPred q1 = Q6_Q_vcmp_gt_VwVw(Q6_V_lo_W(dy_dx), const31);
    HVX_VectorPred q2 = Q6_Q_vcmp_gt_VwVw(Q6_V_hi_W(dy_dx), const31);

    x_mant = Q6_Vw_vasr_VwVw(a_mant, Q6_V_lo_W(dy_dx));
    y_mant = Q6_Vw_vasr_VwVw(b_mant, Q6_V_hi_W(dy_dx));

    x_mant =  Q6_V_vmux_QVV(q1, zero, x_mant);
    y_mant =  Q6_V_vmux_QVV(q2, zero, y_mant);

    q1 = Q6_Q_vcmp_gt_VwVw(x_mant, y_mant);

    return (q1);
}

#ifdef HIGH_QUALITY_INVERSE
// transposed table for VLUT instruction
static const int32_t invTable4HVX[5][16] __attribute__((aligned(2 * QMATH_VLEN))) = {
{2147483645, 2021161078, 1908874352, 1808407281, 1717986917, 1636178017, 1561806289, 1493901668, 1431655765, 1374389534, 1321528399, 1272582902, 1227133513, 1184818564, 1145324612, 1108378657},
{-2147480949, -1902267358, -1696775847, -1522868302, -1374388803, -1246611273, -1135858702, -1039235621, -954436926, -879609105, -813248089, -754123076, -701219049, -653692919, -610839726, -572066348},
{2147133394, 1790125413, 1508070670, 1282288281, 1099416950, 949728418, 826025313, 722905146, 636259083, 562924512, 500440275, 446871672, 400683629, 360647620, 325772560, 295252944},
{-2131482747, -1673845791, -1332652643, -1074101491, -875300862, -720425167, -598328994, -501030369, -422725162, -359133817, -307060463, -264090393, -228379302, -198504392, -173357799, -152069396},
{1844192256, 1373909894, 1040458237, 799566904, 622602786, 490622030, 390832872, 314436642, 255278697, 208987249, 172412890, 143257171, 119822467, 100840945, 85355940, 72638597}
};
#else
// transposed table for VLUT instruction
static const int32_t invTableHVX[4][16] __attribute__((aligned(2 * QMATH_VLEN))) = {
{2147483428, 2021160917, 1908874230, 1808407187, 1717986844, 1636177959, 1561806243, 1493901631, 1431655735, 1374389510, 1321528378, 1272582885, 1227133499, 1184818552, 1145324602, 1108378648},
{-2147369324, -1902184157, -1696712812, -1522819842, -1374351055, -1246581518, -1135834991, -1039216540, -954421431, -879596417, -813237619, -754114375, -701211771, -653686792, -610834539, -572061934},
{2138168719, 1783445051, 1503010473, 1278398830, 1096387761, 947340955, 824123148, 721374580, 635016315, 561906980, 499600726, 446174022, 400100049, 360156443, 325356772, 294899078},
{-1901388351, -1502408533, -1202811112, -974312764, -797591786, -659184687, -549541251, -461776859, -390855055, -333041629, -285533610, -246203050, -213417460, -185912242, -162698913, -142998302}
};
#endif

static inline qm_vqf32_t qm_vqf32_inverse_inl(qm_vqf32_t a)
{
    HVX_Vector aMant = Q6_V_lo_W(a);
    HVX_Vector aExp = Q6_V_hi_W(a);

    HVX_Vector const0 = Q6_V_vzero();
    HVX_Vector constMAXEXP = Q6_V_vsplat_R(HEXAGON_V64_GET_W1(MAXEXP_NEGMAXEXP_32));

    HVX_VectorPred neg = Q6_Q_vcmp_gt_VwVw(const0, aMant);
    HVX_VectorPred zero = Q6_Q_vcmp_eq_VwVw(const0, aMant);
    HVX_VectorPred notInf = Q6_Q_vcmp_gt_VwVw(constMAXEXP, aExp);
    HVX_Vector negA = Q6_Vw_vsub_VwVw_sat(const0, aMant);

    aMant = Q6_V_vmux_QVV(neg, negA, aMant);

    HVX_Vector exp = Q6_Vuw_vcl0_Vuw(aMant);
    aMant = Q6_Vw_vasl_VwVw(aMant, exp);

    HVX_Vector x = Q6_V_vand_VV(aMant, Q6_V_vsplat_R(0x07FFFFFF));
    exp = Q6_Vw_vsub_VwVw(exp,aExp);

    // VRDELTA controls for replicating lowest byte in each lane to all bytes in each lane.
    HVX_Vector vrdeltaCtrl = Q6_V_vsplat_R(0x02020100);
    HVX_Vector range = Q6_Vuw_vlsr_VuwR(aMant, 27);
    range = Q6_V_vand_VV(range, Q6_V_vsplat_R(0xF));
    HVX_Vector rangeOffsets = Q6_V_vsplat_R(0x30201000);
    range = Q6_V_vrdelta_VV(range, vrdeltaCtrl);
    range = Q6_Vw_vadd_VwVw(range, rangeOffsets);

#ifdef HIGH_QUALITY_INVERSE
    HVX_Vector invTabHi = *(HVX_Vector*)(&invTable4HVX[4][0]);
    HVX_VectorPair invTabLo = *(HVX_VectorPair*)invTable4HVX;

    HVX_VectorPair invTab4 = Q6_Wh_vlut16_VbVhR(range, invTabHi, 0);
    invTab4 = Q6_Wh_vlut16or_WhVbVhR(invTab4, range, invTabHi, 2);

    HVX_VectorPair invTab3 = Q6_Wh_vlut16_VbVhR(range, Q6_V_hi_W(invTabLo), 1);
    invTab3 = Q6_Wh_vlut16or_WhVbVhR(invTab3, range, Q6_V_hi_W(invTabLo), 3);

    HVX_Vector y = Q6_Vw_vmpye_VwVuh(x, Q6_V_lo_W(invTab4));
    y = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y, x, Q6_V_lo_W(invTab4));
    y = Q6_Vw_vadd_VwVw(y, Q6_V_hi_W(invTab3));

    HVX_VectorPair invTab2 = Q6_Wh_vlut16_VbVhR(range, Q6_V_hi_W(invTabLo), 0);
    invTab2 = Q6_Wh_vlut16or_WhVbVhR(invTab2, range, Q6_V_hi_W(invTabLo), 2);

    HVX_Vector y2 = Q6_Vw_vmpye_VwVuh(x, y);
    y2 = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y2, x, y);
    y = Q6_Vw_vadd_VwVw(y2, Q6_V_lo_W(invTab2));

    HVX_VectorPair invTab1 = Q6_Wh_vlut16_VbVhR(range, Q6_V_lo_W(invTabLo), 1);
    invTab1 = Q6_Wh_vlut16or_WhVbVhR(invTab1, range, Q6_V_lo_W(invTabLo), 3);

    y2 = Q6_Vw_vmpye_VwVuh(x, y);
    y2 = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y2, x, y);
    y = Q6_Vw_vadd_VwVw(y2, Q6_V_hi_W(invTab1));

    HVX_VectorPair invTab0 = Q6_Wh_vlut16_VbVhR(range, Q6_V_lo_W(invTabLo), 0);
    invTab0 = Q6_Wh_vlut16or_WhVbVhR(invTab0, range, Q6_V_lo_W(invTabLo), 2);

    y2 = Q6_Vw_vmpye_VwVuh(x, y);
    y2 = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y2, x, y);
    y = Q6_Vw_vadd_VwVw(y2, Q6_V_lo_W(invTab0));
#else
    HVX_VectorPair invTab = *(HVX_VectorPair*)invTableHVX;

    HVX_VectorPair invTab3 = Q6_Wh_vlut16_VbVhR(range, Q6_V_hi_W(invTab), 1);
    invTab3 = Q6_Wh_vlut16or_WhVbVhR(invTab3, range, Q6_V_hi_W(invTab), 3);

    HVX_VectorPair invTab2 = Q6_Wh_vlut16_VbVhR(range, Q6_V_hi_W(invTab), 0);
    invTab2 = Q6_Wh_vlut16or_WhVbVhR(invTab2, range, Q6_V_hi_W(invTab), 2);

    HVX_Vector y = Q6_Vw_vmpye_VwVuh(x, Q6_V_hi_W(invTab3));
    y = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y, x, Q6_V_hi_W(invTab3));
    y = Q6_Vw_vadd_VwVw(y, Q6_V_lo_W(invTab2));

    HVX_VectorPair invTab1 = Q6_Wh_vlut16_VbVhR(range, Q6_V_lo_W(invTab), 1);
    invTab1 = Q6_Wh_vlut16or_WhVbVhR(invTab1, range, Q6_V_lo_W(invTab), 3);

    HVX_Vector y2 = Q6_Vw_vmpye_VwVuh(x, y);
    y2 = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y2, x, y);
    y = Q6_Vw_vadd_VwVw(y2, Q6_V_hi_W(invTab1));

    HVX_VectorPair invTab0 = Q6_Wh_vlut16_VbVhR(range, Q6_V_lo_W(invTab), 0);
    invTab0 = Q6_Wh_vlut16or_WhVbVhR(invTab0, range, Q6_V_lo_W(invTab), 2);

    y2 = Q6_Vw_vmpye_VwVuh(x, y);
    y2 = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y2, x, y);
    y = Q6_Vw_vadd_VwVw(y2, Q6_V_lo_W(invTab0));
#endif
    HVX_Vector negY =  Q6_Vw_vsub_VwVw_sat(const0, y);
    y = Q6_V_vmux_QVV(neg, negY, y);
    HVX_Vector k = Q6_Vw_vnormamt_Vw(y);
    exp = Q6_Vw_vsub_VwVw(exp, k);
    y = Q6_Vw_vasl_VwVw(y, k);

    HVX_Vector constMAXNEGEXP = Q6_Vw_vsub_VwVw(const0, constMAXEXP);
    y = Q6_V_vmux_QVV(notInf, y, const0);
    exp = Q6_V_vmux_QVV(notInf, exp, constMAXNEGEXP);
    HVX_Vector constMAXINT =  Q6_V_vsplat_R(0x7FFFFFFF);
    y = Q6_V_vmux_QVV(zero, constMAXINT, y);
    exp = Q6_V_vmux_QVV(zero, constMAXEXP, exp);

    return Q6_W_vcombine_VV(exp, y);
}

// transposed table for VLUT instruction
static const int32_t invsqrtTableHVX[4][16] __attribute__((aligned(2 * QMATH_VLEN))) = {
{1518500207, 1473161596, 1431655740, 1393471377, 1358187897, 1325455670, 1294981354, 1266516751, 1239850255, 1214800194, 1191209596, 1168942033, 1147878290, 1127913667, 1108955785, 1090922782},
{-759227719, -693235353, -636278085, -586714260, -543266756, -504928715, -470896789, -440523027, -413279658, -388732922, -366523390, -346350961, -327963324, -311146970, -295720142, -281527249},
{567615353, 487959048, 423109247, 369708133, 325283405, 287985058, 256408458, 229472999, 206337356, 186339315, 168952404, 153754201, 140402885, 128619671, 118175525, 108881002},
{-426541078, -347111690, -285730996, -237629230, -199457525, -168821067, -143978738, -123645579, -106860273, -92894616, -81190655, -71316359, -62933938, -55776899, -49633226, -44332927}
};

static inline qm_vqf32_t qm_vqf32_invsqrt_inl(qm_vqf32_t a)
{
    HVX_Vector aMant = Q6_V_lo_W(a);
    HVX_Vector aExp = Q6_V_hi_W(a);

    HVX_Vector const0 = Q6_V_vzero();
    HVX_Vector const1 = Q6_V_vsplat_R(1);
    HVX_Vector constMAXEXP = Q6_V_vsplat_R(HEXAGON_V64_GET_W1(MAXEXP_NEGMAXEXP_32));

    HVX_VectorPred neg = Q6_Q_vcmp_gt_VwVw(const0, aMant);
    HVX_VectorPred zero = Q6_Q_vcmp_eq_VwVw(const0, aMant);
    HVX_VectorPred notInf = Q6_Q_vcmp_gt_VwVw(constMAXEXP, aExp);
    // treat negative values like infinity, i.e. 1/sqrt(negative number) = 0
    notInf = Q6_Q_and_QQn(notInf,neg);

    HVX_Vector exp = Q6_Vw_vnormamt_Vw(aMant);
    exp = Q6_Vw_vadd_VwVw(exp, const1);
    aMant = Q6_Vw_vasl_VwVw(aMant, exp);
    HVX_Vector x = Q6_V_vand_VV(aMant, Q6_V_vsplat_R(0x07FFFFFF));
    exp = Q6_Vw_vsub_VwVw(exp,aExp);

    // VRDELTA controls for replicating lowest byte in each lane to all bytes in each lane.
    HVX_Vector vrdeltaCtrl = Q6_V_vsplat_R(0x02020100);
    HVX_Vector range = Q6_Vuw_vlsr_VuwR(aMant, 27);
    range = Q6_V_vand_VV(range, Q6_V_vsplat_R(0xF));
    HVX_Vector rangeOffsets = Q6_V_vsplat_R(0x30201000);
    range = Q6_V_vrdelta_VV(range, vrdeltaCtrl);
    range = Q6_Vw_vadd_VwVw(range, rangeOffsets);

    HVX_VectorPair invTab = *(HVX_VectorPair*)invsqrtTableHVX;

    HVX_VectorPair invTab3 = Q6_Wh_vlut16_VbVhR(range, Q6_V_hi_W(invTab), 1);
    invTab3 = Q6_Wh_vlut16or_WhVbVhR(invTab3, range, Q6_V_hi_W(invTab), 3);

    HVX_VectorPair invTab2 = Q6_Wh_vlut16_VbVhR(range, Q6_V_hi_W(invTab), 0);
    invTab2 = Q6_Wh_vlut16or_WhVbVhR(invTab2, range, Q6_V_hi_W(invTab), 2);

    HVX_Vector y = Q6_Vw_vmpye_VwVuh(x, Q6_V_hi_W(invTab3));
    y = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y, x, Q6_V_hi_W(invTab3));
    y = Q6_Vw_vadd_VwVw(y, Q6_V_lo_W(invTab2));

    HVX_VectorPair invTab1 = Q6_Wh_vlut16_VbVhR(range, Q6_V_lo_W(invTab), 1);
    invTab1 = Q6_Wh_vlut16or_WhVbVhR(invTab1, range, Q6_V_lo_W(invTab), 3);

    HVX_Vector y2 = Q6_Vw_vmpye_VwVuh(x, y);
    y2 = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y2, x, y);
    y = Q6_Vw_vadd_VwVw(y2, Q6_V_hi_W(invTab1));

    HVX_VectorPair invTab0 = Q6_Wh_vlut16_VbVhR(range, Q6_V_lo_W(invTab), 0);
    invTab0 = Q6_Wh_vlut16or_WhVbVhR(invTab0, range, Q6_V_lo_W(invTab), 2);

    y2 = Q6_Vw_vmpye_VwVuh(x, y);
    y2 = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y2, x, y);
    y = Q6_Vw_vadd_VwVw(y2, Q6_V_lo_W(invTab0));

    HVX_Vector isqrt2 = Q6_V_vsplat_R(0x5a827999);
    y2 = Q6_Vw_vmpye_VwVuh(y, isqrt2);
    y2 = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y2, y, isqrt2);
    HVX_Vector lsbit = Q6_V_vand_VV(exp, const1);
    HVX_VectorPred Q0 = Q6_Q_vcmp_eq_VwVw(lsbit, const1);
    y = Q6_V_vmux_QVV(Q0, y, y2);

    exp = Q6_Vw_vasr_VwR(exp, 1);
    exp = Q6_Vw_vadd_VwVw_sat(exp, const1);

    HVX_Vector k = Q6_Vw_vnormamt_Vw(y);
    exp = Q6_Vw_vsub_VwVw(exp, k);
    y = Q6_Vw_vasl_VwVw(y, k);

    HVX_Vector constMAXNEGEXP = Q6_Vw_vsub_VwVw(const0, constMAXEXP);
    y = Q6_V_vmux_QVV(notInf, y, const0);
    exp = Q6_V_vmux_QVV(notInf, exp, constMAXNEGEXP);
    HVX_Vector constMAXINT =  Q6_V_vsplat_R(0x7FFFFFFF);
    y = Q6_V_vmux_QVV(zero, constMAXINT, y);
    exp = Q6_V_vmux_QVV(zero, constMAXEXP, exp);

    return Q6_W_vcombine_VV(exp, y);
}

// transposed table for VLUT instruction
static const int32_t sqrtTableHVX[4][16] __attribute__((aligned(2 * QMATH_VLEN))) = {
{1518500256, 1565234236, 1610612740, 1654747287, 1697734894, 1739660587, 1780599378, 1820617844, 1859775395, 1898125314, 1935715603, 1972589689, 2008787015, 2044343527, 2079292102, 2113662895},
{759246836, 736578139, 715825682, 696733869, 679092422, 662726543, 647489575, 633257429, 619924310, 607399386, 595604177, 584470471, 573938664, 563956407, 554477513, 545461052},
{-189546020, -173096540, -158894783, -146533257, -135694733, -126128963, -117636176, -110055171, -103254594, -97126450, -91581223, -86544153, -81952369, -77752656, -73899705, -70354721},
{87931760, 75898202, 66050005, 57902174, 51095064, 45357810, 40483548, 36312279, 32718822, 29604199, 26889393, 24510757, 22416605, 20564640, 18920007, 17453790}
};

static inline qm_vqf32_t qm_vqf32_sqrt_inl(qm_vqf32_t a)
{
    HVX_Vector aMant = Q6_V_lo_W(a);
    HVX_Vector aExp = Q6_V_hi_W(a);

    HVX_Vector const0 = Q6_V_vzero();
    HVX_Vector const1 = Q6_V_vsplat_R(1);
    HVX_Vector constMAXEXP = Q6_V_vsplat_R(HEXAGON_V64_GET_W1(MAXEXP_NEGMAXEXP_32));

    // for zeros and infinities, leave input unchanged.
    HVX_VectorPred valid = Q6_Q_vcmp_gt_VwVw(aMant, const0);
    HVX_Vector absExp = Q6_Vw_vabs_Vw_sat(aExp);
    valid = Q6_Q_vcmp_gtand_QVuwVuw(valid, constMAXEXP, absExp);

    HVX_Vector exp = Q6_Vw_vnormamt_Vw(aMant);
    exp = Q6_Vw_vadd_VwVw(exp, const1);
    HVX_Vector normMant = Q6_Vw_vasl_VwVw(aMant, exp);
    HVX_Vector x = Q6_V_vand_VV(normMant, Q6_V_vsplat_R(0x07FFFFFF));
    exp = Q6_Vw_vsub_VwVw(aExp,exp);

    // VRDELTA controls for replicating lowest byte in each lane to all bytes in each lane.
    HVX_Vector vrdeltaCtrl = Q6_V_vsplat_R(0x02020100);
    HVX_Vector range = Q6_Vuw_vlsr_VuwR(normMant, 27);
    range = Q6_V_vand_VV(range, Q6_V_vsplat_R(0xF));
    HVX_Vector rangeOffsets = Q6_V_vsplat_R(0x30201000);
    range = Q6_V_vrdelta_VV(range, vrdeltaCtrl);
    range = Q6_Vw_vadd_VwVw(range, rangeOffsets);

    HVX_VectorPair invTab = *(HVX_VectorPair*)sqrtTableHVX;

    HVX_VectorPair invTab3 = Q6_Wh_vlut16_VbVhR(range, Q6_V_hi_W(invTab), 1);
    invTab3 = Q6_Wh_vlut16or_WhVbVhR(invTab3, range, Q6_V_hi_W(invTab), 3);

    HVX_VectorPair invTab2 = Q6_Wh_vlut16_VbVhR(range, Q6_V_hi_W(invTab), 0);
    invTab2 = Q6_Wh_vlut16or_WhVbVhR(invTab2, range, Q6_V_hi_W(invTab), 2);

    HVX_Vector y = Q6_Vw_vmpye_VwVuh(x, Q6_V_hi_W(invTab3));
    y = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y, x, Q6_V_hi_W(invTab3));
    y = Q6_Vw_vadd_VwVw(y, Q6_V_lo_W(invTab2));

    HVX_VectorPair invTab1 = Q6_Wh_vlut16_VbVhR(range, Q6_V_lo_W(invTab), 1);
    invTab1 = Q6_Wh_vlut16or_WhVbVhR(invTab1, range, Q6_V_lo_W(invTab), 3);

    HVX_Vector y2 = Q6_Vw_vmpye_VwVuh(x, y);
    y2 = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y2, x, y);
    y = Q6_Vw_vadd_VwVw(y2, Q6_V_hi_W(invTab1));

    HVX_VectorPair invTab0 = Q6_Wh_vlut16_VbVhR(range, Q6_V_lo_W(invTab), 0);
    invTab0 = Q6_Wh_vlut16or_WhVbVhR(invTab0, range, Q6_V_lo_W(invTab), 2);

    y2 = Q6_Vw_vmpye_VwVuh(x, y);
    y2 = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y2, x, y);
    y = Q6_Vw_vadd_VwVw(y2, Q6_V_lo_W(invTab0));

    // handle overflow corner case
    HVX_VectorPred Q0 = Q6_Q_vcmp_gt_VwVw(const0, y);
    y =  Q6_V_vmux_QVV(Q0, Q6_V_vsplat_R(0x40000000), y);
    HVX_Vector const2 = Q6_Vw_vadd_VwVw(const1,const1);
    exp = Q6_Vw_condacc_QVwVw(Q0, exp, const2);

    HVX_Vector isqrt2 = Q6_V_vsplat_R(0x5a827999);
    y2 = Q6_Vw_vmpye_VwVuh(y, isqrt2);
    y2 = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(y2, y, isqrt2);
    HVX_Vector lsbit = Q6_V_vand_VV(exp, const1);
    Q0 = Q6_Q_vcmp_eq_VwVw(lsbit, const0);
    y = Q6_V_vmux_QVV(Q0, y2, y);

    exp = Q6_Vw_vasr_VwR(exp, 1);
    exp = Q6_Vw_vadd_VwVw_sat(exp, const1);

    HVX_Vector k = Q6_Vw_vnormamt_Vw(y);
    exp = Q6_Vw_vsub_VwVw(exp, k);
    y = Q6_Vw_vasl_VwVw(y, k);

    y = Q6_V_vmux_QVV(valid, y, aMant);
    exp = Q6_V_vmux_QVV(valid, exp, aExp);

    return Q6_W_vcombine_VV(exp, y);
}

static inline HVX_Vector qm_vqf32_getmant(qm_vqf32_t a)
{
    return Q6_V_lo_W(a);
}

static inline HVX_Vector qm_vqf32_getexp(qm_vqf32_t a)
{
    return Q6_V_hi_W(a);
}

