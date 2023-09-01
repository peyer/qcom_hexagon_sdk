/*###############################################################
## Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
## All Rights Reserved.
## Modified by QUALCOMM INCORPORATED on \$Date\$
#################################################################*/
#include <vdelta_helper.h>
//#include <h2.h>
#define    LN 128
#define logLN 7
int gsc[LN][4*logLN];
int mux[2*LN];
int tmux[2*LN];
path top[2*LN];
path bot[2*LN];
int insb[2*LN];
int outsb[2*LN];
int inperm[2*LN];
int result[2*LN];
int hperm[LN];
int outperm[LN];
/*======================================================================*/
// desired perm
#if 0
#define logN 7
int tperm[128] = {
#if 0
//example of permute
 24, 14,  4, 51, 41,119, 59, 45, 44,124, 43, 15, 32, 77, 57, 47, 16, 22, 11, 28,104,  1,107, 49,103, 97, 55,  8, 84, 63, 87, 98,
 27, 29, 72, 21,125, 91, 64, 25, 90,113, 62, 39, 70,110, 73,109,112, 74, 23, 31, 68, 17,117, 76, 18,111, 61,127, 78, 42, 52,102,
 94,105,116, 82, 75, 69, 86,101, 95,  0,122, 96, 53,120,  7, 26, 71, 35, 19, 88, 40, 46, 89,  9, 36,  3,114, 13, 37, 56, 30, 20,
118, 50, 10,121, 58, 99, 48, 60, 67, 66,108, 92, 79, 65, 93, 34, 83,106, 85,126,  2,115, 80,100, 38, 54, 12, 33,123,  6, 81,  5};
#endif
#if 1
//example of part permute with dont cares
 24, 14,  4,  X, 41,119, 59, 45, 44,  X, 43, 15, 32, 77, 57, 47, 16, 22, 11, 28,104,  1,107, 49,103, 97, 55,  8, 84, 63, 87, 98,
 27, 29,  X, 21,  X, 91, 64, 25,  X,113,  X, 39, 70,110, 73,109,112, 74, 23, 31, 68, 17,117, 76, 18,111, 61,127, 78, 42, 52,102,
 94,  X,116, 82, 75,  X, 86,  X, 95,  0,122,  X, 53,120,  7, 26, 71, 35, 19, 88, 40, 46, 89,  9, 36,  3,114, 13, 37, 56, 30, 20,
  X, 50, 10,121, 58, 99,  X, 60, 67, 66,108, 92,  X, 65, 93, 34, 83,106, 85,126,  2,115, 80,100, 38, 54, 12, 33,123,  6, 81,  5};
#endif
#else
#define logN 6
int tperm[64] = {
#if 0
//example of replicate word 0-7 in blocks of 8
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7
#endif
#if 0
//example of replciate word 15
  60, 61, 62, 63, 60, 61, 62, 63, 60, 61, 62, 63, 60, 61, 62, 63,
  60, 61, 62, 63, 60, 61, 62, 63, 60, 61, 62, 63, 60, 61, 62, 63,
  60, 61, 62, 63, 60, 61, 62, 63, 60, 61, 62, 63, 60, 61, 62, 63,
  60, 61, 62, 63, 60, 61, 62, 63, 60, 61, 62, 63, 60, 61, 62, 63
#endif
#if 1
//example of decimate 2/3
   0,  1,  2,  4,  5,  7,  8, 10, 11, 13, 14, 16, 17, 19, 20, 22,
  23, 24, 25, 27, 28, 30, 31, 33, 34, 35, 37, 38, 40, 41, 43, 44,
  46, 47, 49, 50, 52, 53, 55, 56, 58, 59, 61, 62,  X,  X,  X,  X,
   X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X
#endif
#if 0
//example of rotate right by 7
  57, 58, 59, 60, 61, 62, 63,  0,  1,  2,  3,  4,  5,  6,  7,  8,
   9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
  25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56
#endif
};
#endif
/*======================================================================*/
int main()
{
   int i, error, x;
   int wildcard = 0;
   int log_vec_len=logN;
   int vec_len=1<<log_vec_len;

   if(!tryvdelta(tperm, vec_len)) return(0);
   if(!tryvrdelta(tperm, vec_len)) return(0);

   printf("trying Benes network implementation....\n\n");
   for(i=0;i < vec_len; i++) inperm[i] = i;
   for(i=0; i < vec_len; i++) { if(tperm[i] == X) wildcard = 1; }
   if(wildcard) printf("found dont cares in pattern, attempting to create permute...\n");
   if(check_benes(tperm, vec_len)) return(1);
/*======================================================================*/
   invert_permute(tperm, vec_len);
   for(i=0; i < vec_len; i++) outperm[i] = tperm[i];
   gen_switch_cntrl(log_vec_len, top, bot, vec_len, inperm, outperm, insb, outsb,0, 0);
   collect_switches(vec_len, log_vec_len);
   convert_const_geo(vec_len, log_vec_len);
   convert_rev_butterfly(gsc, vec_len, log_vec_len);
   print_hvx_bits(gsc, vec_len, log_vec_len);
   check_perm(gsc, mux, vec_len, log_vec_len);
/*======================================================================*/
   for(x=0;x < vec_len; x++) inperm[x] = x;
   for(x=0;x < vec_len; x++) result[tperm[x]] = x;
   error = 0;
   printf("//final pattern implemented\n");
   for(i=0; i < vec_len; i++) {
     printf("%d, ",result[i]);
     if(mux[i] != result[i]) { error = 1; }
   }
   printf("\n");
   if(error) { printf("there were errors\n"); exit(0);}
/*======================================================================*/
   return(0);
}
/*======================================================================*/
/*                             end of file                              */
/*======================================================================*/
/*                       QUALCOMM                                       */
/*                                                                      */
/*                       Copyright (c) 2015                             */
/*                       All Rights Reserved                            */
/*                                                                      */
/*======================================================================*/
