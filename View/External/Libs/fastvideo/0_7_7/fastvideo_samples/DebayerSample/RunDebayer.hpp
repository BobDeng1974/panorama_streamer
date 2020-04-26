/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __RUN_DEBAYER__
#define __RUN_DEBAYER__

#include <cassert>
#include <iostream>
#include <vector>
#include "string.h"

#include "supported_files.hpp"
#include "helper_pfm.h"
#include "helper_lut.h"

#include "Image.h"
#include "Debayer.h"
#include "DebayerOptions.h"

#include "FastAllocator.h"
#include "FilesParameterChecker.hpp"

static fastStatus_t RunDebayer(DebayerOptions &options) {
	Debayer hDebayer(options.Info);
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

	for (std::list< Image< unsigned char, FastAllocator > >::iterator i = inputImg.begin(); i != inputImg.end(); i++) {
		if ( (*i).surfaceFmt != FAST_I8 && (*i).surfaceFmt != FAST_I12 && (*i).surfaceFmt != FAST_I16 ) {
			fprintf(stderr, "Input file must not be color\n");
			return FAST_IO_ERROR;
		}
		else {
			options.SurfaceFmt = (*i).surfaceFmt;
			options.BitsPerPixel = (*i).bitsPerPixel;
			options.IsRaw = (*i).isRaw;
		}
	}
	
	if (options.BayerType==FAST_DFPD && options.Is2bytesOutput)
	{
		printf ("DFPD debayer is not emplemented for 16 bit output\n");
		return FAST_INVALID_VALUE;
	}
	printf("Input surface format: grayscale\n");
	printf("Pattern: %s\n", patternAnalyze(options.BayerFormat));
	printf("Output surface format: %s\n", surfaceAnalyze(BaseOptions::GetSurfaceFormatFromExtension(options.OutputPath)));
	printf("Debayer algorithm: %s\n", debayerTypeAnalyze(options.BayerType));

	if (options.BaseColorCorrectionEnabled) {
		printf("Correction matrix:\n");
		printf("\t%.3f\t%.3f\t%.3f\t%.3f\n", options.BaseColorCorrection[0], options.BaseColorCorrection[1], options.BaseColorCorrection[ 2], options.BaseColorCorrection[ 3]);
		printf("\t%.3f\t%.3f\t%.3f\t%.3f\n", options.BaseColorCorrection[4], options.BaseColorCorrection[5], options.BaseColorCorrection[ 6], options.BaseColorCorrection[ 7]);
		printf("\t%.3f\t%.3f\t%.3f\t%.3f\n", options.BaseColorCorrection[8], options.BaseColorCorrection[9], options.BaseColorCorrection[10], options.BaseColorCorrection[11]);
	}

	Image<float, FastAllocator> matrixA;
	if ( options.MatrixA != NULL ) {
		unsigned channels;
		bool failed = false;

		printf("\nMatrix A: %s\n", options.MatrixA);
		CHECK_FAST( __loadPitchPFM(options.MatrixA, matrixA.data, matrixA.w, matrixA.wPitch, FAST_ALIGNMENT, matrixA.h, channels) );

		if ( channels != 1 ) {
			fprintf(stderr, "Matrix A file must not be color\n");
			failed = true;
		}

		if ( options.MaxHeight != matrixA.h || options.MaxWidth != matrixA.w ) {
			fprintf(stderr, "Input and matrix A file parameters mismatch\n");
			failed = true;
		}

		if (failed) {
			fprintf(stderr, "Matrix A file reading error. Ignore parameters\n");
			failed = false;
		}
	}

	Image< unsigned char, FastAllocator > matrixB;
	if ( options.MatrixB != NULL ) {
		bool failed = false;

		printf("\nMatrix B: %s\n", options.MatrixB);
		CHECK_FAST( loadFile( std::string(options.MatrixB), std::string(""), matrixB, options.MaxHeight, options.MaxWidth, 8, false ) );

		if ( matrixB.surfaceFmt != FAST_I8 &&  matrixB.surfaceFmt != FAST_I12 &&  matrixB.surfaceFmt != FAST_I16 ) {
			fprintf(stderr, "Matrix B file must not be color\n");
			failed = true;
		}

		if ( options.MaxHeight != matrixB.h || options.MaxWidth != matrixB.w ) {
			fprintf(stderr, "Input and matrix B file parameters mismatch\n");
			failed = true;
		}

		if (failed) {
			fprintf(stderr, "Matrix B file reading error. Ignore parameters\n");
		}
	}

	std::unique_ptr<unsigned char, FastAllocator> lutData;
	if ( options.LutDebayer != NULL ) {
		if ((options.BitsPerPixel == 8  && !options.Is2bytesOutput)
			|| (options.BitsPerPixel != 8 && options.Is2bytesOutput) )
				printf("\nThere is no bits conversation. LUT file is ignored\n", options.Lut);
		else
		{
			printf("\nLUT file: %s\n", options.Lut);
			CHECK_FAST( __loadLut( options.Lut, lutData, 1 << options.BitsPerPixel  ) );
		}
	} else if ( options.BitsPerPixel != 8  && !options.Is2bytesOutput) {
		fprintf(stderr, "LUT file is not set. Lut is needed for %d to 8 bits conversion.\n", options.BitsPerPixel);
		return FAST_INVALID_VALUE;
	}

	CHECK_FAST( hDebayer.Init(options, (void *)lutData.get(), options.MatrixA != NULL ? matrixA.data.get() : NULL, options.MatrixB != NULL ? matrixB.data.get() : NULL ) );
	CHECK_FAST( hDebayer.Transform(inputImg) );

	inputImg.clear();
	CHECK_FAST( hDebayer.Close() );

	return FAST_OK;
}

#endif // __RUN_DEBAYER__