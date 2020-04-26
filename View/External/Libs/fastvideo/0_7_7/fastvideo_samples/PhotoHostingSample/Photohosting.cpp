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

#include "Photohosting.h"
#include "timing.hpp"
#include "memory_interface.h"
#include "supported_files.hpp"

fastStatus_t Photohosting::Init(ResizerOptions &options) {
	fastStatus_t ret;

	this->options = options;
	channelCount = options.SurfaceFmt == FAST_I8 ? 1 : 3;

	CHECK_FAST( fastJpegDecoderCreate(
		&hDecoder,

		options.SurfaceFmt,
		options.MaxWidth,
		options.MaxHeight,

		&d_decoderBuffer
	) );

	if ( options.CropEnabled ) {
		CHECK_FAST( fastCropCreate(
			&hCrop,

			options.SurfaceFmt,

			options.MaxWidth,
			options.MaxHeight,

			options.CropWidth,
			options.CropHeight,

			d_decoderBuffer,
			&d_cropBuffer
		) );
	}

	if ( options.SharpBefore != ImageFilterOptions::DisabledSharpConst ) {
		CHECK_FAST( fastImageFilterCreate(
			&hImageFilterBefore,
			options.SurfaceFmt,

			FAST_GAUSSIAN_SHARPEN,
			NULL,

			options.CropEnabled ? options.CropWidth : options.MaxWidth,
			options.CropEnabled ? options.CropHeight : options.MaxHeight,

			options.CropEnabled ? d_cropBuffer : d_decoderBuffer,
			&d_imageFilterBufferBefore
		) );
	}

	fastDeviceSurfaceBufferHandle_t *bufferPtr = options.SharpBefore != ImageFilterOptions::DisabledSharpConst
												? &d_imageFilterBufferBefore
												: ( options.CropEnabled 
														? &d_cropBuffer
														: &d_decoderBuffer );

	unsigned outputWidth = options.OutputWidth;
	unsigned outputPitch = channelCount * ( ( ( outputWidth + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT );
	unsigned outputHeight = (1.01f * double(options.MaxHeight) ) * (double( options.OutputWidth ) / double( options.MaxWidth ) );

	CHECK_FAST( fastResizerCreate(
		&hResizer,
		options.SurfaceFmt,
		
		options.CropEnabled ? options.CropWidth : options.MaxWidth,
		options.CropEnabled ? options.CropHeight : options.MaxHeight,

		outputWidth,
		outputHeight,

		options.MaxScaleFactor,

		options.ShiftX,
		options.ShiftY,

		*bufferPtr,
		&d_resizerBuffer
	) );

	if ( options.SharpAfter != ImageFilterOptions::DisabledSharpConst ) {
		CHECK_FAST( fastImageFilterCreate(
			&hImageFilterAfter,
			options.SurfaceFmt,

			FAST_GAUSSIAN_SHARPEN,
			NULL,

			outputWidth,
			outputHeight,

			d_resizerBuffer,
			&d_imageFilterBufferAfter
		) );
	}

	h_ResizedJpegStream.reset( (unsigned char *)FastAllocator::allocate<unsigned char>(outputPitch * outputHeight) );

	CHECK_FAST( fastJpegEncoderCreate(
		&hEncoder,
		options.SurfaceFmt,

		outputWidth,
		outputHeight,

		options.SharpAfter != ImageFilterOptions::DisabledSharpConst ? d_imageFilterBufferAfter : d_resizerBuffer
	) );
	
	unsigned requestedMemSpace = 0;
	unsigned tmp;
	CHECK_FAST( fastJpegDecoderGetAllocatedGpuMemorySize( hDecoder, &tmp ) );
	requestedMemSpace += tmp;
	CHECK_FAST( fastJpegEncoderGetAllocatedGpuMemorySize( hEncoder, &tmp ) );
	requestedMemSpace += tmp;
	CHECK_FAST( fastResizerGetAllocatedGpuMemorySize( hResizer, &tmp ) );
	requestedMemSpace += tmp;
	if ( options.SharpAfter != ImageFilterOptions::DisabledSharpConst ) {
		CHECK_FAST( fastImageFiltersGetAllocatedGpuMemorySize( hImageFilterAfter, &tmp ) );
		requestedMemSpace += tmp;
	}
	if ( options.SharpBefore != ImageFilterOptions::DisabledSharpConst ) {
		CHECK_FAST( fastImageFiltersGetAllocatedGpuMemorySize( hImageFilterBefore, &tmp ) );
		requestedMemSpace += tmp;
	}
	if ( options.CropEnabled ) {
		CHECK_FAST( fastCropGetAllocatedGpuMemorySize( hCrop, &tmp ) );
		requestedMemSpace += tmp;
	}
	printf("Requested GPU memory space: %.2f MB\n\n", requestedMemSpace / ( 1024.0 * 1024.0 ) );

	jfifInfo.bytestreamSize = channelCount * options.MaxHeight * options.MaxWidth;
	CHECK_FAST( fastMalloc( (void **)&jfifInfo.h_Bytestream, jfifInfo.bytestreamSize ) );

	return FAST_OK;
}

fastStatus_t Photohosting::Close() {
	fastStatus_t ret;

	CHECK_FAST( fastJpegDecoderDestroy( hDecoder ) );
	CHECK_FAST( fastJpegEncoderDestroy( hEncoder ) );
	CHECK_FAST( fastResizerDestroy( hResizer ) );
	if ( options.SharpBefore != ImageFilterOptions::DisabledSharpConst ) {
		CHECK_FAST( fastImageFiltersDestroy( hImageFilterBefore ) );
	}
	if ( options.SharpAfter != ImageFilterOptions::DisabledSharpConst ) {
		CHECK_FAST( fastImageFiltersDestroy( hImageFilterAfter ) );
	}

	if ( options.CropEnabled ) {
		CHECK_FAST( fastCropDestroy( hCrop ) );
	}

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

fastStatus_t Photohosting::Resize(std::list< Data< unsigned char, FastAllocator > > &inputImages, const char *outputPattern) {
	fastGpuTimerHandle_t decoderTimer, resizeTimer, encoderTimer, imageFilterTimerBefore, imageFilterTimerAfter, cropTimer;
	hostTimer_t host_timer = NULL;
	float elapsedTimeLoadJpeg = 0.;
	float elapsedTimeSaveJpeg = 0.;
	float elapsedTimeGpu = 0.;
	float fullTime = 0.;
	float totalTime = 0.;

	fastStatus_t ret;

	if (info) {
		fastGpuTimerCreate(&decoderTimer);
		fastGpuTimerCreate(&resizeTimer);
		fastGpuTimerCreate(&imageFilterTimerAfter);
		fastGpuTimerCreate(&imageFilterTimerBefore);
		fastGpuTimerCreate(&encoderTimer);
		fastGpuTimerCreate(&cropTimer);
		host_timer = hostTimerCreate();
	}

	for (std::list< Data< unsigned char, FastAllocator > >::iterator i = inputImages.begin(); i != inputImages.end(); i++) {
		jfifInfo.bytestreamSize = channelCount * options.MaxHeight * options.MaxWidth;

		if ( info ) {
			hostTimerStart(host_timer);
		}
		
		CHECK_FAST( fastJfifLoadFromMemory(
			 (*i).data.get(),
			 (*i).size,

			 &jfifInfo
		) );

		if (info) {
			elapsedTimeLoadJpeg = (float)hostTimerEnd(host_timer) * 1000.0 + i->loadTimeMs;
		}

		unsigned channelCount = jfifInfo.jpegFmt == JPEG_Y ? 1 : 3;

		printf("Input image: %s\nInput image size: %dx%d pixels\n", (*i).inputFileName.c_str(), jfifInfo.width, jfifInfo.height);
		printf("Input sampling format: %s\n", samplingAnalyze(jfifInfo.jpegFmt));
		printf("Input restart interval: %d\n\n", jfifInfo.restartInterval);

		if ( options.MaxHeight < jfifInfo.height ||
			 options.MaxWidth < jfifInfo.width ) {
				fprintf(stderr, "No decoder initialized with these parameters\n");
				continue;
		}

		double scaleFactor = double( ( options.CropEnabled ? options.CropWidth : jfifInfo.width ) ) / double( options.OutputWidth );
		unsigned resizedWidth = options.OutputWidth;
		unsigned resizedHeight = 0;

		printf("Output image: %s\nOutput image size: %dx%d pixels\n\n", (*i).outputFileName.c_str(), resizedWidth, resizedHeight);

		if ( scaleFactor > options.MaxScaleFactor * 1.01) {
			fprintf(stderr, "Image scale factor (%.3f) is more than maxScaleFactor (%.3f)\n\n", scaleFactor, options.MaxScaleFactor);
			continue;
		}

		if ( scaleFactor <= ResizerOptions::SCALE_FACTOR_MIN || scaleFactor > ResizerOptions::SCALE_FACTOR_MAX ) {
			fprintf(stderr, "Incorrect image scale factor (%.3f). Supported range of scales is (%d,%d]\n", scaleFactor, ResizerOptions::SCALE_FACTOR_MIN, ResizerOptions::SCALE_FACTOR_MAX);
			continue;
		}
	
		if ( resizedWidth < FAST_MIN_SCALED_SIZE ) {
			fprintf(stderr, "Image width %d is not supported (the smallest image size is %dx%d)\n", resizedWidth, FAST_MIN_SCALED_SIZE, FAST_MIN_SCALED_SIZE);
			continue;
		}

		printf("Image scale factor: %.3f\n\n", scaleFactor);

		if (info) {
			fastGpuTimerStart(decoderTimer);
		}

		CHECK_FAST( fastJpegDecode(
			 hDecoder,
			 &jfifInfo
		) );

		if (info) {
			fastGpuTimerStop(decoderTimer);
			fastGpuTimerGetTime(decoderTimer, &elapsedTimeGpu);
			printf("JFIF load time from HDD to CPU memory = %.2f ms\n", elapsedTimeLoadJpeg );
			printf("\nDecode time (includes host-to-device transfer) = %.2f ms\n", elapsedTimeGpu);

			totalTime = elapsedTimeGpu;
		}

		if ( options.CropEnabled ) {
			if ( ( options.CropWidth + options.CropLeftTopCoordsX ) > jfifInfo.width ) {
				fprintf(stderr, "Crop parameters are incorrect: %d + %d > %d\n", options.CropWidth, options.CropLeftTopCoordsX, jfifInfo.width);
				continue;
			}

			if ( ( options.CropHeight + options.CropLeftTopCoordsY ) > jfifInfo.height ) {
				fprintf(stderr, "Crop parameters are incorrect: %d + %d > %d\n", options.CropHeight, options.CropLeftTopCoordsY, jfifInfo.height);
				continue;
			}

			if (info) {
				fastGpuTimerStart(cropTimer);
			}

			CHECK_FAST ( fastCropTransform(
 				hCrop,

 				jfifInfo.width,
				jfifInfo.height,

				options.CropLeftTopCoordsX,
				options.CropLeftTopCoordsY,
				options.CropWidth,
				options.CropHeight
			) );

			if (info) {
				fastGpuTimerStop(cropTimer);

				fastGpuTimerGetTime(cropTimer, &elapsedTimeGpu);
				printf("Crop time = %.2f ms\n", elapsedTimeGpu);

				totalTime += elapsedTimeGpu;
			}

			jfifInfo.width = options.CropWidth;
			jfifInfo.height = options.CropHeight;
		}

		if ( options.SharpBefore != ImageFilterOptions::DisabledSharpConst ) {
			fastGaussianFilter_t gaussParameters;
			gaussParameters.sigma = options.SharpBefore;

			if (info) {
				fastGpuTimerStart(imageFilterTimerBefore);
			}

			CHECK_FAST ( fastImageFiltersTransform(
 				hImageFilterBefore,
				&gaussParameters,

 				jfifInfo.width,
				jfifInfo.height
			) );

			if (info) {
				fastGpuTimerStop(imageFilterTimerBefore);

				fastGpuTimerGetTime(imageFilterTimerBefore, &elapsedTimeGpu);
				printf("Sharpen filter (before resize) time = %.2f ms\n", elapsedTimeGpu);

				totalTime += elapsedTimeGpu;
			}
		}

		if ( info ) {
			fastGpuTimerStart(resizeTimer);
		}

		CHECK_FAST( fastResizerTransform(
			hResizer, FAST_LANCZOS, jfifInfo.width, jfifInfo.height, resizedWidth, &resizedHeight
		) );

		if (info) {
			fastGpuTimerStop(resizeTimer);

			fastGpuTimerGetTime(resizeTimer, &elapsedTimeGpu);
			printf("Resize time = %.2f ms\n", elapsedTimeGpu);

			totalTime += elapsedTimeGpu;
		}

		if ( options.SharpAfter != ImageFilterOptions::DisabledSharpConst ) {
			fastGaussianFilter_t gaussParameters;
			gaussParameters.sigma = options.SharpAfter;
			
			if (info) {
				fastGpuTimerStart(imageFilterTimerAfter);
			}

			CHECK_FAST ( fastImageFiltersTransform(
 				hImageFilterAfter,
				&gaussParameters,

				resizedWidth,
				resizedHeight
			) );

			if (info) {
				fastGpuTimerStop(imageFilterTimerAfter);

				fastGpuTimerGetTime(imageFilterTimerAfter, &elapsedTimeGpu);
				printf("Sharpen filter (after resize) time = %.2f ms\n", elapsedTimeGpu);

				totalTime += elapsedTimeGpu;
			}
		}

		jfifInfo.restartInterval = options.RestartInterval;
		jfifInfo.jpegFmt = options.SamplingFmt;
		jfifInfo.width = resizedWidth;
		jfifInfo.height = resizedHeight;

		if (info) {
			fastGpuTimerStart(encoderTimer);
		}

		CHECK_FAST( fastJpegEncode(
			hEncoder,
			options.Quality,
			&jfifInfo
		) );

		if (info) {
			fastGpuTimerStop(encoderTimer);

			fastGpuTimerGetTime(encoderTimer, &elapsedTimeGpu);
			printf("Encode time (includes device-to-host transfer) = %.2f ms\n", elapsedTimeGpu);

			totalTime += elapsedTimeGpu;

			hostTimerStart(host_timer);
		}

		CHECK_FAST_SAVE_FILE( fastJfifStoreToFile(
			(*i).outputFileName.c_str(),

			&jfifInfo
		) );

		if ( info ) {
			elapsedTimeSaveJpeg = (float)hostTimerEnd(host_timer) * 1000.0f;

			printf("\nJFIF store time from CPU memory to HDD = %.2f ms\n\n", elapsedTimeSaveJpeg);
			printf("Total time %s = %.2f ms (without HDD I/O)\n\n",(*i).inputFileName.c_str(), totalTime);

			fullTime += totalTime;
		}
	}

	printf("For all output images:\n\tOutput sampling format: %s\n", samplingAnalyze(options.SamplingFmt));
	printf("\tJPEG quality: %d%%\n", options.Quality);
	printf("\tOutput restart interval: %d\n\n", options.RestartInterval);

	if ( info ) {
		printf("Total time for all images = %.2f ms (without HDD I/O)\n", fullTime);
		hostTimerDestroy(host_timer);
		fastGpuTimerDestroy(decoderTimer);
		fastGpuTimerDestroy(resizeTimer);
		fastGpuTimerDestroy(imageFilterTimerAfter);
		fastGpuTimerDestroy(imageFilterTimerBefore);
		fastGpuTimerDestroy(encoderTimer);
		fastGpuTimerDestroy(cropTimer);
	}

	return FAST_OK;
}
