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
#include <cuda_runtime.h>

#include "DebayerFfmpeg.hpp"
#include "timing.hpp"
#include "memory_interface.h"

fastStatus_t DebayerFfmpeg::Init(FfmpegOptions &options, std::unique_ptr<unsigned char, FastAllocator> &lut, float *matrixA, unsigned char *matrixB) {
	fastStatus_t ret;
	cudaError_t ret_cuda;

	quality = options.Quality;
	jfifInfo.restartInterval = options.RestartInterval;
	jfifInfo.jpegFmt = options.SamplingFmt;
	surfaceFmt = FAST_RGB8;

	maxWidth = options.MaxWidth;
	maxHeight = options.MaxHeight;

	frameRate = 25;

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
	}
	
	fastBaseColorCorrection_t colorCorrectionParameter;
	memcpy(colorCorrectionParameter.matrix, options.BaseColorCorrection, 12 * sizeof(float));
				
	CHECK_FAST( fastImageFilterCreate(
		&hColorCorrection,
		options.SurfaceFmt,

		FAST_BASE_COLOR_CORRECTION,
		(void *)&colorCorrectionParameter,

		options.MaxWidth,
		options.MaxHeight,

		hMad != NULL ? madBuffer : srcBuffer,
		&colorCorrectionBuffer
	) );
		
	CHECK_FAST( fastDebayerCreate(
		&hDebayer,

		options.BayerType,
		options.BayerFormat,
		options.SurfaceFmt,
		
		options.MaxWidth,
		options.MaxHeight,
		
		colorCorrectionBuffer,
		&debayerBuffer
	) );

	fastLut_8_8_t lutParameter;
	memcpy(lutParameter.lut, lut.get(), 256 * sizeof(unsigned char));

	CHECK_FAST( fastImageFilterCreate(
		&hLut,
		surfaceFmt,

		FAST_LUT_8_8,
		(void *)&lutParameter,

		options.MaxWidth,
		options.MaxHeight,

		debayerBuffer,
		&lutBuffer
	) );
	
	CHECK_FAST( fastJpegEncoderCreate(
		&hEncoder,

		surfaceFmt,
		
		options.MaxWidth,
		options.MaxHeight,
		
		lutBuffer
	) );

	CHECK_FAST( fastExportToDeviceCreate(
		&hExportToDevice,

		surfaceFmt,
		lutBuffer
	) );

	maxPitch = 3 * ( ( ( options.MaxWidth + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT );
	bufferSize = jfifInfo.bytestreamSize = maxPitch * options.MaxHeight * sizeof(unsigned char);
	CHECK_FAST_ALLOCATION( fastMalloc((void **)&jfifInfo.h_Bytestream, bufferSize) );
	CHECK_FAST_ALLOCATION( fastMalloc((void **)&jpegImage, bufferSize + JPEG_HEADER_SIZE) );
	CHECK_CUDA( cudaMalloc( &d_buffer, bufferSize ) );

	CHECK_FAST( ffmpeg.Initialize(options.SamplingFmt, std::string(options.OutputPath), options.MaxWidth, options.MaxHeight, frameRate) );

	unsigned requestedMemSpace = 0;
	unsigned tmp = 0;
	CHECK_FAST( fastJpegEncoderGetAllocatedGpuMemorySize( hEncoder, &tmp ) );
	requestedMemSpace += tmp;
	CHECK_FAST( fastDebayerGetAllocatedGpuMemorySize( hDebayer, &tmp ) );
	requestedMemSpace += tmp;
	CHECK_FAST( fastImageFiltersGetAllocatedGpuMemorySize( hLut, &tmp ) );
	requestedMemSpace += tmp;
	if ( hMad != NULL ) {
		CHECK_FAST( fastImageFiltersGetAllocatedGpuMemorySize( hMad, &tmp ) );
		requestedMemSpace += tmp;
	}
	CHECK_FAST( fastImageFiltersGetAllocatedGpuMemorySize( hColorCorrection, &tmp ) );
	requestedMemSpace += tmp;
	CHECK_FAST( fastImportFromHostGetAllocatedGpuMemorySize( hImportFromHost, &tmp ) );
	requestedMemSpace += tmp;
	printf("\nRequested GPU memory space: %.2f MB\n\n", requestedMemSpace / ( 1024.0 * 1024.0 ) );

	return FAST_OK;
}

fastStatus_t DebayerFfmpeg::StoreFrame(Image<unsigned char, FastAllocator> &image) {
	float totalTime = 0.;
	float elapsedTimeGpu = 0.;
	
	fastGpuTimerHandle_t hostToDeviceTimer = NULL;
	fastGpuTimerHandle_t madTimer = NULL;
	fastGpuTimerHandle_t colorCorrectionTimer = NULL;
	fastGpuTimerHandle_t debayerTimer = NULL;
	fastGpuTimerHandle_t lutTimer = NULL;
	fastGpuTimerHandle_t encoderTimer = NULL;
	fastGpuTimerHandle_t deviceToSurfaceTimer = NULL;

	if (info) {
		fastGpuTimerCreate(&hostToDeviceTimer);
		fastGpuTimerCreate(&madTimer);
		fastGpuTimerCreate(&colorCorrectionTimer);
		fastGpuTimerCreate(&debayerTimer);
		fastGpuTimerCreate(&lutTimer);
		fastGpuTimerCreate(&encoderTimer);
		fastGpuTimerCreate(&deviceToSurfaceTimer);
	}

	fastStatus_t ret;
	//printf("Input image: %s\nImage size: %dx%d pixels\n\n", image.inputFileName.c_str(), image.w, image.h);

	if ( image.w > maxWidth ||
		 image.h > maxHeight ) {
			 fprintf(stderr, "Unsupported image size\n");
			 return FAST_INVALID_SIZE;
	}

	jfifInfo.width = image.w;
	jfifInfo.height = image.h;
	jfifInfo.bytestreamSize = bufferSize;

	unsigned jpegImageSize = bufferSize + JPEG_HEADER_SIZE;

	if (info) {
		fastGpuTimerStart(hostToDeviceTimer);
	}

	CHECK_FAST( fastImportFromHostCopy(
		hImportFromHost,

		image.data.get(),
		image.w,
		image.wPitch,
		image.h
	) );

	if (info) {
		fastGpuTimerStop(hostToDeviceTimer);
		fastGpuTimerGetTime(hostToDeviceTimer, &elapsedTimeGpu);

		totalTime = elapsedTimeGpu;
		printf("Host-to-device transfer = %.2f ms\n\n", elapsedTimeGpu);
	}

	if ( hMad != NULL ) {
		if (info) {
			fastGpuTimerStart(madTimer);
		}

		CHECK_FAST( fastImageFiltersTransform(
			hMad,
			NULL,

			image.w,
			image.h
		) );

		if (info) {
			fastGpuTimerStop(madTimer);
			fastGpuTimerGetTime(madTimer, &elapsedTimeGpu);

			totalTime += elapsedTimeGpu;
			printf("MAD time = %.2f ms\n", elapsedTimeGpu);
		}
	}

	if (info) {
		fastGpuTimerStart(colorCorrectionTimer);
	}
		
	CHECK_FAST( fastImageFiltersTransform(
		hColorCorrection,
		NULL,

		image.w,
		image.h
	) );

	if (info) {
		fastGpuTimerStop(colorCorrectionTimer);
		fastGpuTimerGetTime(colorCorrectionTimer, &elapsedTimeGpu);

		totalTime += elapsedTimeGpu;
		printf("Color correction time = %.2f ms\n", elapsedTimeGpu);

		fastGpuTimerStart(debayerTimer);
	}

	CHECK_FAST( fastDebayerTransform(
		hDebayer,

		image.w,
		image.h
	) );
		
	if (info) {
		fastGpuTimerStop(debayerTimer);
		fastGpuTimerGetTime(debayerTimer, &elapsedTimeGpu);

		totalTime += elapsedTimeGpu;
		printf("Debayer time = %.2f ms\n", elapsedTimeGpu);

		fastGpuTimerStart(lutTimer);
	}

	CHECK_FAST( fastImageFiltersTransform(
		hLut,
		NULL,

		image.w,
		image.h
	) );

	if (info) {
		fastGpuTimerStop(lutTimer);
		fastGpuTimerGetTime(lutTimer, &elapsedTimeGpu);

		totalTime += elapsedTimeGpu;
		printf("Lut time = %.2f ms\n", elapsedTimeGpu);

		fastGpuTimerStart(deviceToSurfaceTimer);
	}

	CHECK_FAST( fastExportToDeviceCopy(
		hExportToDevice,

		d_buffer,
		image.w,
		image.wPitch * 3 * sizeof(char),
		image.h
	) );
	
	if (info) {
		fastGpuTimerStop(deviceToSurfaceTimer);
		fastGpuTimerGetTime(deviceToSurfaceTimer, &elapsedTimeGpu);

		totalTime += elapsedTimeGpu;
		printf("Device to surface time = %.2f ms\n", elapsedTimeGpu);

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
	}

	CHECK_FAST( fastJfifStoreToMemory(
		jpegImage,
		&jpegImageSize,

		&jfifInfo
	) );

	ffmpeg.WriteFrame( jpegImage, jpegImageSize );

	if (info) {
		fastGpuTimerDestroy(hostToDeviceTimer);
		fastGpuTimerDestroy(debayerTimer);
		fastGpuTimerDestroy(lutTimer);
		fastGpuTimerDestroy(encoderTimer);
	}

	return FAST_OK;
}

fastStatus_t DebayerFfmpeg::Close() {
	fastStatus_t ret;

	CHECK_FAST( fastDebayerDestroy( hDebayer ) );
	CHECK_FAST( fastImageFiltersDestroy( hLut ) );
	if ( hMad != NULL ) {
		CHECK_FAST( fastImageFiltersDestroy( hMad ) );
	}
	CHECK_FAST( fastImageFiltersDestroy( hColorCorrection ) );
	CHECK_FAST( fastJpegEncoderDestroy ( hEncoder ) );
	CHECK_FAST( fastImportFromHostDestroy( hImportFromHost ) );
	CHECK_FAST( fastExportToDeviceDestroy( hExportToDevice ) );

	CHECK_FAST( ffmpeg.Close() );
	
	if (jfifInfo.h_Bytestream != NULL)
		CHECK_FAST_DEALLOCATION( fastFree(jfifInfo.h_Bytestream) );

	for (int i = 0; i < jfifInfo.exifSectionsCount; i++) {
		free(jfifInfo.exifSections[i].exifData);
	}

	if ( jfifInfo.exifSections != NULL ) {
		free(jfifInfo.exifSections);
	}

	CHECK_FAST_DEALLOCATION( fastFree( jpegImage ) );

	cudaError_t ret_cuda;
	CHECK_CUDA( cudaFree( d_buffer ) );

	return FAST_OK;
}

void *DebayerFfmpeg::GetDevicePtr() {
	return d_buffer;
}
