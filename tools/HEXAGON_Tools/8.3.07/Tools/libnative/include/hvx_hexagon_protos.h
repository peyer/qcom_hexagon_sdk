
/* This file contains functions that emulate Hexagon intrinsics.
    Only include this file if you are using a compiler that does not generate
    Hexagon assembly */

#ifndef __HVX_HEXAGON_PROTOS_HEADER_
#define __HVX_HEXAGON_PROTOS_HEADER_ 1

#ifndef __hexagon__

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include "hexagon_circ_brev_intrinsics.h"
#include "hexagon_protos.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __HVXDBL__
 #define MAX_VEC_SIZE_LOGBYTES 7
 #define __HVXDBL_EXTENSION(a) a ## _HVXDBL
#else
 #define MAX_VEC_SIZE_LOGBYTES 6
 #define __HVXDBL_EXTENSION(a) a
#endif

#define MAX_VEC_SIZE_BYTES  (1<<MAX_VEC_SIZE_LOGBYTES)

#define NUM_ZREGS           8
#define NUM_VREGS           32
#define NUM_QREGS           4

typedef uint32_t ZRegMask; // at least NUM_ZREGS bits
typedef uint32_t VRegMask; // at least NUM_VREGS bits
typedef uint32_t QRegMask; // at least NUM_QREGS bits

typedef enum {
    HEXAGON_VECTOR_WAIT    = 0,
    HEXAGON_VECTOR_NO_WAIT = 1,
    HEXAGON_VECTOR_CHECK   = 2
} hexagon_vector_wait_t;

#define SIM_ACQUIRE_HVX __HVXDBL_EXTENSION(acquire_vector_unit)(HEXAGON_VECTOR_WAIT)
#define release_vector_unit()
#define SIM_RELEASE_HVX release_vector_unit()
#define set_double_vector_mode()
#define SIM_SET_HVX_DOUBLE_MODE set_double_vector_mode();
#define clear_double_vector_mode()
#define SIM_CLEAR_HVX_DOUBLE_MODE clear_double_vector_mode();
#define mem_vector_gather_init(X1, X2, X3, X4, X5)
#define mem_vector_gather_finish(X1, X2)
#define mem_vector_scatter_init(X1, X2, X3, X4, X5)
#define mem_vector_scatter_finish(X1, X2, X3)
#define sim_mem_read1(X1, X2, X3) 0
#define LOG_VTCM_BYTE(X1, X2, X3, X4)
#define LOG_VTCM_BANK(X1, X2, X3)

#define MAX_VEC_SIZE_BYTES  (1<<MAX_VEC_SIZE_LOGBYTES)

typedef union {
    size8u_t ud[MAX_VEC_SIZE_BYTES/8];
    size8s_t    d[MAX_VEC_SIZE_BYTES/8];
    size4u_t uw[MAX_VEC_SIZE_BYTES/4];
    size4s_t    w[MAX_VEC_SIZE_BYTES/4];
    size2u_t uh[MAX_VEC_SIZE_BYTES/2];
    size2s_t    h[MAX_VEC_SIZE_BYTES/2];
    size1u_t ub[MAX_VEC_SIZE_BYTES/1];
    size1s_t    b[MAX_VEC_SIZE_BYTES/1];
    size4s_t    f32[MAX_VEC_SIZE_BYTES/4];
} mmvector_t, mmqreg_t;

typedef union {
    size8u_t ud[2*MAX_VEC_SIZE_BYTES/8];
    size8s_t    d[2*MAX_VEC_SIZE_BYTES/8];
    size4u_t uw[2*MAX_VEC_SIZE_BYTES/4];
    size4s_t    w[2*MAX_VEC_SIZE_BYTES/4];
    size2u_t uh[2*MAX_VEC_SIZE_BYTES/2];
    size2s_t    h[2*MAX_VEC_SIZE_BYTES/2];
    size1u_t ub[2*MAX_VEC_SIZE_BYTES/1];
    size1s_t    b[2*MAX_VEC_SIZE_BYTES/1];
    mmvector_t v[2];
} mmvector_pair_t;


typedef union {
    size8u_t ud[4*MAX_VEC_SIZE_BYTES/8];
    size8s_t  d[4*MAX_VEC_SIZE_BYTES/8];
    size4u_t uw[4*MAX_VEC_SIZE_BYTES/4];
    size4s_t  w[4*MAX_VEC_SIZE_BYTES/4];
    size2u_t uh[4*MAX_VEC_SIZE_BYTES/2];
    size2s_t  h[4*MAX_VEC_SIZE_BYTES/2];
    size1u_t ub[4*MAX_VEC_SIZE_BYTES/1];
    size1s_t  b[4*MAX_VEC_SIZE_BYTES/1];
    mmvector_t v[4];
} mmvector_quad_t;

#define DECL_EXT_VREG(VAR) mmvector_t VAR;
#define DECL_EXT_VREG_PAIR(VAR) mmvector_pair_t VAR;
#define DECL_EXT_QREG(VAR) mmqreg_t VAR;
#define DECL_EXT_VREG_QUAD(VAR) mmvector_quad_t VAR;

#ifdef __HVXDBL__
 typedef mmqreg_t        HEXAGON_VecPred128;
 typedef mmvector_t      HEXAGON_Vect1024;
 typedef mmvector_pair_t HEXAGON_Vect2048;
 #define HVX_VectorPred  HEXAGON_VecPred128
 #define HVX_Vector      HEXAGON_Vect1024
 #define HVX_VectorPair  HEXAGON_Vect2048
#else
 typedef mmqreg_t        HEXAGON_VecPred64;
 typedef mmvector_t      HEXAGON_Vect512;
 typedef mmvector_pair_t HEXAGON_Vect1024;
 #define HVX_VectorPred  HEXAGON_VecPred64
 #define HVX_Vector      HEXAGON_Vect512
 #define HVX_VectorPair  HEXAGON_Vect1024
#endif /*HVXDBL*/

#define HVX_VecPred       HVX_VectorPred
#define HVX_VectorAddress HVX_Vector*


typedef struct {
    paddr_t pa;
    int size;
    mmvector_t mask;
    mmvector_t data;
} vstorelog_t;

typedef struct {
    mmvector_t data;
    mmvector_t mask;
    mmvector_pair_t offsets;
    int size;
    paddr_t pa_base;
    vaddr_t va_base;
    paddr_t pa[MAX_VEC_SIZE_BYTES];
    int oob_access;
    int op;
    int op_size;
} vtcm_storelog_t;


typedef struct {
    int num;
    int size;
    int slot;
    int type;
    paddr_t pa[MAX_VEC_SIZE_BYTES];
    union {
        size1u_t b[MAX_VEC_SIZE_BYTES];
        size2u_t h[MAX_VEC_SIZE_BYTES/2];
        size4u_t w[MAX_VEC_SIZE_BYTES/4];
    };
} vtcm_storelog_verif_t;

typedef enum {
  SG_WARN_RAW_POISON=0,
  SG_WARN_WAR_POISON=1,
  SG_WARN_WAW_POISON=2,
  SG_SYNC=3,
  SG_DESYNC=4,
  SG_READ_POISON=5,
  SG_WRITE_POISON=6,
  SG_RDWR_POISON=7,
  SG_DEPOISON=8,
  COUNT_SG_EVENTS=9
} sg_event_type_e;

typedef struct {
    int tnum;
    vaddr_t pc;
    paddr_t pa;
    sg_event_type_e event;
} scaga_callback_info_t;

typedef struct bytes_list_t bytes_list_t;
struct bytes_list_t {
  paddr_t paddr;
  size1u_t rw; // 0=sync, 1=R, 2=W, 3=RW
  int ct;
  bytes_list_t *next;
  bytes_list_t *prev;
};

typedef struct {
    bytes_list_t *bytes[2];
    bytes_list_t *first_byte[2];
    bytes_list_t *last_byte[2];
    size4u_t byte_count[2];
    scaga_callback_info_t scaga_info;
} vtcm_state_t;

typedef struct MMVecxState {
    mmvector_t VRegs[NUM_VREGS];
    mmvector_t future_VRegs[NUM_VREGS];
    mmvector_t tmp_VRegs[NUM_VREGS];
	mmvector_t ZRegs[NUM_ZREGS];

    VRegMask   VRegs_updated_tmp;
    VRegMask   VRegs_updated;
    VRegMask   VRegs_select;
    mmqreg_t QRegs[NUM_QREGS];
    mmqreg_t future_QRegs[NUM_QREGS];
    QRegMask   QRegs_updated;
    QRegMask   QRegs_select;
    vstorelog_t vstore[2];
    size1u_t vstore_pending[2];
    size1u_t gather_issued;
    size1u_t vtcm_pending;
    vtcm_storelog_t vtcm_log;
#ifdef VERIFICATION
    vtcm_storelog_verif_t vtcm_storelog_verif;
#endif

    vtcm_state_t * vtcm_state_ptr;
} mmvecx_t;

struct Instruction {
    unsigned slot:3;
};

typedef struct Instruction insn_t;
struct ThreadState;
typedef struct ThreadState thread_t;
typedef struct mem_access_info {
    paddr_t paddr;
} mem_access_info_t;
struct ThreadState {
    unsigned Regs[32 + 32 + 16 + 48 + 32];
    struct SystemState *system_ptr;
    unsigned threadId;
    int status;
    mem_access_info_t mem_access[4];
};

#define EXEC_STATUS_EXCEPTION   0x0100
#define EXCEPTION_DETECTED 0

static inline mmvecx_t *mmvec_thread_to_struct(thread_t *thread)
{
    static mmvecx_t mmvecx;
    return &mmvecx;
}
#define THREAD2STRUCT mmvec_thread_to_struct(thread)

#define Q6_vmaskedstoreq_QAV    Q6_vmem_QRIV
#define Q6_vmaskedstorenq_QAV   Q6_vmem_QnRIV
#define Q6_vmaskedstorentq_QAV  Q6_vmem_QRIV_nt
#define Q6_vmaskedstorentnq_QAV Q6_vmem_QnRIV_nt

#define Q6_vmem_QRIV __HVXDBL_EXTENSION(Q6_vmem_QRIV)
void __HVXDBL_EXTENSION(Q6_vmem_QRIV)(HVX_VectorPred Qv, HVX_Vector* A, HVX_Vector Vs);
#define Q6_vmem_QnRIV __HVXDBL_EXTENSION(Q6_vmem_QnRIV)
void __HVXDBL_EXTENSION(Q6_vmem_QnRIV)(HVX_VectorPred Qv, HVX_Vector* A, HVX_Vector Vs);
#define Q6_vmem_QRIV_nt __HVXDBL_EXTENSION(Q6_vmem_QRIV_nt)
void __HVXDBL_EXTENSION(Q6_vmem_QRIV_nt)(HVX_VectorPred Qv, HVX_Vector* A, HVX_Vector Vs);
#define Q6_vmem_QnRIV_nt __HVXDBL_EXTENSION(Q6_vmem_QnRIV_nt)
void __HVXDBL_EXTENSION(Q6_vmem_QnRIV_nt)(HVX_VectorPred Qv, HVX_Vector* A, HVX_Vector Vs);
#define Q6_V_valign_VVR __HVXDBL_EXTENSION(Q6_V_valign_VVR)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_valign_VVR)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_V_vlalign_VVR __HVXDBL_EXTENSION(Q6_V_vlalign_VVR)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vlalign_VVR)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_V_valign_VVI __HVXDBL_EXTENSION(Q6_V_valign_VVI)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_valign_VVI)(HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3);
#define Q6_V_vlalign_VVI __HVXDBL_EXTENSION(Q6_V_vlalign_VVI)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vlalign_VVI)(HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3);
#define Q6_V_vror_VR __HVXDBL_EXTENSION(Q6_V_vror_VR)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vror_VR)(HVX_Vector Vu, Word32 Rt);
#define Q6_Wuh_vunpack_Vub __HVXDBL_EXTENSION(Q6_Wuh_vunpack_Vub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuh_vunpack_Vub)(HVX_Vector Vu);
#define Q6_Wh_vunpack_Vb __HVXDBL_EXTENSION(Q6_Wh_vunpack_Vb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vunpack_Vb)(HVX_Vector Vu);
#define Q6_Wuw_vunpack_Vuh __HVXDBL_EXTENSION(Q6_Wuw_vunpack_Vuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vunpack_Vuh)(HVX_Vector Vu);
#define Q6_Ww_vunpack_Vh __HVXDBL_EXTENSION(Q6_Ww_vunpack_Vh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vunpack_Vh)(HVX_Vector Vu);
#define Q6_Wh_vunpackoor_WhVb __HVXDBL_EXTENSION(Q6_Wh_vunpackoor_WhVb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vunpackoor_WhVb)(HVX_VectorPair Vxx, HVX_Vector Vu);
#define Q6_Ww_vunpackoor_WwVh __HVXDBL_EXTENSION(Q6_Ww_vunpackoor_WwVh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vunpackoor_WwVh)(HVX_VectorPair Vxx, HVX_Vector Vu);
#define Q6_Vb_vpacke_VhVh __HVXDBL_EXTENSION(Q6_Vb_vpacke_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vpacke_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vpacke_VwVw __HVXDBL_EXTENSION(Q6_Vh_vpacke_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vpacke_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vpacko_VhVh __HVXDBL_EXTENSION(Q6_Vb_vpacko_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vpacko_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vpacko_VwVw __HVXDBL_EXTENSION(Q6_Vh_vpacko_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vpacko_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vub_vpack_VhVh_sat __HVXDBL_EXTENSION(Q6_Vub_vpack_VhVh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vpack_VhVh_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vpack_VhVh_sat __HVXDBL_EXTENSION(Q6_Vb_vpack_VhVh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vpack_VhVh_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuh_vpack_VwVw_sat __HVXDBL_EXTENSION(Q6_Vuh_vpack_VwVw_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vpack_VwVw_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vpack_VwVw_sat __HVXDBL_EXTENSION(Q6_Vh_vpack_VwVw_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vpack_VwVw_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuh_vzxt_Vub __HVXDBL_EXTENSION(Q6_Wuh_vzxt_Vub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuh_vzxt_Vub)(HVX_Vector Vu);
#define Q6_Wh_vsxt_Vb __HVXDBL_EXTENSION(Q6_Wh_vsxt_Vb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vsxt_Vb)(HVX_Vector Vu);
#define Q6_Wuw_vzxt_Vuh __HVXDBL_EXTENSION(Q6_Wuw_vzxt_Vuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vzxt_Vuh)(HVX_Vector Vu);
#define Q6_Ww_vsxt_Vh __HVXDBL_EXTENSION(Q6_Ww_vsxt_Vh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vsxt_Vh)(HVX_Vector Vu);
#define Q6_Vh_vdmpy_VubRb __HVXDBL_EXTENSION(Q6_Vh_vdmpy_VubRb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vdmpy_VubRb)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vh_vdmpyacc_VhVubRb __HVXDBL_EXTENSION(Q6_Vh_vdmpyacc_VhVubRb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vdmpyacc_VhVubRb)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Wh_vdmpy_WubRb __HVXDBL_EXTENSION(Q6_Wh_vdmpy_WubRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vdmpy_WubRb)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Wh_vdmpyacc_WhWubRb __HVXDBL_EXTENSION(Q6_Wh_vdmpyacc_WhWubRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vdmpyacc_WhWubRb)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Vw_vdmpy_VhRb __HVXDBL_EXTENSION(Q6_Vw_vdmpy_VhRb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpy_VhRb)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vdmpyacc_VwVhRb __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwVhRb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwVhRb)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Ww_vdmpy_WhRb __HVXDBL_EXTENSION(Q6_Ww_vdmpy_WhRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vdmpy_WhRb)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Ww_vdmpyacc_WwWhRb __HVXDBL_EXTENSION(Q6_Ww_vdmpyacc_WwWhRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vdmpyacc_WwWhRb)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Vw_vdmpy_VhVh_sat __HVXDBL_EXTENSION(Q6_Vw_vdmpy_VhVh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpy_VhVh_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vdmpyacc_VwVhVh_sat __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwVhVh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwVhVh_sat)(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vdmpy_VhRh_sat __HVXDBL_EXTENSION(Q6_Vw_vdmpy_VhRh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpy_VhRh_sat)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vdmpyacc_VwVhRh_sat __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwVhRh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwVhRh_sat)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vdmpy_WhRh_sat __HVXDBL_EXTENSION(Q6_Vw_vdmpy_WhRh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpy_WhRh_sat)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Vw_vdmpyacc_VwWhRh_sat __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwWhRh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwWhRh_sat)(HVX_Vector Vx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Vw_vdmpy_VhRuh_sat __HVXDBL_EXTENSION(Q6_Vw_vdmpy_VhRuh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpy_VhRuh_sat)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vdmpyacc_VwVhRuh_sat __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwVhRuh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwVhRuh_sat)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vdmpy_WhRuh_sat __HVXDBL_EXTENSION(Q6_Vw_vdmpy_WhRuh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpy_WhRuh_sat)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Vw_vdmpyacc_VwWhRuh_sat __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwWhRuh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vdmpyacc_VwWhRuh_sat)(HVX_Vector Vx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Wh_vtmpy_WbRb __HVXDBL_EXTENSION(Q6_Wh_vtmpy_WbRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vtmpy_WbRb)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Wh_vtmpyacc_WhWbRb __HVXDBL_EXTENSION(Q6_Wh_vtmpyacc_WhWbRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vtmpyacc_WhWbRb)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Wh_vtmpy_WubRb __HVXDBL_EXTENSION(Q6_Wh_vtmpy_WubRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vtmpy_WubRb)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Wh_vtmpyacc_WhWubRb __HVXDBL_EXTENSION(Q6_Wh_vtmpyacc_WhWubRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vtmpyacc_WhWubRb)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Ww_vtmpy_WhRb __HVXDBL_EXTENSION(Q6_Ww_vtmpy_WhRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vtmpy_WhRb)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Ww_vtmpyacc_WwWhRb __HVXDBL_EXTENSION(Q6_Ww_vtmpyacc_WwWhRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vtmpyacc_WwWhRb)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Vuw_vrmpy_VubRub __HVXDBL_EXTENSION(Q6_Vuw_vrmpy_VubRub)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vrmpy_VubRub)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vuw_vrmpyacc_VuwVubRub __HVXDBL_EXTENSION(Q6_Vuw_vrmpyacc_VuwVubRub)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vrmpyacc_VuwVubRub)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vuw_vrmpy_VubVub __HVXDBL_EXTENSION(Q6_Vuw_vrmpy_VubVub)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vrmpy_VubVub)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuw_vrmpyacc_VuwVubVub __HVXDBL_EXTENSION(Q6_Vuw_vrmpyacc_VuwVubVub)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vrmpyacc_VuwVubVub)(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vrmpy_VbVb __HVXDBL_EXTENSION(Q6_Vw_vrmpy_VbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vrmpy_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vrmpyacc_VwVbVb __HVXDBL_EXTENSION(Q6_Vw_vrmpyacc_VwVbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vrmpyacc_VwVbVb)(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuw_vrmpy_WubRubI __HVXDBL_EXTENSION(Q6_Wuw_vrmpy_WubRubI)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vrmpy_WubRubI)(HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1);
#define Q6_Wuw_vrmpyacc_WuwWubRubI __HVXDBL_EXTENSION(Q6_Wuw_vrmpyacc_WuwWubRubI)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vrmpyacc_WuwWubRubI)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1);
#define Q6_Vw_vrmpy_VubRb __HVXDBL_EXTENSION(Q6_Vw_vrmpy_VubRb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vrmpy_VubRb)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vrmpyacc_VwVubRb __HVXDBL_EXTENSION(Q6_Vw_vrmpyacc_VwVubRb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vrmpyacc_VwVubRb)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Ww_vrmpy_WubRbI __HVXDBL_EXTENSION(Q6_Ww_vrmpy_WubRbI)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vrmpy_WubRbI)(HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1);
#define Q6_Ww_vrmpyacc_WwWubRbI __HVXDBL_EXTENSION(Q6_Ww_vrmpyacc_WwWubRbI)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vrmpyacc_WwWubRbI)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1);
#define Q6_Vw_vrmpy_VubVb __HVXDBL_EXTENSION(Q6_Vw_vrmpy_VubVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vrmpy_VubVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vrmpyacc_VwVubVb __HVXDBL_EXTENSION(Q6_Vw_vrmpyacc_VwVubVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vrmpyacc_VwVubVb)(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuw_vdsad_WuhRuh __HVXDBL_EXTENSION(Q6_Wuw_vdsad_WuhRuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vdsad_WuhRuh)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Wuw_vdsadacc_WuwWuhRuh __HVXDBL_EXTENSION(Q6_Wuw_vdsadacc_WuwWuhRuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vdsadacc_WuwWuhRuh)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Wuw_vrsad_WubRubI __HVXDBL_EXTENSION(Q6_Wuw_vrsad_WubRubI)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vrsad_WubRubI)(HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1);
#define Q6_Wuw_vrsadacc_WuwWubRubI __HVXDBL_EXTENSION(Q6_Wuw_vrsadacc_WuwWubRubI)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vrsadacc_WuwWubRubI)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt, Word32 Iu1);
#define Q6_Vw_vasr_VwR __HVXDBL_EXTENSION(Q6_Vw_vasr_VwR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vasr_VwR)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vasl_VwR __HVXDBL_EXTENSION(Q6_Vw_vasl_VwR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vasl_VwR)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vuw_vlsr_VuwR __HVXDBL_EXTENSION(Q6_Vuw_vlsr_VuwR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vlsr_VuwR)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vasr_VwVw __HVXDBL_EXTENSION(Q6_Vw_vasr_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vasr_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vasl_VwVw __HVXDBL_EXTENSION(Q6_Vw_vasl_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vasl_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vlsr_VwVw __HVXDBL_EXTENSION(Q6_Vw_vlsr_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vlsr_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vasr_VhR __HVXDBL_EXTENSION(Q6_Vh_vasr_VhR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vasr_VhR)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vh_vasl_VhR __HVXDBL_EXTENSION(Q6_Vh_vasl_VhR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vasl_VhR)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vuh_vlsr_VuhR __HVXDBL_EXTENSION(Q6_Vuh_vlsr_VuhR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vlsr_VuhR)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vh_vasr_VhVh __HVXDBL_EXTENSION(Q6_Vh_vasr_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vasr_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vasl_VhVh __HVXDBL_EXTENSION(Q6_Vh_vasl_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vasl_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vlsr_VhVh __HVXDBL_EXTENSION(Q6_Vh_vlsr_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vlsr_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vub_vlsr_VubR __HVXDBL_EXTENSION(Q6_Vub_vlsr_VubR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vlsr_VubR)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vuw_vrotr_VuwVuw __HVXDBL_EXTENSION(Q6_Vuw_vrotr_VuwVuw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vrotr_VuwVuw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vasrinto_WwVwVw __HVXDBL_EXTENSION(Q6_Ww_vasrinto_WwVwVw)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vasrinto_WwVwVw)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vasr_VwVwR __HVXDBL_EXTENSION(Q6_Vh_vasr_VwVwR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vasr_VwVwR)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vh_vasr_VwVwR_sat __HVXDBL_EXTENSION(Q6_Vh_vasr_VwVwR_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vasr_VwVwR_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vh_vasr_VwVwR_rnd_sat __HVXDBL_EXTENSION(Q6_Vh_vasr_VwVwR_rnd_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vasr_VwVwR_rnd_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vuh_vasr_VwVwR_rnd_sat __HVXDBL_EXTENSION(Q6_Vuh_vasr_VwVwR_rnd_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vasr_VwVwR_rnd_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vuh_vasr_VwVwR_sat __HVXDBL_EXTENSION(Q6_Vuh_vasr_VwVwR_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vasr_VwVwR_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vuh_vasr_VuwVuwR_rnd_sat __HVXDBL_EXTENSION(Q6_Vuh_vasr_VuwVuwR_rnd_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vasr_VuwVuwR_rnd_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vuh_vasr_VuwVuwR_sat __HVXDBL_EXTENSION(Q6_Vuh_vasr_VuwVuwR_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vasr_VuwVuwR_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vub_vasr_VhVhR_sat __HVXDBL_EXTENSION(Q6_Vub_vasr_VhVhR_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vasr_VhVhR_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vub_vasr_VhVhR_rnd_sat __HVXDBL_EXTENSION(Q6_Vub_vasr_VhVhR_rnd_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vasr_VhVhR_rnd_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vb_vasr_VhVhR_sat __HVXDBL_EXTENSION(Q6_Vb_vasr_VhVhR_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vasr_VhVhR_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vb_vasr_VhVhR_rnd_sat __HVXDBL_EXTENSION(Q6_Vb_vasr_VhVhR_rnd_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vasr_VhVhR_rnd_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vub_vasr_VuhVuhR_sat __HVXDBL_EXTENSION(Q6_Vub_vasr_VuhVuhR_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vasr_VuhVuhR_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vub_vasr_VuhVuhR_rnd_sat __HVXDBL_EXTENSION(Q6_Vub_vasr_VuhVuhR_rnd_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vasr_VuhVuhR_rnd_sat)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vh_vround_VwVw_sat __HVXDBL_EXTENSION(Q6_Vh_vround_VwVw_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vround_VwVw_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuh_vround_VwVw_sat __HVXDBL_EXTENSION(Q6_Vuh_vround_VwVw_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vround_VwVw_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuh_vround_VuwVuw_sat __HVXDBL_EXTENSION(Q6_Vuh_vround_VuwVuw_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vround_VuwVuw_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vround_VhVh_sat __HVXDBL_EXTENSION(Q6_Vb_vround_VhVh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vround_VhVh_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vub_vround_VhVh_sat __HVXDBL_EXTENSION(Q6_Vub_vround_VhVh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vround_VhVh_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vub_vround_VuhVuh_sat __HVXDBL_EXTENSION(Q6_Vub_vround_VuhVuh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vround_VuhVuh_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vaslacc_VwVwR __HVXDBL_EXTENSION(Q6_Vw_vaslacc_VwVwR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vaslacc_VwVwR)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vasracc_VwVwR __HVXDBL_EXTENSION(Q6_Vw_vasracc_VwVwR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vasracc_VwVwR)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vh_vaslacc_VhVhR __HVXDBL_EXTENSION(Q6_Vh_vaslacc_VhVhR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vaslacc_VhVhR)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vh_vasracc_VhVhR __HVXDBL_EXTENSION(Q6_Vh_vasracc_VhVhR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vasracc_VhVhR)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vb_vadd_VbVb __HVXDBL_EXTENSION(Q6_Vb_vadd_VbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vadd_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vsub_VbVb __HVXDBL_EXTENSION(Q6_Vb_vsub_VbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vsub_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wb_vadd_WbWb __HVXDBL_EXTENSION(Q6_Wb_vadd_WbWb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wb_vadd_WbWb)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Wb_vsub_WbWb __HVXDBL_EXTENSION(Q6_Wb_vsub_WbWb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wb_vsub_WbWb)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vh_vadd_VhVh __HVXDBL_EXTENSION(Q6_Vh_vadd_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vadd_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vsub_VhVh __HVXDBL_EXTENSION(Q6_Vh_vsub_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vsub_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wh_vadd_WhWh __HVXDBL_EXTENSION(Q6_Wh_vadd_WhWh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vadd_WhWh)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Wh_vsub_WhWh __HVXDBL_EXTENSION(Q6_Wh_vsub_WhWh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vsub_WhWh)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vw_vadd_VwVw __HVXDBL_EXTENSION(Q6_Vw_vadd_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vadd_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vsub_VwVw __HVXDBL_EXTENSION(Q6_Vw_vsub_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vsub_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vadd_WwWw __HVXDBL_EXTENSION(Q6_Ww_vadd_WwWw)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vadd_WwWw)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Ww_vsub_WwWw __HVXDBL_EXTENSION(Q6_Ww_vsub_WwWw)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vsub_WwWw)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vub_vadd_VubVub_sat __HVXDBL_EXTENSION(Q6_Vub_vadd_VubVub_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vadd_VubVub_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wub_vadd_WubWub_sat __HVXDBL_EXTENSION(Q6_Wub_vadd_WubWub_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wub_vadd_WubWub_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vub_vsub_VubVub_sat __HVXDBL_EXTENSION(Q6_Vub_vsub_VubVub_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vsub_VubVub_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wub_vsub_WubWub_sat __HVXDBL_EXTENSION(Q6_Wub_vsub_WubWub_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wub_vsub_WubWub_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vuh_vadd_VuhVuh_sat __HVXDBL_EXTENSION(Q6_Vuh_vadd_VuhVuh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vadd_VuhVuh_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuh_vadd_WuhWuh_sat __HVXDBL_EXTENSION(Q6_Wuh_vadd_WuhWuh_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuh_vadd_WuhWuh_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vuh_vsub_VuhVuh_sat __HVXDBL_EXTENSION(Q6_Vuh_vsub_VuhVuh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vsub_VuhVuh_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuh_vsub_WuhWuh_sat __HVXDBL_EXTENSION(Q6_Wuh_vsub_WuhWuh_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuh_vsub_WuhWuh_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vuw_vadd_VuwVuw_sat __HVXDBL_EXTENSION(Q6_Vuw_vadd_VuwVuw_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vadd_VuwVuw_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuw_vadd_WuwWuw_sat __HVXDBL_EXTENSION(Q6_Wuw_vadd_WuwWuw_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vadd_WuwWuw_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vuw_vsub_VuwVuw_sat __HVXDBL_EXTENSION(Q6_Vuw_vsub_VuwVuw_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vsub_VuwVuw_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuw_vsub_WuwWuw_sat __HVXDBL_EXTENSION(Q6_Wuw_vsub_WuwWuw_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vsub_WuwWuw_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vb_vadd_VbVb_sat __HVXDBL_EXTENSION(Q6_Vb_vadd_VbVb_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vadd_VbVb_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wb_vadd_WbWb_sat __HVXDBL_EXTENSION(Q6_Wb_vadd_WbWb_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wb_vadd_WbWb_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vb_vsub_VbVb_sat __HVXDBL_EXTENSION(Q6_Vb_vsub_VbVb_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vsub_VbVb_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wb_vsub_WbWb_sat __HVXDBL_EXTENSION(Q6_Wb_vsub_WbWb_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wb_vsub_WbWb_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vh_vadd_VhVh_sat __HVXDBL_EXTENSION(Q6_Vh_vadd_VhVh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vadd_VhVh_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wh_vadd_WhWh_sat __HVXDBL_EXTENSION(Q6_Wh_vadd_WhWh_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vadd_WhWh_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vh_vsub_VhVh_sat __HVXDBL_EXTENSION(Q6_Vh_vsub_VhVh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vsub_VhVh_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wh_vsub_WhWh_sat __HVXDBL_EXTENSION(Q6_Wh_vsub_WhWh_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vsub_WhWh_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vw_vadd_VwVw_sat __HVXDBL_EXTENSION(Q6_Vw_vadd_VwVw_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vadd_VwVw_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vadd_WwWw_sat __HVXDBL_EXTENSION(Q6_Ww_vadd_WwWw_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vadd_WwWw_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vw_vsub_VwVw_sat __HVXDBL_EXTENSION(Q6_Vw_vsub_VwVw_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vsub_VwVw_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vsub_WwWw_sat __HVXDBL_EXTENSION(Q6_Ww_vsub_WwWw_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vsub_WwWw_sat)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Vub_vavg_VubVub __HVXDBL_EXTENSION(Q6_Vub_vavg_VubVub)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vavg_VubVub)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vub_vavg_VubVub_rnd __HVXDBL_EXTENSION(Q6_Vub_vavg_VubVub_rnd)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vavg_VubVub_rnd)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuh_vavg_VuhVuh __HVXDBL_EXTENSION(Q6_Vuh_vavg_VuhVuh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vavg_VuhVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuh_vavg_VuhVuh_rnd __HVXDBL_EXTENSION(Q6_Vuh_vavg_VuhVuh_rnd)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vavg_VuhVuh_rnd)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuw_vavg_VuwVuw __HVXDBL_EXTENSION(Q6_Vuw_vavg_VuwVuw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vavg_VuwVuw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuw_vavg_VuwVuw_rnd __HVXDBL_EXTENSION(Q6_Vuw_vavg_VuwVuw_rnd)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vavg_VuwVuw_rnd)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vavg_VbVb __HVXDBL_EXTENSION(Q6_Vb_vavg_VbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vavg_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vavg_VbVb_rnd __HVXDBL_EXTENSION(Q6_Vb_vavg_VbVb_rnd)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vavg_VbVb_rnd)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vnavg_VbVb __HVXDBL_EXTENSION(Q6_Vb_vnavg_VbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vnavg_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vavg_VhVh __HVXDBL_EXTENSION(Q6_Vh_vavg_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vavg_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vavg_VhVh_rnd __HVXDBL_EXTENSION(Q6_Vh_vavg_VhVh_rnd)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vavg_VhVh_rnd)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vnavg_VhVh __HVXDBL_EXTENSION(Q6_Vh_vnavg_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vnavg_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vavg_VwVw __HVXDBL_EXTENSION(Q6_Vw_vavg_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vavg_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vavg_VwVw_rnd __HVXDBL_EXTENSION(Q6_Vw_vavg_VwVw_rnd)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vavg_VwVw_rnd)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vnavg_VwVw __HVXDBL_EXTENSION(Q6_Vw_vnavg_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vnavg_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vub_vabsdiff_VubVub __HVXDBL_EXTENSION(Q6_Vub_vabsdiff_VubVub)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vabsdiff_VubVub)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuh_vabsdiff_VuhVuh __HVXDBL_EXTENSION(Q6_Vuh_vabsdiff_VuhVuh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vabsdiff_VuhVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuh_vabsdiff_VhVh __HVXDBL_EXTENSION(Q6_Vuh_vabsdiff_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vabsdiff_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuw_vabsdiff_VwVw __HVXDBL_EXTENSION(Q6_Vuw_vabsdiff_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vabsdiff_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vnavg_VubVub __HVXDBL_EXTENSION(Q6_Vb_vnavg_VubVub)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vnavg_VubVub)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vadd_VwVwQ_carry_sat __HVXDBL_EXTENSION(Q6_Vw_vadd_VwVwQ_carry_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vadd_VwVwQ_carry_sat)(HVX_Vector Vu, HVX_Vector Vv, HVX_VectorPred Qs);
#define Q6_Vw_vadd_VwVwQ_carry __HVXDBL_EXTENSION(Q6_Vw_vadd_VwVwQ_carry)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vadd_VwVwQ_carry)(HVX_Vector Vu, HVX_Vector Vv, HVX_VectorPred* Qp);
#define Q6_Vw_vsub_VwVwQ_carry __HVXDBL_EXTENSION(Q6_Vw_vsub_VwVwQ_carry)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vsub_VwVwQ_carry)(HVX_Vector Vu, HVX_Vector Vv, HVX_VectorPred* Qp);
#define Q6_Vw_vsatdw_VwVw __HVXDBL_EXTENSION(Q6_Vw_vsatdw_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vsatdw_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vub_vadd_VubVb_sat __HVXDBL_EXTENSION(Q6_Vub_vadd_VubVb_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vadd_VubVb_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vub_vsub_VubVb_sat __HVXDBL_EXTENSION(Q6_Vub_vsub_VubVb_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vsub_VubVb_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wh_vadd_VubVub __HVXDBL_EXTENSION(Q6_Wh_vadd_VubVub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vadd_VubVub)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wh_vsub_VubVub __HVXDBL_EXTENSION(Q6_Wh_vsub_VubVub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vsub_VubVub)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vadd_VhVh __HVXDBL_EXTENSION(Q6_Ww_vadd_VhVh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vadd_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vsub_VhVh __HVXDBL_EXTENSION(Q6_Ww_vsub_VhVh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vsub_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vadd_VuhVuh __HVXDBL_EXTENSION(Q6_Ww_vadd_VuhVuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vadd_VuhVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vsub_VuhVuh __HVXDBL_EXTENSION(Q6_Ww_vsub_VuhVuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vsub_VuhVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vaddacc_WwVhVh __HVXDBL_EXTENSION(Q6_Ww_vaddacc_WwVhVh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vaddacc_WwVhVh)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vaddacc_WwVuhVuh __HVXDBL_EXTENSION(Q6_Ww_vaddacc_WwVuhVuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vaddacc_WwVuhVuh)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wh_vaddacc_WhVubVub __HVXDBL_EXTENSION(Q6_Wh_vaddacc_WhVubVub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vaddacc_WhVubVub)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_V_vzero __HVXDBL_EXTENSION(Q6_V_vzero)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vzero)();
#define Q6_W_vzero __HVXDBL_EXTENSION(Q6_W_vzero)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_W_vzero)();
#define Q6_Vb_condacc_QVbVb __HVXDBL_EXTENSION(Q6_Vb_condacc_QVbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_condacc_QVbVb)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vb_condnac_QVbVb __HVXDBL_EXTENSION(Q6_Vb_condnac_QVbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_condnac_QVbVb)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vb_condacc_QnVbVb __HVXDBL_EXTENSION(Q6_Vb_condacc_QnVbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_condacc_QnVbVb)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vb_condnac_QnVbVb __HVXDBL_EXTENSION(Q6_Vb_condnac_QnVbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_condnac_QnVbVb)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vh_condacc_QVhVh __HVXDBL_EXTENSION(Q6_Vh_condacc_QVhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_condacc_QVhVh)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vh_condnac_QVhVh __HVXDBL_EXTENSION(Q6_Vh_condnac_QVhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_condnac_QVhVh)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vh_condacc_QnVhVh __HVXDBL_EXTENSION(Q6_Vh_condacc_QnVhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_condacc_QnVhVh)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vh_condnac_QnVhVh __HVXDBL_EXTENSION(Q6_Vh_condnac_QnVhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_condnac_QnVhVh)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vw_condacc_QVwVw __HVXDBL_EXTENSION(Q6_Vw_condacc_QVwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_condacc_QVwVw)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vw_condnac_QVwVw __HVXDBL_EXTENSION(Q6_Vw_condnac_QVwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_condnac_QVwVw)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vw_condacc_QnVwVw __HVXDBL_EXTENSION(Q6_Vw_condacc_QnVwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_condacc_QnVwVw)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vw_condnac_QnVwVw __HVXDBL_EXTENSION(Q6_Vw_condnac_QnVwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_condnac_QnVwVw)(HVX_VectorPred Qv, HVX_Vector Vx, HVX_Vector Vu);
#define Q6_Vb_vabs_Vb __HVXDBL_EXTENSION(Q6_Vb_vabs_Vb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vabs_Vb)(HVX_Vector Vu);
#define Q6_Vb_vabs_Vb_sat __HVXDBL_EXTENSION(Q6_Vb_vabs_Vb_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vabs_Vb_sat)(HVX_Vector Vu);
#define Q6_Vh_vabs_Vh __HVXDBL_EXTENSION(Q6_Vh_vabs_Vh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vabs_Vh)(HVX_Vector Vu);
#define Q6_Vh_vabs_Vh_sat __HVXDBL_EXTENSION(Q6_Vh_vabs_Vh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vabs_Vh_sat)(HVX_Vector Vu);
#define Q6_Vw_vabs_Vw __HVXDBL_EXTENSION(Q6_Vw_vabs_Vw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vabs_Vw)(HVX_Vector Vu);
#define Q6_Vw_vabs_Vw_sat __HVXDBL_EXTENSION(Q6_Vw_vabs_Vw_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vabs_Vw_sat)(HVX_Vector Vu);
#define Q6_Wh_vmpy_VbVb __HVXDBL_EXTENSION(Q6_Wh_vmpy_VbVb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpy_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wh_vmpyacc_WhVbVb __HVXDBL_EXTENSION(Q6_Wh_vmpyacc_WhVbVb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpyacc_WhVbVb)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuh_vmpy_VubVub __HVXDBL_EXTENSION(Q6_Wuh_vmpy_VubVub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuh_vmpy_VubVub)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuh_vmpyacc_WuhVubVub __HVXDBL_EXTENSION(Q6_Wuh_vmpyacc_WuhVubVub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuh_vmpyacc_WuhVubVub)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wh_vmpy_VubVb __HVXDBL_EXTENSION(Q6_Wh_vmpy_VubVb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpy_VubVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wh_vmpyacc_WhVubVb __HVXDBL_EXTENSION(Q6_Wh_vmpyacc_WhVubVb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpyacc_WhVubVb)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wh_vmpa_WubWb __HVXDBL_EXTENSION(Q6_Wh_vmpa_WubWb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpa_WubWb)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Wh_vmpa_WubWub __HVXDBL_EXTENSION(Q6_Wh_vmpa_WubWub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpa_WubWub)(HVX_VectorPair Vuu, HVX_VectorPair Vvv);
#define Q6_Ww_vmpy_VhVh __HVXDBL_EXTENSION(Q6_Ww_vmpy_VhVh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vmpy_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vmpyacc_WwVhVh __HVXDBL_EXTENSION(Q6_Ww_vmpyacc_WwVhVh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vmpyacc_WwVhVh)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuw_vmpy_VuhVuh __HVXDBL_EXTENSION(Q6_Wuw_vmpy_VuhVuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vmpy_VuhVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuw_vmpyacc_WuwVuhVuh __HVXDBL_EXTENSION(Q6_Wuw_vmpyacc_WuwVuhVuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vmpyacc_WuwVuhVuh)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vmpy_VhVh_s1_rnd_sat __HVXDBL_EXTENSION(Q6_Vh_vmpy_VhVh_s1_rnd_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmpy_VhVh_s1_rnd_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vmpy_VhVuh __HVXDBL_EXTENSION(Q6_Ww_vmpy_VhVuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vmpy_VhVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Ww_vmpyacc_WwVhVuh __HVXDBL_EXTENSION(Q6_Ww_vmpyacc_WwVhVuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vmpyacc_WwVhVuh)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vmpyi_VhVh __HVXDBL_EXTENSION(Q6_Vh_vmpyi_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmpyi_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vmpyiacc_VhVhVh __HVXDBL_EXTENSION(Q6_Vh_vmpyiacc_VhVhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmpyiacc_VhVhVh)(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmpye_VwVuh __HVXDBL_EXTENSION(Q6_Vw_vmpye_VwVuh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpye_VwVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmpyo_VwVh_s1_sat __HVXDBL_EXTENSION(Q6_Vw_vmpyo_VwVh_s1_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyo_VwVh_s1_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmpyo_VwVh_s1_rnd_sat __HVXDBL_EXTENSION(Q6_Vw_vmpyo_VwVh_s1_rnd_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyo_VwVh_s1_rnd_sat)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_W_vmpye_VwVuh __HVXDBL_EXTENSION(Q6_W_vmpye_VwVuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_W_vmpye_VwVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_W_vmpyoacc_WVwVh __HVXDBL_EXTENSION(Q6_W_vmpyoacc_WVwVh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_W_vmpyoacc_WVwVh)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmpyoacc_VwVwVh_s1_sat_shift __HVXDBL_EXTENSION(Q6_Vw_vmpyoacc_VwVwVh_s1_sat_shift)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyoacc_VwVwVh_s1_sat_shift)(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift __HVXDBL_EXTENSION(Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift)(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmpyieo_VhVh __HVXDBL_EXTENSION(Q6_Vw_vmpyieo_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyieo_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmpyie_VwVuh __HVXDBL_EXTENSION(Q6_Vw_vmpyie_VwVuh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyie_VwVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmpyio_VwVh __HVXDBL_EXTENSION(Q6_Vw_vmpyio_VwVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyio_VwVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmpyieacc_VwVwVh __HVXDBL_EXTENSION(Q6_Vw_vmpyieacc_VwVwVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyieacc_VwVwVh)(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmpyieacc_VwVwVuh __HVXDBL_EXTENSION(Q6_Vw_vmpyieacc_VwVwVuh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyieacc_VwVwVuh)(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wuh_vmpy_VubRub __HVXDBL_EXTENSION(Q6_Wuh_vmpy_VubRub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuh_vmpy_VubRub)(HVX_Vector Vu, Word32 Rt);
#define Q6_Wuh_vmpyacc_WuhVubRub __HVXDBL_EXTENSION(Q6_Wuh_vmpyacc_WuhVubRub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuh_vmpyacc_WuhVubRub)(HVX_VectorPair Vxx, HVX_Vector Vu, Word32 Rt);
#define Q6_Wh_vmpy_VubRb __HVXDBL_EXTENSION(Q6_Wh_vmpy_VubRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpy_VubRb)(HVX_Vector Vu, Word32 Rt);
#define Q6_Wh_vmpyacc_WhVubRb __HVXDBL_EXTENSION(Q6_Wh_vmpyacc_WhVubRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpyacc_WhVubRb)(HVX_VectorPair Vxx, HVX_Vector Vu, Word32 Rt);
#define Q6_Wh_vmpa_WubRb __HVXDBL_EXTENSION(Q6_Wh_vmpa_WubRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpa_WubRb)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Wh_vmpaacc_WhWubRb __HVXDBL_EXTENSION(Q6_Wh_vmpaacc_WhWubRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpaacc_WhWubRb)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Wh_vmpa_WubRub __HVXDBL_EXTENSION(Q6_Wh_vmpa_WubRub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpa_WubRub)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Wh_vmpaacc_WhWubRub __HVXDBL_EXTENSION(Q6_Wh_vmpaacc_WhWubRub)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vmpaacc_WhWubRub)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Ww_vmpa_WhRb __HVXDBL_EXTENSION(Q6_Ww_vmpa_WhRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vmpa_WhRb)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Ww_vmpaacc_WwWhRb __HVXDBL_EXTENSION(Q6_Ww_vmpaacc_WwWhRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vmpaacc_WwWhRb)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Ww_vmpa_WuhRb __HVXDBL_EXTENSION(Q6_Ww_vmpa_WuhRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vmpa_WuhRb)(HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Ww_vmpaacc_WwWuhRb __HVXDBL_EXTENSION(Q6_Ww_vmpaacc_WwWuhRb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vmpaacc_WwWuhRb)(HVX_VectorPair Vxx, HVX_VectorPair Vuu, Word32 Rt);
#define Q6_Ww_vmpy_VhRh __HVXDBL_EXTENSION(Q6_Ww_vmpy_VhRh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vmpy_VhRh)(HVX_Vector Vu, Word32 Rt);
#define Q6_Ww_vmpyacc_WwVhRh __HVXDBL_EXTENSION(Q6_Ww_vmpyacc_WwVhRh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vmpyacc_WwVhRh)(HVX_VectorPair Vxx, HVX_Vector Vu, Word32 Rt);
#define Q6_Ww_vmpyacc_WwVhRh_sat __HVXDBL_EXTENSION(Q6_Ww_vmpyacc_WwVhRh_sat)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Ww_vmpyacc_WwVhRh_sat)(HVX_VectorPair Vxx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vh_vmpy_VhRh_s1_sat __HVXDBL_EXTENSION(Q6_Vh_vmpy_VhRh_s1_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmpy_VhRh_s1_sat)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vh_vmpy_VhRh_s1_rnd_sat __HVXDBL_EXTENSION(Q6_Vh_vmpy_VhRh_s1_rnd_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmpy_VhRh_s1_rnd_sat)(HVX_Vector Vu, Word32 Rt);
#define Q6_Wuw_vmpy_VuhRuh __HVXDBL_EXTENSION(Q6_Wuw_vmpy_VuhRuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vmpy_VuhRuh)(HVX_Vector Vu, Word32 Rt);
#define Q6_Wuw_vmpyacc_WuwVuhRuh __HVXDBL_EXTENSION(Q6_Wuw_vmpyacc_WuwVuhRuh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wuw_vmpyacc_WuwVuhRuh)(HVX_VectorPair Vxx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vh_vmpyi_VhRb __HVXDBL_EXTENSION(Q6_Vh_vmpyi_VhRb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmpyi_VhRb)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vh_vmpyiacc_VhVhRb __HVXDBL_EXTENSION(Q6_Vh_vmpyiacc_VhVhRb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmpyiacc_VhVhRb)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vmpyi_VwRb __HVXDBL_EXTENSION(Q6_Vw_vmpyi_VwRb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyi_VwRb)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vmpyiacc_VwVwRb __HVXDBL_EXTENSION(Q6_Vw_vmpyiacc_VwVwRb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyiacc_VwVwRb)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vmpyi_VwRub __HVXDBL_EXTENSION(Q6_Vw_vmpyi_VwRub)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyi_VwRub)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vmpyiacc_VwVwRub __HVXDBL_EXTENSION(Q6_Vw_vmpyiacc_VwVwRub)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyiacc_VwVwRub)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vmpyi_VwRh __HVXDBL_EXTENSION(Q6_Vw_vmpyi_VwRh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyi_VwRh)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vw_vmpyiacc_VwVwRh __HVXDBL_EXTENSION(Q6_Vw_vmpyiacc_VwVwRh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmpyiacc_VwVwRh)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_V_vand_VV __HVXDBL_EXTENSION(Q6_V_vand_VV)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vand_VV)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_V_vor_VV __HVXDBL_EXTENSION(Q6_V_vor_VV)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vor_VV)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_V_vxor_VV __HVXDBL_EXTENSION(Q6_V_vxor_VV)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vxor_VV)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_V_vnot_V __HVXDBL_EXTENSION(Q6_V_vnot_V)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vnot_V)(HVX_Vector Vu);
#define Q6_V_vand_QR __HVXDBL_EXTENSION(Q6_V_vand_QR)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vand_QR)(HVX_VectorPred Qu, Word32 Rt);
#define Q6_V_vandor_VQR __HVXDBL_EXTENSION(Q6_V_vandor_VQR)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vandor_VQR)(HVX_Vector Vx, HVX_VectorPred Qu, Word32 Rt);
#define Q6_V_vand_QnR __HVXDBL_EXTENSION(Q6_V_vand_QnR)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vand_QnR)(HVX_VectorPred Qu, Word32 Rt);
#define Q6_V_vandor_VQnR __HVXDBL_EXTENSION(Q6_V_vandor_VQnR)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vandor_VQnR)(HVX_Vector Vx, HVX_VectorPred Qu, Word32 Rt);
#define Q6_Q_vand_VR __HVXDBL_EXTENSION(Q6_Q_vand_VR)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vand_VR)(HVX_Vector Vu, Word32 Rt);
#define Q6_Q_vandor_QVR __HVXDBL_EXTENSION(Q6_Q_vandor_QVR)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vandor_QVR)(HVX_VectorPred Qx, HVX_Vector Vu, Word32 Rt);
#define Q6_V_vand_QV __HVXDBL_EXTENSION(Q6_V_vand_QV)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vand_QV)(HVX_VectorPred Qv, HVX_Vector Vu);
#define Q6_V_vand_QnV __HVXDBL_EXTENSION(Q6_V_vand_QnV)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vand_QnV)(HVX_VectorPred Qv, HVX_Vector Vu);
#define Q6_Q_vcmp_gt_VwVw __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VwVw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtand_QVwVw __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVwVw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVwVw)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtor_QVwVw __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVwVw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVwVw)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtxacc_QVwVw __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVwVw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVwVw)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eq_VwVw __HVXDBL_EXTENSION(Q6_Q_vcmp_eq_VwVw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eq_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eqand_QVwVw __HVXDBL_EXTENSION(Q6_Q_vcmp_eqand_QVwVw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eqand_QVwVw)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eqor_QVwVw __HVXDBL_EXTENSION(Q6_Q_vcmp_eqor_QVwVw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eqor_QVwVw)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eqxacc_QVwVw __HVXDBL_EXTENSION(Q6_Q_vcmp_eqxacc_QVwVw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eqxacc_QVwVw)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gt_VhVh __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VhVh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtand_QVhVh __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVhVh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVhVh)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtor_QVhVh __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVhVh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVhVh)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtxacc_QVhVh __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVhVh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVhVh)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eq_VhVh __HVXDBL_EXTENSION(Q6_Q_vcmp_eq_VhVh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eq_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eqand_QVhVh __HVXDBL_EXTENSION(Q6_Q_vcmp_eqand_QVhVh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eqand_QVhVh)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eqor_QVhVh __HVXDBL_EXTENSION(Q6_Q_vcmp_eqor_QVhVh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eqor_QVhVh)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eqxacc_QVhVh __HVXDBL_EXTENSION(Q6_Q_vcmp_eqxacc_QVhVh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eqxacc_QVhVh)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gt_VbVb __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VbVb)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtand_QVbVb __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVbVb)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVbVb)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtor_QVbVb __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVbVb)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVbVb)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtxacc_QVbVb __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVbVb)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVbVb)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eq_VbVb __HVXDBL_EXTENSION(Q6_Q_vcmp_eq_VbVb)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eq_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eqand_QVbVb __HVXDBL_EXTENSION(Q6_Q_vcmp_eqand_QVbVb)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eqand_QVbVb)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eqor_QVbVb __HVXDBL_EXTENSION(Q6_Q_vcmp_eqor_QVbVb)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eqor_QVbVb)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_eqxacc_QVbVb __HVXDBL_EXTENSION(Q6_Q_vcmp_eqxacc_QVbVb)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_eqxacc_QVbVb)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gt_VuwVuw __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VuwVuw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VuwVuw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtand_QVuwVuw __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVuwVuw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVuwVuw)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtor_QVuwVuw __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVuwVuw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVuwVuw)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtxacc_QVuwVuw __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVuwVuw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVuwVuw)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gt_VuhVuh __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VuhVuh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VuhVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtand_QVuhVuh __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVuhVuh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVuhVuh)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtor_QVuhVuh __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVuhVuh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVuhVuh)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtxacc_QVuhVuh __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVuhVuh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVuhVuh)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gt_VubVub __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VubVub)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gt_VubVub)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtand_QVubVub __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVubVub)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtand_QVubVub)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtor_QVubVub __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVubVub)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtor_QVubVub)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vcmp_gtxacc_QVubVub __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVubVub)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vcmp_gtxacc_QVubVub)(HVX_VectorPred Qx, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Q_vsetq_R __HVXDBL_EXTENSION(Q6_Q_vsetq_R)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vsetq_R)(Word32 Rt);
#define Q6_Q_vsetq2_R __HVXDBL_EXTENSION(Q6_Q_vsetq2_R)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_vsetq2_R)(Word32 Rt);
#define Q6_Qh_vshuffe_QwQw __HVXDBL_EXTENSION(Q6_Qh_vshuffe_QwQw)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Qh_vshuffe_QwQw)(HVX_VectorPred Qs, HVX_VectorPred Qt);
#define Q6_Qb_vshuffe_QhQh __HVXDBL_EXTENSION(Q6_Qb_vshuffe_QhQh)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Qb_vshuffe_QhQh)(HVX_VectorPred Qs, HVX_VectorPred Qt);
#define Q6_Q_or_QQ __HVXDBL_EXTENSION(Q6_Q_or_QQ)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_or_QQ)(HVX_VectorPred Qs, HVX_VectorPred Qt);
#define Q6_Q_and_QQ __HVXDBL_EXTENSION(Q6_Q_and_QQ)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_and_QQ)(HVX_VectorPred Qs, HVX_VectorPred Qt);
#define Q6_Q_xor_QQ __HVXDBL_EXTENSION(Q6_Q_xor_QQ)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_xor_QQ)(HVX_VectorPred Qs, HVX_VectorPred Qt);
#define Q6_Q_or_QQn __HVXDBL_EXTENSION(Q6_Q_or_QQn)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_or_QQn)(HVX_VectorPred Qs, HVX_VectorPred Qt);
#define Q6_Q_and_QQn __HVXDBL_EXTENSION(Q6_Q_and_QQn)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_and_QQn)(HVX_VectorPred Qs, HVX_VectorPred Qt);
#define Q6_Q_not_Q __HVXDBL_EXTENSION(Q6_Q_not_Q)
HVX_VectorPred __HVXDBL_EXTENSION(Q6_Q_not_Q)(HVX_VectorPred Qs);
#define Q6_V_vmux_QVV __HVXDBL_EXTENSION(Q6_V_vmux_QVV)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vmux_QVV)(HVX_VectorPred Qt, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_W_vswap_QVV __HVXDBL_EXTENSION(Q6_W_vswap_QVV)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_W_vswap_QVV)(HVX_VectorPred Qt, HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vmax_VbVb __HVXDBL_EXTENSION(Q6_Vb_vmax_VbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vmax_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vmin_VbVb __HVXDBL_EXTENSION(Q6_Vb_vmin_VbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vmin_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vub_vmax_VubVub __HVXDBL_EXTENSION(Q6_Vub_vmax_VubVub)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vmax_VubVub)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vub_vmin_VubVub __HVXDBL_EXTENSION(Q6_Vub_vmin_VubVub)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vmin_VubVub)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuh_vmax_VuhVuh __HVXDBL_EXTENSION(Q6_Vuh_vmax_VuhVuh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vmax_VuhVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuh_vmin_VuhVuh __HVXDBL_EXTENSION(Q6_Vuh_vmin_VuhVuh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vmin_VuhVuh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vmax_VhVh __HVXDBL_EXTENSION(Q6_Vh_vmax_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmax_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vmin_VhVh __HVXDBL_EXTENSION(Q6_Vh_vmin_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmin_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmax_VwVw __HVXDBL_EXTENSION(Q6_Vw_vmax_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmax_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vw_vmin_VwVw __HVXDBL_EXTENSION(Q6_Vw_vmin_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vmin_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vub_vsat_VhVh __HVXDBL_EXTENSION(Q6_Vub_vsat_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vub_vsat_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vsat_VwVw __HVXDBL_EXTENSION(Q6_Vh_vsat_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vsat_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuh_vsat_VuwVuw __HVXDBL_EXTENSION(Q6_Vuh_vsat_VuwVuw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vsat_VuwVuw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vshuffe_VbVb __HVXDBL_EXTENSION(Q6_Vb_vshuffe_VbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vshuffe_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vb_vshuffo_VbVb __HVXDBL_EXTENSION(Q6_Vb_vshuffo_VbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vshuffo_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vshuffe_VhVh __HVXDBL_EXTENSION(Q6_Vh_vshuffe_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vshuffe_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vshuffo_VhVh __HVXDBL_EXTENSION(Q6_Vh_vshuffo_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vshuffo_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_W_vshuff_VVR __HVXDBL_EXTENSION(Q6_W_vshuff_VVR)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_W_vshuff_VVR)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_W_vdeal_VVR __HVXDBL_EXTENSION(Q6_W_vdeal_VVR)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_W_vdeal_VVR)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Wh_vshuffoe_VhVh __HVXDBL_EXTENSION(Q6_Wh_vshuffoe_VhVh)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vshuffoe_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Wb_vshuffoe_VbVb __HVXDBL_EXTENSION(Q6_Wb_vshuffoe_VbVb)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wb_vshuffoe_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vdeal_Vh __HVXDBL_EXTENSION(Q6_Vh_vdeal_Vh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vdeal_Vh)(HVX_Vector Vu);
#define Q6_Vb_vdeal_Vb __HVXDBL_EXTENSION(Q6_Vb_vdeal_Vb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vdeal_Vb)(HVX_Vector Vu);
#define Q6_Vb_vdeale_VbVb __HVXDBL_EXTENSION(Q6_Vb_vdeale_VbVb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vdeale_VbVb)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vshuff_Vh __HVXDBL_EXTENSION(Q6_Vh_vshuff_Vh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vshuff_Vh)(HVX_Vector Vu);
#define Q6_Vb_vshuff_Vb __HVXDBL_EXTENSION(Q6_Vb_vshuff_Vb)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vshuff_Vb)(HVX_Vector Vu);
#define Q6_R_vextract_VR __HVXDBL_EXTENSION(Q6_R_vextract_VR)
Word32 __HVXDBL_EXTENSION(Q6_R_vextract_VR)(HVX_Vector Vu, Word32 Rs);
#define Q6_Vw_vinsert_VwR __HVXDBL_EXTENSION(Q6_Vw_vinsert_VwR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vinsert_VwR)(HVX_Vector Vx, Word32 Rt);
#define Q6_V_vsplat_R __HVXDBL_EXTENSION(Q6_V_vsplat_R)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vsplat_R)(Word32 Rt);
#define Q6_Vh_vsplat_R __HVXDBL_EXTENSION(Q6_Vh_vsplat_R)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vsplat_R)(Word32 Rt);
#define Q6_Vb_vsplat_R __HVXDBL_EXTENSION(Q6_Vb_vsplat_R)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vsplat_R)(Word32 Rt);
#define Q6_W_equals_W __HVXDBL_EXTENSION(Q6_W_equals_W)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_W_equals_W)(HVX_VectorPair Vuu);
#define Q6_V_equals_V __HVXDBL_EXTENSION(Q6_V_equals_V)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_equals_V)(HVX_Vector Vu);
#define Q6_W_vcombine_VV __HVXDBL_EXTENSION(Q6_W_vcombine_VV)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_W_vcombine_VV)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_V_vdelta_VV __HVXDBL_EXTENSION(Q6_V_vdelta_VV)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vdelta_VV)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_V_vrdelta_VV __HVXDBL_EXTENSION(Q6_V_vrdelta_VV)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_vrdelta_VV)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vuw_vcl0_Vuw __HVXDBL_EXTENSION(Q6_Vuw_vcl0_Vuw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vcl0_Vuw)(HVX_Vector Vu);
#define Q6_Vuh_vcl0_Vuh __HVXDBL_EXTENSION(Q6_Vuh_vcl0_Vuh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuh_vcl0_Vuh)(HVX_Vector Vu);
#define Q6_Vw_vnormamt_Vw __HVXDBL_EXTENSION(Q6_Vw_vnormamt_Vw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vnormamt_Vw)(HVX_Vector Vu);
#define Q6_Vh_vnormamt_Vh __HVXDBL_EXTENSION(Q6_Vh_vnormamt_Vh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vnormamt_Vh)(HVX_Vector Vu);
#define Q6_Vw_vadd_vclb_VwVw __HVXDBL_EXTENSION(Q6_Vw_vadd_vclb_VwVw)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_vadd_vclb_VwVw)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vadd_vclb_VhVh __HVXDBL_EXTENSION(Q6_Vh_vadd_vclb_VhVh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vadd_vclb_VhVh)(HVX_Vector Vu, HVX_Vector Vv);
#define Q6_Vh_vpopcount_Vh __HVXDBL_EXTENSION(Q6_Vh_vpopcount_Vh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vpopcount_Vh)(HVX_Vector Vu);
#define Q6_Vb_vlut32_VbVbR __HVXDBL_EXTENSION(Q6_Vb_vlut32_VbVbR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vlut32_VbVbR)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vb_vlut32or_VbVbVbR __HVXDBL_EXTENSION(Q6_Vb_vlut32or_VbVbVbR)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vlut32or_VbVbVbR)(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Wh_vlut16_VbVhR __HVXDBL_EXTENSION(Q6_Wh_vlut16_VbVhR)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vlut16_VbVhR)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Wh_vlut16or_WhVbVhR __HVXDBL_EXTENSION(Q6_Wh_vlut16or_WhVbVhR)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vlut16or_WhVbVhR)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vb_vlut32_VbVbI __HVXDBL_EXTENSION(Q6_Vb_vlut32_VbVbI)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vlut32_VbVbI)(HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3);
#define Q6_Vb_vlut32or_VbVbVbI __HVXDBL_EXTENSION(Q6_Vb_vlut32or_VbVbVbI)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vlut32or_VbVbVbI)(HVX_Vector Vx, HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3);
#define Q6_Wh_vlut16_VbVhI __HVXDBL_EXTENSION(Q6_Wh_vlut16_VbVhI)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vlut16_VbVhI)(HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3);
#define Q6_Wh_vlut16or_WhVbVhI __HVXDBL_EXTENSION(Q6_Wh_vlut16or_WhVbVhI)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vlut16or_WhVbVhI)(HVX_VectorPair Vxx, HVX_Vector Vu, HVX_Vector Vv, Word32 Iu3);
#define Q6_Vb_vlut32_VbVbR_nomatch __HVXDBL_EXTENSION(Q6_Vb_vlut32_VbVbR_nomatch)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_vlut32_VbVbR_nomatch)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Wh_vlut16_VbVhR_nomatch __HVXDBL_EXTENSION(Q6_Wh_vlut16_VbVhR_nomatch)
HVX_VectorPair __HVXDBL_EXTENSION(Q6_Wh_vlut16_VbVhR_nomatch)(HVX_Vector Vu, HVX_Vector Vv, Word32 Rt);
#define Q6_Vh_vmpa_VhVhVhPh_sat __HVXDBL_EXTENSION(Q6_Vh_vmpa_VhVhVhPh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmpa_VhVhVhPh_sat)(HVX_Vector Vx, HVX_Vector Vu, Word64 Rtt);
#define Q6_Vh_vmpa_VhVhVuhPuh_sat __HVXDBL_EXTENSION(Q6_Vh_vmpa_VhVhVuhPuh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmpa_VhVhVuhPuh_sat)(HVX_Vector Vx, HVX_Vector Vu, Word64 Rtt);
#define Q6_Vh_vmps_VhVhVuhPuh_sat __HVXDBL_EXTENSION(Q6_Vh_vmps_VhVhVuhPuh_sat)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vmps_VhVhVuhPuh_sat)(HVX_Vector Vx, HVX_Vector Vu, Word64 Rtt);
#define Q6_Vh_vlut4_VuhPh __HVXDBL_EXTENSION(Q6_Vh_vlut4_VuhPh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_vlut4_VuhPh)(HVX_Vector Vu, Word64 Rtt);
#define Q6_Vuw_vmpye_VuhRuh __HVXDBL_EXTENSION(Q6_Vuw_vmpye_VuhRuh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vmpye_VuhRuh)(HVX_Vector Vu, Word32 Rt);
#define Q6_Vuw_vmpyeacc_VuwVuhRuh __HVXDBL_EXTENSION(Q6_Vuw_vmpyeacc_VuwVuhRuh)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vuw_vmpyeacc_VuwVuhRuh)(HVX_Vector Vx, HVX_Vector Vu, Word32 Rt);
#define Q6_V_hi_W __HVXDBL_EXTENSION(Q6_V_hi_W)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_hi_W)(HVX_VectorPair Vss);
#define Q6_V_lo_W __HVXDBL_EXTENSION(Q6_V_lo_W)
HVX_Vector __HVXDBL_EXTENSION(Q6_V_lo_W)(HVX_VectorPair Vss);
#define Q6_vgather_ARMVw __HVXDBL_EXTENSION(Q6_vgather_ARMVw)
void __HVXDBL_EXTENSION(Q6_vgather_ARMVw)(HVX_Vector* A, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv);
#define Q6_vgather_ARMVh __HVXDBL_EXTENSION(Q6_vgather_ARMVh)
void __HVXDBL_EXTENSION(Q6_vgather_ARMVh)(HVX_Vector* A, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv);
#define Q6_vgather_ARMWw __HVXDBL_EXTENSION(Q6_vgather_ARMWw)
void __HVXDBL_EXTENSION(Q6_vgather_ARMWw)(HVX_Vector* A, HVX_Vector* Rb, Word32 Mu, HVX_VectorPair Vvv);
#define Q6_vgather_AQRMVw __HVXDBL_EXTENSION(Q6_vgather_AQRMVw)
void __HVXDBL_EXTENSION(Q6_vgather_AQRMVw)(HVX_Vector* A, HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv);
#define Q6_vgather_AQRMVh __HVXDBL_EXTENSION(Q6_vgather_AQRMVh)
void __HVXDBL_EXTENSION(Q6_vgather_AQRMVh)(HVX_Vector* A, HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv);
#define Q6_vgather_AQRMWw __HVXDBL_EXTENSION(Q6_vgather_AQRMWw)
void __HVXDBL_EXTENSION(Q6_vgather_AQRMWw)(HVX_Vector* A, HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_VectorPair Vvv);
#define Q6_vscatter_RMVwV __HVXDBL_EXTENSION(Q6_vscatter_RMVwV)
void __HVXDBL_EXTENSION(Q6_vscatter_RMVwV)(HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw);
#define Q6_vscatter_RMVhV __HVXDBL_EXTENSION(Q6_vscatter_RMVhV)
void __HVXDBL_EXTENSION(Q6_vscatter_RMVhV)(HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw);
#define Q6_vscatteracc_RMVwV __HVXDBL_EXTENSION(Q6_vscatteracc_RMVwV)
void __HVXDBL_EXTENSION(Q6_vscatteracc_RMVwV)(HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw);
#define Q6_vscatteracc_RMVhV __HVXDBL_EXTENSION(Q6_vscatteracc_RMVhV)
void __HVXDBL_EXTENSION(Q6_vscatteracc_RMVhV)(HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw);
#define Q6_vscatter_QRMVwV __HVXDBL_EXTENSION(Q6_vscatter_QRMVwV)
void __HVXDBL_EXTENSION(Q6_vscatter_QRMVwV)(HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw);
#define Q6_vscatter_QRMVhV __HVXDBL_EXTENSION(Q6_vscatter_QRMVhV)
void __HVXDBL_EXTENSION(Q6_vscatter_QRMVhV)(HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_Vector Vv, HVX_Vector Vw);
#define Q6_vscatter_RMWwV __HVXDBL_EXTENSION(Q6_vscatter_RMWwV)
void __HVXDBL_EXTENSION(Q6_vscatter_RMWwV)(HVX_Vector* Rb, Word32 Mu, HVX_VectorPair Vvv, HVX_Vector Vw);
#define Q6_vscatter_QRMWwV __HVXDBL_EXTENSION(Q6_vscatter_QRMWwV)
void __HVXDBL_EXTENSION(Q6_vscatter_QRMWwV)(HVX_VectorPred Qs, HVX_Vector* Rb, Word32 Mu, HVX_VectorPair Vvv, HVX_Vector Vw);
#define Q6_vscatteracc_RMWwV __HVXDBL_EXTENSION(Q6_vscatteracc_RMWwV)
void __HVXDBL_EXTENSION(Q6_vscatteracc_RMWwV)(HVX_Vector* Rb, Word32 Mu, HVX_VectorPair Vvv, HVX_Vector Vw);
#define Q6_Vb_prefixsum_Q __HVXDBL_EXTENSION(Q6_Vb_prefixsum_Q)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vb_prefixsum_Q)(HVX_VectorPred Qv);
#define Q6_Vh_prefixsum_Q __HVXDBL_EXTENSION(Q6_Vh_prefixsum_Q)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vh_prefixsum_Q)(HVX_VectorPred Qv);
#define Q6_Vw_prefixsum_Q __HVXDBL_EXTENSION(Q6_Vw_prefixsum_Q)
HVX_Vector __HVXDBL_EXTENSION(Q6_Vw_prefixsum_Q)(HVX_VectorPred Qv);


#ifdef __cplusplus
} /* extern C */
#endif

#else /* __hexagon__ */
#error "You are using a compiler that generates Hexagon object code. This is the wrong version of hvx_hexagon_protos.h for the Hexagon compiler."
#endif /* __hexagon__ */

#endif /* __HVX_HEXAGON_PROTOS_HEADER_ */

