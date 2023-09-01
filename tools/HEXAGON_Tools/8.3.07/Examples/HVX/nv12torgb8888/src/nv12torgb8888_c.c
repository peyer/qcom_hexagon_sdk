/* ======================================================================== */
/*  QUALCOMM TECHNOLOGIES, INC.                                             */
/*                                                                          */
/*  HEXAGON HVX Image/Video Processing Library                              */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*          Copyright (c) 2014 QUALCOMM TECHNOLOGIES Incorporated.          */
/*                           All Rights Reserved.                           */
/*                  QUALCOMM Confidential and Proprietary                   */
/* ======================================================================== */

/*[========================================================================]*/
/*[ FUNCTION                                                               ]*/
/*[     color_NV12toRGB8888                                                ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function converts image from NV12 format to RGB8888 format.   ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include "nv12torgb8888.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
color_NV12toRGB8888(
    unsigned char* __restrict yuv420sp,
    unsigned char* __restrict uv420sp,
    unsigned char* __restrict rgb,
    int                       height,
    int                       width,
    int                       stride
)
{
    int yp = 0;
    int j;
    int uvp;
    int i;
    unsigned int* __restrict  dst_ptr = (unsigned int*)rgb;

    for (j = 0; j < height; j++)
    {
        int u = 0;
        int v = 0;

        uvp = (j >> 1) * width;
        for (i = 0; i < width; i++, yp++)
        {
            int y = (0xff & ((int)yuv420sp[yp])) - 16;
            int y1192;
            int r;
            int g;
            int b;
            unsigned rgb1;

            if (y < 0)
                y = 0;
            if ((i & 1) == 0)
            {
                v = (0xff & uv420sp[uvp++]) - 128;
                u = (0xff & uv420sp[uvp++]) - 128;
            }

            y1192 = 1192 * y;
            r = (y1192 + 1634 * v);
            g = (y1192 - 833 * v - 400 * u);
            b = (y1192 + 2066 * u);

            if (r < 0)
                r = 0;
            else if (r > 262143)
                r = 262143;
            if (g < 0)
                g = 0;
            else if (g > 262143)
                g = 262143;
            if (b < 0)
                b = 0;
            else if (b > 262143)
                b = 262143;

            rgb1 = 0xff000000 | ((b << 6) & 0xff0000)
                | ((g >> 2) & 0xff00) | ((r >> 10) & 0xff);
            dst_ptr[yp] = rgb1;
        }
    }
}

