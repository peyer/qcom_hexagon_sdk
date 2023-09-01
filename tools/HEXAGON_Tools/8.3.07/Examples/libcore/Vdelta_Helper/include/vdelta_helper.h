/*======================================================================*/
/*                       QUALCOMM                                       */
/*                                                                      */
/*                       Copyright (c) 2014                             */
/*                       All Rights Reserved                            */
/*                                                                      */
/*======================================================================*/
/*  FUNCTIONS      : Shuffle Exchange Control Generator                 */
/*  ARCHITECTURE   :  any                                               */
/*  VERSION        : 2.1                                                */
/*                                                                      */
/*======================================================================*/
/*  REVISION HISTORY:                                                   */
/*  =================                                                   */
/*                                                                      */
/*  Author       Date       Comments                                    */
/*  -------------------------------------------------------------       */
/*  DJH          02/09/11   created iniitial Benese Netowrk Generator   */
/*  DJH          07/06/11   modified for shuffle Exchange INsturction   */
/*  DJH          08/26/11   reversed control bits, changed sense of the */
/*                          permute understandign input by the algorithm*/
/*  DJH          06/06/13   modified for vrdelta and vdelta networks    */
/*======================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define u16  unsigned short
#define u32  unsigned int
#define X -1
/*======================================================================*/
 typedef struct {
    int in;    //input
    int out;   //is the path that the input goes to
    int dst;   //dst is what shows up on this output
 } path;  //128bits per path

/*======================================================================*/
void vrdelta(int * Vd, int * Vu, int *Vv, int n);
void vdelta(int * Vd, int * Vu, int *Vv, int n);
int tryvrdelta(int *, int n);
int tryvdelta(int *, int n);
int check_benes(int * tperm, int vec_len);
void printint(int a) ;
u32 brev(u32 x, u32 order) ;
void rand_perm(int perm[], int n);
void collect_switches(int n, int logn);
void orthogonalizen(path top[], path bot[], int inperm[], int outperm[], int revperm[], int present[], int n);
int gen_switch_cntrl(int logn, path *top, path * bot, int n, int * inperm, int * outperm, int * insb, int * outsb, int column, int row);
void convert_const_geo(int, int) ;
void convert_rev_butterfly(int ptr[][4*7], int, int);
void convert_butterfly(int ptr[][4*7], int, int);
void invert_permute(int *outperm, int);
void print_hvx_bits(int ptr[][4*7], int, int);
void print_control_bits(int ptr[][4*7], int, int);
void print_bfly_flow(int ptr[][4*7], int, int);
void print_geo_flow(int, int);
void check_perm(int gsc[][4*7], int * mux, int len, int log_len);
/*======================================================================*/
/*                             end of file                              */
/*======================================================================*/
/*                       QUALCOMM                                       */
/*                                                                      */
/*                       Copyright (c) 2014                             */
/*                       All Rights Reserved                            */
/*                                                                      */
/*======================================================================*/
