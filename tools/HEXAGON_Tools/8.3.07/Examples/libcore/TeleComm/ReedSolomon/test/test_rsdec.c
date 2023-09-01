#include <stdio.h>
#include "typedef.h"
#include "rs.h"
#include "testdata.h"

#include "hexagon_sim_timer.h"

int main()
{
    int i, k, L;
    UWord8 syndrome[RS_2T], roots[RS_T], error_values[RS_T];
    UWord16 logLambda[RS_T];

    long long start_time, end_time; 
    int Total_cycles = 0;

    start_time = hexagon_sim_read_pcycles();
    calculate_syndrome( rv_symbol, syndrome );
    end_time = hexagon_sim_read_pcycles();

    Total_cycles += (int)(end_time - start_time);

    printf("Syndrome: %d\n", (int)(end_time - start_time));

    start_time = hexagon_sim_read_pcycles();
    L = belerkamp_massey( syndrome, logLambda);
    end_time = hexagon_sim_read_pcycles();

    Total_cycles += (int)(end_time - start_time);

    printf("Belerkmp: %d\n", (int)(end_time - start_time));

    start_time = hexagon_sim_read_pcycles();
    k = chien_search( logLambda, roots);
    end_time = hexagon_sim_read_pcycles();

    Total_cycles += (int)(end_time - start_time);

    printf("ChienSrh: %d\n", (int)(end_time - start_time));

    start_time = hexagon_sim_read_pcycles();
    forney( syndrome, logLambda, roots, k, error_values );
    end_time = hexagon_sim_read_pcycles();

    Total_cycles += (int)(end_time - start_time);

    printf("Forneys : %d\n", (int)(end_time - start_time));

    printf("Cycle-count %d\n", Total_cycles);

    //=========================
    // check syndrome  
    //=========================
    for (i=0; i<RS_2T; i++)
    {
        if ( syndrome[i] != expected_syn_out[i])
        {
            printf("### FAILED (Syndrome)!!!\n");
            return 1;
        }
    }

    //=========================
    // check Belerkamp-massey
    //=========================
    if ( L!= 8)
    {
        printf("### FAILED (Belerkamp)!!!\n");
        return 1;
    }
    for (i=0; i<RS_T; i++)
    {
        if (logLambda[i] != expected_logLambda[i])
        {
            printf("### FAILED (Belerkamp)!!!\n");
            return 1;
        }
    }

    //=========================
    // check Chien search
    //=========================
    if (k !=8) 
    {
        printf("### FAILED (Root search)!!!\n");
        return 1;
    }

    for (i=0; i<k; i++)
    {
        if (roots[i] != expected_roots[i])
        {
            printf("### FAILED (Root search)!!!\n");
            return 1;
        }
    }

    //=========================
    // check forney's 
    //=========================
    for (i=0; i<k; i++)
    {
        if (error_values[i] != expected_error_values[i])
        {
            printf("### FAILED (Forney)!!!\n");
            return 1;
        }
    }

    printf("*** PASS!\n");

    return 0;
}
