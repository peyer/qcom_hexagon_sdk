/**=============================================================================

@file
   scatter_gather_imp.cpp

@brief
   implementation file for scatter/gather RPC interface.

Copyright (c) 2017-2019 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

//==============================================================================
// Include Files
//==============================================================================

// enable message outputs for profiling by defining _DEBUG and including HAP_farf.h
#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"
#include "qurt_thread.h"

#include "q6cache.h"

// profile DSP execution time (without RPC overhead) via HAP_perf api's.
#include "HAP_perf.h"

#if (__HEXAGON_ARCH__ >= 65)
#include "HAP_vtcm_mgr.h"
#else
static void* HAP_request_VTCM(int a, int b) {return 0;}
static void HAP_release_VTCM(void *a)   {}
#endif

#include "AEEStdErr.h"

// includes
#include "benchmark.h"
#include "benchmark_asm.h"

#include "dspCV_worker.h"

#include "hexagon_types.h"
#include "hexagon_protos.h"

#include "HAP_compute_res.h"

/*===========================================================================
    DEFINITIONS
===========================================================================*/
#define PROFILING_ON

// (128-byte is only mode supported in this example)
#define VLEN (128)

// 16x8 patches (assumed to be 2-byte aligned)
#define PATCH_WID    (16)
#define PATCH_HGT    (8)

#define MIN_GATHER_SCATTER_SZ        (32*1024)
#define MAX_GATHER_SCATTER_SZ        (64*1024)
#define MIN_VTCM_SZ                  (64*1024)

#define N_MAX_SEGMENT_NUM 	(150)
#define N_MAX_SUBPATCH_NUM 	(300)


// inline assembly definition for scatter_release operation
#define SCATTER_RELEASE(ADDR)  __asm__ volatile("vmem(%0 + #0):scatter_release\n" \
                        :: "r" (ADDR) );

/*===========================================================================
    DECLARATIONS
===========================================================================*/
/*Each element of this array represent the offset for each half word element within one patch. vgather/vscatter operates either in half word
or word granularity, here two consecutive byte within the patch are manipulated.*/
static uint16_t  g_offsetTbl[64] __attribute__ ((aligned (128)));

//construct two arrays to generate the offset with user specified width and offset=g_offsetBase+width*g_offsetIncBy2
static const uint16_t g_offsetIncBy2[64] __attribute__ ((aligned (128))) =
{
	0,2,4,6,8,10,12,14,
	0,2,4,6,8,10,12,14,
	0,2,4,6,8,10,12,14,
	0,2,4,6,8,10,12,14,	
	0,2,4,6,8,10,12,14,
	0,2,4,6,8,10,12,14,
	0,2,4,6,8,10,12,14,
	0,2,4,6,8,10,12,14
};

static const uint16_t g_offsetBase[64]	__attribute__ ((aligned (128))) =
{
	0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,
	4,4,4,4,4,4,4,4,
	5,5,5,5,5,5,5,5,
	6,6,6,6,6,6,6,6,
	7,7,7,7,7,7,7,7
};

//a 0-63 array to use the predicate mechanism when the number of elements vgather/vscatter operate on is less than 64
static const uint16_t g_inc_vec[64] __attribute__ ((aligned (128))) =
{
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9,10,11,12,13,14,15,
   16,17,18,19,20,21,22,23,
   24,25,26,27,28,29,30,31,
   32,33,34,35,36,37,38,39,
   40,41,42,43,44,45,46,47,
   48,49,50,51,52,53,54,55,
   56,57,58,59,60,61,62,63
};

/*===========================================================================
    TYPEDEF
===========================================================================*/
typedef struct
{
	unsigned int		 x1;				// x coordinate of the upper left corner
	unsigned int         y1;				// y coordinate of the upper left corner
	unsigned int         x2;				// x coordinate of the lower right corner 
	unsigned int         y2;				// y coordinate of the lower right corner 
}pos_pair_t;

typedef struct
{
	int 				  numImageSeg;		// the number of total image segment
	int 				  segmentHgtVec[2];	// segmentHgtVec[0] is the height of left image segment, segmentHgtVec[1] is the height of the last image segment
	int 				 *pNumPatchPerSeg;  // point to the array of number of patches(including sub patches) within an image segment
	int 				 *pNumPatchHgtAcc;  // point to the array of accumulated height of all the patch(including sub patches) within an image segment
	pos_pair_t 			 *pPatchPos;	    // point to the array of positions of each patches, patches within different segments are stored sequentially
}segment_para_t;

// multi-threading context structure. This structure contains everything the multi-threaded callback needs to process the right portion of the input image.
typedef struct
{
    dspCV_synctoken_t    *token;            // worker pool token
    unsigned int          nPatches;         // number of patches for the current segment
    unsigned int          patchCount;       // patch counter for the current segment, shared by all workers
    HVX_Vector           *vtcmBase;         // vtcm pointer, note that it is usually the same for all the segment
    HVX_Vector           *vtmp;             // point to the temp buffer of current image segment,  note that it is usually the same for all the segment
    unsigned int          width;            // width of the current image segment
    unsigned int          height;           // height of the current image segment
	uint16_t             *pOffsetTbl;       // pointer to the 64 elements offset table	
    pos_pair_t		     *pPatchPos;        // point to the array of positions of each patches within the current segment
} gather_callback_t;

typedef long HEXAGON_Vect1024_UN __attribute__((__vector_size__(128)))__attribute__((aligned(4)));

static pos_pair_t g_SegmentPosVec[N_MAX_SEGMENT_NUM];
static int g_numPatchPerSeg[N_MAX_SEGMENT_NUM];
static int g_numPatchHgtAcc[N_MAX_SEGMENT_NUM];
static pos_pair_t g_patchPosVec[N_MAX_SUBPATCH_NUM];

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/

/*function to split the height*width image into multiple segments, then calculate the positions of each segment and the positions of patches within each segment.
  Note that when a patch overlaps two segments, it is divided into two sub-patches, and each is considered as a new patch when gather/scatter.*/
static int segment_image(int32 width, int32 height, int32 nPatches, int32 horizStep, int32 vertStep, segment_para_t *pSegmentPara, int32 segmentSz)
{	
	int segmentHgt;
	int numImageSeg;
	int lastSegmentHgt;
	
	if(width*height<=segmentSz)//then no need to do the splitting
	{
		segmentHgt=height;
		numImageSeg=1;
	}
	else
	{
		segmentHgt=segmentSz/width;//the maximum height within a segment
		numImageSeg=height/segmentHgt;
    	lastSegmentHgt=height-numImageSeg*segmentHgt;

		if(lastSegmentHgt>0)//corresponds to the height of the last image segment does not equal to segmentHgt
		{
			numImageSeg++;
		}
	}
	
	//check whether the size of static variables satisfies the actual size
	if(sizeof(g_SegmentPosVec)/sizeof(g_SegmentPosVec[0])<numImageSeg||
	   sizeof(g_numPatchPerSeg)/sizeof(g_numPatchPerSeg[0])<numImageSeg||
	   sizeof(g_numPatchHgtAcc)/sizeof(g_numPatchHgtAcc[0])<numImageSeg||
	   sizeof(g_patchPosVec)/sizeof(pos_pair_t)<2*nPatches)//assume 2xnPatches is the max number of patches and sub patches, instead of calculating after traversing the whole image
	{
		return AEE_EBADPARM;
	}

    int *pSegmentHgt=pSegmentPara->segmentHgtVec;
	pos_pair_t *pSegmentPos = g_SegmentPosVec;
		
	if(numImageSeg>1)
	{
		//set the positions of the segments except the last one
		for(int i = 0; i<numImageSeg-1; i++)
		{
			pSegmentPos[i].x1 = 0;
			pSegmentPos[i].y1 = i*segmentHgt;
			pSegmentPos[i].x2 = width-1;
			pSegmentPos[i].y2 = (i+1)*segmentHgt-1;
		}
		pSegmentHgt[0]=segmentHgt;
		
		//deal with the last segment
		pSegmentPos[numImageSeg-1].x1 = 0;
		pSegmentPos[numImageSeg-1].x2 = width-1;
		pSegmentPos[numImageSeg-1].y2 = height-1;	
		
		if(lastSegmentHgt>0)
		{
			pSegmentPos[numImageSeg-1].y1 = height-lastSegmentHgt;		
			pSegmentHgt[1]=lastSegmentHgt;		
		}
		else
		{
			pSegmentPos[numImageSeg-1].y1 = height-segmentHgt;		
			pSegmentHgt[1]=segmentHgt;
		}
	}
	else
	{	
		pSegmentPos[0].x1 = 0;
		pSegmentPos[0].y1 = 0;
		pSegmentPos[0].x2 = width-1;
		pSegmentPos[0].y2 = height-1;

		pSegmentHgt[0]=height;
		pSegmentHgt[1]=height;
	}
	
	//traverse all the patches, to locate each patch within a specific image segment
	pos_pair_t *pPatchPos = g_patchPosVec;	

	int *pNumPatchPerSeg = g_numPatchPerSeg;
	memset(pNumPatchPerSeg, 0,numImageSeg*sizeof(int));
	
	int *pNumPatchHgtAcc = g_numPatchHgtAcc;
	memset(pNumPatchHgtAcc, 0, numImageSeg*sizeof(int));
	
	int numSubPatches=0;
	int isPatchNotDivided;
	for(int i = 0; i < nPatches; i++)
	{
		int patchY1 = i * vertStep;//since x,y never wrap around, there is no need to use %
		int patchY2 = patchY1 + PATCH_HGT - 1;

		isPatchNotDivided = 0;

		for(int j=0; j<numImageSeg; j++)//for all the image segment
		{
			int segY1=pSegmentPos[j].y1;
			int segY2=pSegmentPos[j].y2;
			pos_pair_t *pPatchPosCur=&pPatchPos[numSubPatches];

			//totally five cases
			if(patchY1>=segY1&&patchY2<=segY2)//when the patch locates entirely within a segment
			{
				pPatchPosCur->y1=patchY1;
				pPatchPosCur->y2=patchY2;
				
				isPatchNotDivided = 1;		
			}						
			else if(patchY2<segY1||patchY1>segY2)//two cases that the patch locates outside of the current segment
			{
				continue;
			}			
			else if( (patchY1<segY1)&&(patchY2>=segY1)&&(patchY2<=segY2) )//when only the lower part the patch locates within the segment
			{
				pPatchPosCur->y1=segY1;
				pPatchPosCur->y2=patchY2;	
			}
			else//when only the upper part the patch locates within the segment
			{	
				pPatchPosCur->y1=patchY1;
				pPatchPosCur->y2=segY2;	
			}			

			//adjust patchX1, patchX2 if exceed the boundary
			int patchX1 = i * horizStep;
			if(patchX1 + PATCH_WID - 1>width-1)
			{
				patchX1 = width - PATCH_WID;
			}			
			pPatchPosCur->x1=patchX1;
			pPatchPosCur->x2 = patchX1 + PATCH_WID - 1;

			if(pPatchPosCur->y2>height-1)
			{
				pPatchPosCur->y1 = height - PATCH_HGT;
				pPatchPosCur->y2 = height - 1;
			}

			//subtract Y1 with segment offset, finally the patchX1/patchY1 should start from the current segment instead of the entire image
			pPatchPosCur->y1 -= segY1;
			pPatchPosCur->y2 -= segY1;
		
			numSubPatches++;
			pNumPatchPerSeg[j]++;
			pNumPatchHgtAcc[j]+=pPatchPosCur->y2-pPatchPosCur->y1+1;

			if(1==isPatchNotDivided)//if a patch entirely locates within a segment, then there is no need to traverse the left segments
			{
				break;
			}
		}
	}	

	//save the result to the pSegmentPara
	pSegmentPara->numImageSeg=numImageSeg;
	pSegmentPara->pNumPatchPerSeg=pNumPatchPerSeg;
	pSegmentPara->pNumPatchHgtAcc=pNumPatchHgtAcc;
	pSegmentPara->pPatchPos=pPatchPos;

	return AEE_SUCCESS;	
}

// multi-threading callback function
static void gather_callback(
    void* data
)
{
    gather_callback_t    *dptr = (gather_callback_t*)data;

    unsigned int nPatches    = dptr->nPatches;
    unsigned int width       = dptr->width;
    unsigned int height      = dptr->height;
    pos_pair_t *pPatchPos    = dptr->pPatchPos;

    // Set pointers to appropriate line of image for this stripe
    HVX_Vector *vSrc = dptr->vtcmBase;
    HVX_Vector *vTmp = dptr->vtmp;
    HVX_VectorPred Qs;
    const HVX_Vector VincVec= *(HVX_Vector*)g_inc_vec;		
	const HVX_Vector V0 = *(HVX_Vector*)dptr->pOffsetTbl;	
    HVX_Vector VnumHeight;	

    int i;
    while((i = dspCV_atomic_inc_return(&(dptr->patchCount)) - 1) < nPatches)
    {    	
        int x = pPatchPos[i].x1;
        int y = pPatchPos[i].y1;		
        int patchHgt=pPatchPos[i].y2-pPatchPos[i].y1+1;

        uint16_t patchStart = y*width + x;
        HVX_Vector V1 = Q6_Vh_vsplat_R(patchStart);        
        V1 = Q6_Vuh_vadd_VuhVuh_sat(V0, V1);

        VnumHeight = Q6_Vh_vsplat_R((uint16)(8*patchHgt));
        
        Qs = Q6_Q_vcmp_gt_VuhVuh( VnumHeight, VincVec);

		// For patch's height being less than PATCH_HGT, the data is stored at the beginning of the address vTmp points, next round(if the patch lies at
		// the beginning of the segment) vTmp points to the address that is 128byte larger
        Q6_vgather_AQRMVh( vTmp+i, Qs, (uint32_t)vSrc, width*height-1, V1 );
    }

    // release multi-threading job token
    dspCV_worker_pool_synctoken_jobdone(dptr->token);
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
AEEResult benchmark_gather(
    remote_handle64 handle,
    const uint8* src, 
    int inpLen, 
    uint8* dst, 
    int outpLen, 
    int32 width, 
    int32 height, 
    int32 nPatches, 
    int32 horizStep, 
    int32 vertStep, 
    int32 LOOPS,
    int32 wakeupOnly,
    int32 useComputRes,
    int32* dspUsec,
    int32* dspCyc
)
{
    *dspUsec = 0, *dspCyc = 0;
    if (wakeupOnly)
    {
        return AEE_SUCCESS;
    }

// only supporting HVX version in this example.
#if (__HEXAGON_ARCH__ < 60)
    return AEE_EUNSUPPORTED;
#endif

    // record start time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 startTime = HAP_perf_get_time_us();
    uint64 startCycles = HAP_perf_get_pcycles();
#endif

    // Only supporting 128-byte aligned and even step sizes!! 
    if (!(src && dst && ((((uint32)src | (uint32)dst | width) & 127) == 0)
            && (height >= PATCH_HGT+2) && (((horizStep | vertStep) & 1) == 0)
            && (vertStep>=PATCH_HGT)                       //Patches can be overlapped horizontally but not vertically
            &&(horizStep*(nPatches-1)+PATCH_WID-1<width)   //This and next line to ensure that patches never locate across the image boundary
            && (vertStep*(nPatches-1)+PATCH_HGT-1<height)))
    {
        return AEE_EBADPARM;
    }

    for (int loops = 0; loops < LOOPS; loops++)
    {
        // Allocate VTCM
        unsigned int avail_block_size;
        unsigned int max_page_size;
        unsigned int num_pages;
        int segmentSz;
        if(0==HAP_query_avail_VTCM(&avail_block_size, &max_page_size, &num_pages))
        {
            if(max_page_size<MIN_VTCM_SZ)
            {
                FARF(ERROR,"Available VTCM size less than %d KiB, aborting...", MIN_VTCM_SZ/1024);
                return AEE_ENOMEMORY;                
            }
            else if(max_page_size>=128*1024)
            {
                segmentSz=MAX_GATHER_SCATTER_SZ;
            }
            else 
            {
                segmentSz=MIN_GATHER_SCATTER_SZ;
            }
			
			if(nPatches*PATCH_WID*PATCH_HGT>max_page_size-segmentSz)
            {
                FARF(ERROR,"Not enough space for the intermediate buffer, aborting...");
                return AEE_ENOMEMORY;                  
            }
        }
        else
        {
            FARF(ERROR, "Fail to call HAP_query_avail_VTCM,  aborting...");
            return AEE_ENOMEMORY;
        }

        compute_res_attr_t compute_res;
        unsigned int context_id = 0;
        HVX_Vector *vtcmBase=NULL;
        
        if(useComputRes&&compute_resource_attr_init)
        {
            compute_resource_attr_init(&compute_res);
            compute_resource_attr_set_serialize(&compute_res, 1);
            compute_resource_attr_set_vtcm_param(&compute_res, max_page_size, 0);
            
            context_id=compute_resource_acquire(&compute_res, 100000); // wait till 100ms   
            
            if(context_id==0)
            {
                return AEE_ERESOURCENOTFOUND;
            }
            
            vtcmBase=compute_resource_attr_get_vtcm_ptr(&compute_res);            
        }
        else
        {
            if(useComputRes&&(!compute_resource_attr_init))
            {
                FARF(HIGH, "Compute resource APIs not supported. Use legacy methods instead.");                            
            }
        
            vtcmBase = (HVX_Vector*)HAP_request_VTCM(max_page_size, 1);    
            
            if (!vtcmBase)
            {
                FARF(ERROR,"Could not allocate VTCM, aborting...");
                return AEE_ENOMEMORY;
            }            
        }
                
    	//calculate each image segment position
		segment_para_t segmentPara;
		int retVal=segment_image(width, height, nPatches, horizStep, vertStep, &segmentPara, segmentSz);
		if(AEE_SUCCESS!=retVal)
		{
            if(AEE_EBADPARM==retVal) // when this happens, it means the width&height is too large for the intermediate buffer
            {
                FARF(ERROR,"Too big image width&height, approximately width*height should be less than %d, aborting...", N_MAX_SEGMENT_NUM*segmentSz);
            }
            else // deal with other error codes
            {
                FARF(ERROR,"Error found in segment_image(), aborting...");
            }
            HAP_release_VTCM((void*)vtcmBase);
            return retVal;			
		}

		int numImageSeg=segmentPara.numImageSeg;
		int *pSegmentHgt=segmentPara.segmentHgtVec;
		int *pNumPatchPerSeg=segmentPara.pNumPatchPerSeg;
		int *pNumPatchHgtAcc=segmentPara.pNumPatchHgtAcc;
		pos_pair_t *pPatchPos=segmentPara.pPatchPos;		

		uint8 *pSrcImageSeg=(uint8 *)src;
		uint8 *pDstImageSeg=dst;
		int numSubPatches=0;

		//generate the offset matrix, with the user defined width. This can be generated only once if the width is specified.			
		HVX_Vector V0 = *(HVX_Vector*)g_offsetIncBy2;
		HVX_Vector V2 = *(HVX_Vector*)g_offsetBase;
		HVX_Vector V3 = Q6_Vh_vsplat_R(width);
		V0 = Q6_Vh_vmpyiacc_VhVhVh(V0, V2, V3); 	
	    HVX_Vector *pVtmp=(HVX_Vector *)g_offsetTbl;
		*pVtmp=V0;//save the offset table to a global variable
				
		//the following block copy the first segment into the VTCM
		int segmentHgt = pSegmentHgt[0];
		uint64_t L2FETCH_REGISTER = (1ULL <<48) | ((uint64_t)width<<32) | ((uint64_t)width<<16) | segmentHgt;
		L2fetch((uint32_t)pSrcImageSeg, L2FETCH_REGISTER);		
		HVX_Vector *vSrc = (HVX_Vector*)pSrcImageSeg;
		HVX_Vector	*vTmp = vtcmBase;
		for (int j = 0; j < segmentHgt * width / VLEN; j++)
		{
			*vTmp++ = *vSrc++;
		}
		
		for(int i = 0; i < numImageSeg; i++)
		{			
	        int numWorkers = dspCV_num_hvx128_contexts;

	        // split src image into horizontal stripes, for multi-threading.
	        dspCV_worker_job_t   job;
	        dspCV_synctoken_t    token;

	        // init the synchronization token for this dispatch.
	        dspCV_worker_pool_synctoken_init(&token, numWorkers);

	        // init job function pointer
	        job.fptr = gather_callback;

	        // init job data pointer. In this case, all the jobs can share the same copy of job data.
	        gather_callback_t dptr;
	        dptr.token = &token;
	        dptr.nPatches = segmentPara.pNumPatchPerSeg[i];
	        dptr.patchCount = 0;
	        dptr.vtcmBase = vtcmBase;
			dptr.vtmp = vtcmBase + (segmentSz / sizeof(*vTmp));//currently the output region of vgather locates tightly with the input of it
	        dptr.width = width;
	        dptr.height = segmentHgt;
	        dptr.pPatchPos=&pPatchPos[numSubPatches];	   
			dptr.pOffsetTbl=g_offsetTbl;

	        job.dptr = (void *)&dptr;

	        for (unsigned int j = 0; j < numWorkers; j++)
	        {
	            // It is observed that this example runs slightly faster when single-threaded, likely due to full congestion of the
	            // scatter/gather network for this simple example. Use in-context function calls instead of callbacks to force single-threading.

	            // for multi-threaded impl, use this line.
	            //(void) dspCV_worker_pool_submit(job);

	            // This line can be used instead of the above to directly invoke the
	            // callback function without dispatching to the worker pool.				
	            job.fptr(job.dptr);				
	        }
	        dspCV_worker_pool_synctoken_wait(&token);			

			//save the pointer(s) for the copy of current segment's data
			uint8 *pDstImageSegCur=pDstImageSeg;
						
			//copy the segment into the VTCM for the next round of loop use
			if(i<=numImageSeg-2)//the last segment don't need copy
			{
				//update for the next segment use			
				pSrcImageSeg +=segmentHgt*width;
				pDstImageSeg +=pNumPatchHgtAcc[i]*PATCH_WID;
				numSubPatches +=pNumPatchPerSeg[i];
				segmentHgt = (i<numImageSeg-2)?pSegmentHgt[0] : pSegmentHgt[1];
			
				uint64_t L2FETCH_REGISTER = (1ULL <<48) | ((uint64_t)width<<32) | ((uint64_t)width<<16) | segmentHgt;
				L2fetch((uint32_t)pSrcImageSeg, L2FETCH_REGISTER);
				vSrc = (HVX_Vector*)pSrcImageSeg;
				vTmp = vtcmBase;
				
				for (int j = 0; j < segmentHgt * width / VLEN; j++)
				{
					*vTmp++ = *vSrc++;
				}				
			}

	        // Need to copy the gathered data from VTCM back to destination. Note that this will stall
	        // (as necessary) until gathers have completed.		
			int numVmem=pNumPatchPerSeg[i]*PATCH_HGT*PATCH_WID/VLEN;
			int addrMod128=(int)pDstImageSegCur&0x7F;
			vTmp=(HVX_Vector*)dptr.vtmp;
			
			if(0!=addrMod128)
			{	
				if((numImageSeg-1==i)&&(pNumPatchHgtAcc[i]<PATCH_HGT))// if it is the last segment and there is only one patch whose height<PATCH_GHT, then it almost reach the output boundary
																	  // that cannot use a 128 byte memory store.
				{
					uint8_t *pTmp=(uint8_t *)dptr.vtmp;
					uint8_t *pDst=(uint8_t *)pDstImageSegCur;
					for(int j=0; j<pNumPatchHgtAcc[i]*PATCH_WID; j++)
					{
						*pDst++=*pTmp++;
					}					
				}
				else
				{				
					HEXAGON_Vect1024_UN *vDstUN=(HEXAGON_Vect1024_UN *)pDstImageSegCur;
					*vDstUN=*vTmp++;
				
					addrMod128 = 128-addrMod128;
				}

				numVmem--;
			}
						
			HVX_Vector *vDst = (HVX_Vector*)(pDstImageSegCur+addrMod128);		
			for (int j = 0; j < numVmem; j++)//if the address of the patch does not align to 128byte, then still store 128byte, since the subsequent patch can overlap this.
			{
				*vDst++ = *vTmp++;
			}					
		}

        if(useComputRes&&compute_resource_attr_init)
        {
            compute_resource_release(context_id);               
        }
        else
        {        
            HAP_release_VTCM((void*)vtcmBase);
        }
    }

    // record end time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 endCycles = HAP_perf_get_pcycles();
    uint64 endTime = HAP_perf_get_time_us();
    *dspUsec = (int)(endTime - startTime);
    *dspCyc = (int32)(endCycles - startCycles);
    FARF(HIGH,"gather profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
        (int)((endCycles - startCycles) / (endTime - startTime)));
#endif

	return AEE_SUCCESS;
}

AEEResult benchmark_scatter(
    remote_handle64 handle,
    const uint8* src, 
    int inpLen, 
    uint8* dst, 
    int outpLen, 
    int32 width, 
    int32 height, 
    int32 nPatches, 
    int32 horizStep, 
    int32 vertStep, 
    int32 LOOPS,
    int32 wakeupOnly,
    int32 useComputRes,
    int32* dspUsec,
    int32* dspCyc
)
{
    *dspUsec = 0, *dspCyc = 0;
    if (wakeupOnly)
    {
        return AEE_SUCCESS;
    }

// only supporting HVX version in this example.
#if (__HEXAGON_ARCH__ < 60)
    return AEE_EUNSUPPORTED;
#endif

    // record start time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 startTime = HAP_perf_get_time_us();
    uint64 startCycles = HAP_perf_get_pcycles();
#endif
    // Only supporting 128-byte aligned and even step sizes!! 
    if (!(src && dst && ((((uint32)src | (uint32)dst | width) & 127) == 0)
            && (height >= PATCH_HGT+2) && (((horizStep | vertStep) & 1) == 0)
            && (vertStep>=PATCH_HGT)                        //Patches can be overlapped horizontally but not vertically
            &&(horizStep*(nPatches-1)+PATCH_WID-1<width)    //This and next line to ensure that patches never locate across the image boundary
            && (vertStep*(nPatches-1)+PATCH_HGT-1<height)))

    {
        return AEE_EBADPARM;
    }

    for (int loops = 0; loops < LOOPS; loops++)
    {
        // Allocate VTCM
        unsigned int avail_block_size;
        unsigned int max_page_size;
        unsigned int num_pages;
        int segmentSz;
        if(0==HAP_query_avail_VTCM(&avail_block_size, &max_page_size, &num_pages))
        {
            if(max_page_size<MIN_VTCM_SZ)
            {
                FARF(ERROR,"Available VTCM size less than %d KiB, aborting...", MIN_VTCM_SZ/1024);
                return AEE_ENOMEMORY;                
            }
            
            segmentSz=MAX_GATHER_SCATTER_SZ;// it is possible for scatter to use less than 64KiB, but not much tested.
        }
        else
        {
            FARF(ERROR, "Fail to call HAP_query_avail_VTCM,  aborting...");
            return AEE_ENOMEMORY;
        }	

        compute_res_attr_t compute_res;
        unsigned int context_id = 0;
        HVX_Vector *vtcmBase=NULL;
        
        if(useComputRes&&compute_resource_attr_init)
        {
            compute_resource_attr_init(&compute_res);
            compute_resource_attr_set_serialize(&compute_res, 1);
            compute_resource_attr_set_vtcm_param(&compute_res, segmentSz, 0);

            context_id=compute_resource_acquire(&compute_res, 100000); // wait till 100ms
            
            if(context_id==0)
            {
                return AEE_ERESOURCENOTFOUND;
            }
            
            vtcmBase=compute_resource_attr_get_vtcm_ptr(&compute_res);
        }
        else
        {   
            if(useComputRes&&(!compute_resource_attr_init))  
            {
                FARF(HIGH, "Compute resource APIs not supported. Use legacy methods instead.");
            }            
            
            vtcmBase = (HVX_Vector*)HAP_request_VTCM(segmentSz, 1);
            
            if (!vtcmBase)
            {
                FARF(ERROR,"Could not allocate VTCM, aborting...");
                return AEE_ENOMEMORY;
            }
        }       
        
    	segment_para_t segmentPara;	
		int retVal=segment_image(width, height, nPatches, horizStep, vertStep,	&segmentPara, segmentSz);
		if(AEE_SUCCESS!=retVal)
		{
            if(AEE_EBADPARM==retVal) // when this happens, it means the width&height is too large for the intermediate buffer
            {
                FARF(ERROR,"Too big image width&height, approximately width*height should be less than %d, aborting...", N_MAX_SEGMENT_NUM*MAX_GATHER_SCATTER_SZ);
            }
            else // deal with other error codes
            {
                FARF(ERROR,"Error found in segment_image(), aborting...");
            }

            HAP_release_VTCM((void*)vtcmBase);
            return retVal;			
		}
		
		int numImageSeg=segmentPara.numImageSeg;		
		int *pSegmentHgt=segmentPara.segmentHgtVec;
		int *pNumPatchPerSeg=segmentPara.pNumPatchPerSeg;
		int *pNumPatchHgtAcc=segmentPara.pNumPatchHgtAcc;
		pos_pair_t *pPatchPos=segmentPara.pPatchPos;	

		uint8 *pSrcImageSeg=(uint8 *)src;
		uint8 *pDstImageSeg=dst;
		int    numSubPatches=0;
	
		//generate the offset matrix, with the user defined width				
		HVX_Vector V0 = *(HVX_Vector*)g_offsetIncBy2;
		HVX_Vector V2 = *(HVX_Vector*)g_offsetBase;
		HVX_Vector V3 = Q6_Vh_vsplat_R(width);
		V0 = Q6_Vh_vmpyiacc_VhVhVh(V0, V2, V3);		

		for(int i = 0; i < numImageSeg; i++)
		{
			int segmentHgt = (i<numImageSeg-1)?pSegmentHgt[0] : pSegmentHgt[1];

			// Copy dst image segment into VTCM
			uint64_t L2FETCH_REGISTER = (1ULL <<48) | ((uint64_t)width<<32) | ((uint64_t)width<<16) | segmentHgt;
			L2fetch((uint32_t)pDstImageSeg, L2FETCH_REGISTER);
			L2FETCH_REGISTER = (1ULL <<48) | ((uint64_t)pNumPatchPerSeg[i]<<32) | ((uint64_t)pNumPatchPerSeg[i]<<16) | PATCH_WID*PATCH_HGT;
			L2fetch((uint32_t)pSrcImageSeg, L2FETCH_REGISTER);

			HVX_Vector *vDst = (HVX_Vector*)pDstImageSeg, *vTmp = vtcmBase;
			for (int j = 0; j < segmentHgt * width / VLEN; j++)
			{
				*vTmp++ = *vDst++;
			}			

			HVX_Vector *vSrc = (HVX_Vector*)pSrcImageSeg;
			HEXAGON_Vect1024_UN *vSrcUN = (HEXAGON_Vect1024_UN*)pSrcImageSeg;
			
		    HVX_VectorPred Qs;
		    const HVX_Vector VincVec= *(HVX_Vector*)g_inc_vec;
		    HVX_Vector VnumHeight;

			pos_pair_t *pPatchPosPerSeg=pPatchPos+numSubPatches;
			for (int j = 0; j < pNumPatchPerSeg[i]; j++)
			{
		        int x = pPatchPosPerSeg[j].x1;
		        int y = pPatchPosPerSeg[j].y1;
		        int patchHgt=pPatchPosPerSeg[j].y2-pPatchPosPerSeg[j].y1+1;

				uint16_t patchStart = y*width + x;
				HVX_Vector V1 = Q6_Vh_vsplat_R(patchStart);
				V1 = Q6_Vh_vsplat_R(patchStart);
				V1 = Q6_Vuh_vadd_VuhVuh_sat(V0, V1);

			    VnumHeight = Q6_Vh_vsplat_R((uint16)(8*patchHgt));			   
			    Qs = Q6_Q_vcmp_gt_VuhVuh( VnumHeight, VincVec);
				
				// Note that the length of region must be given as (length-1).
				int addrMod128=(int)pSrcImageSeg&0x7F;
				if((j==0)&&(addrMod128!=0))
				{
					Q6_vscatter_QRMVhV(Qs, (uint32_t)vtcmBase, width*segmentHgt-1, V1, *vSrcUN);
					vSrc = (HVX_Vector*)(pSrcImageSeg+128-addrMod128);//update for the next iterator use					
				}
				else
				{
					Q6_vscatter_QRMVhV(Qs, (uint32_t)vtcmBase, width*segmentHgt-1, V1, *vSrc++);					
				}
							
				// Note that we need to issue a scatter-release and dummy load to ensure ordering,
				// because the patches being scattered in this example can possibly overlap.
				// This has some performance impact, so should only be done once at the end of scattering
				// if there were guaranteed no overlaps.
				SCATTER_RELEASE(vtcmBase);
				volatile HVX_Vector dummy = *vtcmBase;
				(void) dummy;
			}

			// This call indicates that all previous scatters must be allowed to complete prior to the
			// next attempt to load from vctmBase.
			SCATTER_RELEASE(vtcmBase);
			
			vTmp = vtcmBase;
			vDst = (HVX_Vector*)pDstImageSeg;
			for (int j = 0; j < segmentHgt * width / VLEN; j++)
			{
				*vDst++ = *vTmp++;
			}

			pSrcImageSeg+=pNumPatchHgtAcc[i]*PATCH_WID;
			pDstImageSeg+=width*segmentHgt;
			numSubPatches+=pNumPatchPerSeg[i];
		}
        
        if(useComputRes&&compute_resource_attr_init)
        {
            compute_resource_release(context_id);               
        }
        else
        {        
            HAP_release_VTCM((void*)vtcmBase);	  
        }        
    }

    // record end time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 endCycles = HAP_perf_get_pcycles();
    uint64 endTime = HAP_perf_get_time_us();
    *dspUsec = (int)(endTime - startTime);
    *dspCyc = (int32)(endCycles - startCycles);
    FARF(HIGH,"scatter profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
        (int)((endCycles - startCycles) / (endTime - startTime)));
#endif

	return AEE_SUCCESS;
}


