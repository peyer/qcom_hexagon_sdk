/**=============================================================================

@file
   qfxp.c

@brief
   High-level post-processing functions of R and V registers.
   
Copyright (c) 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

#include "qfxp.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "HAP_farf.h"
#undef FARF_HIGH
#define FARF_HIGH 1
#undef FARF_ERROR
#define FARF_ERROR 0

// Sign-extend fixed-point representation of fp passed its declared size
#define QF_SIGN_EXTEND(fp) if (fp->f+fp->m+fp->s==16) {\
  if (fp->s) { \
    fp->raw=fp->raw>0?fp->raw&0x7fff:fp->raw|0xffff8000;\
  } else { \
    fp->raw&=0xffff;\
  } \
}

#ifdef QF_NO_STATS
#define QF_RETURN(reference,fp)	return 1.;
#else
#define QF_RETURN(reference,fp) fp->ref=(reference); QF_SIGN_EXTEND(fp); return qf_updateStats(fp);
#endif

#include <hexagon_protos.h>
#include "hexagon_types.h"
#include "mathLibs.h"

int numErrorsDisplayed=0;
#define ERROR_FP(...) do {if (numErrorsDisplayed++<QF_MAX_NUM_ERRORS_DISPLAYED) FARF(ERROR, __VA_ARGS__);\
  if (numErrorsDisplayed==QF_MAX_NUM_ERRORS_DISPLAYED) FARF(ERROR,"!!! Fixed-point library errors will no longer be reported.");} while(0)

int numWarningsDisplayed=0;
#define WARNING_FP(...) do {if (numWarningsDisplayed++<QF_MAX_NUM_WARNINGS_DISPLAYED) FARF(HIGH, __VA_ARGS__);\
  if (numWarningsDisplayed==QF_MAX_NUM_WARNINGS_DISPLAYED) FARF(HIGH,"!!! Fixed-point library warnings will no longer be reported.");} while(0)

	  
/* Important note: Support for unsigned arithmetic is NOT complete. Many functions, including clamping, are not correct for s==0.*/	  
int qf_clamp(int x, int width, int sign) {
	int upperThreshold, lowerThreshold;
	int maxBits=width-sign;
	if (sign) {
		upperThreshold = (int)((1ul<<maxBits)-1);
		lowerThreshold = (int)((-1l<<maxBits));
	} else {
		upperThreshold = (int)((1ul<<maxBits)-1);
		lowerThreshold = 0;
	}
	
	return  x > upperThreshold ? upperThreshold :  (x < lowerThreshold ? lowerThreshold : x);
}

/* Right shift with optional rounding if shift amount > 0. Left shift otherwise. */
int qf_shift(long long int in, int rshift, int rnd) {
	return (int) (rshift>0?(in + (rnd<<(rshift-1)))>>rshift:in<<-rshift);
}

float qf_updateStats(qf_t* fp) {
	fp->stats.acc+=fp->ref;
	fp->stats.accAbs+=fp->ref<0?-fp->ref:fp->ref;
	if (fp->stats.max<fp->ref) {
		fp->stats.max=fp->ref;
	}
	if (fp->stats.min>fp->ref) {
		fp->stats.min=fp->ref;
	}
	double error = qf_getFloatApprox(*fp)-fp->ref;
	double absError = error<0?-error:error;
	fp->stats.count++;
	fp->stats.accError+=error;
	fp->stats.accAbsError+=absError;
	if (fp->stats.maxError<absError) {
		fp->stats.maxError=absError;
	}
	return (float)absError;
}

QFXP_API void qf_printStats(qf_t fp,char* name) {
	double maxErr = -fp.stats.min>fp.stats.max?-fp.stats.min:fp.stats.max;
	double log2MaxErr = maxErr==0?0:log2(maxErr);
	int numBits = log2MaxErr>0?1 + (int)log2MaxErr:log2MaxErr;
	FARF(HIGH,"%s:(%s%d.%d). Avg: %f. Avg abs: %f. [%f;%f] (%s%d). Avg err: %.3e. Avg |err|: %.3e. Max err: %.3e on %d iter",
			name,fp.s?"S":"U",fp.m,fp.f,fp.stats.acc/fp.stats.count,fp.stats.accAbs/fp.stats.count,fp.stats.min,fp.stats.max,
			fp.stats.min<0?"S":"U",numBits,
			fp.stats.accError/fp.stats.count,fp.stats.accAbsError/fp.stats.count,fp.stats.maxError,fp.stats.count);
}

QFXP_API int qf_getFxpApprox(float ref, int s, int m, int f, int rnd) {
	int width = s+m+f; 
	if (f>=32) {
		ref*=1<<30;
		f-=30;
	}
	if (f>=0) {
		return (int) qf_clamp( (int)((ref*(1ul<<f)+(ref>0?.5:-.5)*rnd)  ) , width, s);
	} else {
		return (int) qf_clamp( (int)(ref/(1ul<<-f)) , width, s);
	}
}	

QFXP_API float qf_floatToFix(qf_t* fp, float ref, int rnd, int mode) {
	
	HVX_Vector in_hvx;
	HVX_Vector frac_hvx;
	int* floatInput = (int*)&ref;
	
	switch (mode) {
		
		case QF_IDEAL:
		fp->raw=qf_getFxpApprox(ref,fp->s,fp->m,fp->f,rnd);
		break;
		
		case QF_HVX:
		in_hvx = Q6_V_vsplat_R(*floatInput);
		frac_hvx = Q6_V_vsplat_R(fp->f);
		hvx_float2frac(&in_hvx, &frac_hvx, &in_hvx, 1);		
		fp->raw = Q6_R_vextract_VR(in_hvx,0);
		break;

		default:
		ERROR_FP("Unsupported mode %d for qf_floatToFix");
	}
	QF_RETURN(ref,fp);
}

QFXP_API float qf_init(qf_t* fp, float ref, int s, int m, int f, int rnd, int mode) {
	if (s==0) {
		WARNING_FP("WARNING: Support for unsigned type is not complete. You should only use signed types for now.");
	}
	int width = s+m+f;
	if ((width!=16)&(width!=32)) 
	{
		ERROR_FP("Failed initialization of %f as %s%d.%d number. Only 16-bit and 32-bit currently supported.",ref,s?"S":"U",m,f);
	}
	fp->s=s;
	fp->m=m;
	fp->f=f;
	fp->stats=(qf_stats_t)QF_NEW_STATS;
	return qf_floatToFix(fp,ref,rnd,mode);
}

/* Return raw value representing fixed-point number. */
QFXP_API int qf_getRaw(qf_t fp) {
	return fp.raw;
}

QFXP_API void qf_setRaw(qf_t* fp, int raw) {
	fp->raw=raw;
	fp->ref=qf_getFloatApprox(*fp);
}

/* Return float approximation represented by fixed-point number. */
QFXP_API float qf_getFloatApprox(qf_t fp) {
	if (fp.f>=32) {
		double result = (double)qf_getRaw(fp) / (1ul<<31);
		return (float) (result / (1ul<<(fp.f-31)));
	} else if (fp.f>=0) {
		return (float) ((double)qf_getRaw(fp) / (1ul<<fp.f));
	} else {
		return (float) ((double)qf_getRaw(fp) * (1ul<<-fp.f));
	}
}

QFXP_API int qf_getIntApprox(qf_t fp, int rnd) {
	return	qf_shift(fp.raw, fp.f, rnd);
}

/* Return float being approximated. */
QFXP_API float qf_getFloatRef(qf_t fp){
	return fp.ref;
}

QFXP_API float qf_fixToFix(qf_t* res, qf_t in, int rnd) {
	res->raw = qf_shift(in.raw,in.f-res->f,rnd);
	QF_RETURN(in.ref,res);
}

QFXP_API float qf_add(qf_t* res, qf_t in1, qf_t in2) {	
	int in1Raw = qf_shift(in1.raw,in1.f-res->f,1);
	int in2Raw = qf_shift(in2.raw,in2.f-res->f,1);
	res->raw = in1Raw+in2Raw;
	QF_RETURN(in1.ref+in2.ref,res);
}

QFXP_API float qf_sub(qf_t* res, qf_t in1, qf_t in2) {
	int in1Raw = qf_shift(in1.raw,in1.f-res->f,1);
	int in2Raw = qf_shift(in2.raw,in2.f-res->f,1);
	res->raw = in1Raw-in2Raw;
	QF_RETURN(in1.ref-in2.ref,res);
}

QFXP_API float qf_mult(qf_t* res, qf_t in1, qf_t in2, int rnd, int mode) {
	HVX_Vector in1_hvx, in2_hvx, res_hvx;
	long long int tmp;
	
	// Left-align input arguments if necessary to treat 16-bit and 32-bit math the same
	int in1Raw=in1.s+in1.m+in1.f==16?in1.raw<<16:in1.raw;
	int in2Raw=in2.s+in2.m+in2.f==16?in2.raw<<16:in2.raw;
	switch (mode) {
		
		case QF_IDEAL:
		tmp = (long long int) in1Raw * in2Raw;
		res->raw = qf_shift(tmp,31 - in1.m - in2.m + res->m,rnd);
		break;
		
		case QF_DSP:
		res->raw = qf_shift(Q6_R_mpy_RR_s1_sat(in1Raw,in2Raw),res->m - in1.m - in2.m,rnd);
		break;
		
		case QF_HVX_MULT_32_32:
		in1_hvx = Q6_V_vsplat_R(in1Raw);
		in2_hvx = Q6_V_vsplat_R(in2Raw);
		res_hvx = Q6_Vw_vmpye_VwVuh(in1_hvx,in2_hvx);
		res_hvx = Q6_Vw_vmpyoacc_VwVwVh_s1_rnd_sat_shift(res_hvx,in1_hvx,in2_hvx);
		res->raw = qf_shift(Q6_R_vextract_VR(res_hvx,0),res->m - in1.m - in2.m,rnd);
		break;
		
		case QF_HVX_MULT_32_16:
		in1_hvx = Q6_V_vsplat_R(in1Raw);
		in2_hvx = Q6_V_vsplat_R(in2Raw);
		res_hvx = Q6_Vw_vmpyo_VwVh_s1_rnd_sat(in1_hvx,in2_hvx);
		res->raw = qf_shift(Q6_R_vextract_VR(res_hvx,0),res->m - in1.m - in2.m,rnd);
		break;
		
		default:
		ERROR_FP("Unsupported mode %d for qf_mult");
	}
	res->raw=res->s+res->m+res->f==16?res->raw>>16:res->raw;

	QF_RETURN(in1.ref*in2.ref,res);
}


QFXP_API float qf_mac(qf_t* acc, qf_t in1, qf_t in2, int rnd, int mode) {
	long long int tmp;

	if ((acc->m - in1.m - in2.m) | (mode==QF_HVX_MULT_32_16) | (mode==QF_HVX_MULT_32_32) | (acc->s+acc->m+acc->f==16)) {
		if (acc->m - in1.m - in2.m) {
			WARNING_FP("qf_mac operation will not be mapped into a signed fractional mac instruction because of mantissas: %d x %d -> %d != %d",
						in1.m,in2.m,acc->m,in1.m+in2.m-1);
		} else if (acc->s+acc->m+acc->f==16) {
			WARNING_FP("macf will not be mapped into a signed fractional mac instructions because accumulator must be 32-bit.");
		}  else {
			WARNING_FP("macf will not be mapped into a signed fractional mac instructions because HVX does not support a 32x32->32 mac instructions.");
		}
		qf_t tmp;
		qf_init(&tmp,0,acc->s,acc->m,acc->f,0,QF_IDEAL);
		qf_mult(&tmp,in1,in2,rnd,mode);
		return qf_add(acc,tmp,*acc);
	}
	
	// Now we are guaranteed to only need a fractional mac

	// Left-align input arguments if necessary to treat 16-bit and 32-bit math the same
	int in1Raw=in1.s+in1.m+in1.f==16?in1.raw<<16:in1.raw;
	int in2Raw=in2.s+in2.m+in2.f==16?in2.raw<<16:in2.raw;

	switch (mode) {
		case QF_IDEAL:
		tmp = (long long int) in1Raw * in2Raw;
		acc->raw += qf_shift(tmp,31,rnd);
		break;
		
		case QF_DSP:
		acc->raw = Q6_R_mpyacc_RR_s1_sat(acc->raw,in1Raw,in2Raw);
		break;

		default:
		ERROR_FP("Unsupported mode %d for qf_mac");
	}
	QF_RETURN(acc->ref+in1.ref*in2.ref,acc);
}

QFXP_API float qf_multpwr2(qf_t *out, qf_t in, int pwr2, int rnd) {
	out->raw=qf_shift(in.raw,-pwr2-out->f+in.f,rnd);
	QF_RETURN(pwr2>0?in.ref*(1<<pwr2):in.ref/(1<<-pwr2),out);
}


QFXP_API float qf_invert(qf_t* res, qf_t in, int mode) {
	HVX_Vector arg;	
    // Minimum size to guarantee that the invert HVX implementation writes within bounds
	HVX_Vector inverse_mant_asm[2], inverse_exp_asm[3];
	int rshift, result, exp, shift;
	result_scale_t val;
	
	switch (mode) {
		case QF_IDEAL:
		res->raw = qf_getFxpApprox(1/(qf_getFloatApprox(in)),res->s,res->m,res->f,1);
		break;
		
		case QF_DSP:
		val = dsplib_invert(qf_getRaw(in));
		rshift = (val.scale - in.f - res->f);
		res->raw = rshift>0 ? (val.result >> rshift) : (val.result << -rshift);
		break;
		
		case QF_HVX:
		arg = Q6_V_vsplat_R(in.raw);	
		hvx_invert((short*)&arg, in.f, (short*)&inverse_mant_asm[0], (short*)&inverse_exp_asm[0], 128);
		result = (Q6_R_vextract_VR(inverse_mant_asm[0],0) << 16) >> 16;
		exp    =  (Q6_R_vextract_VR(inverse_exp_asm[0],0) << 16) >> 16;
		shift = res->f + exp - 15;
		res->raw = result << shift;
		break;
		
		case QF_DSP_APPROX:
		val = dsplib_approx_invert(qf_getRaw(in));
		rshift = (30- val.scale - in.f - res->f);
		res->raw = rshift>0 ? (val.result >> rshift) : (val.result << -rshift);
		break;
		
		default:
		ERROR_FP("Unsupported mode %d for qf_invert");
	}
	QF_RETURN(1/in.ref,res);
}

QFXP_API float qf_sqrt(qf_t* res, qf_t in, int mode) {
	HVX_Vector src[4];
	HVX_Vector dst[4];
	switch (mode) {
		case QF_IDEAL:
		res->raw = qf_getFxpApprox(sqrtf(qf_getFloatApprox(in)),res->s,res->m,res->f,1);
		break;

		case QF_HVX:
		src[0] = Q6_V_vsplat_R(in.f%2?in.raw>>1:in.raw);	
		hvx_sqrt((const uint32_t*)src, 128, (uint32_t*) dst, 128);	
		int result = Q6_R_vextract_VR(dst[0],0);
		res->raw=qf_shift(result,16+in.f/2-res->f,1);
		break;
		
		case QF_DSP:
		if (in.f%2) {
			res->raw = qf_shift(dsplib_sqrt(in.raw>>1,0),16+in.f/2-res->f,1);
		} else {
			res->raw = qf_shift(dsplib_sqrt(in.raw,0),16+in.f/2-res->f,1);
		}
		break;
		
		default:
		ERROR_FP("Unsupported mode %d for qf_sqrt");
	}
		
	QF_RETURN(sqrtf(in.ref),res);
}

QFXP_API float qf_frac(qf_t* res, qf_t in) {
	int frac=in.raw&QF_FRACTIONAL_MASK(in.f);
	res->raw=qf_shift(frac,in.f-res->f,0);
	QF_RETURN(in.ref-(int)in.ref,res);
}
