/**=============================================================================
Copyright (c) 2016 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

void crash10_ref   (unsigned char   *input,
    int             stride,
    int             width,
    int             height,
    unsigned char   *output
    )
{
    for(int y=0; y<height; y++)
    {
        for(int x=0; x<width; x++)
        {
            output[y*stride+x] = input[y*stride+x];
        }
    }

    return;
}
