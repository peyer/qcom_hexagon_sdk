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
/*[     sigma9x9                                                           ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function performs 9x9 sigma filtering on an image block       ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     OCT-21-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/

/* ======================================================================== */
/*  Reference C version of sigma9x9()                                       */
/* ======================================================================== */
const int invLUT[128] __attribute__((aligned(128))) = {
    0,32768,16384,10923,8192,6554,5461,4681,4096,3641,3277,2979,2731,2521,2341,2185,
    2048,1928,1820,1725,1638,1560,1489,1425,1365,1311,1260,1214,1170,1130,1092,1057,
    1024,993,964,936,910,886,862,840,819,799,780,762,745,728,712,697,
    683,669,655,643,630,618,607,596,585,575,565,555,546,537,529,520,
    512,504,496,489,482,475,468,462,455,449,443,437,431,426,420,415,
    410,405,400,395,390,386,381,377,372,368,364,360,356,352,349,345,
    341,338,334,331,328,324,321,318,315,312,309,306,303,301,298,295,
    293,290,287,285,282,280,278,275,273,271,269,266,264,262,260,258
};


/* ======================================================================== */
void sigma9x9(
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
    int sum, cnt, out;

    for (y = 4; y < height - 4; y++)
    {
        for (x = 4; x < width - 4; x++)
        {
            center = src[y*stride + x];

            sum = 0;
            cnt = 0;
            for (t = -4; t <= 4; t++)
            {
                for (s = -4; s <= 4; s++)
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

            out = (sum * invLUT[cnt] + (1<<14)) >> 15;
            dst[y*stride + x] = (unsigned char)(out > 255 ? 255: out);
        }
    }
}
