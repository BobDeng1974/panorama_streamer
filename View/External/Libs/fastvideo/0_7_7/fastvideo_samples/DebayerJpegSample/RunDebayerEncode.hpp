/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __RUN_DEBAYER_JPEG__
#define __RUN_DEBAYER_JPEG__

#include <cassert>
#include <iostream>
#include <vector>

#include "supported_files.hpp"

#include "helper_pfm.h"
#include "helper_lut.h"

#include <string>

#include "Image.h"
#include "DebayerJpeg.h"
#include "DebayerJpegOptions.h"

#include "FastAllocator.h"
#include "FilesParameterChecker.hpp"

static fastStatus_t RunDebayerEncode(DebayerJpegOptions &options){
	DebayerJpeg hDebayerJpeg(options.Info);
		
	fastStatus_t ret;
	std::list< Image< unsigned char, FastAllocator > > inputImg;
	if ( options.IsFolder ) {
		CHECK_FAST( loadImages( options.InputPath, options.OutputPath, inputImg, options.RawWidth, options.RawHeight, options.BitsPerPixel, false ) );
	} else {
		Image< unsigned char, FastAllocator > img;

		ret = loadFile( std::string(options.InputPath), std::string(options.OutputPath), img, options.RawHeight, options.RawWidth, options.BitsPerPixel, false );
		if (ret == FAST_IO_ERROR) {
			fprintf(stderr, "Input image file %s has not been found!\n", options.InputPath);
			return ret;
		}
		if (ret != FAST_OK)
			return ret;

		options.MaxHeight = options.MaxHeight == 0 ? img.h : options.MaxHeight;
		options.MaxWidth = options.MaxWidth == 0 ? img.w : options.MaxWidth;
		inputImg.push_back(img);
	}

	if ( options.BitsPerPixel != 8 ) {
		fprintf(stderr, "Unsupported image format (just 8 bits)\n");
		return FAST_INVALID_VALUE;
	}

	for (std::list< Image< unsigned char, FastAllocator > >::iterator i = inputImg.begin(); i != inputImg.end(); i++) {
		if ( (*i).surfaceFmt != FAST_I8 ) {
			// validate input images
			fprintf(stderr, "Input file must not be color\n");
			return FAST_IO_ERROR;
		}
	}

	printf("Input surface format: grayscale\n");
	printf("Pattern: %s\n", patternAnalyze(options.BayerFormat));
	printf("Output surface format: %s\n", surfaceAnalyze(options.SurfaceFmt));
	printf("Output sampling format: %s\n", samplingAnalyze(options.SamplingFmt));
	printf("Debayer algorithm: %s\n", debayerTypeAnalyze(options.BayerType));
	printf("JPEG quality: %d%%\n", options.Quality);
	printf("Restart interval: %d\n", options.RestartInterval);

	std::unique_ptr<unsigned char, FastAllocator> lutData;
	if ( options.Lut != NULL ) {
		printf("\nLUT file: %s\n", options.Lut);
		CHECK_FAST( __loadLut( options.Lut, lutData, 256 ) );
	}

	CHECK_FAST( hDebayerJpeg.Init(options, lutData.get() ) );
	CHECK_FAST( hDebayerJpeg.Transform(inputImg) );

	inputImg.clear();
	CHECK_FAST( hDebayerJpeg.Close() );

	return FAST_OK;
}

#endif // __RUN_DEBAYER_JPEG__