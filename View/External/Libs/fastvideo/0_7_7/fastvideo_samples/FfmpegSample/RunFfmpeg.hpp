/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __RUN_FFMPEG__
#define __RUN_FFMPEG__

#include <stdio.h>

#include "FfmpegEncoder.h"
#include "FfmpegDecoder.h"

#include "EncoderOptions.h"

#include "FastAllocator.h"
#include "memory_interface.h"
#include "supported_files.hpp"

static fastStatus_t RunFfmpegEncode(EncoderOptions options) {
	std::list< Image< unsigned char, FastAllocator > > inputImg;
	
	fastStatus_t ret;
	if ( options.IsFolder ) {
		CHECK_FAST( loadImages(options.InputPath, options.OutputPath, inputImg, 0, 0, 0, false) );

		options.SurfaceFmt = (*(inputImg.begin())).surfaceFmt;
	} else {
		return FAST_INVALID_VALUE;
	}

	for (std::list< Image< unsigned char, FastAllocator > >::iterator i = inputImg.begin(); i != inputImg.end(); i++) {
		(*i).samplingFmt = options.SamplingFmt;

		if ( ((*i).surfaceFmt == FAST_I8) ) {
			fprintf(stderr, "Grayscale images are not supported\n");
			return FAST_INVALID_FORMAT;
		}

		if ( (*i).surfaceFmt != options.SurfaceFmt ) {
			fprintf(stderr, "Images has different image surface format\n");
			return FAST_INVALID_FORMAT;
		}
	}

	printf("Surface format: %s\n", surfaceAnalyze(options.SurfaceFmt));
	printf("Sampling format: %s\n", samplingAnalyze(options.SamplingFmt));
	printf("JPEG quality: %d%%\n", options.Quality);
	printf("Restart interval: %d\n", options.RestartInterval);

	FfmpegEncoder hFFMPEG(options.Info);
	CHECK_FAST( hFFMPEG.Init(options) );
	CHECK_FAST( hFFMPEG.Encode(inputImg) );
	CHECK_FAST( hFFMPEG.Close() );

	return FAST_OK;
}

static fastStatus_t RunFfmpegDecode(BaseOptions &options){
	fastStatus_t ret;

	if ( options.IsFolder ) {
		return FAST_INVALID_VALUE;
	}
	
	FfmpegDecoder hDecoder(options.Info);
	CHECK_FAST( hDecoder.Init(options) );
	CHECK_FAST( hDecoder.Decode() );
	CHECK_FAST( hDecoder.Close() );

	return FAST_OK;
}

#endif // __RUN_FFMPEG__