#ifndef _HVX_HEXAGON_PROTOS_H_
#define _HVX_HEXAGON_PROTOS_H_ 1

#ifdef _Q6PROTOS_H_
#warning Do not #include both hexagon_protos.h and q6protos.h
#endif

#ifdef __HVX__
#ifdef __HVXDBL__
#define __BUILTIN_VECTOR_WRAP(a) a ## _128B
#else
#define __BUILTIN_VECTOR_WRAP(a) a
#endif
#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (Qv4) vmem(Rt32+#s4)=Vs32
   C Intrinsic Prototype: void Q6_vmem_QRIV(HVX_VectorPred Qv, HVX_Vector* A, HVX_Vector Vs)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vmem_QRIV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vS32b_qpred_ai)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (!Qv4) vmem(Rt32+#s4)=Vs32
   C Intrinsic Prototype: void Q6_vmem_QnRIV(HVX_VectorPred Qv, HVX_Vector* A, HVX_Vector Vs)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vmem_QnRIV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vS32b_nqpred_ai)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (Qv4) vmem(Rt32+#s4):nt=Vs32
   C Intrinsic Prototype: void Q6_vmem_QRIV_nt(HVX_VectorPred Qv, HVX_Vector* A, HVX_Vector Vs)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vmem_QRIV_nt __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vS32b_nt_qpred_ai)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (!Qv4) vmem(Rt32+#s4):nt=Vs32
   C Intrinsic Prototype: void Q6_vmem_QnRIV_nt(HVX_VectorPred Qv, HVX_Vector* A, HVX_Vector Vs)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vmem_QnRIV_nt __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vS32b_nt_nqpred_ai)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=valign(Vu32,Vv32,Rt8)
   C Intrinsic Prototype: HVX_Vector Q6_V_valign_VVR(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_valign_VVR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_valignb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vlalign(Vu32,Vv32,Rt8)
   C Intrinsic Prototype: HVX_Vector Q6_V_vlalign_VVR(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vlalign_VVR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlalignb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=valign(Vu32,Vv32,#u3)
   C Intrinsic Prototype: HVX_Vector Q6_V_valign_VVI(HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_valign_VVI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_valignbi)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vlalign(Vu32,Vv32,#u3)
   C Intrinsic Prototype: HVX_Vector Q6_V_vlalign_VVI(HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vlalign_VVI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlalignbi)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vror(Vu32,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vror_VR(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vror_VR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vror)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uh=vunpack(Vu32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuh_vunpack_Vub(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wuh_vunpack_Vub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vunpackub)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vunpack(Vu32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vunpack_Vb(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vunpack_Vb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vunpackb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uw=vunpack(Vu32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vunpack_Vuh(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wuw_vunpack_Vuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vunpackuh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vunpack(Vu32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vunpack_Vh(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Ww_vunpack_Vh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vunpackh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.h|=vunpacko(Vu32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vunpackoor_WhVb(HVX_VectorPair Vxx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vunpackoor_WhVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vunpackob)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.w|=vunpacko(Vu32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vunpackoor_WwVh(HVX_VectorPair Vxx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Ww_vunpackoor_WwVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vunpackoh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vpacke(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vpacke_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vpacke_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vpackeb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vpacke(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vpacke_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vpacke_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vpackeh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vpacko(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vpacko_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vpacko_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vpackob)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vpacko(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vpacko_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vpacko_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vpackoh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vpack(Vu32.h,Vv32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vpack_VhVh_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vpack_VhVh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vpackhub_sat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vpack(Vu32.h,Vv32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vpack_VhVh_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vpack_VhVh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vpackhb_sat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vpack(Vu32.w,Vv32.w):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vpack_VwVw_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vpack_VwVw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vpackwuh_sat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vpack(Vu32.w,Vv32.w):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vpack_VwVw_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vpack_VwVw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vpackwh_sat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uh=vzxt(Vu32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuh_vzxt_Vub(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wuh_vzxt_Vub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vzb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vsxt(Vu32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vsxt_Vb(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vsxt_Vb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uw=vzxt(Vu32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vzxt_Vuh(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wuw_vzxt_Vuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vzh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vsxt(Vu32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vsxt_Vh(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Ww_vsxt_Vh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vdmpy(Vu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vdmpy_VubRb(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vh_vdmpy_VubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpybus)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.h+=vdmpy(Vu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vdmpyacc_VhVubRb(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vh_vdmpyacc_VhVubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpybus_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vdmpy(Vuu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vdmpy_WubRb(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vdmpy_WubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpybus_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.h+=vdmpy(Vuu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vdmpyacc_WhWubRb(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vdmpyacc_WhWubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpybus_dv_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vdmpy(Vu32.h,Rt32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpy_VhRb(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpy_VhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vdmpy(Vu32.h,Rt32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpyacc_VwVhRb(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpyacc_VwVhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhb_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vdmpy(Vuu32.h,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vdmpy_WhRb(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vdmpy_WhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhb_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.w+=vdmpy(Vuu32.h,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vdmpyacc_WwWhRb(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vdmpyacc_WwWhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhb_dv_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vdmpy(Vu32.h,Vv32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpy_VhVh_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpy_VhVh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhvsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vdmpy(Vu32.h,Vv32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpyacc_VwVhVh_sat(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpyacc_VwVhVh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhvsat_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vdmpy(Vu32.h,Rt32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpy_VhRh_sat(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpy_VhRh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vdmpy(Vu32.h,Rt32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpyacc_VwVhRh_sat(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpyacc_VwVhRh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhsat_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vdmpy(Vuu32.h,Rt32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpy_WhRh_sat(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpy_WhRh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhisat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vdmpy(Vuu32.h,Rt32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpyacc_VwWhRh_sat(HVX_Vector Vx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpyacc_VwWhRh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhisat_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vdmpy(Vu32.h,Rt32.uh):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpy_VhRuh_sat(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpy_VhRuh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhsusat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vdmpy(Vu32.h,Rt32.uh):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpyacc_VwVhRuh_sat(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpyacc_VwVhRuh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhsusat_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vdmpy(Vuu32.h,Rt32.uh,#1):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpy_WhRuh_sat(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpy_WhRuh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhsuisat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vdmpy(Vuu32.h,Rt32.uh,#1):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vdmpyacc_VwWhRuh_sat(HVX_Vector Vx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vdmpyacc_VwWhRuh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdmpyhsuisat_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vtmpy(Vuu32.b,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vtmpy_WbRb(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vtmpy_WbRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vtmpyb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.h+=vtmpy(Vuu32.b,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vtmpyacc_WhWbRb(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vtmpyacc_WhWbRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vtmpyb_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vtmpy(Vuu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vtmpy_WubRb(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vtmpy_WubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vtmpybus)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.h+=vtmpy(Vuu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vtmpyacc_WhWubRb(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vtmpyacc_WhWubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vtmpybus_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vtmpy(Vuu32.h,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vtmpy_WhRb(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vtmpy_WhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vtmpyhb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.w+=vtmpy(Vuu32.h,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vtmpyacc_WwWhRb(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vtmpyacc_WwWhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vtmpyhb_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uw=vrmpy(Vu32.ub,Rt32.ub)
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vrmpy_VubRub(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vuw_vrmpy_VubRub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpyub)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.uw+=vrmpy(Vu32.ub,Rt32.ub)
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vrmpyacc_VuwVubRub(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vuw_vrmpyacc_VuwVubRub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpyub_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uw=vrmpy(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vrmpy_VubVub(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vuw_vrmpy_VubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpyubv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.uw+=vrmpy(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vrmpyacc_VuwVubVub(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vuw_vrmpyacc_VuwVubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpyubv_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vrmpy(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vrmpy_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vrmpy_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpybv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vrmpy(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vrmpyacc_VwVbVb(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vrmpyacc_VwVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpybv_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uw=vrmpy(Vuu32.ub,Rt32.ub,#u1)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vrmpy_WubRubI(HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuw_vrmpy_WubRubI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpyubi)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.uw+=vrmpy(Vuu32.ub,Rt32.ub,#u1)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vrmpyacc_WuwWubRubI(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuw_vrmpyacc_WuwWubRubI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpyubi_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vrmpy(Vu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vrmpy_VubRb(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vrmpy_VubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpybus)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vrmpy(Vu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vrmpyacc_VwVubRb(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vrmpyacc_VwVubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpybus_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vrmpy(Vuu32.ub,Rt32.b,#u1)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vrmpy_WubRbI(HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vrmpy_WubRbI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpybusi)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.w+=vrmpy(Vuu32.ub,Rt32.b,#u1)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vrmpyacc_WwWubRbI(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vrmpyacc_WwWubRbI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpybusi_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vrmpy(Vu32.ub,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vrmpy_VubVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vrmpy_VubVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpybusv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vrmpy(Vu32.ub,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vrmpyacc_VwVubVb(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vrmpyacc_VwVubVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrmpybusv_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uw=vdsad(Vuu32.uh,Rt32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vdsad_WuhRuh(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuw_vdsad_WuhRuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdsaduh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.uw+=vdsad(Vuu32.uh,Rt32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vdsadacc_WuwWuhRuh(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuw_vdsadacc_WuwWuhRuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdsaduh_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uw=vrsad(Vuu32.ub,Rt32.ub,#u1)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vrsad_WubRubI(HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuw_vrsad_WubRubI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrsadubi)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.uw+=vrsad(Vuu32.ub,Rt32.ub,#u1)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vrsadacc_WuwWubRubI(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuw_vrsadacc_WuwWubRubI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrsadubi_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vasr(Vu32.w,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vasr_VwR(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vasr_VwR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vasl(Vu32.w,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vasl_VwR(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vasl_VwR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaslw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uw=vlsr(Vu32.uw,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vlsr_VuwR(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuw_vlsr_VuwR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlsrw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vasr(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vasr_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vasr_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrwv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vasl(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vasl_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vasl_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaslwv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vlsr(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vlsr_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vlsr_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlsrwv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vasr(Vu32.h,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vasr_VhR(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vasr_VhR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vasl(Vu32.h,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vasl_VhR(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vasl_VhR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaslh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vlsr(Vu32.uh,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vlsr_VuhR(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vlsr_VuhR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlsrh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vasr(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vasr_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vasr_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrhv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vasl(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vasl_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vasl_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaslhv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vlsr(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vlsr_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vlsr_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlsrhv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vlsr(Vu32.ub,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vlsr_VubR(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vlsr_VubR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlsrb)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 66
/* ==========================================================================
   Assembly Syntax:       Vd32.uw=vrotr(Vu32.uw,Vv32.uw)
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vrotr_VuwVuw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuw_vrotr_VuwVuw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrotr)
#endif /* __HVX_ARCH__ >= 66 */

#if __HVX_ARCH__ >= 66
/* ==========================================================================
   Assembly Syntax:       Vxx32.w=vasrinto(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vasrinto_WwVwVw(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Ww_vasrinto_WwVwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasr_into)
#endif /* __HVX_ARCH__ >= 66 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vasr(Vu32.w,Vv32.w,Rt8)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vasr_VwVwR(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vasr_VwVwR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrwh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vasr(Vu32.w,Vv32.w,Rt8):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vasr_VwVwR_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vasr_VwVwR_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrwhsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vasr(Vu32.w,Vv32.w,Rt8):rnd:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vasr_VwVwR_rnd_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vasr_VwVwR_rnd_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrwhrndsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vasr(Vu32.w,Vv32.w,Rt8):rnd:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vasr_VwVwR_rnd_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vasr_VwVwR_rnd_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrwuhrndsat)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vasr(Vu32.w,Vv32.w,Rt8):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vasr_VwVwR_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vasr_VwVwR_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrwuhsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vasr(Vu32.uw,Vv32.uw,Rt8):rnd:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vasr_VuwVuwR_rnd_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vasr_VuwVuwR_rnd_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasruwuhrndsat)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vasr(Vu32.uw,Vv32.uw,Rt8):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vasr_VuwVuwR_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vasr_VuwVuwR_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasruwuhsat)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vasr(Vu32.h,Vv32.h,Rt8):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vasr_VhVhR_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vasr_VhVhR_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrhubsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vasr(Vu32.h,Vv32.h,Rt8):rnd:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vasr_VhVhR_rnd_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vasr_VhVhR_rnd_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrhubrndsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vasr(Vu32.h,Vv32.h,Rt8):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vasr_VhVhR_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vasr_VhVhR_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrhbsat)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vasr(Vu32.h,Vv32.h,Rt8):rnd:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vasr_VhVhR_rnd_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vasr_VhVhR_rnd_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrhbrndsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vasr(Vu32.uh,Vv32.uh,Rt8):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vasr_VuhVuhR_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vasr_VuhVuhR_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasruhubsat)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vasr(Vu32.uh,Vv32.uh,Rt8):rnd:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vasr_VuhVuhR_rnd_sat(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vasr_VuhVuhR_rnd_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasruhubrndsat)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vround(Vu32.w,Vv32.w):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vround_VwVw_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vround_VwVw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vroundwh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vround(Vu32.w,Vv32.w):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vround_VwVw_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vround_VwVw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vroundwuh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vround(Vu32.uw,Vv32.uw):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vround_VuwVuw_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vround_VuwVuw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrounduwuh)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vround(Vu32.h,Vv32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vround_VhVh_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vround_VhVh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vroundhb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vround(Vu32.h,Vv32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vround_VhVh_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vround_VhVh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vroundhub)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vround(Vu32.uh,Vv32.uh):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vround_VuhVuh_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vround_VuhVuh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrounduhub)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vasl(Vu32.w,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vaslacc_VwVwR(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vaslacc_VwVwR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaslw_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vasr(Vu32.w,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vasracc_VwVwR(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vasracc_VwVwR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrw_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vx32.h+=vasl(Vu32.h,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vaslacc_VhVhR(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vaslacc_VhVhR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaslh_acc)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vx32.h+=vasr(Vu32.h,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vasracc_VhVhR(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vasracc_VhVhR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vasrh_acc)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vadd(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vadd_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vadd_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vsub(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vsub_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vsub_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.b=vadd(Vuu32.b,Vvv32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wb_vadd_WbWb(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wb_vadd_WbWb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddb_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.b=vsub(Vuu32.b,Vvv32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wb_vsub_WbWb(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wb_vsub_WbWb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubb_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vadd(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vadd_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vadd_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vsub(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vsub_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vsub_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vadd(Vuu32.h,Vvv32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vadd_WhWh(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vadd_WhWh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddh_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vsub(Vuu32.h,Vvv32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vsub_WhWh(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vsub_WhWh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubh_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vadd(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vadd_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vadd_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vsub(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vsub_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vsub_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vadd(Vuu32.w,Vvv32.w)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vadd_WwWw(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Ww_vadd_WwWw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddw_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vsub(Vuu32.w,Vvv32.w)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vsub_WwWw(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Ww_vsub_WwWw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubw_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vadd(Vu32.ub,Vv32.ub):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vadd_VubVub_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vadd_VubVub_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddubsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.ub=vadd(Vuu32.ub,Vvv32.ub):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Wub_vadd_WubWub_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wub_vadd_WubWub_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddubsat_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vsub(Vu32.ub,Vv32.ub):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vsub_VubVub_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vsub_VubVub_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsububsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.ub=vsub(Vuu32.ub,Vvv32.ub):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Wub_vsub_WubWub_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wub_vsub_WubWub_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsububsat_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vadd(Vu32.uh,Vv32.uh):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vadd_VuhVuh_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vadd_VuhVuh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vadduhsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uh=vadd(Vuu32.uh,Vvv32.uh):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuh_vadd_WuhWuh_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wuh_vadd_WuhWuh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vadduhsat_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vsub(Vu32.uh,Vv32.uh):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vsub_VuhVuh_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vsub_VuhVuh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubuhsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uh=vsub(Vuu32.uh,Vvv32.uh):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuh_vsub_WuhWuh_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wuh_vsub_WuhWuh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubuhsat_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.uw=vadd(Vu32.uw,Vv32.uw):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vadd_VuwVuw_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuw_vadd_VuwVuw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vadduwsat)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vdd32.uw=vadd(Vuu32.uw,Vvv32.uw):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vadd_WuwWuw_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wuw_vadd_WuwWuw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vadduwsat_dv)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.uw=vsub(Vu32.uw,Vv32.uw):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vsub_VuwVuw_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuw_vsub_VuwVuw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubuwsat)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vdd32.uw=vsub(Vuu32.uw,Vvv32.uw):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vsub_WuwWuw_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wuw_vsub_WuwWuw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubuwsat_dv)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vadd(Vu32.b,Vv32.b):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vadd_VbVb_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vadd_VbVb_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddbsat)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vdd32.b=vadd(Vuu32.b,Vvv32.b):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Wb_vadd_WbWb_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wb_vadd_WbWb_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddbsat_dv)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vsub(Vu32.b,Vv32.b):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vsub_VbVb_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vsub_VbVb_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubbsat)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vdd32.b=vsub(Vuu32.b,Vvv32.b):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Wb_vsub_WbWb_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wb_vsub_WbWb_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubbsat_dv)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vadd(Vu32.h,Vv32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vadd_VhVh_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vadd_VhVh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddhsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vadd(Vuu32.h,Vvv32.h):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vadd_WhWh_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vadd_WhWh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddhsat_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vsub(Vu32.h,Vv32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vsub_VhVh_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vsub_VhVh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubhsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vsub(Vuu32.h,Vvv32.h):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vsub_WhWh_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vsub_WhWh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubhsat_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vadd(Vu32.w,Vv32.w):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vadd_VwVw_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vadd_VwVw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddwsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vadd(Vuu32.w,Vvv32.w):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vadd_WwWw_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Ww_vadd_WwWw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddwsat_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vsub(Vu32.w,Vv32.w):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vsub_VwVw_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vsub_VwVw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubwsat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vsub(Vuu32.w,Vvv32.w):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vsub_WwWw_sat(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Ww_vsub_WwWw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubwsat_dv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vavg(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vavg_VubVub(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vavg_VubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavgub)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vavg(Vu32.ub,Vv32.ub):rnd
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vavg_VubVub_rnd(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vavg_VubVub_rnd __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavgubrnd)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vavg(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vavg_VuhVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vavg_VuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavguh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vavg(Vu32.uh,Vv32.uh):rnd
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vavg_VuhVuh_rnd(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vavg_VuhVuh_rnd __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavguhrnd)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.uw=vavg(Vu32.uw,Vv32.uw)
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vavg_VuwVuw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuw_vavg_VuwVuw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavguw)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.uw=vavg(Vu32.uw,Vv32.uw):rnd
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vavg_VuwVuw_rnd(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuw_vavg_VuwVuw_rnd __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavguwrnd)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vavg(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vavg_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vavg_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavgb)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vavg(Vu32.b,Vv32.b):rnd
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vavg_VbVb_rnd(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vavg_VbVb_rnd __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavgbrnd)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vnavg(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vnavg_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vnavg_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vnavgb)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vavg(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vavg_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vavg_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavgh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vavg(Vu32.h,Vv32.h):rnd
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vavg_VhVh_rnd(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vavg_VhVh_rnd __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavghrnd)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vnavg(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vnavg_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vnavg_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vnavgh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vavg(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vavg_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vavg_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavgw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vavg(Vu32.w,Vv32.w):rnd
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vavg_VwVw_rnd(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vavg_VwVw_rnd __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vavgwrnd)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vnavg(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vnavg_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vnavg_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vnavgw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vabsdiff(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vabsdiff_VubVub(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vub_vabsdiff_VubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vabsdiffub)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vabsdiff(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vabsdiff_VuhVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vuh_vabsdiff_VuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vabsdiffuh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vabsdiff(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vabsdiff_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vuh_vabsdiff_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vabsdiffh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uw=vabsdiff(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vabsdiff_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vuw_vabsdiff_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vabsdiffw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vnavg(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vnavg_VubVub(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vnavg_VubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vnavgub)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 66
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vadd(Vu32.w,Vv32.w,Qs4):carry:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vadd_VwVwQ_carry_sat(HVX_Vector Vu, HVX_Vector Vv, HVX_VectorPred Qs)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vadd_VwVwQ_carry_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddcarrysat)
#endif /* __HVX_ARCH__ >= 66 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vadd(Vu32.w,Vv32.w,Qx4):carry
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vadd_VwVwQ_carry(HVX_Vector Vu, HVX_Vector Vv, HVX_VectorPred* Qp)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vadd_VwVwQ_carry __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddcarry)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vsub(Vu32.w,Vv32.w,Qx4):carry
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vsub_VwVwQ_carry(HVX_Vector Vu, HVX_Vector Vv, HVX_VectorPred* Qp)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vsub_VwVwQ_carry __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubcarry)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 66
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vsatdw(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vsatdw_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vsatdw_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsatdw)
#endif /* __HVX_ARCH__ >= 66 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vadd(Vu32.ub,Vv32.b):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vadd_VubVb_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vadd_VubVb_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddububb_sat)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vsub(Vu32.ub,Vv32.b):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vsub_VubVb_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vsub_VubVb_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubububb_sat)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vadd(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vadd_VubVub(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vadd_VubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddubh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vsub(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vsub_VubVub(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vsub_VubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsububh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vadd(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vadd_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vadd_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddhw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vsub(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vsub_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vsub_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubhw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vadd(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vadd_VuhVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vadd_VuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vadduhw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vsub(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vsub_VuhVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vsub_VuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubuhw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vxx32.w+=vadd(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vaddacc_WwVhVh(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vaddacc_WwVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddhw_acc)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vxx32.w+=vadd(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vaddacc_WwVuhVuh(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vaddacc_WwVuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vadduhw_acc)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vxx32.h+=vadd(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vaddacc_WhVubVub(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vaddacc_WhVubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddubh_acc)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=#0
   C Intrinsic Prototype: HVX_Vector Q6_V_vzero()
   Instruction Type:      MAPPING
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vzero __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vd0)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vdd32=#0
   C Intrinsic Prototype: HVX_VectorPair Q6_W_vzero()
   Instruction Type:      MAPPING
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_W_vzero __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdd0)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (Qv4) Vx32.b+=Vu32.b
   C Intrinsic Prototype: HVX_Vector Q6_Vb_condacc_QVbVb(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_condacc_QVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddbq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (Qv4) Vx32.b-=Vu32.b
   C Intrinsic Prototype: HVX_Vector Q6_Vb_condnac_QVbVb(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_condnac_QVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubbq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (!Qv4) Vx32.b+=Vu32.b
   C Intrinsic Prototype: HVX_Vector Q6_Vb_condacc_QnVbVb(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_condacc_QnVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddbnq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (!Qv4) Vx32.b-=Vu32.b
   C Intrinsic Prototype: HVX_Vector Q6_Vb_condnac_QnVbVb(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_condnac_QnVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubbnq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (Qv4) Vx32.h+=Vu32.h
   C Intrinsic Prototype: HVX_Vector Q6_Vh_condacc_QVhVh(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_condacc_QVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddhq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (Qv4) Vx32.h-=Vu32.h
   C Intrinsic Prototype: HVX_Vector Q6_Vh_condnac_QVhVh(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_condnac_QVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubhq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (!Qv4) Vx32.h+=Vu32.h
   C Intrinsic Prototype: HVX_Vector Q6_Vh_condacc_QnVhVh(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_condacc_QnVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddhnq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (!Qv4) Vx32.h-=Vu32.h
   C Intrinsic Prototype: HVX_Vector Q6_Vh_condnac_QnVhVh(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_condnac_QnVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubhnq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (Qv4) Vx32.w+=Vu32.w
   C Intrinsic Prototype: HVX_Vector Q6_Vw_condacc_QVwVw(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_condacc_QVwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddwq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (Qv4) Vx32.w-=Vu32.w
   C Intrinsic Prototype: HVX_Vector Q6_Vw_condnac_QVwVw(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_condnac_QVwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubwq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (!Qv4) Vx32.w+=Vu32.w
   C Intrinsic Prototype: HVX_Vector Q6_Vw_condacc_QnVwVw(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_condacc_QnVwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddwnq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       if (!Qv4) Vx32.w-=Vu32.w
   C Intrinsic Prototype: HVX_Vector Q6_Vw_condnac_QnVwVw(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_condnac_QnVwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsubwnq)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vabs(Vu32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vabs_Vb(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vabs_Vb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vabsb)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vabs(Vu32.b):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vabs_Vb_sat(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vabs_Vb_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vabsb_sat)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vabs(Vu32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vabs_Vh(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vabs_Vh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vabsh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vabs(Vu32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vabs_Vh_sat(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vabs_Vh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vabsh_sat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vabs(Vu32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vabs_Vw(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vabs_Vw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vabsw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vabs(Vu32.w):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vabs_Vw_sat(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vabs_Vw_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vabsw_sat)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vmpy(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpy_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpy_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpybv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.h+=vmpy(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpyacc_WhVbVb(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpyacc_WhVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpybv_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uh=vmpy(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuh_vmpy_VubVub(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuh_vmpy_VubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyubv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.uh+=vmpy(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuh_vmpyacc_WuhVubVub(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuh_vmpyacc_WuhVubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyubv_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vmpy(Vu32.ub,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpy_VubVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpy_VubVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpybusv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.h+=vmpy(Vu32.ub,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpyacc_WhVubVb(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpyacc_WhVubVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpybusv_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vmpa(Vuu32.ub,Vvv32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpa_WubWb(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpa_WubWb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpabusv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vmpa(Vuu32.ub,Vvv32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpa_WubWub(HVX_VectorPair Vuu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpa_WubWub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpabuuv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vmpy(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vmpy_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vmpy_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyhv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.w+=vmpy(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vmpyacc_WwVhVh(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vmpyacc_WwVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyhv_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uw=vmpy(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vmpy_VuhVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuw_vmpy_VuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyuhv)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.uw+=vmpy(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vmpyacc_WuwVuhVuh(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuw_vmpyacc_WuwVuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyuhv_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vmpy(Vu32.h,Vv32.h):<<1:rnd:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmpy_VhVh_s1_rnd_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vh_vmpy_VhVh_s1_rnd_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyhvsrs)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vmpy(Vu32.h,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vmpy_VhVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vmpy_VhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyhus)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.w+=vmpy(Vu32.h,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vmpyacc_WwVhVuh(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vmpyacc_WwVhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyhus_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vmpyi(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmpyi_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vh_vmpyi_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyih)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.h+=vmpyi(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmpyiacc_VhVhVh(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vh_vmpyiacc_VhVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyih_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vmpye(Vu32.w,Vv32.uh)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpye_VwVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpye_VwVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyewuh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vmpyo(Vu32.w,Vv32.h):<<1:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyo_VwVh_s1_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyo_VwVh_s1_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyowh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vmpyo(Vu32.w,Vv32.h):<<1:rnd:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyo_VwVh_s1_rnd_sat(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyo_VwVh_s1_rnd_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyowh_rnd)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vdd32=vmpye(Vu32.w,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_W_vmpye_VwVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_W_vmpye_VwVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyewuh_64)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vxx32+=vmpyo(Vu32.w,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_W_vmpyoacc_WVwVh(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_W_vmpyoacc_WVwVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyowh_64_acc)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vmpyo(Vu32.w,Vv32.h):<<1:sat:shift
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyoacc_VwVwVh_s1_sat_shift(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyoacc_VwVwVh_s1_sat_shift __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyowh_sacc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vmpyo(Vu32.w,Vv32.h):<<1:rnd:sat:shift
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyowh_rnd_sacc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vmpyieo(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyieo_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyieo_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyieoh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vmpyie(Vu32.w,Vv32.uh)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyie_VwVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyie_VwVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyiewuh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vmpyio(Vu32.w,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyio_VwVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyio_VwVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyiowh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vmpyie(Vu32.w,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyieacc_VwVwVh(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyieacc_VwVwVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyiewh_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vmpyie(Vu32.w,Vv32.uh)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyieacc_VwVwVuh(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyieacc_VwVwVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyiewuh_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uh=vmpy(Vu32.ub,Rt32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuh_vmpy_VubRub(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuh_vmpy_VubRub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyub)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.uh+=vmpy(Vu32.ub,Rt32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuh_vmpyacc_WuhVubRub(HVX_VectorPair Vxx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuh_vmpyacc_WuhVubRub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyub_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vmpy(Vu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpy_VubRb(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpy_VubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpybus)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.h+=vmpy(Vu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpyacc_WhVubRb(HVX_VectorPair Vxx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpyacc_WhVubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpybus_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vmpa(Vuu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpa_WubRb(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpa_WubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpabus)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.h+=vmpa(Vuu32.ub,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpaacc_WhWubRb(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpaacc_WhWubRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpabus_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vmpa(Vuu32.ub,Rt32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpa_WubRub(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpa_WubRub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpabuu)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vxx32.h+=vmpa(Vuu32.ub,Rt32.ub)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vmpaacc_WhWubRub(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wh_vmpaacc_WhWubRub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpabuu_acc)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vmpa(Vuu32.h,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vmpa_WhRb(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vmpa_WhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpahb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.w+=vmpa(Vuu32.h,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vmpaacc_WwWhRb(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vmpaacc_WwWhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpahb_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vmpa(Vuu32.uh,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vmpa_WuhRb(HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vmpa_WuhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpauhb)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vxx32.w+=vmpa(Vuu32.uh,Rt32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vmpaacc_WwWuhRb(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vmpaacc_WwWuhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpauhb_acc)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.w=vmpy(Vu32.h,Rt32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vmpy_VhRh(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vmpy_VhRh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vxx32.w+=vmpy(Vu32.h,Rt32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vmpyacc_WwVhRh(HVX_VectorPair Vxx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vmpyacc_WwVhRh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyh_acc)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.w+=vmpy(Vu32.h,Rt32.h):sat
   C Intrinsic Prototype: HVX_VectorPair Q6_Ww_vmpyacc_WwVhRh_sat(HVX_VectorPair Vxx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Ww_vmpyacc_WwVhRh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyhsat_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vmpy(Vu32.h,Rt32.h):<<1:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmpy_VhRh_s1_sat(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vh_vmpy_VhRh_s1_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyhss)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vmpy(Vu32.h,Rt32.h):<<1:rnd:sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmpy_VhRh_s1_rnd_sat(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vh_vmpy_VhRh_s1_rnd_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyhsrs)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.uw=vmpy(Vu32.uh,Rt32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vmpy_VuhRuh(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuw_vmpy_VuhRuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyuh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.uw+=vmpy(Vu32.uh,Rt32.uh)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wuw_vmpyacc_WuwVuhRuh(HVX_VectorPair Vxx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Wuw_vmpyacc_WuwVuhRuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyuh_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vmpyi(Vu32.h,Rt32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmpyi_VhRb(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vh_vmpyi_VhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyihb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.h+=vmpyi(Vu32.h,Rt32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmpyiacc_VhVhRb(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vh_vmpyiacc_VhVhRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyihb_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vmpyi(Vu32.w,Rt32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyi_VwRb(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyi_VwRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyiwb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vmpyi(Vu32.w,Rt32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyiacc_VwVwRb(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyiacc_VwVwRb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyiwb_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vmpyi(Vu32.w,Rt32.ub)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyi_VwRub(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyi_VwRub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyiwub)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vmpyi(Vu32.w,Rt32.ub)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyiacc_VwVwRub(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyiacc_VwVwRub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyiwub_acc)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vmpyi(Vu32.w,Rt32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyi_VwRh(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyi_VwRh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyiwh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w+=vmpyi(Vu32.w,Rt32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmpyiacc_VwVwRh(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vmpyiacc_VwVwRh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyiwh_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vand(Vu32,Vv32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vand_VV(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vand_VV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vand)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vor(Vu32,Vv32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vor_VV(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vor_VV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vxor(Vu32,Vv32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vxor_VV(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vxor_VV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vxor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vnot(Vu32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vnot_V(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vnot_V __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vnot)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vand(Qu4,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vand_QR(HVX_VectorPred Qu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_V_vand_QR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vandqrt)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32|=vand(Qu4,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vandor_VQR(HVX_Vector Vx, HVX_VectorPred Qu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_V_vandor_VQR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vandqrt_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32=vand(!Qu4,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vand_QnR(HVX_VectorPred Qu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_V_vand_QnR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vandnqrt)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vx32|=vand(!Qu4,Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vandor_VQnR(HVX_Vector Vx, HVX_VectorPred Qu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_V_vandor_VQnR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vandnqrt_acc)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=vand(Vu32,Rt32)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vand_VR(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Q_vand_VR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vandvrt)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4|=vand(Vu32,Rt32)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vandor_QVR(HVX_VectorPred Qx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Q_vandor_QVR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vandvrt_acc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32=vand(Qv4,Vu32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vand_QV(HVX_VectorPred Qv, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vand_QV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vandvqv)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32=vand(!Qv4,Vu32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vand_QnV(HVX_VectorPred Qv, HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vand_QnV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vandvnqv)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=vcmp.gt(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gt_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gt_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4&=vcmp.gt(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtand_QVwVw(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtand_QVwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtw_and)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4|=vcmp.gt(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtor_QVwVw(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtor_QVwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtw_or)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4^=vcmp.gt(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtxacc_QVwVw(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtxacc_QVwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtw_xor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=vcmp.eq(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eq_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eq_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4&=vcmp.eq(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eqand_QVwVw(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eqand_QVwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqw_and)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4|=vcmp.eq(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eqor_QVwVw(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eqor_QVwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqw_or)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4^=vcmp.eq(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eqxacc_QVwVw(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eqxacc_QVwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqw_xor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=vcmp.gt(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gt_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gt_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgth)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4&=vcmp.gt(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtand_QVhVh(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtand_QVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgth_and)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4|=vcmp.gt(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtor_QVhVh(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtor_QVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgth_or)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4^=vcmp.gt(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtxacc_QVhVh(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtxacc_QVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgth_xor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=vcmp.eq(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eq_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eq_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4&=vcmp.eq(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eqand_QVhVh(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eqand_QVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqh_and)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4|=vcmp.eq(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eqor_QVhVh(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eqor_QVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqh_or)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4^=vcmp.eq(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eqxacc_QVhVh(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eqxacc_QVhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqh_xor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=vcmp.gt(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gt_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gt_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4&=vcmp.gt(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtand_QVbVb(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtand_QVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtb_and)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4|=vcmp.gt(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtor_QVbVb(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtor_QVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtb_or)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4^=vcmp.gt(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtxacc_QVbVb(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtxacc_QVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtb_xor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=vcmp.eq(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eq_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eq_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4&=vcmp.eq(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eqand_QVbVb(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eqand_QVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqb_and)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4|=vcmp.eq(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eqor_QVbVb(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eqor_QVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqb_or)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4^=vcmp.eq(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_eqxacc_QVbVb(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_eqxacc_QVbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_veqb_xor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=vcmp.gt(Vu32.uw,Vv32.uw)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gt_VuwVuw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gt_VuwVuw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtuw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4&=vcmp.gt(Vu32.uw,Vv32.uw)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtand_QVuwVuw(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtand_QVuwVuw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtuw_and)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4|=vcmp.gt(Vu32.uw,Vv32.uw)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtor_QVuwVuw(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtor_QVuwVuw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtuw_or)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4^=vcmp.gt(Vu32.uw,Vv32.uw)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtxacc_QVuwVuw(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtxacc_QVuwVuw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtuw_xor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=vcmp.gt(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gt_VuhVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gt_VuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtuh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4&=vcmp.gt(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtand_QVuhVuh(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtand_QVuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtuh_and)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4|=vcmp.gt(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtor_QVuhVuh(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtor_QVuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtuh_or)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4^=vcmp.gt(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtxacc_QVuhVuh(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtxacc_QVuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtuh_xor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=vcmp.gt(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gt_VubVub(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gt_VubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtub)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4&=vcmp.gt(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtand_QVubVub(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtand_QVubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtub_and)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4|=vcmp.gt(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtor_QVubVub(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtor_QVubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtub_or)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qx4^=vcmp.gt(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vcmp_gtxacc_QVubVub(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vcmp_gtxacc_QVubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgtub_xor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=vsetq(Rt32)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vsetq_R(Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vsetq_R __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_pred_scalar2)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Qd4=vsetq2(Rt32)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_vsetq2_R(Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_vsetq2_R __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_pred_scalar2v2)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Qd4.h=vshuffe(Qs4.w,Qt4.w)
   C Intrinsic Prototype: HVX_VectorPred Q6_Qh_vshuffe_QwQw(HVX_VectorPred Qs, HVX_VectorPred Qt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Qh_vshuffe_QwQw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_shuffeqw)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Qd4.b=vshuffe(Qs4.h,Qt4.h)
   C Intrinsic Prototype: HVX_VectorPred Q6_Qb_vshuffe_QhQh(HVX_VectorPred Qs, HVX_VectorPred Qt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Qb_vshuffe_QhQh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_shuffeqh)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=or(Qs4,Qt4)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_or_QQ(HVX_VectorPred Qs, HVX_VectorPred Qt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_or_QQ __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_pred_or)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=and(Qs4,Qt4)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_and_QQ(HVX_VectorPred Qs, HVX_VectorPred Qt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_and_QQ __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_pred_and)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=xor(Qs4,Qt4)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_xor_QQ(HVX_VectorPred Qs, HVX_VectorPred Qt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_xor_QQ __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_pred_xor)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=or(Qs4,!Qt4)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_or_QQn(HVX_VectorPred Qs, HVX_VectorPred Qt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_or_QQn __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_pred_or_n)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=and(Qs4,!Qt4)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_and_QQn(HVX_VectorPred Qs, HVX_VectorPred Qt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_and_QQn __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_pred_and_n)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Qd4=not(Qs4)
   C Intrinsic Prototype: HVX_VectorPred Q6_Q_not_Q(HVX_VectorPred Qs)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Q_not_Q __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_pred_not)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vmux(Qt4,Vu32,Vv32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vmux_QVV(HVX_VectorPred Qt, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vmux_QVV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmux)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32=vswap(Qt4,Vu32,Vv32)
   C Intrinsic Prototype: HVX_VectorPair Q6_W_vswap_QVV(HVX_VectorPred Qt, HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_W_vswap_QVV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vswap)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vmax(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vmax_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vmax_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmaxb)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vmin(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vmin_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vmin_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vminb)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vmax(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vmax_VubVub(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vmax_VubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmaxub)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vmin(Vu32.ub,Vv32.ub)
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vmin_VubVub(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vmin_VubVub __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vminub)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vmax(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vmax_VuhVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vmax_VuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmaxuh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vmin(Vu32.uh,Vv32.uh)
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vmin_VuhVuh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vmin_VuhVuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vminuh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vmax(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmax_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vmax_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmaxh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vmin(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmin_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vmin_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vminh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vmax(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmax_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vmax_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmaxw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vmin(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vmin_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vmin_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vminw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.ub=vsat(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vub_vsat_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vub_vsat_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsathub)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vsat(Vu32.w,Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vsat_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vsat_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsatwh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vsat(Vu32.uw,Vv32.uw)
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vsat_VuwVuw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vsat_VuwVuw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vsatuwuh)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vshuffe(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vshuffe_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vshuffe_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vshuffeb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vshuffo(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vshuffo_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vshuffo_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vshuffob)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vshuffe(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vshuffe_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vshuffe_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vshufeh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vshuffo(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vshuffo_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vshuffo_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vshufoh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32=vshuff(Vu32,Vv32,Rt8)
   C Intrinsic Prototype: HVX_VectorPair Q6_W_vshuff_VVR(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_W_vshuff_VVR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vshuffvdd)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32=vdeal(Vu32,Vv32,Rt8)
   C Intrinsic Prototype: HVX_VectorPair Q6_W_vdeal_VVR(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_W_vdeal_VVR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdealvdd)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vshuffoe(Vu32.h,Vv32.h)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vshuffoe_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vshuffoe_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vshufoeh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.b=vshuffoe(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wb_vshuffoe_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wb_vshuffoe_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vshufoeb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vdeal(Vu32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vdeal_Vh(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vdeal_Vh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdealh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vdeal(Vu32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vdeal_Vb(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vdeal_Vb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdealb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vdeale(Vu32.b,Vv32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vdeale_VbVb(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vdeale_VbVb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdealb4w)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vshuff(Vu32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vshuff_Vh(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vshuff_Vh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vshuffh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vshuff(Vu32.b)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vshuff_Vb(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vshuff_Vb __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vshuffb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Rd32=vextract(Vu32,Rs32)
   C Intrinsic Prototype: Word32 Q6_R_vextract_VR(HVX_Vector Vu, Word32 Rs)
   Instruction Type:      LD
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_R_vextract_VR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_extractw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.w=vinsert(Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vinsert_VwR(HVX_Vector Vx, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vw_vinsert_VwR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vinsertwr)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vsplat(Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vsplat_R(Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_V_vsplat_R __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_lvsplatw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vsplat(Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vsplat_R(Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vh_vsplat_R __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_lvsplath)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vsplat(Rt32)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vsplat_R(Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vb_vsplat_R __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_lvsplatb)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32=Vuu32
   C Intrinsic Prototype: HVX_VectorPair Q6_W_equals_W(HVX_VectorPair Vuu)
   Instruction Type:      MAPPING
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_W_equals_W __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vassignp)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=Vu32
   C Intrinsic Prototype: HVX_Vector Q6_V_equals_V(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_equals_V __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vassign)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32=vcombine(Vu32,Vv32)
   C Intrinsic Prototype: HVX_VectorPair Q6_W_vcombine_VV(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_W_vcombine_VV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vcombine)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vdelta(Vu32,Vv32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vdelta_VV(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vdelta_VV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vdelta)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=vrdelta(Vu32,Vv32)
   C Intrinsic Prototype: HVX_Vector Q6_V_vrdelta_VV(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_vrdelta_VV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vrdelta)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uw=vcl0(Vu32.uw)
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vcl0_Vuw(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuw_vcl0_Vuw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vcl0w)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.uh=vcl0(Vu32.uh)
   C Intrinsic Prototype: HVX_Vector Q6_Vuh_vcl0_Vuh(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vuh_vcl0_Vuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vcl0h)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vnormamt(Vu32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vnormamt_Vw(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vnormamt_Vw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vnormamtw)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vnormamt(Vu32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vnormamt_Vh(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vnormamt_Vh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vnormamth)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.w=vadd(vclb(Vu32.w),Vv32.w)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_vadd_vclb_VwVw(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_vadd_vclb_VwVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddclbw)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vadd(vclb(Vu32.h),Vv32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vadd_vclb_VhVh(HVX_Vector Vu, HVX_Vector Vv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vadd_vclb_VhVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vaddclbh)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vpopcount(Vu32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vpopcount_Vh(HVX_Vector Vu)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_vpopcount_Vh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vpopcounth)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vlut32(Vu32.b,Vv32.b,Rt8)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vlut32_VbVbR(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vlut32_VbVbR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlutvvb)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vx32.b|=vlut32(Vu32.b,Vv32.b,Rt8)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vlut32or_VbVbVbR(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vlut32or_VbVbVbR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlutvvb_oracc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vlut16(Vu32.b,Vv32.h,Rt8)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vlut16_VbVhR(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vlut16_VbVhR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlutvwh)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vxx32.h|=vlut16(Vu32.b,Vv32.h,Rt8)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vlut16or_WhVbVhR(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vlut16or_WhVbVhR __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlutvwh_oracc)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vlut32(Vu32.b,Vv32.b,#u3)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vlut32_VbVbI(HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vlut32_VbVbI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlutvvbi)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vx32.b|=vlut32(Vu32.b,Vv32.b,#u3)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vlut32or_VbVbVbI(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vlut32or_VbVbVbI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlutvvb_oracci)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vlut16(Vu32.b,Vv32.h,#u3)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vlut16_VbVhI(HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vlut16_VbVhI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlutvwhi)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vxx32.h|=vlut16(Vu32.b,Vv32.h,#u3)
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vlut16or_WhVbVhI(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vlut16or_WhVbVhI __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlutvwh_oracci)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vd32.b=vlut32(Vu32.b,Vv32.b,Rt8):nomatch
   C Intrinsic Prototype: HVX_Vector Q6_Vb_vlut32_VbVbR_nomatch(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_vlut32_VbVbR_nomatch __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlutvvb_nm)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 62
/* ==========================================================================
   Assembly Syntax:       Vdd32.h=vlut16(Vu32.b,Vv32.h,Rt8):nomatch
   C Intrinsic Prototype: HVX_VectorPair Q6_Wh_vlut16_VbVhR_nomatch(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Wh_vlut16_VbVhR_nomatch __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlutvwh_nm)
#endif /* __HVX_ARCH__ >= 62 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vx32.h=vmpa(Vx32.h,Vu32.h,Rtt32.h):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmpa_VhVhVhPh_sat(HVX_Vector Vx, HVX_Vector Vu, Word64 Rtt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT2
   ========================================================================== */

#define Q6_Vh_vmpa_VhVhVhPh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpahhsat)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vx32.h=vmpa(Vx32.h,Vu32.uh,Rtt32.uh):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmpa_VhVhVuhPuh_sat(HVX_Vector Vx, HVX_Vector Vu, Word64 Rtt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT2
   ========================================================================== */

#define Q6_Vh_vmpa_VhVhVuhPuh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpauhuhsat)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vx32.h=vmps(Vx32.h,Vu32.uh,Rtt32.uh):sat
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vmps_VhVhVuhPuh_sat(HVX_Vector Vx, HVX_Vector Vu, Word64 Rtt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT2
   ========================================================================== */

#define Q6_Vh_vmps_VhVhVuhPuh_sat __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpsuhuhsat)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.h=vlut4(Vu32.uh,Rtt32.h)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_vlut4_VuhPh(HVX_Vector Vu, Word64 Rtt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT2
   ========================================================================== */

#define Q6_Vh_vlut4_VuhPh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vlut4)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.uw=vmpye(Vu32.uh,Rt32.uh)
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vmpye_VuhRuh(HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vuw_vmpye_VuhRuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyuhe)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vx32.uw+=vmpye(Vu32.uh,Rt32.uh)
   C Intrinsic Prototype: HVX_Vector Q6_Vuw_vmpyeacc_VuwVuhRuh(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT23
   ========================================================================== */

#define Q6_Vuw_vmpyeacc_VuwVuhRuh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vmpyuhe_acc)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=hi(Vss32)
   C Intrinsic Prototype: HVX_Vector Q6_V_hi_W(HVX_VectorPair Vss)
   Instruction Type:      MAPPING
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_hi_W __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_hi)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 60
/* ==========================================================================
   Assembly Syntax:       Vd32=lo(Vss32)
   C Intrinsic Prototype: HVX_Vector Q6_V_lo_W(HVX_VectorPair Vss)
   Instruction Type:      MAPPING
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_V_lo_W __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_lo)
#endif /* __HVX_ARCH__ >= 60 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       vtmp.w=vgather(Rt32,Mu2,Vv32.w).w
   C Intrinsic Prototype: void Q6_vgather_ARMVw(HVX_Vector* A, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT1
   ========================================================================== */

#define Q6_vgather_ARMVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgathermw)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       vtmp.h=vgather(Rt32,Mu2,Vv32.h).h
   C Intrinsic Prototype: void Q6_vgather_ARMVh(HVX_Vector* A, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT1
   ========================================================================== */

#define Q6_vgather_ARMVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgathermh)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       vtmp.h=vgather(Rt32,Mu2,Vvv32.w).h
   C Intrinsic Prototype: void Q6_vgather_ARMWw(HVX_Vector* A, HVX_Vector* Rb, Word32 Mu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT1
   ========================================================================== */

#define Q6_vgather_ARMWw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgathermhw)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       if (Qs4) vtmp.w=vgather(Rt32,Mu2,Vv32.w).w
   C Intrinsic Prototype: void Q6_vgather_AQRMVw(HVX_Vector* A, HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT1
   ========================================================================== */

#define Q6_vgather_AQRMVw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgathermwq)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       if (Qs4) vtmp.h=vgather(Rt32,Mu2,Vv32.h).h
   C Intrinsic Prototype: void Q6_vgather_AQRMVh(HVX_Vector* A, HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT1
   ========================================================================== */

#define Q6_vgather_AQRMVh __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgathermhq)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       if (Qs4) vtmp.h=vgather(Rt32,Mu2,Vvv32.w).h
   C Intrinsic Prototype: void Q6_vgather_AQRMWw(HVX_Vector* A, HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_VectorPair Vvv)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT1
   ========================================================================== */

#define Q6_vgather_AQRMWw __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vgathermhwq)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       vscatter(Rt32,Mu2,Vv32.w).w=Vw32
   C Intrinsic Prototype: void Q6_vscatter_RMVwV(HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vscatter_RMVwV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vscattermw)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       vscatter(Rt32,Mu2,Vv32.h).h=Vw32
   C Intrinsic Prototype: void Q6_vscatter_RMVhV(HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vscatter_RMVhV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vscattermh)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       vscatter(Rt32,Mu2,Vv32.w).w+=Vw32
   C Intrinsic Prototype: void Q6_vscatteracc_RMVwV(HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vscatteracc_RMVwV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vscattermw_add)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       vscatter(Rt32,Mu2,Vv32.h).h+=Vw32
   C Intrinsic Prototype: void Q6_vscatteracc_RMVhV(HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vscatteracc_RMVhV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vscattermh_add)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       if (Qs4) vscatter(Rt32,Mu2,Vv32.w).w=Vw32
   C Intrinsic Prototype: void Q6_vscatter_QRMVwV(HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vscatter_QRMVwV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vscattermwq)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       if (Qs4) vscatter(Rt32,Mu2,Vv32.h).h=Vw32
   C Intrinsic Prototype: void Q6_vscatter_QRMVhV(HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vscatter_QRMVhV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vscattermhq)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       vscatter(Rt32,Mu2,Vvv32.w).h=Vw32
   C Intrinsic Prototype: void Q6_vscatter_RMWwV(HVX_Vector* Rb, Word32 Mu, HVX_VectorPair Vvv, HVX_Vector Vw)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vscatter_RMWwV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vscattermhw)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       if (Qs4) vscatter(Rt32,Mu2,Vvv32.w).h=Vw32
   C Intrinsic Prototype: void Q6_vscatter_QRMWwV(HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_VectorPair Vvv, HVX_Vector Vw)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vscatter_QRMWwV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vscattermhwq)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       vscatter(Rt32,Mu2,Vvv32.w).h+=Vw32
   C Intrinsic Prototype: void Q6_vscatteracc_RMWwV(HVX_Vector* Rb, Word32 Mu, HVX_VectorPair Vvv, HVX_Vector Vw)
   Instruction Type:      COPROC_VMEM
   Execution Slots:       SLOT0
   ========================================================================== */

#define Q6_vscatteracc_RMWwV __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vscattermhw_add)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.b=prefixsum(Qv4)
   C Intrinsic Prototype: HVX_Vector Q6_Vb_prefixsum_Q(HVX_VectorPred Qv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vb_prefixsum_Q __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vprefixqb)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.h=prefixsum(Qv4)
   C Intrinsic Prototype: HVX_Vector Q6_Vh_prefixsum_Q(HVX_VectorPred Qv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vh_prefixsum_Q __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vprefixqh)
#endif /* __HVX_ARCH__ >= 65 */

#if __HVX_ARCH__ >= 65
/* ==========================================================================
   Assembly Syntax:       Vd32.w=prefixsum(Qv4)
   C Intrinsic Prototype: HVX_Vector Q6_Vw_prefixsum_Q(HVX_VectorPred Qv)
   Instruction Type:      COPROC_VX
   Execution Slots:       SLOT0123
   ========================================================================== */

#define Q6_Vw_prefixsum_Q __BUILTIN_VECTOR_WRAP(__builtin_HEXAGON_V6_vprefixqw)
#endif /* __HVX_ARCH__ >= 65 */

#endif /* __HVX__ */

#endif
