/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include <sstream>
#include <stdio.h>
#include <cstring>

#include "Debayer.h"
#include "timing.hpp"
#include "supported_files.hpp"
#include "memory_interface.h"
#include "BaseOptions.h"


fastStatus_t Debayer::Init(DebayerOptions &options, void *lut, float *matrixA, unsigned char *matrixB) {
	fastStatus_t ret;

	folder = options.IsFolder;
	surfaceFmt = BaseOptions::GetSurfaceFormatFromExtension(options.OutputPath);
	maxWidth = options.MaxWidth;
	maxHeight = options.MaxHeight;
	bitsPerPixel = options.BitsPerPixel;
	bytesPerPixel = (bitsPerPixel+7)/8;
	surfaceFmt = (surfaceFmt == FAST_RGB16 && bitsPerPixel==12 )?FAST_RGB12:surfaceFmt;
	isRaw = options.IsRaw;
	is2bytesOutput = options.Is2bytesOutput;

	fastDeviceSurfaceBufferHandle_t *bufferPtr = &srcBuffer;
	if (options.IsRaw ) {
			CHECK_FAST( fastRawUnpackerCreate(
				&hRawUnpacker,
				FAST_XIMEA12,

				options.SurfaceFmt,
				options.MaxWidth,
				options.MaxHeight,

				&srcBuffer
			) );
			bufferPtr = &srcBuffer;
	}
	else 
	{
		CHECK_FAST( fastImportFromHostCreate(
			&hHostToDeviceAdapter,

			options.SurfaceFmt,
			options.MaxWidth,
			options.MaxHeight,

			&srcBuffer
		) );
		bufferPtr = &srcBuffer;
	}

	if ( matrixA != NULL || matrixB != NULL ) {
		fastMad_t madParameter;
		madParameter.correctionMatrix = matrixA;
		madParameter.blackShiftMatrix = matrixB;

		CHECK_FAST( fastImageFilterCreate(
			&hMad,
			options.SurfaceFmt,

			FAST_MAD,
			(void *)&madParameter,

			options.MaxWidth,
			options.MaxHeight,

			srcBuffer,
			&madBuffer
		) );

		bufferPtr = &madBuffer;
	}

	if ( options.BaseColorCorrectionEnabled ) {
		fastBaseColorCorrection_t colorCorrectionParameter;
		memcpy(colorCorrectionParameter.matrix, options.BaseColorCorrection, 12 * sizeof(float));
		colorCorrectionParameter.bayerPattern = options.BayerFormat;
				
		CHECK_FAST( fastImageFilterCreate(
			&hColorCorrection,
			options.SurfaceFmt,

			FAST_BASE_COLOR_CORRECTION,
			(void *)&colorCorrectionParameter,

			options.MaxWidth,
			options.MaxHeight,

			*bufferPtr,
			&colorCorrectionBuffer
		) );
		
		bufferPtr = &colorCorrectionBuffer;
	}

	if (options.BitsPerPixel>8 && !options.Is2bytesOutput)
	{
		fastLut_12_8_t lutParameter;
		memcpy(lutParameter.lut, lut, 4096 * sizeof(unsigned char));
		CHECK_FAST( fastImageFilterCreate(
			&hLut12to8,
			FAST_I12,

			FAST_LUT_12_8,
			(void *)&lutParameter,

			options.MaxWidth,
			options.MaxHeight,

			*bufferPtr,
			&lut12Buffer
		) );

		options.SurfaceFmt = FAST_I8;
		bufferPtr = &lut12Buffer;
	}
	
	CHECK_FAST( fastDebayerCreate(
		&hDebayer,

		options.BayerType,
		options.BayerFormat,
		options.SurfaceFmt,

		options.MaxWidth,
		options.MaxHeight,
		
		*bufferPtr,
		&dstBuffer
	) );


	CHECK_FAST( fastExportToHostCreate(
		&hDeviceToHostAdapter,
		surfaceFmt,
		dstBuffer
	) );

	unsigned pitch = 3 * ( ( ( options.MaxWidth * bytesPerPixel + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT );
	CHECK_FAST_ALLOCATION( h_Result.reset( (unsigned char *)FastAllocator::allocate<unsigned char>(pitch * options.MaxHeight) ) );
		
	unsigned requestedMemSpace = 0;
	unsigned tmp = 0;
	CHECK_FAST( fastDebayerGetAllocatedGpuMemorySize( hDebayer, &tmp ) );
	requestedMemSpace += tmp;
	if ( hRawUnpacker != NULL ) {
		CHECK_FAST( fastRawUnpackerGetAllocatedGpuMemorySize( hRawUnpacker, &tmp ) );
		requestedMemSpace += tmp;
	}
	if ( hLut12to8 != NULL ) {
		CHECK_FAST( fastImageFiltersGetAllocatedGpuMemorySize( hLut12to8, &tmp ) );
		requestedMemSpace += tmp;
	}
	if ( hMad != NULL ) {
		CHECK_FAST( fastImageFiltersGetAllocatedGpuMemorySize( hMad, &tmp ) );
		requestedMemSpace += tmp;
	}
	if ( hColorCorrection != NULL ) {
		CHECK_FAST( fastImageFiltersGetAllocatedGpuMemorySize( hColorCorrection, &tmp ) );
		requestedMemSpace += tmp;
	}
	if ( hHostToDeviceAdapter != NULL ) {
		CHECK_FAST( fastImportFromHostGetAllocatedGpuMemorySize( hHostToDeviceAdapter, &tmp ) );
		requestedMemSpace += tmp;
	}
	printf("\nRequested GPU memory space: %.2f MB\n\n", requestedMemSpace / ( 1024.0 * 1024.0 ) );

	return FAST_OK;
}

fastStatus_t Debayer::Transform(std::list< Image<unsigned char, FastAllocator> > &image) {
	float fullTime = 0.;
	float elapsedTimeGpu = 0.;
	
	fastGpuTimerHandle_t hostToDeviceTimer = NULL;
	fastGpuTimerHandle_t debayerTimer = NULL;
	fastGpuTimerHandle_t rawUnpackerTimer = NULL;
	fastGpuTimerHandle_t lut12to8Timer = NULL;
	fastGpuTimerHandle_t madTimer = NULL;
	fastGpuTimerHandle_t colorCorrectionTimer = NULL;
	fastGpuTimerHandle_t deviceToHostTimer = NULL;

	if (info) {
		fastGpuTimerCreate(&hostToDeviceTimer);
		fastGpuTimerCreate(&debayerTimer);
		fastGpuTimerCreate(&rawUnpackerTimer);
		fastGpuTimerCreate(&lut12to8Timer);
		fastGpuTimerCreate(&madTimer);
		fastGpuTimerCreate(&colorCorrectionTimer);
		fastGpuTimerCreate(&deviceToHostTimer);
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

		if (isRaw) {
			if (info) {
				fastGpuTimerStart(rawUnpackerTimer);
			}

			CHECK_FAST( fastRawUnpackerDecode(
				hRawUnpacker,
				img.data.get(),

				img.w,
				img.h
			) );

			if (info) {
				fastGpuTimerStop(rawUnpackerTimer);
				fastGpuTimerGetTime(rawUnpackerTimer, &elapsedTimeGpu);

				fullTime += elapsedTimeGpu;
				printf("Raw unpacker time = %.2f ms\n", elapsedTimeGpu);
			}
		}
		else
		{
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

				fullTime += elapsedTimeGpu;
				printf("Host-to-device transfer = %.2f ms\n\n", elapsedTimeGpu);
			}
		}

		if ( hMad != NULL ) {
			if (info) {
				fastGpuTimerStart(madTimer);
			}

			CHECK_FAST( fastImageFiltersTransform(
				hMad,
				NULL,

				img.w,
				img.h
			) );

			if (info) {
				fastGpuTimerStop(madTimer);
				fastGpuTimerGetTime(madTimer, &elapsedTimeGpu);

				fullTime += elapsedTimeGpu;
				printf("MAD time = %.2f ms\n", elapsedTimeGpu);
			}
		}

		if ( hColorCorrection != NULL ) {
			if (info) {
				fastGpuTimerStart(colorCorrectionTimer);
			}

			CHECK_FAST( fastImageFiltersTransform(
				hColorCorrection,
				NULL,

				img.w,
				img.h
			) );

			if (info) {
				fastGpuTimerStop(colorCorrectionTimer);
				fastGpuTimerGetTime(colorCorrectionTimer, &elapsedTimeGpu);

				fullTime += elapsedTimeGpu;
				printf("Color correction time = %.2f ms\n", elapsedTimeGpu);
			}
		}


		if (bitsPerPixel>8 && !is2bytesOutput)
		{
			if (info) {
				fastGpuTimerStart(lut12to8Timer);
			}

			CHECK_FAST( fastImageFiltersTransform(
				hLut12to8,
				NULL,

				img.w,
				img.h
			) );

			if (info) {
				fastGpuTimerStop(lut12to8Timer);
				fastGpuTimerGetTime(lut12to8Timer, &elapsedTimeGpu);

				fullTime += elapsedTimeGpu;
				printf("LUT 12 to 8 time = %.2f ms\n", elapsedTimeGpu);
			}
		}
		
		if (info) {
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

			fullTime += elapsedTimeGpu;
			printf("Effective debayer performance = %.2f Gpixel/s (%.2f ms)\n\n", double(img.h * img.w) / elapsedTimeGpu * 1E-6, elapsedTimeGpu);
		}
		
		if (info) {
			fastGpuTimerStart(deviceToHostTimer);
		}

		CHECK_FAST( fastExportToHostCopy(
			hDeviceToHostAdapter,

			h_Result.get(),
			img.w,
			3 * ( ( ( img.w * img.GetBytesPerPixel()+ FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT ),
			img.h
		) );
		
		if (info) {
			fastGpuTimerStop(deviceToHostTimer);
			fastGpuTimerGetTime(deviceToHostTimer, &elapsedTimeGpu);

			fullTime += elapsedTimeGpu;
			printf("Device-to-host transfer = %.2f ms\n\n", elapsedTimeGpu);
		}

		printf("Output image: %s\n\n", img.outputFileName.c_str());

		CHECK_FAST_SAVE_FILE( saveToFile(
			(char *)img.outputFileName.c_str(),
			h_Result,
			surfaceFmt,
			img.h,
			img.w,
			3 * ( ( ( img.w * img.GetBytesPerPixel() + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT ),
			false
		) );
	}

	if (info) {
		printf("Total time for all images = %.2f ms\n", fullTime);
		fastGpuTimerDestroy(hostToDeviceTimer);
		fastGpuTimerDestroy(debayerTimer);
		fastGpuTimerDestroy(rawUnpackerTimer);
		fastGpuTimerDestroy(lut12to8Timer);
		fastGpuTimerDestroy(madTimer);
		fastGpuTimerDestroy(colorCorrectionTimer);
		fastGpuTimerDestroy(deviceToHostTimer);
	}

	return FAST_OK;
}

fastStatus_t Debayer::Close()
{
	fastStatus_t ret;

	CHECK_FAST( fastDebayerDestroy( hDebayer ) );
	if ( hRawUnpacker != NULL ) {
		CHECK_FAST( fastRawUnpackerDestroy( hRawUnpacker ) );
	}
	if ( hLut12to8 != NULL ) {
		CHECK_FAST( fastImageFiltersDestroy( hLut12to8 ) );
	}
	if ( hMad != NULL ) {
		CHECK_FAST( fastImageFiltersDestroy( hMad ) );
	}
	if ( hColorCorrection != NULL ) {
		CHECK_FAST( fastImageFiltersDestroy( hColorCorrection ) );
	}
	if ( hHostToDeviceAdapter != NULL ) {
		CHECK_FAST( fastImportFromHostDestroy( hHostToDeviceAdapter ) );
	}
	CHECK_FAST( fastExportToHostDestroy( hDeviceToHostAdapter ) );

	return FAST_OK;
}
