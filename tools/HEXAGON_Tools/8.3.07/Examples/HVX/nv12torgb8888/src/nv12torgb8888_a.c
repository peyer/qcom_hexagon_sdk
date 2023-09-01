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

/* ======================================================================== */
/*  Functions defined in Assembly                                           */
/* ======================================================================== */
void
color_NV12toRGB8888_line(
    unsigned char* __restrict yuv420sp,
    unsigned char* __restrict uv420sp,
    unsigned char* __restrict rgb,
    int                       width,
    int                       stride
);

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
    int j;
    unsigned *rgbdst = (unsigned *)rgb;

    for (j = 0; j < height; j += 2)
    {
        color_NV12toRGB8888_line(yuv420sp, uv420sp, (unsigned char *)rgbdst, width, stride);
        yuv420sp += 2 * stride;
        uv420sp += stride;
        rgbdst += 2 * stride;
    }
}
