/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:02 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if defined(__hexagon__)
#include "hexagon_standalone.h"
#include "subsys.h"
#endif
#include "hvx.cfg.h"
#include "io.h"
#include "ncc.h"


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const char* statusString [] = { "OK",
                                "SEARCH RADIUS TOO LARGE",
                                "INVALID SEARCH CENTER X/Y",
                                "NOT FOUND",
                                "PATCH HAS TOO LOW VARIANCE",
                                "IMAGE REGION HAS TOO LOW VARIANCE"
                                "UNINITIALIZED"
                              };

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const char** retrieveTrackingStatus(int statusIndex)
{
    switch(statusIndex)
    {
        case 0:
            return &statusString[0];
        case 1:
            return &statusString[1];
        case 2:
            return &statusString[2];
        case 3:
            return &statusString[3];
        case 4:
            return &statusString[4];
        case 5:
            return &statusString[5];
        default:
            return &statusString[6];
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char** argv)
{
    unsigned char *patchBuffer, *scratchBuffer;
    unsigned short *srchXBuffer, *srchYBuffer, *bestXBuffer, *bestYBuffer;
    unsigned int * bestNCCBuffer;
    int * resultBuffer;
    unsigned long long int startCycle=0, totalCycles=0;
    unsigned srcRefWidth, srcRefHeight, srcTstWidth, srcTstHeight;
    FH fp;
    FH fp1;
    FH fp2;
    int patchWidth = 8, patchHeight = 8, numUnitTests, wBlocks, hBlocks, c, i, j, k, l;
    unsigned char *refBuffer, *tstBuffer;

    /* -----------------------------------------------------*/
    /*  Get input parameters                                */
    /* -----------------------------------------------------*/
    if (argc != 8) {
        printf("usage: %s <8-bit input0.pgm> refWidth refHeight <8-bit input0.pgm> tstWidth tstHeight <output.txt>\n", argv[0]);
        return 1;
    }

    srcRefWidth = atoi(argv[2]);
    srcRefHeight = atoi(argv[3]);
    srcTstWidth = atoi(argv[5]);
    srcTstHeight = atoi(argv[6]);

    /* -----------------------------------------------------*/
    /*  Allocate memory for input/output                    */
    /* -----------------------------------------------------*/
    refBuffer = (unsigned char*)memalign(VLEN, srcRefWidth * srcRefHeight);
    assert(refBuffer);
    tstBuffer = (unsigned char*)memalign(VLEN, srcTstWidth * srcTstHeight);
    assert(tstBuffer);

    numUnitTests = (srcRefWidth/24)*(srcRefHeight/24);
    wBlocks = srcRefWidth/24;
    hBlocks = srcRefHeight/24;

    patchBuffer = (unsigned char*)memalign(VLEN, numUnitTests*patchWidth*patchHeight);
    srchXBuffer = (unsigned short*)memalign(VLEN, numUnitTests*sizeof(unsigned short));
    srchYBuffer = (unsigned short*)memalign(VLEN, numUnitTests*sizeof(unsigned short));
    bestXBuffer = (unsigned short*)memalign(VLEN, numUnitTests*sizeof(unsigned short));
    bestYBuffer = (unsigned short*)memalign(VLEN, numUnitTests*sizeof(unsigned short));
    bestNCCBuffer = (unsigned int*)memalign(VLEN, numUnitTests*sizeof(unsigned int));
    resultBuffer = (int*)memalign(VLEN, numUnitTests*sizeof(int));
    scratchBuffer = (unsigned char*)memalign(VLEN, 1024+VLEN*2);
    assert(patchBuffer && srchXBuffer && srchYBuffer && bestXBuffer && bestYBuffer &&
        bestNCCBuffer && resultBuffer && scratchBuffer);

    /* -----------------------------------------------------*/
    /*  Read image input from file                          */
    /* -----------------------------------------------------*/
    fp1 = open(argv[1],O_RDONLY);
    if(IS_INVALID_FILE_HANDLE(fp1)) {
        printf("Error: Cannot open %s\n", argv[1]);
    }
    if (sizeof(unsigned char)*srcRefWidth * srcRefHeight != read(fp1, refBuffer,  sizeof(unsigned char)*srcRefWidth * srcRefHeight))
    {
        exit(-1);
    }
    close(fp1);

    fp2 = open(argv[4],O_RDONLY);
    if(IS_INVALID_FILE_HANDLE(fp2)) {
        printf("Error: Cannot open %s\n", argv[4]);
    }
    if (sizeof(unsigned char)*srcTstWidth * srcTstHeight != read(fp2, tstBuffer,  sizeof(unsigned char)*srcTstWidth * srcTstHeight))
    {
        exit(-1);
    }
    close(fp2);

    for (i=0, c=0; i<hBlocks; i++)
    {
        for (j=0; j<wBlocks; j++)
        {
            unsigned char* ptr = patchBuffer+8*8*c;
            for (k=-4; k<4; k++)
            {
                for (l=-4; l<4; l++)
                {
                    *ptr = *(refBuffer + (i*24+11+k)*srcRefWidth + j*24+11+l);
                    ptr++;
                }
            }
            *(((unsigned short *)srchXBuffer) + c) = (j * 24) + 11;
            *(((unsigned short *)srchYBuffer) + c) = (i * 24) + 11;
            c++;
        }
    }

    printf("Running Test Vector: %s with unit tests = %d (wBlocks=%d hBlocks=%d)\n", argv[1], (int)numUnitTests, (int)wBlocks, (int)hBlocks);



#if defined(__hexagon__)
    subsys_enable();
    SIM_ACQUIRE_HVX;
#if LOG2VLEN == 7
    SIM_SET_HVX_DOUBLE_MODE;
#endif
#endif
    /* -----------------------------------------------------*/
    /*  Call function                                       */
    /* -----------------------------------------------------*/
    startCycle = READ_PCYCLES();
    for (k=0; k<numUnitTests; k++)
    {
        resultBuffer[k] =
            fcvNCCPatchOnSquare8x8u8 (
                patchBuffer+64*k,
                tstBuffer, //window buffer
                srcTstWidth, //image width
                srcTstHeight, // image height
                srchXBuffer[k], //center X of search window
                srchYBuffer[k], //center Y of search window
                11, //width of search square (<=11)
                0, //min. variance for correlation
                bestXBuffer + k, //center X location of best NCC match
                bestYBuffer + k, //center Y location of best NCC match
                bestNCCBuffer + k, //best NCC score at bestX/Y location
                0, //flag to enable/disable
                NULL, //sub-pixel X estimate relative to bestX
                NULL,
                scratchBuffer); //sub-pixel Y estimate relative to bestX
    }
    totalCycles = READ_PCYCLES() - startCycle;
    printf("Total Pcycles: %lld\n", totalCycles);
    printf("AppReported: Pcycles/patch: %.5f\n", ((double)totalCycles)/(numUnitTests));

    /* -----------------------------------------------------*/
    /*  Write image output to file                          */
    /* -----------------------------------------------------*/
    fp = open(argv[7], O_CREAT_WRONLY_TRUNC, 0777);
    if (!IS_INVALID_FILE_HANDLE(fp)) {
        for (k=0; k<numUnitTests; k++)  {
            char str[1024];
            sprintf(str, "block%4d %s ", (int)k,*retrieveTrackingStatus(resultBuffer[k]));
            write(fp,str, strlen(str));
            if(*((int*)resultBuffer +k)==0) {
                sprintf(str, "bestX %5d bestY %5d bestNCC %5d", bestXBuffer[k], bestYBuffer[k], bestNCCBuffer[k]);
                write(fp,str, strlen(str));
            }
            sprintf(str, "\n");
            write(fp,str, strlen(str));
        }
        close(fp);
    }

    free(refBuffer);
    free(tstBuffer);
    free(patchBuffer);
    free(srchXBuffer);
    free(srchYBuffer);
    free(bestXBuffer);
    free(bestYBuffer);
    free(bestNCCBuffer);
    free(resultBuffer);
    free(scratchBuffer);

    return 0;
}
