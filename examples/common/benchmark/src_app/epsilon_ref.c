/**=============================================================================
Copyright (c) 2016 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

static unsigned short invLUT[128] __attribute__((aligned(128))) = {
0,16384,8192,5461,4096,3276,2730,2340,2048,1820,1638,1489,1365,1260,1170,1092,
1024,963,910,862,819,780,744,712,682,655,630,606,585,564,546,528,
512,496,481,468,455,442,431,420,409,399,390,381,372,364,356,348,
341,334,327,321,315,309,303,297,292,287,282,277,273,268,264,260,
256,252,248,244,240,237,234,230,227,224,221,218,215,212,210,207,
204,202,199,197,195,192,190,188,186,184,182,180,178,176,174,172,
170,168,167,165,163,162,160,159,157,156,154,153,151,150,148,147,
146,144,143,142,141,140,138,137,136,135,134,133,132,131,130,129
};

void epsilon_ref (unsigned char *src, int stride, int width, int height, int threshold, unsigned char *dst)
{
  int x, y, s, t;
  unsigned short sum, cnt;
  int tmp;

  for (y = 4; y < height - 4; y++)
  {
    for (x = 4; x < width - 4; x++)
    {
        const unsigned char center = src[y*stride + x];
        sum = 0;
        cnt = 0;

        int xyoff = x - 4 * stride;

        for (t = -4; t <= 4; t++)
        {
            for (s = -4; s <= 4; s++)
            {
                const unsigned char p = src[y*stride + xyoff + s];
                if ((p > center ? p - center : center - p) <= threshold)
                {
                    sum += p;
                    cnt++;
                }
            }
            xyoff += stride;
        }

        tmp = (sum*invLUT[cnt])>> 14;
        dst[y*stride + x] = (unsigned char)((tmp>255) ? 255 : tmp);
    } 
  }
}

