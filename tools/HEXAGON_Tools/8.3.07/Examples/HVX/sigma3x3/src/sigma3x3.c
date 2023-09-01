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
/*[     sigma3x3                                                           ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function performs 3x3 sigma filtering on an image block       ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/

/* ======================================================================== */
/*  Reference C version of sigma3x3()                                       */
/* ======================================================================== */
const int invTable[10] = {
    0,32768,16384,10922,8192,6553,5461,4681,4096,3640
};


/* ======================================================================== */
void sigma3x3(
    unsigned char   *src,
    int             stride,
    int             width,
    int             height,
    unsigned char   threshold,
    unsigned char   *dst
    )
{
    int x, y, s, t;
    int p, center, diff;
    int sum, cnt;

    for (y = 1; y < height - 1; y++)
    {
        for (x = 1; x < width - 1; x++)
        {
            center = src[y*stride + x];

            sum = 0;
            cnt = 0;
            for (t = -1; t <= 1; t++)
            {
                for (s = -1; s <= 1; s++)
                {
                    p = src[(y+t)*stride + x + s];
                    diff = p > center ? (p - center) : (center - p);

                    if (diff <= threshold)
                    {
                        sum += p;
                        cnt++;
                    }
                }
            }

            dst[y*stride + x] = (sum * invTable[cnt] + (1<<14))>>15;
        }
    }
}
