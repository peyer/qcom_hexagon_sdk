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
/*[     bilateral9x9                                                       ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function applies a 9x9 bilateral filter to a image.           ]*/
/*[ The intensity value at each pixel in an image is replaced by a weighted]*/
/*[ average of intensity vaules from nearby pixels.                        ]*/
/*[     It is widely used for image smoothing with edge-perserving.        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#define ABS(x)      ((x) > 0 ? (x) : -(x))
/* ======================================================================== */
/*  Reference C version of bilateral9x9().                                  */
/* ======================================================================== */
void bilateral9x9(
    unsigned char   *input,
    int             stride,
    int             width,
    int             height,
    unsigned char   *gauss_LUT,
    unsigned char   *range_LUT,
    unsigned char   *output
    )
{
    int x, y, bx, by;
    unsigned char centr, pixel, weight;
    unsigned int  filtered, weights;

    for(y = 4; y < (height-4); y++)
    {
        for(x = 4; x < (width-4); x++)
        {
            // block processing
            filtered = 0;
            weights = 0;

            centr = input[y*stride+x];

            for(by = -4; by <= 4; by++)
            {
                for(bx = -4; bx <= 4; bx++)
                {
                    pixel = input[(y+by)*stride + (x+bx)];
                    weight = (range_LUT[ABS(pixel-centr)]*gauss_LUT[(by+4)*9+(bx+4)])>>8;
                    filtered += pixel*weight;
                    weights  += weight;
                }
            }

            output[y*stride+x] = weights ? filtered/weights : 0;
        }
    }

    return;
}

