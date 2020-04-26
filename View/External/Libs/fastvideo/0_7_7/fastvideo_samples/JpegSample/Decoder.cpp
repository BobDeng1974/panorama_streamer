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

#include "Decoder.h"
#include "timing.hpp"
#include "memory_interface.h"
#include "supported_files.hpp"

template<class T>
inline unsigned Decoder<T>::uDivUp(unsigned a, unsigned b) {
	return (a / b) + (a % b != 0);
}

template<class T>
fastStatus_t Decoder<T>::Init(BaseOptions &options) {
	fastStatus_t ret;

	surfaceFmt = options.SurfaceFmt;
	maxWidth = options.MaxWidth;
	maxHeight = options.MaxHeight;

	CHECK_FAST( fastJpegDecoderCreate(
		&hDecoder,

		surfaceFmt == FAST_BGR8 ? FAST_RGB8 : surfaceFmt,
		options.MaxWidth,
		options.MaxHeight,

		&dstBuffer
	) );

	CHECK_FAST( fastExportToHostCreate(
		&hDeviceToHostAdapter,

		surfaceFmt,

		dstBuffer
	) );

	unsigned requestedMemSpace = 0;
	CHECK_FAST( fastJpegDecoderGetAllocatedGpuMemorySize( hDecoder, &requestedMemSpace ) );
	printf("Requested GPU memory space: %.2f MB\n\n", requestedMemSpace / ( 1024.0 * 1024.0 ) );

	return FAST_OK;
}

template<class T>
fastStatus_t Decoder<T>::Close() {
	fastStatus_t ret;

	CHECK_FAST( fastJpegDecoderDestroy(hDecoder) );
	CHECK_FAST( fastExportToHostDestroy( hDeviceToHostAdapter ) );

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

template<class T>
fastStatus_t Decoder<T>::Decode(std::list< Data< unsigned char, FastAllocator > > &inputImg, std::list< Image< T, FastAllocator > > &outputImg) {
	hostTimer_t host_timer = NULL;
	hostTimer_t decode_timer = NULL;
	fastGpuTimerHandle_t deviceToHostTimer = NULL;
	fastGpuTimerHandle_t jpegDecoderTimer = NULL;

	float elapsedTimeLoadJpeg = 0.;
	float elapsedTimeGpu = 0.;
	float elapsedTotalDecodeTime = 0.;
	float elapsedTime = 0.;
	float fullTime = 0.;

	if (info) {
		host_timer = hostTimerCreate();
		decode_timer = hostTimerCreate();
		fastGpuTimerCreate(&deviceToHostTimer);
		fastGpuTimerCreate(&jpegDecoderTimer);
	}

	fastStatus_t ret;
	for (std::list< Data< unsigned char, FastAllocator > >::iterator i = inputImg.begin(); i != inputImg.end(); i++) {
		Image<T, FastAllocator> img;
		img.inputFileName = (*i).inputFileName;
		img.outputFileName = (*i).outputFileName;

		if ( info ) {
			hostTimerStart(host_timer);
		}

		jfifInfo.bytestreamSize = (*i).size;
		CHECK_FAST( fastMalloc( (void **)&jfifInfo.h_Bytestream, jfifInfo.bytestreamSize ) );

		CHECK_FAST( fastJfifLoadFromMemory(
			 (*i).data.get(),
			 (*i).size,

			 &jfifInfo
		) );

		if (info) {
			elapsedTimeLoadJpeg = (float)hostTimerEnd(host_timer)*1000.0 + i->loadTimeMs;
		}

		printf("Input image: %s\nInput image size: %dx%d pixels\n", img.inputFileName.c_str(), jfifInfo.width, jfifInfo.height);
		printf("Input sampling format: %s\n", samplingAnalyze(jfifInfo.jpegFmt));
		printf("Input restart interval: %d\n\n", jfifInfo.restartInterval);
		const unsigned channelCount = (jfifInfo.jpegFmt == JPEG_Y) ? 1 : 3;
		if ( jfifInfo.width > maxWidth ||
			 jfifInfo.height > maxHeight ) {
				 fprintf(stderr, "Unsupported image size\n");
				 continue;
		}

		img.w = jfifInfo.width;
		img.h = jfifInfo.height;
		img.wPitch = ((img.w * channelCount + FAST_ALIGNMENT - 1) / FAST_ALIGNMENT) * FAST_ALIGNMENT;
		img.surfaceFmt = surfaceFmt;
		CHECK_FAST_ALLOCATION( img.data.reset( (T *)FastAllocator::allocate<T>(img.h * img.wPitch) ) );
	
		if ( info ) {
			fastGpuTimerStart(jpegDecoderTimer);
			hostTimerStart(decode_timer);
		}

		CHECK_FAST( fastJpegDecode(
			 hDecoder,

			 &jfifInfo
		) );

		if (info) {
			fastGpuTimerStop(jpegDecoderTimer);
			fastGpuTimerStart(deviceToHostTimer);
		}

		CHECK_FAST( fastExportToHostCopy(
			hDeviceToHostAdapter,

			img.data.get(),
			img.w,
			img.wPitch,
			img.h
		) );

		if (info) {
			float elapsedDecodeGpu = 0.;
			float elapsedDeviceToHost = 0.;

			fastGpuTimerStop(deviceToHostTimer);
			elapsedTotalDecodeTime = (float)hostTimerEnd(decode_timer)*1000.0;
			fastGpuTimerGetTime(jpegDecoderTimer, &elapsedDecodeGpu);
			fastGpuTimerGetTime(deviceToHostTimer, &elapsedDeviceToHost);

			elapsedTotalDecodeTime = elapsedTotalDecodeTime - elapsedDecodeGpu - elapsedDeviceToHost;
			float elapsedDecode = elapsedDecodeGpu + ((elapsedTotalDecodeTime>0.0f)?elapsedTotalDecodeTime:0.0f);

			printf("JFIF load time from HDD to CPU memory = %.2f ms\n", elapsedTimeLoadJpeg);
			printf("Decode time (includes host-to-device transfer) = %.2f ms\n", elapsedDecode);
			printf("Device-To-Host transfer = %.2f ms\n\n", elapsedDeviceToHost);

			fullTime += elapsedDecode;
			fullTime += elapsedDeviceToHost;
		}

		printf("Output image: %s\n\n", img.outputFileName.c_str());
		outputImg.push_back(img);

		CHECK_FAST_DEALLOCATION( fastFree(jfifInfo.h_Bytestream) );
		jfifInfo.h_Bytestream = NULL;
	}

	if ( info ) {
		printf("Total time for all images = %.2f ms (without HDD I/O)\n", fullTime);
		hostTimerDestroy(host_timer);
		fastGpuTimerDestroy(deviceToHostTimer);
		fastGpuTimerDestroy(jpegDecoderTimer);
	}

	return FAST_OK;
}

template class Decoder< unsigned char >;
template class Decoder< float >;

