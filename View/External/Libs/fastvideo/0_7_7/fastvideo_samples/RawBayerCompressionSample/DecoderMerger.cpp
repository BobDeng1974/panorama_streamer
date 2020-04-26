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

#include "DecoderMerger.h"
#include "timing.hpp"
#include "memory_interface.h"
#include "supported_files.hpp"
#include "ExifInfo.hpp"

fastStatus_t DecoderMerger::Init(DebayerJpegOptions &options, unsigned maxRestoredWidth, unsigned maxRestoredHeight) {
	fastStatus_t ret;

	maxSrcWidth = options.MaxWidth;
	maxSrcHeight = options.MaxHeight;

	CHECK_FAST( fastJpegDecoderCreate(
		&hDecoder,

		options.SurfaceFmt,
		maxSrcWidth,
		maxSrcHeight,

		&decoderBuffer
	) );
	
	CHECK_FAST( fastBayerMergerCreate(
		&hBayerMerger,
		
		options.SurfaceFmt,
		
		maxRestoredWidth,
		maxRestoredHeight,

		decoderBuffer,
		&bayerMergerBuffer
	) );

	CHECK_FAST( fastDebayerCreate(
		&hDebayer,
		
		options.BayerType,
		options.BayerFormat,
		options.SurfaceFmt,
		
		maxRestoredWidth,
		maxRestoredHeight,

		bayerMergerBuffer,
		&debayerBuffer
	) );

	surfaceFmt = options.SurfaceFmt = BaseOptions::GetSurfaceFormatFromExtension( options.OutputPath );

	CHECK_FAST( fastExportToHostCreate(
		&hExportToHost,

		options.SurfaceFmt,

		debayerBuffer
	) );

	unsigned pitch = 3 * ( ( ( maxRestoredWidth + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT );
	CHECK_FAST_ALLOCATION( h_Result.reset( (unsigned char *)FastAllocator::allocate<unsigned char>(pitch * maxRestoredHeight) ) );

	unsigned requestedMemSpace = 0;
	unsigned tmp = 0;

	CHECK_FAST( fastJpegDecoderGetAllocatedGpuMemorySize( hDecoder, &tmp ) );
	requestedMemSpace += tmp;
	
	CHECK_FAST( fastBayerMergerGetAllocatedGpuMemorySize( hBayerMerger, &tmp ) );
	requestedMemSpace += tmp;

	CHECK_FAST( fastDebayerGetAllocatedGpuMemorySize( hDebayer, &tmp ) );
	requestedMemSpace += tmp;

	printf("Requested GPU memory space: %.2f MB\n\n", requestedMemSpace / ( 1024.0 * 1024.0 ) );
	
	return FAST_OK;
}

fastStatus_t DecoderMerger::Close() {
	fastStatus_t ret;

	CHECK_FAST( fastJpegDecoderDestroy( hDecoder ) );
	CHECK_FAST( fastBayerMergerDestroy( hBayerMerger ) );
	CHECK_FAST( fastDebayerDestroy( hDebayer ) );
	CHECK_FAST( fastExportToHostDestroy( hExportToHost ) );

	return FAST_OK;
}

fastStatus_t DecoderMerger::Transform(std::list< Data<unsigned char, FastAllocator> > &inputImages) {
	float fullTime = 0.;
	float elapsedTimeGpu = 0.;
	float elapsedTimeLoadJpeg = 0.;

	unsigned restoredWidth, restoredHeight;
	
	hostTimer_t host_timer = NULL;
	fastGpuTimerHandle_t decoderTimer = NULL;
	fastGpuTimerHandle_t bayerMergerTimer = NULL;
	fastGpuTimerHandle_t debayerTimer = NULL;
	fastGpuTimerHandle_t exportToHostTimer = NULL;

	if (info) {
		fastGpuTimerCreate(&decoderTimer);
		fastGpuTimerCreate(&bayerMergerTimer);
		fastGpuTimerCreate(&debayerTimer);
		fastGpuTimerCreate(&exportToHostTimer);
	}

	fastStatus_t ret;
	for (std::list< Data<unsigned char, FastAllocator> >::iterator i = inputImages.begin(); i != inputImages.end(); i++) {
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
		
		if ( maxSrcWidth < jfifInfo.width ||
			 maxSrcHeight < jfifInfo.height ) {
				fprintf(stderr, "No decoder initialized with these parameters\n");
				continue;
		}

		printf("Input image: %s\nInput image size: %dx%d pixels\n", (*i).inputFileName.c_str(), jfifInfo.width, jfifInfo.height);
		bool bayerExifExist = false;

		if ( jfifInfo.exifSections != NULL ) {
			for (int i = 0; i < jfifInfo.exifSectionsCount; i++) {
				fastBayerPattern_t bayerFormat;
				if ( ParseSplitterExif( &jfifInfo.exifSections[i], bayerFormat, restoredWidth, restoredHeight ) == FAST_OK ) {
					bayerExifExist = true;
				}

				free(jfifInfo.exifSections[i].exifData);
			}

			free( jfifInfo.exifSections );
		}

		if ( !bayerExifExist ) {
			fprintf(stderr, "Incorrect JPEG (%s): debayer parameters in EXIF sections was not found\n", (*i).inputFileName.c_str());
			continue;
		}

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
			printf("Decode time = %.2f ms\n\n", elapsedTimeGpu);

			fullTime += elapsedTimeGpu;
		} 

		if (info) {
			fastGpuTimerStart(bayerMergerTimer);
		}

		CHECK_FAST( fastBayerMergerMerge(
			hBayerMerger,

			restoredWidth,
			restoredHeight
		) );

		if (info) {
			fastGpuTimerStop(bayerMergerTimer);
			fastGpuTimerGetTime(bayerMergerTimer, &elapsedTimeGpu);
			printf("DecoderMerger merger time = %.2f ms\n\n", elapsedTimeGpu);

			fullTime += elapsedTimeGpu;
			fastGpuTimerStart(debayerTimer);
		}
		
		CHECK_FAST( fastDebayerTransform(
			hDebayer,

			restoredWidth,
			restoredHeight
		) );

		if (info) {
			fastGpuTimerStop(debayerTimer);
			fastGpuTimerGetTime(debayerTimer, &elapsedTimeGpu);
			printf("Debayer time = %.2f ms\n\n", elapsedTimeGpu);

			fullTime += elapsedTimeGpu;
			fastGpuTimerStart(exportToHostTimer);
		}
		
		CHECK_FAST( fastExportToHostCopy(
			hExportToHost,

			h_Result.get(),
			restoredWidth,
			3 * ( ( ( restoredWidth + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT ),
			restoredHeight
		) );
		
		if (info) {
			fastGpuTimerStop(exportToHostTimer);
			fastGpuTimerGetTime(exportToHostTimer, &elapsedTimeGpu);

			fullTime += elapsedTimeGpu;
			printf("Device-to-host transfer = %.2f ms\n\n", elapsedTimeGpu);
		}
		
		CHECK_FAST( saveToFile(
			(char *)(*i).outputFileName.c_str(),
			h_Result,
			surfaceFmt,
			restoredHeight,
			restoredWidth,
			3 * ( ( ( restoredWidth + FAST_ALIGNMENT - 1 ) / FAST_ALIGNMENT ) * FAST_ALIGNMENT ),
			false
		) );
		
		printf("Output image: %s\nOutput image size: %dx%d pixels\n\n", (*i).outputFileName.c_str(), restoredWidth, restoredHeight);
	}

	if ( info ) {
		printf("Total for all images = %.2f ms\n", fullTime);

		fastGpuTimerDestroy(decoderTimer);
		fastGpuTimerDestroy(bayerMergerTimer);
		fastGpuTimerDestroy(debayerTimer);
		fastGpuTimerDestroy(exportToHostTimer);
	}

	return FAST_OK;
}
