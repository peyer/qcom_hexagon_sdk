/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "hexagon_sim_timer.h"
#include "typedef.h"
#include "cxfir.h"

CWord2x16 Coeffs[16] __attribute__((aligned(8))) =
{ 
     21562664,26867319,8256531,49738850,40957186,2095825,49273962,-24624907,
     24658165,-49161110,-1967407,-40831742,-49613726,-8127469,-26741129,-21560024
};

CWord2x16 Input[56] __attribute__((aligned(8))) =
{ 
     -12518007,16516142,23856670,45747513,63640690,55317555,70657977,25949920,
     29476937,-12332069,-47395501,-85663061,-152441787,-157811806,-262147663,-224143595,
     -248388857,-206831417,-168552890,-129231738,-49984629,1397720,100474436,147134250,
     235277418,244318773,288426730,292024452,269084433,242286483,215476235,185067235,
     125497003,91018560,10149775,-5442434,-66852337,-100537935,-154602936,-175179673,
     -216464747,-253892889,-247276391,-267850426,-218034287,-185329275,-138076234,-51813989,
     12079469,117975061,183707040,264048606,294199875,329782911,333047525,285273681
};

CWord2x16 Expected_results[40]=
{
 0x00befc1d, 0x00c4fb93, 0x0070fcbe, 0x0076fd90,
 0xffdf0014, 0xffb40197, 0xff240211, 0xfecefdd9,
 0xfedcfb4b, 0xfe9bfd85, 0xff040099, 0xff07000e,
 0xff5002b1, 0xffbc08ce, 0x000d0439, 0x0078ff8b,
 0x00d20022, 0x010ffe52, 0x010cffbc, 0x0116ffd2,
 0x00eafd21, 0x00aa0070, 0x007e00f1, 0x0068fe69,
 0x00300095, 0x0017ff05, 0xfff4fd11, 0xffd50064,
 0xffe500aa, 0xffdcffaa, 0xffbb0055, 0xffb5008d,
 0xff990220, 0xff1effb5, 0xff2bfb89, 0xfed1fb58,
 0xfeb2fe0b, 0xfef900df, 0xfecdff28, 0xff4c0413,
};


int main()
{
   CWord2x16 Output[56]  __attribute__((aligned(8)));
   int i;
   long long start_time;
   int overhead, Total_cycles;

    printf("Calling CXFIR Kernel\n");

    start_time = hexagon_sim_read_pcycles();
    cxfir(Input,Coeffs,16,0,19,Output); //check 0 escape case
    overhead = (int)(hexagon_sim_read_pcycles() - start_time) - 1;

    start_time = hexagon_sim_read_pcycles();
    cxfir(Input,Coeffs,16,40,19,Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

    
    for (i=0;i<40;i++) {
        if (Output[i]!=Expected_results[i]) {
            printf("### FAILED!!!\n");
            return 1;
        }
    }
    printf("*** PASS!\n");
    printf("Cycle-count: %d\n", Total_cycles);
    return 0;
}
