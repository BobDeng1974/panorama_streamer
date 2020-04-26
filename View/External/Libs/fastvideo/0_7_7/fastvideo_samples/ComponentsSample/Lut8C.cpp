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

#include "Lut8C.h"

#include "timing.hpp"
#include "supported_files.hpp"
#include "memory_interface.h"

fastStatus_t Lut8C::Init(
	LutOptions &options,
	void *lut_R,
	void *lut_G,
	void *lut_B
) {
	fastStatus_t ret;

	folder = options.IsFolder;
	maxWidth = options.MaxWidth;
	maxHeight = options.MaxHeight;
	surfaceFmt = options.SurfaceFmt;

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

	fastLut_8_8_C_t lutParameter;
	memcpy(lutParameter.lut_R, lut_R, 256 * sizeof(unsigned char));
	memcpy(lutParameter.lut_G, lut_G, 256 * sizeof(unsigned char));
	memcpy(lutParameter.lut_B, lut_B, 256 * sizeof(unsigned char));

	CHECK_FAST( fastImageFilterCreate(
		&hLut,

		options.SurfaceFmt,
		FAST_LUT_8_8_C,
		(void *)&lutParameter,
		
		options.MaxWidth,
		options.MaxHeight,
		
		srcBuffer,
		&lutBuffer
	) );
	
	CHECK_FAST( fastExportToHostCreate(
		&hDeviceToHostAdapter,

		surfaceFmt,
		
		lutBuffer
	) );

	unsigned channels = options.SurfaceFmt == FAST_I8 ? 1 : 3;
	unsigned pitch = channels * ( ( ( options.MaxWidth + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT );
	CHECK_FAST_ALLOCATION( buffer.reset( (unsigned char *)FastAllocator::allocate<unsigned char>(pitch * options.MaxHeight) ) );

	unsigned requestedMemSpace = 0;
	unsigned tmp;
	CHECK_FAST( fastImportFromHostGetAllocatedGpuMemorySize( hHostToDeviceAdapter, &tmp ) );
	requestedMemSpace += tmp;
	CHECK_FAST( fastImageFiltersGetAllocatedGpuMemorySize( hLut, &tmp ) );
	requestedMemSpace += tmp;
	printf("\nRequested GPU memory space: %.2f MB\n\n", requestedMemSpace / ( 1024.0 * 1024.0 ) );

	return FAST_OK;
}

fastStatus_t Lut8C::Transform(std::list< Image<unsigned char, FastAllocator> > &image) {
	fastGpuTimerHandle_t hostToDeviceTimer = NULL;
	fastGpuTimerHandle_t deviceToHostTimer = NULL;
	fastGpuTimerHandle_t imageFilterTimer = NULL;
	
	float elapsedTimeGpu = 0.;
	float totalTime = 0.;
	
	if (info) {
		fastGpuTimerCreate(&hostToDeviceTimer);
		fastGpuTimerCreate(&deviceToHostTimer);
		fastGpuTimerCreate(&imageFilterTimer);
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

			totalTime += elapsedTimeGpu;
			printf("Host-to-device transfer = %.2f ms\n", elapsedTimeGpu);

			fastGpuTimerStart(imageFilterTimer);
		}

		CHECK_FAST( fastImageFiltersTransform(
			hLut,
			NULL,

			img.w,
			img.h
		) );

		if (info) {
			fastGpuTimerStop(imageFilterTimer);
			fastGpuTimerGetTime(imageFilterTimer, &elapsedTimeGpu);

			totalTime += elapsedTimeGpu;
			printf("LUT time = %.2f ms\n", elapsedTimeGpu);

			fastGpuTimerStart(deviceToHostTimer);
		}

		CHECK_FAST( fastExportToHostCopy(
			hDeviceToHostAdapter,
				
			buffer.get(),
			img.w,
			img.wPitch,
			img.h
		) );
		
		if (info) {
			fastGpuTimerStop(deviceToHostTimer);
			fastGpuTimerGetTime(deviceToHostTimer, &elapsedTimeGpu);

			printf("Device-to-host transfer = %.2f ms\n", elapsedTimeGpu);
			totalTime += elapsedTimeGpu;
		}

		printf("Output image: %s\n\n", img.outputFileName.c_str());

		CHECK_FAST_SAVE_FILE( saveToFile(
			(char *)img.outputFileName.c_str(),

			buffer,
			surfaceFmt,
			img.h,
			img.w,
			img.wPitch,
			false
		) );
	}

	if (info) {
		printf("Total time for all images = %.2f ms\n", totalTime);
		fastGpuTimerDestroy(hostToDeviceTimer);
		fastGpuTimerDestroy(imageFilterTimer);
		fastGpuTimerDestroy(deviceToHostTimer);
	}

	return FAST_OK;
}

fastStatus_t Lut8C::Close() {
	fastStatus_t ret;

	CHECK_FAST( fastImageFiltersDestroy( hLut ) );
	CHECK_FAST( fastExportToHostDestroy(hDeviceToHostAdapter) );
	CHECK_FAST( fastImportFromHostDestroy( hHostToDeviceAdapter ) );
	
	return FAST_OK;
}
