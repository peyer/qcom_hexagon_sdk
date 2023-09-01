/* ======================================================================== */
/*  QUALCOMM TECHNOLOGIES, INC.                                             */
/*                                                                          */
/*  HEXAGON HVX Image/Video Processing Library                              */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*          Copyright (c) 2015-2016 QUALCOMM TECHNOLOGIES Incorporated.     */
/*                           All Rights Reserved.                           */
/*                  QUALCOMM Confidential and Proprietary                   */
/* ======================================================================== */

#include "hexagon_types.h"
#define FARF_HIGH 1
#define FARF_ERRROR 1
#include "HAP_farf.h"
#include "testData.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "qfxp.h"

// Leave uncommented unless you want to generate new test vectors on the simulator
//#define GENERATE_NEW_TEST_VECTORS

/* Define a version number between 1 and 7 (here or as argument VERSION with make command)
 *
 * All versions are supposed to pass the test except version==6 (too aggressive
 * quantization)
 * All versions are documented in the powerpoint presentation from the doc folder
 */
#ifndef VERSION
#define VERSION 7
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define N 256
#define THRESHOLD .005

// Example of float macros that will need to be converted to fixed point
#define CENTER_X 2.345
#define CENTER_Y 4.567
#define DISTANCE_SHIFT 4

// Example of a global variable that will need to be converted to fixed point
float normFactor=98.7654;

// Reference floating-point code that will need to be converted to fixed point

float normalize(float x, float y, float z) {
	float squareX = (x-CENTER_X)*(x-CENTER_X);
	float squareY = (y-CENTER_Y)*(y-CENTER_Y);
	float distance = sqrt(squareX+squareY)/(1<<DISTANCE_SHIFT);
	return normFactor*z/distance;
}

void normalizeArray(float* outputptr, float* xptr, float* yptr, float* zptr) {
	for (int i=0;i<N-1;i++) {
		outputptr[i] = normalize(xptr[i],yptr[i],zptr[i]);
	}	
}

// Only needed to generate new test vectors
#ifdef GENERATE_NEW_TEST_VECTORS

float randomValue(float scalar, float min) {
	float x = ((rand()%1000)-500)/scalar;
	x+=x>0?min:-min;
	return x;
}

void generateTestVector(char* name, float scalar, float min) {
	printf("float %s[%d] = {",name,N);
	for (int i=0;i<N-1;i++) {
		if (!(i%8)) {
			printf("\n");
		}
		printf("%f, ",randomValue(scalar,min)); 
	}
	printf("%f};\n",randomValue(scalar,min)); 
}

void generateInputTestVectors() {
	generateTestVector("inX",1.2345, 10);
	generateTestVector("inY",3.2345, 10);
	generateTestVector("inZ",1234.6, 0);
}
	
void generateOutputTestVector() {
	printf("float ref[%d] = {",N);
	for (int i=0;i<N-1;i++) {
		if (!(i%8)) {
			printf("\n");
		}
		printf("%f, ", normalize(inX[i],inY[i],inZ[i])); 
	}
	printf("%f};\n", normalize(inX[N-1],inY[N-1],inZ[N-1])); 
}
#endif

// Utility to capture max error occuring over an array
float maxError(float* ref, float* actual) {
	float maxError = 0;
	for (int i=0;i<N-1;i++) {
		float error = ref[i]-actual[i];
		error = error>0?error:-error;
		maxError = maxError>error?maxError:error;
	}
	return maxError;
}

////////////////////////////////////////////////////////////////////////////////////

// Fixed-point variable declarations

#if VERSION==1
qf_t x=S(32,0),y=S(32,0),z=S(32,0),ctrX=S(32,0),ctrY=S(32,0);
qf_t squareX=S(32,0),squareY=S(32,0),square=S(32,0);
qf_t distance=S(32,0), invDistance=S(32,0),normDistance=S(32,0);
qf_t normZ=S(32,0),result=S(32,0);
qf_t CENTER_X_FP=S(32,0), CENTER_Y_FP=S(32,0), normFactor_FP=S(32,0);
#elif (VERSION==2)|(VERSION==3)|(VERSION==4)
qf_t x=S(32,9),y=S(32,8),z=S(32,0),ctrX=S(32,9),ctrY=S(32,8);
qf_t squareX=S(32,18),squareY=S(32,15),square=S(32,18);
qf_t distance=S(32,9), invDistance=S(32,0),normDistance=S(16,5);  // S(16,5) will PASS; S(32,5) will FAIL
qf_t normZ=S(32,6),result=S(32,4);
qf_t CENTER_X_FP=S(32,2), CENTER_Y_FP=S(32,3), normFactor_FP=S(32,7);
#elif VERSION==5
qf_t x=S(32,9),y=S(32,9),z=S(32,0),ctrX=S(32,9),ctrY=S(32,9);
qf_t squareX=S(32,18),squareY=S(32,18),square=S(32,18);
qf_t distance=S(32,9), invDistance=S(32,0),normDistance=S(16,5);
qf_t normZ=S(32,7),result=S(32,7);
qf_t CENTER_X_FP=S(32,9), CENTER_Y_FP=S(32,9), normFactor_FP=S(32,7);
#elif VERSION==6
qf_t x=S(16,9),y=S(16,9),z=S(16,0),ctrX=S(16,9),ctrY=S(16,9);
qf_t squareX=S(16,18),squareY=S(16,18),square=S(16,18);
qf_t distance=S(16,9), invDistance=S(16,0),normDistance=S(16,5);
qf_t normZ=S(16,7),result=S(32,7);
qf_t CENTER_X_FP=S(16,9), CENTER_Y_FP=S(16,9), normFactor_FP=S(16,7);
#elif VERSION==7
qf_t x=S(16,9),y=S(16,9),z=S(16,0),ctrX=S(16,9),ctrY=S(16,9);
qf_t squareX=S(32,18),squareY=S(32,18),square=S(32,18);
qf_t distance=S(16,9), invDistance=S(16,0),normDistance=S(16,5);
qf_t normZ=S(16,7),result=S(32,7);
qf_t CENTER_X_FP=S(16,9), CENTER_Y_FP=S(16,9), normFactor_FP=S(16,7);
#endif

// Fixed-point implementation of reference floating-point code
void normalizeArrayqf_t(float* outputptr, float* xptr, float* yptr, float* zptr) {
	
	qf_floatToFix(&CENTER_X_FP,CENTER_X,1,QF_IDEAL);
	qf_floatToFix(&CENTER_Y_FP,CENTER_Y,1,QF_IDEAL);
	qf_floatToFix(&normFactor_FP,normFactor,1,QF_IDEAL);
		
	#if (VERSION==1)|(VERSION==2)
	for (int i=0;i<N;i++) {
		qf_floatToFix(&x,xptr[i],0,QF_IDEAL);
		qf_floatToFix(&y,yptr[i],0,QF_IDEAL);
		qf_floatToFix(&z,zptr[i],0,QF_IDEAL);
		qf_sub(&ctrX,x,CENTER_X_FP);
		qf_sub(&ctrY,y,CENTER_Y_FP);
		qf_mult(&squareX,ctrX,ctrX,1,QF_IDEAL);
		qf_mult(&squareY,ctrY,ctrY,1,QF_IDEAL);
		qf_add(&square,squareX,squareY);
		qf_sqrt(&distance,square,QF_IDEAL);
		qf_multpwr2(&normDistance,distance,-DISTANCE_SHIFT,1);
		qf_invert(&invDistance,normDistance,QF_IDEAL);
		qf_mult(&normZ,normFactor_FP,z,1,QF_IDEAL);
		qf_mult(&result,normZ,invDistance,1,QF_IDEAL);
		outputptr[i]=qf_getFloatRef(result);
	}
	#elif VERSION==3
	for (int i=0;i<N;i++) {
		qf_floatToFix(&x,xptr[i],0,QF_IDEAL);
		qf_floatToFix(&y,yptr[i],0,QF_IDEAL);
		qf_floatToFix(&z,zptr[i],0,QF_IDEAL);
		qf_sub(&ctrX,x,CENTER_X_FP);
		qf_sub(&ctrY,y,CENTER_Y_FP);
		qf_mult(&squareX,ctrX,ctrX,1,QF_IDEAL);
		qf_mult(&squareY,ctrY,ctrY,1,QF_IDEAL);
		qf_add(&square,squareX,squareY);
		qf_sqrt(&distance,square,QF_IDEAL);
		qf_multpwr2(&normDistance,distance,-DISTANCE_SHIFT,1);
		qf_invert(&invDistance,normDistance,QF_IDEAL);
		qf_mult(&normZ,normFactor_FP,z,1,QF_IDEAL);
		qf_mult(&result,normZ,invDistance,1,QF_IDEAL);
		outputptr[i]=qf_getFloatApprox(result);
	}		
	#elif VERSION==4
	for (int i=0;i<N;i++) {
		qf_floatToFix(&x,xptr[i],0,QF_IDEAL);
		qf_floatToFix(&y,yptr[i],0,QF_IDEAL);
		qf_floatToFix(&z,zptr[i],0,QF_IDEAL);
		qf_sub(&ctrX,x,CENTER_X_FP);
		qf_sub(&ctrY,y,CENTER_Y_FP);
		qf_mult(&squareX,ctrX,ctrX,1,QF_IDEAL);
		qf_mult(&squareY,ctrY,ctrY,1,QF_IDEAL);
		qf_add(&square,squareX,squareY);
		qf_sqrt(&distance,square,QF_IDEAL);
		qf_multpwr2(&normDistance,distance,-DISTANCE_SHIFT,1);
		qf_invert(&invDistance,normDistance,QF_HVX);
		qf_mult(&normZ,normFactor_FP,z,1,QF_IDEAL);
		qf_mult(&result,normZ,invDistance,1,QF_IDEAL);
		outputptr[i]=qf_getFloatApprox(result);
	}		
	#elif VERSION==5
	for (int i=0;i<N;i++) {
		qf_floatToFix(&x,xptr[i],0,QF_HVX);
		qf_floatToFix(&y,yptr[i],0,QF_HVX);
		qf_floatToFix(&z,zptr[i],0,QF_HVX);
		qf_sub(&ctrX,x,CENTER_X_FP);
		qf_sub(&ctrY,y,CENTER_Y_FP);
		qf_mult(&squareX,ctrX,ctrX,1,QF_HVX_MULT_32_16);
		qf_mult(&squareY,ctrY,ctrY,1,QF_HVX_MULT_32_16);
		qf_add(&square,squareX,squareY);
		qf_sqrt(&distance,square,QF_HVX);
		qf_multpwr2(&normDistance,distance,-DISTANCE_SHIFT,1);
		qf_invert(&invDistance,normDistance,QF_HVX);
		qf_mult(&normZ,normFactor_FP,z,1,QF_HVX_MULT_32_16);
		qf_mult(&result,normZ,invDistance,1,QF_HVX_MULT_32_16);
		outputptr[i]=qf_getFloatApprox(result);
	}		
	#elif (VERSION==6)|(VERSION==7)
	for (int i=0;i<N;i++) {
		qf_floatToFix(&x,xptr[i],0,QF_HVX);
		qf_floatToFix(&y,yptr[i],0,QF_HVX);
		qf_floatToFix(&z,zptr[i],0,QF_HVX);
		qf_sub(&ctrX,x,CENTER_X_FP);
		qf_sub(&ctrY,y,CENTER_Y_FP);
		qf_mult(&square,ctrX,ctrX,1,QF_IDEAL);
		qf_mac(&square,ctrY,ctrY,1,QF_IDEAL);
		qf_sqrt(&distance,square,QF_HVX);
		qf_multpwr2(&normDistance,distance,-DISTANCE_SHIFT,1);
		qf_invert(&invDistance,normDistance,QF_HVX);
		qf_mult(&normZ,normFactor_FP,z,1,QF_IDEAL);
		qf_mult(&result,normZ,invDistance,1,QF_IDEAL);
		outputptr[i]=qf_getFloatApprox(result);
	}		
	#endif
}

// Generate a report on all fixed-point variables
void printfixedPointStats() {
	QF_PRINT_STATS(x);
	QF_PRINT_STATS(y);
	QF_PRINT_STATS(z);
	QF_PRINT_STATS(ctrX);
	QF_PRINT_STATS(ctrY);
	QF_PRINT_STATS(squareX);
	QF_PRINT_STATS(squareY);
	QF_PRINT_STATS(square);
	QF_PRINT_STATS(distance);
	QF_PRINT_STATS(normDistance);
	QF_PRINT_STATS(invDistance);
	QF_PRINT_STATS(normZ);
	QF_PRINT_STATS(result);
	QF_PRINT_STATS(CENTER_X_FP);
	QF_PRINT_STATS(CENTER_Y_FP);
	QF_PRINT_STATS(normFactor_FP);
}

// Call reference floating-point and fixed-point implementation and compare their output
void example()
{
#ifdef GENERATE_NEW_TEST_VECTORS
	generateInputTestVectors();
    generateOutputTestVector();
#else

	float* out = (float*) malloc(N*sizeof(float));
	normalizeArray(out,inX,inY,inZ);
	float error = maxError(ref,out);
	FARF(HIGH,"%s. error=%f",error<THRESHOLD?"PASSED":"FAILED",error);
	
	normalizeArrayqf_t(out,inX,inY,inZ);
	printfixedPointStats();
	error = maxError(ref,out);
	FARF(HIGH,"%s. error=%f",error<THRESHOLD?"PASSED":"FAILED",error);
	
	free(out);

	#endif
}

