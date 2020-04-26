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

#include "FfmpegDecoder.h"
#include "timing.hpp"
#include "memory_interface.h"
#include "supported_files.hpp"

fastStatus_t FfmpegDecoder::Init(BaseOptions &options) {
	fastStatus_t ret;

	CHECK_FAST( ffmpeg.Initialize(options.InputPath) );

	height = ffmpeg.GetHeight();
	width  = ffmpeg.GetWidth();
	pitch = 3 * ( ( ( width + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT );
	totalFrames = ffmpeg.GetTotalFrames();

	surfaceFmt = options.SurfaceFmt;
	outputFilePattern = options.OutputPath;

	CHECK_FAST( fastJpegDecoderCreate(
		&hDecoder,

		options.SurfaceFmt == FAST_BGR8 ? FAST_RGB8 : options.SurfaceFmt,
		width,
		height,

		&dstBuffer
	) );

	CHECK_FAST( fastExportToHostCreate(
		&hDeviceToHostAdapter,

		options.SurfaceFmt,

		dstBuffer
	) );

	CHECK_FAST_ALLOCATION( h_RestoredStream.reset( (unsigned char *)FastAllocator::allocate<unsigned char>(height * pitch) ) );

	unsigned requestedMemSpace = 0;
	CHECK_FAST( fastJpegDecoderGetAllocatedGpuMemorySize( hDecoder, &requestedMemSpace ) );
	printf("Requested GPU memory space: %.2f MB\n\n", requestedMemSpace / ( 1024.0 * 1024.0 ) );

	return FAST_OK;
}

fastStatus_t FfmpegDecoder::Close() {
	fastStatus_t ret;

	CHECK_FAST( fastJpegDecoderDestroy(hDecoder) );
	CHECK_FAST( fastExportToHostDestroy( hDeviceToHostAdapter ) );
	CHECK_FAST( ffmpeg.Close() );

	return FAST_OK;
}

fastStatus_t FfmpegDecoder::Decode() {
	hostTimer_t host_timer = NULL;
	float elapsedTime = 0.;
	float fullTime = 0.;

	unsigned char *buffer;
	unsigned int bufferSize;

	if (info) {
		host_timer = hostTimerCreate();
	}

	fastStatus_t ret;
	for (int i = 0; i < totalFrames; i++) {
		ffmpeg.GetNextFrame(&buffer, &bufferSize);

		jfifInfo.bytestreamSize = bufferSize;
		CHECK_FAST( fastMalloc( (void **)&jfifInfo.h_Bytestream, jfifInfo.bytestreamSize ) );
		
		CHECK_FAST( fastJfifLoadFromMemory(
			 buffer,
			 bufferSize,

			 &jfifInfo
		) );

		if (info) {
			hostTimerStart(host_timer);
		}
		
		CHECK_FAST( fastJpegDecode(
			 hDecoder,

			 &jfifInfo
		) );

		if (info) {
			elapsedTime = (float)hostTimerEnd(host_timer) * 1000.0f;
			printf("Decode time (includes host-to-device and device-to-host transfers) = %.2f ms\n", elapsedTime);
			fullTime += elapsedTime;
		}

		CHECK_FAST( fastExportToHostCopy(
			hDeviceToHostAdapter,

			h_RestoredStream.get(),
			jfifInfo.width,
			pitch,
			jfifInfo.height
		) );

		std::string outputFileName = generateOutputFileName(outputFilePattern, i);
		CHECK_FAST( saveToFile((char *)outputFileName.c_str(), h_RestoredStream, surfaceFmt, jfifInfo.height, jfifInfo.width, pitch, false) );

		CHECK_FAST_DEALLOCATION( fastFree(jfifInfo.h_Bytestream) );
		jfifInfo.h_Bytestream = NULL;

		for (int i = 0; i < jfifInfo.exifSectionsCount; i++) {
			free(jfifInfo.exifSections[i].exifData);
		}

		if ( jfifInfo.exifSections != NULL ) {
			free(jfifInfo.exifSections);
		}
	}

	if ( info ) {
		printf("Total time for all images = %.2f ms (without HDD I/O)\n", fullTime);
		hostTimerDestroy(host_timer);
	}

	return FAST_OK;
}
