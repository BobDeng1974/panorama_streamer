/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include "stdio.h"
#include "string.h"
#include <list>

#include "DebayerJpeg.h"

#include "timing.hpp"
#include "supported_files.hpp"
#include "memory_interface.h"

fastStatus_t DebayerJpeg::Init(DebayerJpegOptions &options, void *lut) {
	fastStatus_t ret;

	quality = options.Quality;
	folder = options.IsFolder;
	maxWidth = options.MaxWidth;
	maxHeight = options.MaxHeight;

	jfifInfo.restartInterval = options.RestartInterval;
	jfifInfo.jpegFmt = options.SamplingFmt;

	CHECK_FAST( fastImportFromHostCreate(
		&hHostToDeviceAdapter,

		options.SurfaceFmt,
		options.MaxWidth,
		options.MaxHeight,

		&srcBuffer
	) );

	if ( options.SurfaceFmt == FAST_BGR8 ) {
		options.SurfaceFmt = FAST_RGB8;
	}

	CHECK_FAST( fastDebayerCreate(
		&hDebayer,

		options.BayerType,
		options.BayerFormat,
		options.SurfaceFmt,
		
		options.MaxWidth,
		options.MaxHeight,
		
		srcBuffer,
		&debayerBuffer
	) );
	
	CHECK_FAST( fastJpegEncoderCreate(
		&hEncoder,

		FAST_RGB8,
		
		options.MaxWidth,
		options.MaxHeight,
		
		debayerBuffer
	) );

	unsigned pitch = 3 * ( ( ( options.MaxWidth + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT );
	CHECK_FAST_ALLOCATION( fastMalloc((void **)&jfifInfo.h_Bytestream, pitch * options.MaxHeight * sizeof(unsigned char)) );

	unsigned requestedMemSpace = 0;
	unsigned tmp;
	CHECK_FAST( fastJpegEncoderGetAllocatedGpuMemorySize( hEncoder, &tmp ) );
	requestedMemSpace += tmp;
	CHECK_FAST( fastDebayerGetAllocatedGpuMemorySize( hDebayer, &tmp ) );
	requestedMemSpace += tmp;
	CHECK_FAST( fastImportFromHostGetAllocatedGpuMemorySize( hHostToDeviceAdapter, &tmp ) );
	requestedMemSpace += tmp;
	printf("\nRequested GPU memory space: %.2f MB\n\n", requestedMemSpace / ( 1024.0 * 1024.0 ) );

	return FAST_OK;
}

fastStatus_t DebayerJpeg::Transform(std::list< Image<unsigned char, FastAllocator> > &image) {
	fastGpuTimerHandle_t hostToDeviceTimer = NULL;
	fastGpuTimerHandle_t debayerTimer = NULL;
	fastGpuTimerHandle_t encoderTimer = NULL;

	float elapsedTimeGpu = 0.;
	float fullTime = 0.;
	float totalTime = 0.;

	if (info) {
		fastGpuTimerCreate(&hostToDeviceTimer);
		fastGpuTimerCreate(&debayerTimer);
		fastGpuTimerCreate(&encoderTimer);
	}

	fastStatus_t ret;
	for (std::list< Image<unsigned char, FastAllocator> >::iterator i = image.begin(); i != image.end(); i++) {
		Image<unsigned char, FastAllocator> &img = *i;

		printf("Input image: %s\nImage size: %dx%d pixels\n\n", img.inputFileName.c_str(), img.w, img.h);

		if ( img.w > maxWidth ||
			 img.h > maxHeight ) {
				 fprintf(stderr, "Unsupported image size\n");
				 continue;
		}

		jfifInfo.width = img.w;
		jfifInfo.height = img.h;

		if (info) {
			fastGpuTimerStart(hostToDeviceTimer);
		}

		CHECK_FAST( fastImportFromHostCopy(
			hHostToDeviceAdapter,

			img.data.get(),
			img.w,
			img.wPitch,
			img.h
		) );

		if (info) {
			fastGpuTimerStop(hostToDeviceTimer);
			fastGpuTimerGetTime(hostToDeviceTimer, &elapsedTimeGpu);

			totalTime = elapsedTimeGpu;
			printf("Host-to-device transfer = %.2f ms\n\n", elapsedTimeGpu);

			fastGpuTimerStart(debayerTimer);
		}

		CHECK_FAST( fastDebayerTransform(
			hDebayer,

			img.w,
			img.h
		) );
		
		if (info) {
			fastGpuTimerStop(debayerTimer);
			fastGpuTimerGetTime(debayerTimer, &elapsedTimeGpu);

			totalTime += elapsedTimeGpu;
			printf("Debayer time = %.2f ms\n", elapsedTimeGpu);

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

			printf("Encode time (includes device-to-host transfer) = %.2f ms\n", elapsedTimeGpu);
			totalTime += elapsedTimeGpu;

			printf("\nEffective performance = %.2f Gpixel/s (total time = %.2f ms)\n\n", double(img.h * img.w) / totalTime * 1E-6, totalTime);
			fullTime += totalTime;
		}

		printf("Output image: %s\n\n", img.outputFileName.c_str());

		CHECK_FAST_SAVE_FILE( fastJfifStoreToFile(
			img.outputFileName.c_str(),
			&jfifInfo
		) );
	}

	if (info) {
		printf("Total time for all images = %.2f ms\n", fullTime);
		fastGpuTimerDestroy(hostToDeviceTimer);
		fastGpuTimerDestroy(debayerTimer);
		fastGpuTimerDestroy(encoderTimer);
	}

	return FAST_OK;
}

fastStatus_t DebayerJpeg::Close() {
	fastStatus_t ret;

	CHECK_FAST( fastDebayerDestroy( hDebayer ) );
	CHECK_FAST( fastJpegEncoderDestroy(hEncoder) );
	CHECK_FAST( fastImportFromHostDestroy( hHostToDeviceAdapter ) );

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
