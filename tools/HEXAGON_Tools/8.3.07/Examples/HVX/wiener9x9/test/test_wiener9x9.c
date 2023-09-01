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
#include "wiener9x9.h"


int main (int argc, char *argv[])
{
    FH Infile;
    FH Outfile;
    long long start_time, total_cycles;
    int width, height, window = 9, noise, stride, window_2, y;
    unsigned char *src, *dst;

    char *usage = "Usage: %s width height noise_level inputfile outputfile\n";

    /* -----------------------------------------------------*/
    /*  Get input parameters                                */
    /* -----------------------------------------------------*/
    if (argc != 6)
    {
        printf(usage, argv[0]);
        return -1;
    }

    width = atoi(argv[1]);
    height = atoi(argv[2]);
    noise = atoi(argv[3]);
    stride = (width + VLEN - 1)&(-VLEN);  // make stride a multiple of HVX vector size
    window_2 = window / 2;

    /* -----------------------------------------------------*/
    /*  Allocate memory for input/output                    */
    /* -----------------------------------------------------*/
    src = (unsigned char *)memalign(VLEN, sizeof(src[0]) * stride * (height + 1));
    dst = (unsigned char *)memalign(VLEN, sizeof(dst[0]) * stride * height);

    if ( src == NULL || dst == NULL ){
        printf("Error: Could not allocate Memory for image\n");
        return 1;
    }

    memset(src, 0, sizeof(src[0]) * stride);
    src += stride;

    /* -----------------------------------------------------*/
    /*  Read image input from file                          */
    /* -----------------------------------------------------*/
    if((Infile = open(argv[4], O_RDONLY)) < 0 )
    {
        printf("Error: Cannot open %s for input\n", argv[4]);
        return 1;
    }

    for (y = 0; y < height; y++)
    {
        if (read(Infile, &src[y*stride],  sizeof(src[0]) * width) != sizeof(src[0])* width)
        {
            printf("Error, Unable to read from input file %s\n", argv[4]);
            return 1;
        }
    }

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
    RESET_PMU();
    start_time = READ_PCYCLES();

    filter_wiener9x9 (dst, src, width, width, height, noise);

    total_cycles = READ_PCYCLES() - start_time;
    DUMP_PMU();

#if defined(__hexagon__)
    printf("AppReported (HVX%db-mode): Image %dx%d - Wiener9x9: %0.4f cycles/pixel\n", VLEN, (int)width, (int)height, (float)total_cycles/width/height);
#endif
    /* -----------------------------------------------------*/
    /*  Write image output to file                          */
    /* -----------------------------------------------------*/
    if((Outfile = open(argv[5], O_CREAT_WRONLY_TRUNC, 0777)) < 0)
    {
        printf("Error: Cannot open %s for output\n", argv[5]);
        return 1;
    }

    for (y = window_2; y < height-window_2; y++)
    {
        if (write(Outfile, dst+y*stride, sizeof(dst[0])* width) != sizeof(dst[0])* width)
        {
            printf("Error, Unable to write to output\n");
            return 1;
        }
    }

    src -= stride;
    free(src);
    free(dst);
    close(Infile);
    close(Outfile);

    return 0;
}
