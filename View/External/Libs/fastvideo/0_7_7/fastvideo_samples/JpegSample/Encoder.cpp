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
#include <sstream>

#include "Encoder.h"
#include "timing.hpp"
#include "Decoder.h"
#include "memory_interface.h"
#include "supported_files.hpp"


template< class T>
fastStatus_t Encoder<T>::Init(EncoderOptions &options) {
	Quality = options.Quality;
	maxWidth = options.MaxWidth;
	maxHeight = options.MaxHeight;

	jfifInfo.restartInterval = options.RestartInterval;
	jfifInfo.jpegFmt = options.SamplingFmt;

	folder = options.IsFolder;

	fastStatus_t ret;

	CHECK_FAST( fastImportFromHostCreate(
		&hImportFromHost,

		options.SurfaceFmt,
		options.MaxWidth,
		options.MaxHeight,

		&srcBuffer
	) );

	if ( options.SurfaceFmt == FAST_BGR8 ) {
		options.SurfaceFmt = FAST_RGB8;
	}
	
	CHECK_FAST( fastJpegEncoderCreate(
		&hEncoder,

		options.SurfaceFmt,
		
		options.MaxWidth,
		options.MaxHeight,

		srcBuffer
	) );

	unsigned requestedMemSpace = 0;
	unsigned tmp = 0;

	CHECK_FAST( fastJpegEncoderGetAllocatedGpuMemorySize( hEncoder, &tmp ) );
	requestedMemSpace += tmp;

	CHECK_FAST( fastImportFromHostGetAllocatedGpuMemorySize( hImportFromHost, &tmp ) );
	requestedMemSpace += tmp;

	printf("Requested GPU memory space: %.2f MB\n\n", requestedMemSpace / ( 1024.0 * 1024.0 ) );

	unsigned channelCount = options.SurfaceFmt == FAST_I8 ? 1 : 3;
	unsigned pitch = channelCount * ( ( ( options.MaxWidth + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT );
	CHECK_FAST_ALLOCATION( fastMalloc((void **)&jfifInfo.h_Bytestream, pitch * options.MaxHeight * sizeof(unsigned char) ) );
	return FAST_OK;
}

template< class T>
fastStatus_t Encoder<T>::Close() {
	fastStatus_t ret;
	CHECK_FAST( fastJpegEncoderDestroy(hEncoder) );
	CHECK_FAST( fastImportFromHostDestroy( hImportFromHost ) );

	if (jfifInfo.h_Bytestream != NULL)
		CHECK_FAST_DEALLOCATION( fastFree(jfifInfo.h_Bytestream) );

	for (int i = 0; i < jfifInfo.exifSectionsCount; i++) {
		free(jfifInfo.exifSections[i].exifData);
	}

	if ( jfifInfo.exifSections != NULL ) {
		free(jfifInfo.exifSections);
	}

	return FAST_OK;
}

template< class T>
fastStatus_t Encoder<T>::Encode(std::list< Image<T, FastAllocator> > &inputImg) {
	fastStatus_t ret;

	float fullTime = 0.;
	float elapsedTimeGpu = 0.;

	fastGpuTimerHandle_t importFromHostTimer = NULL;
	fastGpuTimerHandle_t jpegEncoderTimer = NULL;
	
	if (info) {
		fastGpuTimerCreate(&importFromHostTimer);
		fastGpuTimerCreate(&jpegEncoderTimer);
	}

	for (typename std::list< Image<T, FastAllocator> >::iterator i = inputImg.begin(); i != inputImg.end(); i++) {
		Image<T, FastAllocator> &img = *i;
		printf("Input image: %s\nInput image size: %dx%d pixels\n", img.inputFileName.c_str(), img.w, img.h);
		printf("Input sampling format: %s\n\n", samplingAnalyze(img.samplingFmt));
		
		if ( img.w > maxWidth ||
			 img.h > maxHeight ) {
				 fprintf(stderr, "Unsupported image size\n");
				 continue;
		}

		jfifInfo.width = img.w;
		jfifInfo.height = img.h;

		if (info) {
			fastGpuTimerStart(importFromHostTimer);
		}

		CHECK_FAST( fastImportFromHostCopy(
			hImportFromHost,

			img.data.get(),
			img.w,
			img.wPitch,
			img.h
		) );
		
		if (info) {
			fastGpuTimerStop(importFromHostTimer);
			fastGpuTimerGetTime(importFromHostTimer, &elapsedTimeGpu);

			fullTime += elapsedTimeGpu;
			printf("Host-to-device transfer = %.2f ms\n\n", elapsedTimeGpu);

			fastGpuTimerStart(jpegEncoderTimer);
		}

		CHECK_FAST( fastJpegEncode(
			hEncoder,
			
			Quality,
			&jfifInfo
		) );
		
		if (info) {
			fastGpuTimerStop(jpegEncoderTimer);
			fastGpuTimerGetTime(jpegEncoderTimer, &elapsedTimeGpu);

			const unsigned surfaceSize = img.h * img.w * ((img.surfaceFmt == FAST_I8) ? 1 : 3);

			fullTime += elapsedTimeGpu;
			printf("Effective encoding performance (includes device-to-host transfer) = %.2f GB/s (%.2f ms)\n\n", double(surfaceSize) / elapsedTimeGpu * 1E-6, elapsedTimeGpu);
		}

		printf("Output image: %s\n\n", img.outputFileName.c_str());
		CHECK_FAST_SAVE_FILE( fastJfifStoreToFile(
			img.outputFileName.c_str(),
			&jfifInfo
		) );

		int inSize = fileSize(img.inputFileName.c_str());
		int outSize = fileSize(img.outputFileName.c_str());
		printf("Input file size: %.2f KB\nOutput file size: %.2f KB\nCompression ratio: %.2f\n\n", inSize / 1024.0, outSize / 1024.0, float(inSize) / outSize);
	}

	if (info) {
		printf("Total time for all images = %.2f ms\n", fullTime);
		fastGpuTimerDestroy(importFromHostTimer);
		fastGpuTimerDestroy(jpegEncoderTimer);
	}

	return FAST_OK;
}

template class Encoder< unsigned char >;