/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:02 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__hexagon__)
#include "hexagon_standalone.h"
#include "subsys.h"
#endif
#include "io.h"
#include "hvx.cfg.h"
#include "harris.h"


int main(int argc, char *argv[])
{
    int i;
    int  width, height, stride, threshold;
    unsigned int  maxcorners, numCorners;
    FH fp;

    long long start_time, total_cycles;

    /* -----------------------------------------------------*/
    /*  Get input parameters                                */
    /* -----------------------------------------------------*/
    if (argc !=7)
    {
       printf("Wrong input arguments! : inputfile width height threshold maxcorners outputfile\n");
       return 1;
    }

    width      = atoi(argv[2]);
    height     = atoi(argv[3]);
    threshold  = atoi(argv[4]);
    maxcorners = atoi(argv[5]);

    stride = (width + VLEN-1)&(-VLEN);
                
    /* -----------------------------------------------------*/
    /*  Allocate memory for input/output                    */
    /* -----------------------------------------------------*/
    unsigned char *input = (unsigned char *)memalign(128, stride*height*sizeof(input[0]));

    unsigned int  *xy    = (unsigned int  *)malloc(2*maxcorners*sizeof(xy[0]));

    if ( input == NULL || xy == NULL ){
        printf("Error: Could not allocate Memory for image\n");
        return 1;
    }

    /* -----------------------------------------------------*/
    /*  Read image input from file                          */
    /* -----------------------------------------------------*/
    if((fp = open(argv[1], O_RDONLY)) < 0) {
        printf("Error: can not open %s\n", argv[1]);
        return 1;
    }

    for (i=0; i<height; i++)
    {
        if (read(fp, &input[i*stride],  sizeof(char)*width) != width)
        {
            printf("Error, Unable to read from %s\n", argv[1]);
            close(fp);
            return 1;
        }
    }
    close(fp);

#if defined(__hexagon__)
    subsys_enable();
    SIM_ACQUIRE_HVX;
#if LOG2VLEN == 7
    SIM_SET_HVX_DOUBLE_MODE;
#endif
#endif
    /* -----------------------------------------------------*/
    /*  Call fuction                                        */
    /* -----------------------------------------------------*/

	RESET_PMU();
    start_time = READ_PCYCLES();

    CornerHarrisu8(input, width, height, stride, 5, xy, maxcorners, &numCorners, threshold);

    total_cycles = READ_PCYCLES() - start_time;
	DUMP_PMU();

    printf("Number of Corners detected = %d\n", numCorners);

#if defined(__hexagon__)
    printf("AppReported (HVX%db-mode): Image %dx%d - Harris Corner: %0.4f cycles/pixel\n", VLEN, (int)width, (int)height, (float)total_cycles/width/height);
#endif

    /* -----------------------------------------------------*/
    /*  Write image output to file                          */
    /* -----------------------------------------------------*/
    if((fp = open(argv[6], O_CREAT_WRONLY_TRUNC, 0777)) < 0) 
    {
        printf("Error: can not open file to write\n");
        return 1;
    }

    if(write(fp, xy, sizeof(xy[0])*2*numCorners)!=(sizeof(xy[0])*2*numCorners))
    {
        printf("Error: Writing file: %s\n", argv[6]);
    }
    close(fp);

    free(input);
    free(xy);

    return 0;
}
