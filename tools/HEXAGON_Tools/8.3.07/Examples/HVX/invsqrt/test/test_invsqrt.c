/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:02 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined(__hexagon__)
#include "hexagon_standalone.h"
#include "subsys.h"
#endif
#include "io.h"
#include "hvx.cfg.h"
#include "invsqrt.h"

#define NUM 1920

int main (int argc, char *argv[])
{
    FH Outfile;
    long long start_time, total_cycles;
    int i;

    char *usage = "Usage: %s outputfile\n";

    /* -----------------------------------------------------*/
    /*  Get input parameters                                */
    /* -----------------------------------------------------*/
    if (argc != 2)
    {
        printf(usage, argv[0]);
        return -1;
    }

    /* -----------------------------------------------------*/
    /*  Allocate memory for input/output                    */
    /* -----------------------------------------------------*/
    unsigned short *shft  = (unsigned short *)memalign(VLEN, NUM*sizeof(shft[0]));
    unsigned short *val   = (unsigned short *)memalign(VLEN, NUM*sizeof(val[0]));
    unsigned short *input = (unsigned short *)memalign(VLEN, NUM*sizeof(input[0]));

    if ( input == NULL || val == NULL || shft == NULL) {
        printf("Error: Could not allocate Memory \n");
        return 1;
    }

    /* -----------------------------------------------------*/
    /*  Generate test data                                  */
    /* -----------------------------------------------------*/
    for (i=0; i<NUM; i++)
    {
        input[i] = i;
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

    invsqrt(input, shft, val, NUM);

    total_cycles = READ_PCYCLES() - start_time;
    DUMP_PMU();

#if defined(__hexagon__)
    printf("AppReported (HVX%db-mode): # of input %d - 1/sqrt(x): %0.4f cycles/value\n", VLEN, (int)NUM, (float)total_cycles/NUM);
#endif
    /* -----------------------------------------------------*/
    /*  Write image output to file                          */
    /* -----------------------------------------------------*/
    if((Outfile = open(argv[1], O_CREAT_WRONLY_TRUNC, 0777)) < 0)
    {
        printf("Error: Cannot open %s for output\n", argv[1]);
        return 1;
    }

    if (write(Outfile, val, sizeof(val[0])* NUM) != sizeof(val[0])* NUM)
    {
        printf("Error, Unable to write to output\n");
        return 1;
    }
    if (write(Outfile, shft, sizeof(shft[0])* NUM) != sizeof(shft[0])* NUM)
    {
        printf("Error, Unable to write to output\n");
        return 1;
    }

    close(Outfile);

    free(input);
    free(shft);
    free(val);

    return 0;
}
