/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include <stdio.h>
#include <math.h>
#include <sstream>

#include "SplitterEncoder.h"
#include "timing.hpp"
#include "memory_interface.h"
#include "supported_files.hpp"

#include "ExifInfo.hpp"

fastStatus_t SplitterEncoder::Init(DebayerJpegOptions &options) {
	fastStatus_t ret;

	maxSrcWidth = options.MaxWidth;
	maxSrcHeight = options.MaxHeight;

	quality = options.Quality;

	jfifInfo.restartInterval = options.RestartInterval;
	jfifInfo.jpegFmt = JPEG_Y;

	jfifInfo.exifSectionsCount = 1;
	jfifInfo.exifSections = GenerateSplitterExif( options.BayerFormat, maxSrcWidth, maxSrcHeight );

	CHECK_FAST( fastImportFromHostCreate(
		&hImportFromHost,

		options.SurfaceFmt,
		options.MaxWidth,
		options.MaxHeight,

		&srcBuffer
	) );
	
	CHECK_FAST( fastBayerSplitterCreate(
		&hBayerSplitter,
		options.SurfaceFmt,

		options.MaxWidth,
		options.MaxHeight,

		&maxDstWidth,
		&maxDstHeight,

		srcBuffer,
		&bayerSplitterBuffer
	) );

	CHECK_FAST( fastJpegEncoderCreate(
		&hEncoder,

		options.SurfaceFmt,
		maxDstWidth,
		maxDstHeight,

		bayerSplitterBuffer
	) );
	
	unsigned pitch = ( ( ( maxDstWidth + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT );
	CHECK_FAST_ALLOCATION( fastMalloc((void **)&jfifInfo.h_Bytestream, pitch * maxDstHeight * sizeof(unsigned char)) );

	unsigned requestedMemSpace = 0;
	unsigned tmp = 0;

	CHECK_FAST( fastImportFromHostGetAllocatedGpuMemorySize( hImportFromHost, &tmp ) );
	requestedMemSpace += tmp;

	CHECK_FAST( fastBayerSplitterGetAllocatedGpuMemorySize( hBayerSplitter, &tmp ) );
	requestedMemSpace += tmp;

	CHECK_FAST( fastJpegEncoderGetAllocatedGpuMemorySize( hEncoder, &tmp ) );
	requestedMemSpace += tmp;

	printf("Requested GPU memory space: %.2f MB\n\n", requestedMemSpace / ( 1024.0 * 1024.0 ) );
	
	return FAST_OK;
}

fastStatus_t SplitterEncoder::Close() {
	fastStatus_t ret;

	CHECK_FAST( fastImportFromHostDestroy( hImportFromHost ) );
	CHECK_FAST( fastJpegEncoderDestroy( hEncoder ) );
	CHECK_FAST( fastBayerSplitterDestroy( hBayerSplitter ) );

	if ( jfifInfo.exifSections != NULL ) {
		for ( int i = 0; i < jfifInfo.exifSectionsCount; i++) {
			delete jfifInfo.exifSections[i].exifData;
		}
		delete jfifInfo.exifSections;
	}

	return FAST_OK;
}

fastStatus_t SplitterEncoder::Transform(std::list< Image<unsigned char, FastAllocator> > &inputImages) {
	float fullTime = 0.;
	float elapsedTimeGpu = 0.;

	unsigned dstWidth, dstHeight;
	
	fastGpuTimerHandle_t importFromHostTimer = NULL;
	fastGpuTimerHandle_t bayerSplitterTimer = NULL;
	fastGpuTimerHandle_t encoderTimer = NULL;

	if (info) {
		fastGpuTimerCreate(&importFromHostTimer);
		fastGpuTimerCreate(&bayerSplitterTimer);
		fastGpuTimerCreate(&encoderTimer);
	}

	fastStatus_t ret;
	for (std::list< Image<unsigned char, FastAllocator> >::iterator i = inputImages.begin(); i != inputImages.end(); i++) {
		int width = (*i).w;
		int height = (*i).h;

		if ( maxSrcWidth < width ||
			 maxSrcHeight < height ) {
				fprintf(stderr, "No decoder initialized with these parameters\n");
				continue;
		}

		printf("Input image: %s\nInput image size: %dx%d pixels\n", (*i).inputFileName.c_str(), width, height);

		if (info) {
			fastGpuTimerStart(importFromHostTimer);
		}

		CHECK_FAST( fastImportFromHostCopy(
			hImportFromHost,

			(*i).data.get(),
			(*i).w,
			(*i).wPitch,
			(*i).h
		) );
		
		if (info) {
			fastGpuTimerStop(importFromHostTimer);
			fastGpuTimerGetTime(importFromHostTimer, &elapsedTimeGpu);

			fullTime += elapsedTimeGpu;
			printf("Host-to-device transfer = %.2f ms\n\n", elapsedTimeGpu);

			fastGpuTimerStart(bayerSplitterTimer);
		}
		
		CHECK_FAST( fastBayerSplitterSplit(
			hBayerSplitter,

			width,
			height,

			&dstWidth,
			&dstHeight
		) );

		if (info) {
			fastGpuTimerStop(bayerSplitterTimer);
			fastGpuTimerGetTime(bayerSplitterTimer, &elapsedTimeGpu);
			printf("Resize time = %.2f ms\n\n", elapsedTimeGpu);

			fullTime += elapsedTimeGpu;
		}

		jfifInfo.width = dstWidth;
		jfifInfo.height = dstHeight;
		CHECK_FAST( UpdateSplitterExif( jfifInfo.exifSections, width, height ) );

		if (info) {
			fastGpuTimerStart(encoderTimer);
		}

		CHECK_FAST( fastJpegEncode(
			hEncoder,

			quality,
			&jfifInfo
		) );

		if (info) {
			fastGpuTimerStop(encoderTimer);
			fastGpuTimerGetTime(encoderTimer, &elapsedTimeGpu);
			printf("Encode time = %.2f ms\n\n", elapsedTimeGpu);

			fullTime += elapsedTimeGpu;
		}

		CHECK_FAST( fastJfifStoreToFile(
			(char *)(*i).outputFileName.c_str(),
			&jfifInfo
		) );
		
		printf("Output image: %s\nOutput image size: %dx%d pixels\n\n", (*i).outputFileName.c_str(), width, height);
	}

	if ( info ) {
		printf("Total for all images = %.2f ms\n", fullTime);

		fastGpuTimerDestroy(importFromHostTimer);
		fastGpuTimerDestroy(bayerSplitterTimer);
		fastGpuTimerDestroy(encoderTimer);
	}

	return FAST_OK;
}
