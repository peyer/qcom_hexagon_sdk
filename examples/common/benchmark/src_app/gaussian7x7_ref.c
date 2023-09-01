/**=============================================================================
Copyright (c) 2016 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
/* ======================================================================== */
/*  Reference C version of gaussian7x7u8()                                  */
/* ======================================================================== */
static const int GAUSS_7x7[7*7] = {
   1,   6,  15,  20,  15,   6,  1,
   6,  36,  90, 120,  90,  36,  6,
  15,  90, 225, 300, 225,  90, 15,
  20, 120, 300, 400, 300, 120, 20,
  15,  90, 225, 300, 225,  90, 15,
   6,  36,  90, 120,  90,  36,  6,
   1,   6,  15,  20,  15,   6,  1
};


void gaussian7x7_ref(
    unsigned char *src,
    int            width,
    int            height,
    int            stride,
    unsigned char *dst,
    int            dstStride
    )
{

    int x, y, s, t;
    int sum, out;

    for (y = 3; y < height - 3; y++)
    {
        for (x = 3; x < width - 3; x++)
        {
            sum = 0;
            for (t = -3; t <= 3; t++)
            {
                for (s = -3; s <= 3; s++)
                {
                    sum += src[(y+t)*stride + x+s] * GAUSS_7x7[((t+3)*7)+(s+3)];
                }
            }
            out  = sum >> 12;
            out = out < 0 ? 0 : out > 255 ? 255 : out;
            dst[y*dstStride + x] = (unsigned char)out;
        }
    }
}

