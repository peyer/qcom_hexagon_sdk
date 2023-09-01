/*###############################################################
## Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
## All Rights Reserved.
## Modified by QUALCOMM INCORPORATED on \$Date\$
#################################################################*/
/* notes
    tperm is specified as before but now  there can be don't cares, this only
	creates the controls for a single vrdelta instruction
    pattern does not have to be a true permute
    can still fail
 */
#include "vdelta_helper.h"
#include <hexagon_protos.h>

#define LN 128
static int gsb[LN][4*7];
static int vperm[LN];
static int block=0;
extern int inperm[];
extern int outperm[];

void vrdelta(int * Vd, int * Vu, int *Vv, int n)
{
   int offset, k;
      for (offset=1; offset<n; offset<<=1){
        for (k = 0; k<n; k++) {
            Vd[k] = (Vv[k]&offset) ? Vu[k^offset] : Vu[k];
        }
        for (k = 0; k<n; k++) {
            Vu[k] = Vd[k];
        }
      }
}

void vdelta(int * Vd, int * Vu, int *Vv, int n)
{
   int offset, k;
      for (offset=n; offset>=1; offset>>=1){
        for (k = 0; k<n; k++) {
            Vd[k] = (Vv[k]&offset) ? Vu[k^offset] : Vu[k];
        }
        for (k = 0; k<n; k++) {
            Vu[k] = Vd[k];
        }
      }
}
int tryvdelta(int * tperm, int n)
{
  int i,j;
  int pos,vec;

     printf("trying simple delta network\n");
     printf("desired outputs N = %d\n",n);
     for(i=0; i < n; i++)
     {
       if(tperm[i] != X) printf("%d,",tperm[i]); else printf("X,");
       inperm[i] = i;
       vperm[i] = 0;  //clear all muxes to pass through
     }
     printf("\n");


     for(i=0; i < n; i++)
     {
       vec = tperm[i];
       pos = i;
       if(vec != X)for(j=1; j < n; j<<=1)
       {
          int k;
          k = (pos & j) ^ (vec & j) ;
          vperm[pos] |= k;
          pos ^= k;
       }
     }
     vdelta(outperm, inperm, vperm, n);
     for(i=0; i < n; i++)
     {
         if(outperm[i] != tperm[i] && tperm[i] != X) {
           printf("pattern not possible using only vdelta\n");
           return(1);
         }
     }
     printf("//actual outputs \n//");
     for(i=0; i < n; i++) printf("%d,",outperm[i]);
     printf("\n");
     printf("};\n//vdelta controls.\nconst unsigned char vrd[%d] __attribute__((aligned(%d))) = {\n",n, n);
     for(i=0; i < n; i+=16)
     {
         for(j=0;j<16;j++) printf("0x%02X,",vperm[i+j]);
         printf("\n");
     }
     printf("\n};\n");

  return(0);
}

int tryvrdelta(int * tperm, int n)
{
  int i,j;
  int pos,vec;

     printf("trying simple reverse delta network\n");
     printf("desired outputs N = %d\n",n);
     for(i=0; i < n; i++)
     {
       if(tperm[i] != X) printf("%d,",tperm[i]); else printf("X,");
       inperm[i] = i;
       vperm[i] = 0;  //clear all muxes to pass through
     }
     printf("\n");


     for(i=0; i < n; i++)
     {
       vec = tperm[i];
       pos = i;
       if(vec != X)for(j=n/2; j > 0; j>>=1)
       {
          int k;
          k = (pos & j) ^ (vec & j) ;
          vperm[pos] |= k;
          pos ^= k;
       }
     }
     vrdelta(outperm, inperm, vperm, n);
     for(i=0; i < n; i++)
     {
         if(outperm[i] != tperm[i] && tperm[i] != X) {
           printf("pattern not possible using only vrdelta\n");
           return(1);
         }
     }
     printf("//actual outputs \n//");
     for(i=0; i < n; i++) printf("%d,",outperm[i]);
     printf("\n");
     printf("};\n//vrdelta controls.\nconst unsigned char vrd[%d] __attribute__((aligned(%d))) = {\n",n, n);
     for(i=0; i < n; i+=16)
     {
         for(j=0;j<16;j++) printf("0x%02X,",vperm[i+j]);
         printf("\n");
     }
     printf("\n};\n");

  return(0);
}
/*======================================================================*/
void printint(int a) {
   if(a < 10)
     printf("%d ",a);
   else if(a < 100)
     printf("%d",a);
   else
     printf("%d",a);
   return;
}
unsigned int brev(u32 x, u32 order) {
  u32 y;
  u32 mask = 0xffffffff >> (32-order) ;
  u32 base = x & ~mask;
  y = ((u32)Q6_R_brev_R(x)) >> (32-order);
  return( base | y);
}

void rand_perm(int perm[], int n){
    int i,j,k,tmp;

    for(i=0; i < n; i++)
    {
      perm[i] = i;
    }
    for(k=0; k < 32111; k++)
    {
       i=rand() % n;
       i ^= 0x5A5A5 % n;
       j=rand() % n;
       j ^= 0xA5A5A % n;
       tmp = perm[i];
       perm[i] = perm[j];
       perm[j] = tmp;
    }
}

void collect_switches(int n, int logn)
{
  int i,j;
  for(i=0; i < n/2; i++)
    gsb[i][logn-1] ^= gsb[i][logn];
  for(j=logn; j < 2*logn-1; j++)
  for(i=0; i < n/2; i++)
    gsb[i][j] = gsb[i][j+1];
}

void orthogonalizen(path top[], path bot[], int inperm[], int outperm[], int revperm[], int present[], int n)
{
  int i;
  int row, start;
  int sib, node;
  short group[LN*2]; //group of 4 x n/2 element top is even bot i odd


  for(i=0; i < 2*n; i++) group[i]=-1;
  for(start = 0; start < n; start+=2) {
    node = start;
    row = node/2;
    if(group[4*row] == -1) {
      group[4*row  ] = inperm[node];   group[4*row+1] = outperm[node];
      group[4*row+2] = inperm[node+1]; group[4*row+3] = outperm[node+1];
      sib = revperm[outperm[node]^1];
      while(sib != (start+1)) {
        row = sib/2;
        group[4*row  ] = inperm[sib^1];
        group[4*row+1] = outperm[sib^1];
        group[4*row+2] = inperm[sib];
        group[4*row+3] = outperm[sib];
        node = sib^1;
        sib = revperm[outperm[node]^1];
      }
    }//end if
  }//end start

  for(i=0; i < n/2; i++) {
    top[i].in = group[4*i];
    top[i].out= group[4*i+1];
    present[group[4*i  ]]=0;
    bot[i].in = group[4*i+2];
    bot[i].out= group[4*i+3];
    present[group[4*i+2]]=1;
  }
}

int gen_switch_cntrl(int logn, path *top, path * bot, int n, int * inperm, int * outperm, int * insb, int * outsb, int column, int row)
{
   int i, tmp;
   int inpermh[LN];
   int inperml[LN];
   int outpermh[LN];
   int outperml[LN];
   int outvec[LN];
   int present[LN];

   if(n == 1) return(0);

   for(i=0; i < n; i++) outvec[outperm[i]] = inperm[i];
   if(n==32) {
      //printf("inner switch\n");
      for(i=0; i < 32; i++) {
         vperm[i+32*block] = brev(outvec[brev(i,5)], 5);
      }
      block++;
   }

   if(n > 1) orthogonalizen(top, bot, inperm, outperm, outvec, present, n);
   for(i=0; i < n/2; i++)
   {
      top[i].dst = outvec[top[i].in];
      bot[i].dst = outvec[bot[i].in];
      if(inperm[2*i]== top[i].in) insb[i] = 0;  else insb[i] = 1;
      outsb[i] = present[outvec[2*i]];

      gsb[row+i][column] = insb[i];
      gsb[row+i][2*logn-1-column] = outsb[i];
   }
   for(i=0; i < n/2; i++)
   {
      //initial output values
      outvec[2*i+1] >>= 1;
      outvec[2*i] >>= 1;
      tmp = outvec[2*i+1];
      if(outsb[i]) {
        outvec[2*i+1] = outvec[2*i];
        outvec[2*i] = tmp;
      }
      //now the path controls
      top[i].in >>= 1;
      bot[i].in >>= 1;
      top[outvec[2*i]].out  = top[i].in ;
      bot[outvec[2*i+1]].out = bot[i].in ;
   }
   for(i=0; i < n/2; i++)
   {
      inpermh[i] = top[i].in; outpermh[i] = top[i].out;
      inperml[i] = bot[i].in; outperml[i] = bot[i].out;
   }
   //printf("calling switch size %d \n",n/2);
   gen_switch_cntrl(logn, top, bot, n/2, inpermh, outpermh, insb, outsb, column+1, row);
   gen_switch_cntrl(logn, top, bot, n/2, inperml, outperml, insb, outsb, column+1, row+n/4);

   return(0);
}

//takes the basic benes for and converts to constant geometry

void convert_const_geo(int vec_len, int log_vec_len) {

   int temp[LN];
   int i,j,k,m,n, s;
   m = vec_len/2;

         for(n=2,s=2, k=4; n < log_vec_len; n++,k<<=1,s++) {
           for(i=0; i < k; i++) {
              for(j=0; j < m/k; j++) {
                temp[i*m/k + j] = gsb[(m/k)*brev(i,s) + j][n];
              }
           }
           for(i=0; i < m; i++) gsb[i][n] = temp[i];
         }
         k>>=2; s-=2;
         for(n=log_vec_len; n < 2*log_vec_len-3; n++,k>>=1,s--) {
           for(i=0; i < k; i++) {
              for(j=0; j < m/k; j++) {
                temp[i*m/k + j] = gsb[(m/k)*brev(i,s) + j][n];
              }
           }
           for(i=0; i < m; i++) gsb[i][n] = temp[i];
         }
}


//takes the constant geometry and converts to butterfly network
//start with bit reversw
//2 x bit reverse n/2
//bit reverse groups of x bit reverse n/4

void convert_rev_butterfly(int gsc[][4*7], int n, int logn)
{
   int temp[LN];
   int i,j,m;
   int lo, hi;
   int logN2 = logn-1;

         lo = (1<<(logN2))-1;
         hi = 0;
         for(j=0; j < logn; j++) {
           for(i=0; i < n/2; i++) {
               m =  brev(lo & i, logN2-j);
               m |= brev(hi & (i>>(logN2-j)),j)<<(logN2-j);
               temp[m] = gsb[i][j];
           }
           lo >>= 1;
           hi = (hi<<1)|1;
           for(i=0; i < n/2; i++) gsc[i][j] = temp[brev(i,logN2)];
         }
         lo = (1<<(logN2))-1;
         hi = 0;
         for(j=0; j < logn-1; j++) {
           for(i=0; i < n/2; i++) {
               m =  brev(lo & i, logN2-j);
               m |= brev(hi & (i>>(logN2-j)),j)<<(logN2-j);
               temp[m] = gsb[i][2*logn-2-j];
           }
           for(i=0; i < n/2; i++) gsc[i][2*logN2-j] = temp[brev(i,logN2)];
           lo >>= 1;
           hi = (hi<<1)|1;
         }
}

void convert_butterfly(int gsc[][4*7], int n, int logn)
{
   int temp[LN];
   int i,j,m;
   int lo, hi;
   int logN2 = logn-1;

         lo = (1<<(logN2))-1;
         hi = 0;
         for(j=0; j < logn; j++) {
           //printf(" - %d -\n",logN2-j);
           for(i=0; i < n/2; i++) {
               m =  brev(lo & i, logN2-j);
               m |= brev(hi & (i>>(logN2-j)),j)<<(logN2-j); 
               temp[m] = gsb[i][j];
               //printf("%d \n",m);
           }
           lo >>= 1;
           hi = (hi<<1)|1;
           for(i=0; i < n/2; i++) gsc[i][j] = temp[i];
         }
         lo = (1<<(logN2))-1;
         hi = 0;
         for(j=0; j < logn-1; j++) {
           //printf(" - %d -\n",logN2-j);
           for(i=0; i < n/2; i++) {
               m =  brev(lo & i, logN2-j);
               m |= brev(hi & (i>>(logN2-j)),j)<<(logN2-j);
               temp[m] = gsb[i][2*logn-2-j];
               //printf("%d \n",m);
           }
           for(i=0; i < n/2; i++) gsc[i][2*logN2-j] = temp[i];
           lo >>= 1;
           hi = (hi<<1)|1;
         }
}

void invert_permute(int *outperm, int n)
{
   int i;
   int gtemp[2*LN];

   for(i=0; i < n; i++) {
     gtemp[outperm[i]] = i;
   }
   for(i=0; i < n; i++) {
     outperm[i] = gtemp[i];
   }
   return;
}

//output byte based control information for Hoya vbenes instruction.
void print_hvx_bits(int gsc[][4*7], int n, int logn) {

   int stride;
   int Vv[2*LN];
   int i,j,k,l;
         for(k=0; k < 2*n; k++) Vv[k] = 0x0;

         stride = 1;
         for(k=0; k < logn; k++) {
           for(l=0,j=0; j < n; j+=2*stride) {
             for(i=0; i < stride; i++) {
               Vv[j+i]        |= gsc[l][k]<<k;
               Vv[j+i+stride] |= gsc[l][k]<<k; l++;
             }
           }
           stride <<= 1;
         }

         stride >>= 2;
         for(k=logn-2; k >= 0; k--) {
           for(l=0,j=0; j < n; j+=2*stride) {
             for(i=0; i < stride; i++) {
               Vv[n+j+i       ] |= gsc[l][2*logn-2-k]<<k; ;
               Vv[n+stride+j+i] |= gsc[l][2*logn-2-k]<<k; l++;
             }
           }
           stride >>= 1;
         }

         printf("//vrdelta controls.\nconst unsigned char vrd[%d] __attribute__((aligned(%d))) = {\n",n,n);
         for(i=0; i < n; i+=8){
           for(j=0; j < 8; j++) {
              printf(" 0x%02X,",Vv[j+i]);
           }
           printf("\n");
         }
         printf("};\n//vdelta controls.\nconst unsigned char vd[%d] __attribute__((aligned(%d))) = {\n",n,n);
         for(i=n; i < 2*n; i+=8){
           for(j=0; j < 8; j++) {
              printf(" 0x%02X,",Vv[j+i]);
           }
           printf("\n");
         }
         printf("};\n");
}

//reversed to make it look right
void print_control_bits(int gsc[][4*7], int n, int logn) {

   int temp;
   int i,j,k;

         for(k=0; k < logn; k++) {
           //printf(" 0x");
           for(j=0; j < n/2; j+=32) {
             temp = 0;
             for(i=0; i < 32; i++) {
               temp |= gsc[j+i][k]<<i;
             }
             printf("0x%08X,   //level %d control \n",temp,k);
           }
         }
         for(k=logn; k < 2*logn-1; k++) {
           for(j=0; j < n/2; j+=32) {
             temp = 0;
             for(i=0; i < 32; i++) {
               temp |= gsc[j+i][k]<<i;
             }
             printf("0x%08X,   //level %d control \n",temp,k);
           }
         }
         printf("\n");
}
void print_bfly_flow(int gsc[][4*7], int n, int logn) {
    int i,j;
   printf("Butterfly Controls\n");
   for(i=0; i < n/2; i++)
   {
     for(j=0; j < 2*logn-1; j++)
        if(gsc[i][j]) printf("X  "); else printf("=  ");
     printf("\n");
   }
 }
void print_geo_flow(int n, int logn){
    int i,j;
   printf("Const GEO Controls\n");
   for(i=0; i < n/2; i++)
   {
     for(j=0; j < 2*logn-1; j++)
        if(gsb[i][j]) printf("X  "); else printf("=  ");
     printf("\n");
   }
 }

void check_perm(int gsc[][4*7], int * mux, int vec_len, int log_vec_len)
{
   int i,j,k,ki, stride;
   int tmux[128];
   for(i=0; i < vec_len; i++) mux[i] = i;
   for(j=0, stride=1; j < log_vec_len; j++, stride<<=1) {
     ki=0;
     for(k=0; k < vec_len; k+= 2*stride) {
       for(i=0; i < stride; i++)
       {
         if(gsc[ki][j]) {
           tmux[k+i]       = mux[k+i+stride];
           tmux[k+i+stride]= mux[k+i];
         } else {
           tmux[k+i]       = mux[k+i];
           tmux[k+i+stride]= mux[k+i+stride];
         }
         ki++;
       }
     }
     for(i=0; i < vec_len; i++) mux[i] = tmux[i];
   }
   for(j=log_vec_len, stride=vec_len/4; j < 2*log_vec_len-1; j++, stride>>=1) {
     ki=0;
     for(k=0; k < vec_len; k+= 2*stride) {
       for(i=0; i < stride; i++)
       {
         if(gsc[ki][j]) {
           tmux[k+i]       = mux[k+i+stride];
           tmux[k+i+stride]= mux[k+i];
         } else {
           tmux[k+i]       = mux[k+i];
           tmux[k+i+stride]= mux[k+i+stride];
         }
         ki++;
       }
     }
     for(i=0; i < vec_len; i++) mux[i] = tmux[i];
   }
}

int check_benes(int * tperm, int vec_len)
{
    int i, j, error = 0;;
    int hperm[128];

    for(i=0; i < vec_len; i++) hperm[i] = 0;
    for(i=0; i < vec_len; i++) if(tperm[i] != X) hperm[tperm[i]] += 1;

    for(i=0, j=0; i < vec_len; i++) {
      if(hperm[i] == 0) {  //found a missing element in permute
           hperm[i] = 1;
           while(tperm[j] != X && j < vec_len) j++;  //fill in next X
           tperm[j] = i;
      }
    }
    for(i=0; i < vec_len; i++) {
      if(hperm[i] != 1) { error = 1; printf("\n Benes network not possible\n"); break;}
    }
   return(error);
}

/*======================================================================*/
/*                             end of file                              */
/*======================================================================*/
/*                       QUALCOMM                                       */
/*                                                                      */
/*                       Copyright (c) 2014                             */
/*                       All Rights Reserved                            */
/*                                                                      */
/*======================================================================*/
