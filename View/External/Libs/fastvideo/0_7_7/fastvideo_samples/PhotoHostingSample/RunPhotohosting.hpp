/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#ifndef __RUN_PHOTOHOSTING__
#define __RUN_PHOTOHOSTING__

#include <stdio.h>

#include "Photohosting.h"

#include "ResizerOptions.h"

#include "memory_interface.h"
#include "supported_files.hpp"
#include "FastAllocator.h"

static fastStatus_t DecodeJpeg(unsigned char *data, unsigned dataSize, fastJfifInfo_t *jfifInfo) {
	return fastJfifLoadFromMemory(
			 data,
			 dataSize,

			 jfifInfo
		);
}

static fastStatus_t RunPhotohosting(ResizerOptions &options) {
	Photohosting hPhotohosting(options.Info);
	std::list< Data< unsigned char, FastAllocator > > inputImg;

	fastStatus_t ret;
	if ( options.IsFolder ) {
		CHECK_FAST( loadJpegImages(options.InputPath, inputImg, options.Info) );
		int idx = 0;
		for ( std::list< Data< unsigned char, FastAllocator > >::iterator i = inputImg.begin(); i != inputImg.end(); i++, idx++) {
			i->outputFileName = generateOutputFileName(options.OutputPath, idx);
		}

	} else {
		ret = loadJpeg(std::string(options.InputPath), inputImg, options.Info);
		if (ret == FAST_IO_ERROR) {
			fprintf(stderr, "Input image file %s has not been found!\n", options.InputPath);
			return ret;
		}
		if (ret != FAST_OK)
			return ret;

		(--inputImg.end())->outputFileName =  std::string(options.OutputPath);
	}

	if ( options.CropEnabled ) {
		printf("Crop left border coords: %dx%d pixels\n", options.CropLeftTopCoordsX, options.CropLeftTopCoordsY);
		printf("Cropped image size: %dx%d pixels\n", options.CropWidth, options.CropHeight);

		if ( options.OutputWidth > options.CropWidth ) {
			fprintf(stderr, "Output width (%d) is bigger than cropped (%d)\n", options.OutputWidth, options.CropWidth);
			return FAST_INVALID_SIZE;
		}
	}

	fastJfifInfo_t jfifInfo;
	jfifInfo.h_Bytestream = NULL;
	jfifInfo.exifSections = NULL;
	jfifInfo.exifSectionsCount = 0;

	jfifInfo.bytestreamSize = (*inputImg.begin()).size;
	CHECK_FAST( fastMalloc( (void **)&jfifInfo.h_Bytestream, jfifInfo.bytestreamSize ) );
	
	CHECK_FAST( DecodeJpeg( (*inputImg.begin()).data.get(), (*inputImg.begin()).size, &jfifInfo ) );

	if (jfifInfo.h_Bytestream != NULL)
		CHECK_FAST_DEALLOCATION( fastFree(jfifInfo.h_Bytestream) );

	for (int i = 0; i < jfifInfo.exifSectionsCount; i++) {
		free(jfifInfo.exifSections[i].exifData);
	}

	if ( jfifInfo.exifSections != NULL ) {
		free(jfifInfo.exifSections);
	}

	options.SurfaceFmt = jfifInfo.jpegFmt == JPEG_Y ? FAST_I8 : FAST_RGB8;
	options.SamplingFmt = jfifInfo.jpegFmt == JPEG_Y ? JPEG_Y : options.SamplingFmt;

	if ( !options.IsFolder ) {
		options.MaxHeight = options.MaxHeight == 0 ? jfifInfo.height : options.MaxHeight;
		options.MaxWidth = options.MaxWidth == 0 ? jfifInfo.width : options.MaxWidth;

		double tmpScale = double( options.CropEnabled ? options.CropWidth : options.MaxWidth ) / double( options.OutputWidth );
		options.MaxScaleFactor = tmpScale;

		if ( options.MaxScaleFactor <= ResizerOptions::SCALE_FACTOR_MIN || options.MaxScaleFactor > ResizerOptions::SCALE_FACTOR_MAX ) {
			fprintf(stderr, "Incorrect image scale factor (%.3f). Supported range of scales is (%d,%d]\n", options.MaxScaleFactor, ResizerOptions::SCALE_FACTOR_MIN, ResizerOptions::SCALE_FACTOR_MAX);
			return FAST_IO_ERROR;
		}
	}

	if ( options.SharpBefore != ImageFilterOptions::DisabledSharpConst ) {
		printf("sharp_before parameters: r = 1.000, sigma = %.3f\n", options.SharpBefore);
	}

	if ( options.SharpAfter != ImageFilterOptions::DisabledSharpConst ) {
		printf("sharp_after parameters: r = 1.000, sigma = %.3f\n", options.SharpAfter);
	}

	if ( options.IsFolder ) {
		printf("Maximum scale factor: %.3f\n", options.MaxScaleFactor);
	}

	CHECK_FAST( hPhotohosting.Init(options) );
	CHECK_FAST( hPhotohosting.Resize(inputImg, options.OutputPath) );
	CHECK_FAST( hPhotohosting.Close() );

	return FAST_OK;
}

#endif // __RUN_PHOTOHOSTING__