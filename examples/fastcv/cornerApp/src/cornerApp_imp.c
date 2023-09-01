/**=============================================================================

@file
   corner_detection_app_imp.c

@brief
   skel implementation for FastCV API sequences in corner detection app.

Copyright (c) 2012, 2015, 2017 Qualcomm Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary

Export of this technology or software is regulated by the U.S.
Government. Diversion contrary to U.S. law prohibited.

All ideas, data and information contained in or disclosed by
this document are confidential and proprietary information of
Qualcomm Technologies Incorporated and all rights therein are expressly reserved.
By accepting this material the recipient agrees that this material
and the information contained therein are held in confidence and in
trust and will not be used, copied, reproduced in whole or in part,
nor its contents revealed in any manner to others without the express
written permission of Qualcomm Technologies Incorporated.

=============================================================================**/
#include "fastcv.h"
#include "cornerApp.h"
#include "AEEStdErr.h"

/*===========================================================================
    REMOTED FUNCTION
===========================================================================*/
/*-------------------------------begin corner detection app functions---------------------------------------------------------------------*/

AEEResult cornerApp_filterGaussianAndCornerFastQ(const uint8* src,
		int srcLen, uint32 srcWidth, uint32 srcHeight, uint8* dst, int dstLen, int32 blurBorder,
		uint32 srcStride, int32 barrier, uint32 border, uint32* xy, int xylen, uint32 nCornersMax,
		uint32* nCorners,uint32* renderBuffer, int renderBufferLen)
{
	// Perform color conversion from YUV to RGB. Output renderBuffer is the original 
    // buffer after conversion to RGB, upon which the caller of this function must draw 
    // the corners detected in the desired manner. 
	fcvColorYUV420toRGB565u8(src, srcWidth, srcHeight, renderBuffer);

	// Gaussian blur the image and then detect corners using Fast9 detection algorithm.
    // Return the list of corners for application to draw upon the rendered image.
	fcvFilterGaussian3x3u8(src, srcWidth, srcHeight, dst, blurBorder);

    fcvCornerFast9u8(dst, srcWidth, srcHeight, srcStride, barrier, border, xy, nCornersMax, nCorners);

	return AEE_SUCCESS;
}
