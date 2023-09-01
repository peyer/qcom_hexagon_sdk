/* ======================================================================== */
/*  QUALCOMM TECHNOLOGIES, INC.                                             */
/*                                                                          */
/*  HEXAGON HVX Image/Video Processing Library                              */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*          Copyright (c) 2015 QUALCOMM TECHNOLOGIES Incorporated.          */
/*                           All Rights Reserved.                           */
/*                  QUALCOMM Confidential and Proprietary                   */
/* ======================================================================== */

/*[========================================================================]*/
/*[ FUNCTION                                                               ]*/
/*[     Mipi to Raw16                                                      ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function decompresses the packed 10-bit bayer to 16-bit       ]*/
/*[     output.                                                            ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     SEPT-24-2015                                                       ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include "hvx.cfg.h"

/* ======================================================================== */
/*  Macros                                                                  */
/* ======================================================================== */
#define roundup_t(a, m)    (((a)+(m)-1)&(-m))
#define vmem(A)  *((HVX_Vector*)(A))
#if defined(__hexagon__)
typedef long HEXAGON_Vect_UN __attribute__((__vector_size__(VLEN)))__attribute__((aligned(4)));
#define vmemu(A) *((HEXAGON_Vect_UN*)(A))
#endif

#define LINES 2

/* ======================================================================== */
/*  Global Variables.                                                       */
/* ======================================================================== */
const unsigned char CtrlPerm[128*11] __attribute__((aligned(128))) = {
//0,1,2,3,5,6,7,8,10,11,12,13,15,16,17,18,20,21,22,23,25,26,27,28,30,31,32,33,35,36,37,38,40,41,42,43,45,46,47,48,50,51,52,53,55,56,57,58,60,61,62,63,65,66,67,68,70,71,72,73,75,76,77,78,80,81,82,83,85,86,87,88,90,91,92,93,95,96,97,98,100,101,102,103,105,106,107,108,110,111,112,113,115,116,117,118,120,121,122,123,125,126,127,108,110,111,107,106,105,96,97,98,100,101,102,103,65,66,67,68,70,71,72,73,75,76,77,78,
0x00,0x00,0x00,0x00,0x01,0x02,0x01,0x09,0x02,0x03,0x04,0x06,0x02,0x10,0x13,0x16,
0x05,0x05,0x06,0x05,0x08,0x0C,0x0C,0x0C,0x05,0x06,0x21,0x21,0x26,0x2F,0x2C,0x2A,
0x0A,0x08,0x0B,0x0A,0x0D,0x09,0x0A,0x11,0x10,0x10,0x18,0x18,0x19,0x1A,0x19,0x19,
0x0A,0x0B,0x0C,0x0E,0x42,0x40,0x43,0x46,0x4D,0x4D,0x5E,0x5D,0x58,0x54,0x54,0x54,
0x15,0x16,0x11,0x11,0x16,0x17,0x14,0x12,0x1A,0x18,0x13,0x12,0x15,0x21,0x22,0x21,
0x20,0x20,0x20,0x20,0x31,0x32,0x31,0x39,0x32,0x33,0x34,0x36,0x32,0x30,0x33,0x36,
0x15,0x15,0x16,0x15,0x18,0x1C,0x1C,0x0C,0x05,0x06,0x01,0x01,0x06,0x0F,0x0C,0x0A,
0x1A,0x18,0x1B,0x1A,0x3D,0x39,0x3A,0x31,0x30,0x30,0x28,0x28,0x29,0x2A,0x29,0x29,

//4,4,4,4,9,9,9,9,14,14,14,14,19,19,19,19,24,24,24,24,29,29,29,29,34,34,34,34,39,39,39,39,44,44,44,44,49,49,49,49,54,54,54,54,59,59,59,59,64,64,64,64,69,69,69,69,74,74,74,74,79,79,79,79,84,84,84,84,89,89,89,89,94,94,94,94,99,99,99,99,104,104,104,104,109,109,109,109,114,114,114,114,119,119,119,119,124,124,124,124,109,109,109,109,104,104,104,104,99,99,99,99,64,64,64,64,69,69,69,69,74,74,74,74,79,79,79,79,
0x04,0x04,0x04,0x04,0x08,0x09,0x0A,0x0A,0x05,0x04,0x06,0x06,0x16,0x16,0x14,0x15,
0x0A,0x0A,0x09,0x08,0x0C,0x0C,0x0C,0x0C,0x20,0x21,0x22,0x22,0x29,0x28,0x2A,0x2A,
0x0A,0x0A,0x08,0x09,0x12,0x12,0x11,0x10,0x1C,0x1C,0x1C,0x1C,0x18,0x19,0x1A,0x1A,
0x45,0x44,0x46,0x46,0x46,0x46,0x44,0x45,0x5A,0x5A,0x59,0x58,0x54,0x54,0x54,0x54,
0x10,0x11,0x12,0x12,0x11,0x10,0x12,0x12,0x12,0x12,0x10,0x11,0x22,0x22,0x21,0x20,
0x34,0x34,0x34,0x34,0x38,0x39,0x3A,0x3A,0x35,0x34,0x36,0x36,0x36,0x36,0x34,0x35,
0x1A,0x1A,0x19,0x18,0x0C,0x0C,0x0C,0x0C,0x00,0x01,0x02,0x02,0x09,0x08,0x0A,0x0A,
0x3A,0x3A,0x38,0x39,0x32,0x32,0x31,0x30,0x2C,0x2C,0x2C,0x2C,0x28,0x29,0x2A,0x2A,

//32,33,34,35,37,38,39,40,42,43,44,45,47,48,49,50,52,53,54,55,57,58,59,60,62,63,64,65,67,68,69,70,72,73,74,75,77,78,79,80,82,83,84,85,87,88,89,90,92,93,94,95,97,98,99,100,102,103,104,105,107,108,109,110,112,113,114,115,117,118,119,120,122,123,124,125,127,88,89,90,92,93,94,95,87,85,84,80,82,83,64,65,67,68,69,70,72,73,74,75,77,78,79,0,2,3,4,5,7,8,9,10,12,13,14,15,17,18,19,20,22,23,24,25,27,28,29,30,
0x2A,0x2B,0x2C,0x2E,0x22,0x20,0x23,0x26,0x2D,0x2D,0x2E,0x2D,0x28,0x24,0x24,0x24,
0x35,0x36,0x31,0x31,0x26,0x27,0x24,0x22,0x2A,0x28,0x43,0x42,0x45,0x41,0x42,0x41,
0x40,0x40,0x40,0x40,0x41,0x42,0x41,0x69,0x62,0x63,0x64,0x66,0x62,0x70,0x73,0x76,
0x65,0x65,0x66,0x65,0x68,0x6C,0x6C,0x6C,0x65,0x66,0x61,0x61,0x66,0x6F,0x6C,0x6A,
0x2A,0x28,0x2B,0x2A,0x2D,0x29,0x2A,0x31,0x30,0x30,0x38,0x38,0x39,0x1A,0x19,0x19,
0x0A,0x0B,0x0C,0x0E,0x02,0x00,0x03,0x06,0x0D,0x0D,0x1E,0x1D,0x18,0x14,0x14,0x14,
0x35,0x36,0x31,0x31,0x36,0x37,0x34,0x72,0x7A,0x78,0x73,0x72,0x75,0x61,0x62,0x61,
0x60,0x60,0x60,0x60,0x51,0x52,0x51,0x59,0x52,0x53,0x54,0x56,0x52,0x50,0x53,0x56,

//36,36,36,36,41,41,41,41,46,46,46,46,51,51,51,51,56,56,56,56,61,61,61,61,66,66,66,66,71,71,71,71,76,76,76,76,81,81,81,81,86,86,86,86,91,91,91,91,96,96,96,96,101,101,101,101,106,106,106,106,111,111,111,111,116,116,116,116,121,121,121,121,126,126,126,126,91,91,91,91,86,86,86,86,81,81,81,81,66,66,66,66,71,71,71,71,76,76,76,76,1,1,1,1,6,6,6,6,11,11,11,11,16,16,16,16,21,21,21,21,26,26,26,26,31,31,31,31,
0x25,0x24,0x26,0x26,0x26,0x26,0x24,0x25,0x2A,0x2A,0x29,0x28,0x24,0x24,0x24,0x24,
0x20,0x21,0x22,0x22,0x21,0x20,0x22,0x22,0x42,0x42,0x40,0x41,0x42,0x42,0x41,0x40,
0x44,0x44,0x44,0x44,0x68,0x69,0x6A,0x6A,0x65,0x64,0x66,0x66,0x76,0x76,0x74,0x75,
0x6A,0x6A,0x69,0x68,0x6C,0x6C,0x6C,0x6C,0x60,0x61,0x62,0x62,0x69,0x68,0x6A,0x6A,
0x2A,0x2A,0x28,0x29,0x32,0x32,0x31,0x30,0x3C,0x3C,0x3C,0x3C,0x18,0x19,0x1A,0x1A,
0x05,0x04,0x06,0x06,0x06,0x06,0x04,0x05,0x1A,0x1A,0x19,0x18,0x14,0x14,0x14,0x14,
0x30,0x31,0x32,0x32,0x71,0x70,0x72,0x72,0x72,0x72,0x70,0x71,0x62,0x62,0x61,0x60,
0x54,0x54,0x54,0x54,0x58,0x59,0x5A,0x5A,0x55,0x54,0x56,0x56,0x56,0x56,0x54,0x55,

//64,65,66,67,69,70,71,72,74,75,76,77,79,80,81,82,84,85,86,87,89,90,91,92,94,95,96,97,99,100,101,102,104,105,106,107,109,110,111,112,114,115,116,117,119,120,121,122,124,125,126,127,49,50,51,52,54,55,56,57,59,60,61,62,64,65,66,67,69,70,71,72,74,75,76,77,79,0,1,2,4,5,6,7,9,10,11,12,14,15,16,17,19,20,21,22,24,25,26,27,29,30,31,32,34,35,36,37,39,40,41,42,44,45,46,47,49,50,51,52,54,55,56,57,59,60,61,62,
0x55,0x55,0x56,0x55,0x58,0x5C,0x5C,0x4C,0x45,0x46,0x41,0x41,0x46,0x4F,0x4C,0x4A,
0x5A,0x58,0x5B,0x5A,0x5D,0x59,0x5A,0x51,0x50,0x50,0x48,0x48,0x49,0x4A,0x49,0x49,
0x6A,0x6B,0x6C,0x6E,0x62,0x60,0x63,0x46,0x4D,0x4D,0x4E,0x4D,0x48,0x44,0x44,0x44,
0x55,0x56,0x51,0x51,0x06,0x07,0x04,0x02,0x0A,0x08,0x03,0x02,0x05,0x01,0x02,0x01,
0x00,0x00,0x00,0x00,0x01,0x02,0x01,0x09,0x02,0x03,0x04,0x06,0x02,0x50,0x53,0x56,
0x45,0x45,0x46,0x45,0x48,0x4C,0x4C,0x4C,0x45,0x46,0x61,0x61,0x66,0x6F,0x6C,0x6A,
0x4A,0x48,0x4B,0x4A,0x4D,0x49,0x4A,0x51,0x50,0x50,0x58,0x58,0x59,0x5A,0x59,0x59,
0x4A,0x4B,0x4C,0x4E,0x42,0x40,0x43,0x46,0x4D,0x4D,0x5E,0x5D,0x58,0x54,0x54,0x54,

//68,68,68,68,73,73,73,73,78,78,78,78,83,83,83,83,88,88,88,88,93,93,93,93,98,98,98,98,103,103,103,103,108,108,108,108,113,113,113,113,118,118,118,118,123,123,123,123,48,48,48,48,53,53,53,53,58,58,58,58,63,63,63,63,68,68,68,68,73,73,73,73,78,78,78,78,3,3,3,3,8,8,8,8,13,13,13,13,18,18,18,18,23,23,23,23,28,28,28,28,33,33,33,33,38,38,38,38,43,43,43,43,48,48,48,48,53,53,53,53,58,58,58,58,63,63,63,63,
0x5A,0x5A,0x59,0x58,0x4C,0x4C,0x4C,0x4C,0x40,0x41,0x42,0x42,0x49,0x48,0x4A,0x4A,
0x5A,0x5A,0x58,0x59,0x52,0x52,0x51,0x50,0x4C,0x4C,0x4C,0x4C,0x48,0x49,0x4A,0x4A,
0x65,0x64,0x66,0x66,0x46,0x46,0x44,0x45,0x4A,0x4A,0x49,0x48,0x44,0x44,0x44,0x44,
0x00,0x01,0x02,0x02,0x01,0x00,0x02,0x02,0x02,0x02,0x00,0x01,0x02,0x02,0x01,0x00,
0x04,0x04,0x04,0x04,0x08,0x09,0x0A,0x0A,0x05,0x04,0x06,0x06,0x56,0x56,0x54,0x55,
0x4A,0x4A,0x49,0x48,0x4C,0x4C,0x4C,0x4C,0x60,0x61,0x62,0x62,0x69,0x68,0x6A,0x6A,
0x4A,0x4A,0x48,0x49,0x52,0x52,0x51,0x50,0x5C,0x5C,0x5C,0x5C,0x58,0x59,0x5A,0x5A,
0x45,0x44,0x46,0x46,0x46,0x46,0x44,0x45,0x5A,0x5A,0x59,0x58,0x54,0x54,0x54,0x54,

//96,97,98,99,101,102,103,104,106,107,108,109,111,112,113,114,116,117,118,119,121,122,123,124,126,127,48,49,51,52,53,54,56,57,58,59,61,62,63,44,46,47,43,42,41,32,33,34,36,37,38,39,1,2,3,4,6,7,8,9,11,12,13,14,16,17,18,19,21,22,23,24,26,27,28,29,31,32,33,34,36,37,38,39,41,42,43,44,46,47,48,49,51,52,53,54,56,57,58,59,61,62,63,64,66,67,68,69,71,72,73,74,76,77,78,79,81,82,83,84,86,87,88,89,91,92,93,94,
0x55,0x56,0x51,0x51,0x56,0x57,0x54,0x52,0x5A,0x58,0x53,0x52,0x55,0x61,0x62,0x61,
0x60,0x60,0x60,0x60,0x71,0x72,0x71,0x79,0x72,0x73,0x34,0x36,0x32,0x30,0x33,0x36,
0x15,0x15,0x16,0x15,0x18,0x1C,0x1C,0x0C,0x05,0x06,0x01,0x01,0x06,0x0F,0x0C,0x0A,
0x1A,0x18,0x1B,0x1A,0x3D,0x39,0x3A,0x31,0x30,0x30,0x28,0x28,0x29,0x2A,0x29,0x29,
0x6A,0x6B,0x6C,0x6E,0x62,0x60,0x63,0x66,0x6D,0x6D,0x6E,0x6D,0x68,0x64,0x64,0x64,
0x75,0x76,0x71,0x71,0x66,0x67,0x64,0x62,0x6A,0x68,0x43,0x42,0x45,0x41,0x42,0x41,
0x40,0x40,0x40,0x40,0x41,0x42,0x41,0x29,0x22,0x23,0x24,0x26,0x22,0x30,0x33,0x36,
0x25,0x25,0x26,0x25,0x28,0x2C,0x2C,0x2C,0x25,0x26,0x21,0x21,0x26,0x2F,0x2C,0x2A,

//100,100,100,100,105,105,105,105,110,110,110,110,115,115,115,115,120,120,120,120,125,125,125,125,50,50,50,50,55,55,55,55,60,60,60,60,45,45,45,45,40,40,40,40,35,35,35,35,0,0,0,0,5,5,5,5,10,10,10,10,15,15,15,15,20,20,20,20,25,25,25,25,30,30,30,30,35,35,35,35,40,40,40,40,45,45,45,45,50,50,50,50,55,55,55,55,60,60,60,60,65,65,65,65,70,70,70,70,75,75,75,75,80,80,80,80,85,85,85,85,90,90,90,90,95,95,95,95,
0x50,0x51,0x52,0x52,0x51,0x50,0x52,0x52,0x52,0x52,0x50,0x51,0x62,0x62,0x61,0x60,
0x74,0x74,0x74,0x74,0x78,0x79,0x7A,0x7A,0x35,0x34,0x36,0x36,0x36,0x36,0x34,0x35,
0x1A,0x1A,0x19,0x18,0x0C,0x0C,0x0C,0x0C,0x00,0x01,0x02,0x02,0x09,0x08,0x0A,0x0A,
0x3A,0x3A,0x38,0x39,0x32,0x32,0x31,0x30,0x2C,0x2C,0x2C,0x2C,0x28,0x29,0x2A,0x2A,
0x65,0x64,0x66,0x66,0x66,0x66,0x64,0x65,0x6A,0x6A,0x69,0x68,0x64,0x64,0x64,0x64,
0x60,0x61,0x62,0x62,0x61,0x60,0x62,0x62,0x42,0x42,0x40,0x41,0x42,0x42,0x41,0x40,
0x44,0x44,0x44,0x44,0x28,0x29,0x2A,0x2A,0x25,0x24,0x26,0x26,0x36,0x36,0x34,0x35,
0x2A,0x2A,0x29,0x28,0x2C,0x2C,0x2C,0x2C,0x20,0x21,0x22,0x22,0x29,0x28,0x2A,0x2A,

//48,49,50,51,53,54,55,56,58,59,60,61,63,24,25,26,28,29,30,31,23,21,20,16,18,19,0,1,3,4,5,6,8,9,10,11,13,14,15,16,18,19,20,21,23,24,25,26,28,29,30,31,33,34,35,36,38,39,40,41,43,44,45,46,48,49,50,51,53,54,55,56,58,59,60,61,63,64,65,66,68,69,70,71,73,74,75,76,78,79,80,81,83,84,85,86,88,89,90,91,93,94,95,96,98,99,100,101,103,104,105,106,108,109,110,111,113,114,115,116,118,119,120,121,123,124,125,126,
0x2A,0x28,0x2B,0x2A,0x2D,0x29,0x2A,0x31,0x30,0x30,0x38,0x38,0x39,0x1A,0x19,0x19,
0x0A,0x0B,0x0C,0x0E,0x02,0x00,0x03,0x06,0x0D,0x0D,0x1E,0x1D,0x18,0x14,0x14,0x14,
0x35,0x36,0x31,0x31,0x36,0x37,0x34,0x32,0x3A,0x38,0x33,0x32,0x35,0x21,0x22,0x21,
0x20,0x20,0x20,0x20,0x11,0x12,0x11,0x19,0x12,0x13,0x14,0x16,0x12,0x10,0x13,0x16,
0x55,0x55,0x56,0x55,0x58,0x5C,0x5C,0x4C,0x45,0x46,0x41,0x41,0x46,0x0F,0x0C,0x0A,
0x1A,0x18,0x1B,0x1A,0x1D,0x19,0x1A,0x11,0x10,0x10,0x08,0x08,0x09,0x0A,0x09,0x09,
0x2A,0x2B,0x2C,0x2E,0x22,0x20,0x23,0x06,0x0D,0x0D,0x0E,0x0D,0x08,0x04,0x04,0x04,
0x15,0x16,0x11,0x11,0x06,0x07,0x04,0x02,0x0A,0x08,0x03,0x02,0x05,0x01,0x02,0x01,

//52,52,52,52,57,57,57,57,62,62,62,62,27,27,27,27,22,22,22,22,17,17,17,17,2,2,2,2,7,7,7,7,12,12,12,12,17,17,17,17,22,22,22,22,27,27,27,27,32,32,32,32,37,37,37,37,42,42,42,42,47,47,47,47,52,52,52,52,57,57,57,57,62,62,62,62,67,67,67,67,72,72,72,72,77,77,77,77,82,82,82,82,87,87,87,87,92,92,92,92,97,97,97,97,102,102,102,102,107,107,107,107,112,112,112,112,117,117,117,117,122,122,122,122,127,127,127,127,
0x2A,0x2A,0x28,0x29,0x32,0x32,0x31,0x30,0x3C,0x3C,0x3C,0x3C,0x18,0x19,0x1A,0x1A,
0x05,0x04,0x06,0x06,0x06,0x06,0x04,0x05,0x1A,0x1A,0x19,0x18,0x14,0x14,0x14,0x14,
0x30,0x31,0x32,0x32,0x31,0x30,0x32,0x32,0x32,0x32,0x30,0x31,0x22,0x22,0x21,0x20,
0x14,0x14,0x14,0x14,0x18,0x19,0x1A,0x1A,0x15,0x14,0x16,0x16,0x16,0x16,0x14,0x15,
0x5A,0x5A,0x59,0x58,0x4C,0x4C,0x4C,0x4C,0x40,0x41,0x42,0x42,0x09,0x08,0x0A,0x0A,
0x1A,0x1A,0x18,0x19,0x12,0x12,0x11,0x10,0x0C,0x0C,0x0C,0x0C,0x08,0x09,0x0A,0x0A,
0x25,0x24,0x26,0x26,0x06,0x06,0x04,0x05,0x0A,0x0A,0x09,0x08,0x04,0x04,0x04,0x04,
0x00,0x01,0x02,0x02,0x01,0x00,0x02,0x02,0x02,0x02,0x00,0x01,0x02,0x02,0x01,0x00,

//8 Q registers for muxing
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f,0x7f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,
0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,
0x1f,0x1f,0x1f,0x1f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x07,0x03,0x03,0x03,
0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
0x03,0x03,0x03,0x03,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

/* ======================================================================== */
/*  Functions defined in Assembly                                           */
/* ======================================================================== */
void convertMipiToRaw16_aligned_PerNRow(
    unsigned char  *input,
    unsigned        istride,
    unsigned        width,
    unsigned        height,
    unsigned short *output,
    unsigned        ostride
    );


/* ======================================================================== */
void convertMipiToRaw16(
    unsigned char  *input,
    unsigned        istride,
    unsigned        width,
    unsigned        height,
    unsigned short *output,
    unsigned        ostride
    )
{
    int ys;

    for (ys = 0; ys < height; ys += LINES)
    {
        convertMipiToRaw16_aligned_PerNRow(input, istride, width, LINES, output, ostride);
        input += LINES * istride;
        output += LINES * ostride;
    }
}
