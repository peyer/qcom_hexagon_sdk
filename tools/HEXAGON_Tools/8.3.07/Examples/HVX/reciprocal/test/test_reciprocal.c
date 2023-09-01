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
#include "reciprocal.h"

#define TNUM 1920

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
    unsigned short *input = (unsigned short *)memalign(VLEN,TNUM*sizeof(unsigned short));
    unsigned short *value = (unsigned short *)memalign(VLEN,TNUM*sizeof(unsigned short));
             short *shftn = (         short *)memalign(VLEN,TNUM*sizeof(         short));

    if ( input == NULL || value == NULL || shftn == NULL ){
        printf("Error: Could not allocate Memory for image\n");
        return 1;
    }

    /* -----------------------------------------------------*/
    /*  Generate input data                                 */
    /* -----------------------------------------------------*/
    for (i = 0; i < TNUM; i++)
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

    reciprocal(input, value, shftn, TNUM);

    total_cycles = READ_PCYCLES() - start_time;
    DUMP_PMU();

#if defined(__hexagon__)
    printf("AppReported (HVX%db-mode): # of input %d - 1/x: %0.4f cycles/value\n", VLEN, (int)TNUM, (float)total_cycles/TNUM);
#endif
    /* -----------------------------------------------------*/
    /*  Write image output to file                          */
    /* -----------------------------------------------------*/
    if((Outfile = open(argv[1], O_CREAT_WRONLY_TRUNC, 0777)) < 0)
    {
        printf("Error: Cannot open %s for output\n", argv[1]);
        return 1;
    }

    if (write(Outfile, value, sizeof(value[0])* TNUM) != sizeof(value[0])* TNUM)
    {
        printf("Error, Unable to write to output\n");
        return 1;
    }
    if (write(Outfile, shftn, sizeof(shftn[0])* TNUM) != sizeof(shftn[0])* TNUM)
    {
        printf("Error, Unable to write to output\n");
        return 1;
    }

    close(Outfile);

    free(input);
    free(value);
    free(shftn);

    return 0;
}
