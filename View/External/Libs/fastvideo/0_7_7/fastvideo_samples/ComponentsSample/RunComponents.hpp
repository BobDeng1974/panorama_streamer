/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __RUN_COMPONENTS__
#define __RUN_COMPONENTS__

#include "BaseColorCorrection.h"
#include "Lut8C.h"
#include "Affine.h"

#include "GrayscaleCorrectionOptions.h"
#include "LutOptions.h"
#include "AffineOptions.h"

#include "Image.h"
#include "FastAllocator.h"

#include "supported_files.hpp"
#include "helper_lut.h"

static fastStatus_t RunBaseColorCorrection(GrayscaleCorrectionOptions &options){
	BaseColorCorrection hBaseColorCorrection(options.Info);
		
	fastStatus_t ret;
	std::list< Image< unsigned char, FastAllocator > > inputImg;
	if ( options.IsFolder ) {
		CHECK_FAST( loadImages( options.InputPath, options.OutputPath, inputImg, 0, 0, 8, false ) );
	} else {
		Image< unsigned char, FastAllocator > img;

		ret = loadFile( std::string(options.InputPath), std::string(options.OutputPath), img, 0, 0, 8, false );
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
		if ( (*i).surfaceFmt == FAST_I8 ) {
			// validate input images
			fprintf(stderr, "Input file must be color\n");
			return FAST_IO_ERROR;
		}
	}

	printf("Input surface format: %s\n", surfaceAnalyze((*inputImg.begin()).surfaceFmt));
	printf("Output surface format: %s\n", surfaceAnalyze(options.SurfaceFmt));

	if ( !options.BaseColorCorrectionEnabled ) {
		return FAST_INVALID_VALUE;
	}

	printf("Correction matrix:\n");
	printf("\t%.3f\t%.3f\t%.3f\t%.3f\n", options.BaseColorCorrection[0], options.BaseColorCorrection[1], options.BaseColorCorrection[ 2], options.BaseColorCorrection[ 3]);
	printf("\t%.3f\t%.3f\t%.3f\t%.3f\n", options.BaseColorCorrection[4], options.BaseColorCorrection[5], options.BaseColorCorrection[ 6], options.BaseColorCorrection[ 7]);
	printf("\t%.3f\t%.3f\t%.3f\t%.3f\n", options.BaseColorCorrection[8], options.BaseColorCorrection[9], options.BaseColorCorrection[10], options.BaseColorCorrection[11]);
	
	CHECK_FAST( hBaseColorCorrection.Init(options) );
	CHECK_FAST( hBaseColorCorrection.Transform(inputImg) );

	inputImg.clear();
	CHECK_FAST( hBaseColorCorrection.Close() );

	return FAST_OK;
}

static fastStatus_t RunLut8c(LutOptions &options){
	Lut8C hLut(options.Info);
		
	fastStatus_t ret;
	std::list< Image< unsigned char, FastAllocator > > inputImg;
	if ( options.IsFolder ) {
		CHECK_FAST( loadImages( options.InputPath, options.OutputPath, inputImg, 0, 0, 8, false ) );
	} else {
		Image< unsigned char, FastAllocator > img;

		ret = loadFile( std::string(options.InputPath), std::string(options.OutputPath), img, 0, 0, 8, false );
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
		if ( (*i).surfaceFmt == FAST_I8 ) {
			// validate input images
			fprintf(stderr, "Input file must be color\n");
			return FAST_IO_ERROR;
		}
	}

	printf("Input surface format: %s\n", surfaceAnalyze((*inputImg.begin()).surfaceFmt));
	printf("Output surface format: %s\n", surfaceAnalyze(options.SurfaceFmt));
	
	std::unique_ptr<unsigned char, FastAllocator> lutDataR, lutDataG, lutDataB;
	printf("\nLUT (R channel) file: %s\n", options.Lut_R);
	CHECK_FAST( __loadLut( options.Lut_R, lutDataR, 256 ) );
	printf("LUT (G channel) file: %s\n", options.Lut_R);
	CHECK_FAST( __loadLut( options.Lut_G, lutDataG, 256 ) );
	printf("LUT (B channel) file: %s\n", options.Lut_R);
	CHECK_FAST( __loadLut( options.Lut_B, lutDataB, 256 ) );
	
	CHECK_FAST( hLut.Init(options, lutDataR.get(), lutDataG.get(), lutDataB.get() ) );
	CHECK_FAST( hLut.Transform(inputImg) );

	inputImg.clear();
	CHECK_FAST( hLut.Close() );

	return FAST_OK;
}

static fastStatus_t RunAffine(AffineOptions options) {
	Affine hAffine(options.Info);
	fastStatus_t ret;

	std::list< Image< unsigned char, FastAllocator > > inputImg;

	if ( options.IsFolder ) {
		CHECK_FAST( loadImages( options.InputPath, options.OutputPath, inputImg, 0, 0, 8, false ) );
	} else {
		Image< unsigned char, FastAllocator > img;

		ret = loadFile( std::string(options.InputPath), std::string(options.OutputPath), img, 0, 0, 8, false );
		if (ret != FAST_OK) {
			if (ret == FAST_IO_ERROR) {
				fprintf(stderr, "Input image file %s has not been found!\n", options.InputPath);
			}
			return ret;
		}

		options.MaxHeight = options.MaxHeight == 0 ? img.h : options.MaxHeight;
		options.MaxWidth = options.MaxWidth == 0 ? img.w : options.MaxWidth;
		inputImg.push_back(img);
	}

	printf("Input surface format: %s\n", surfaceAnalyze(options.SurfaceFmt));
	
	CHECK_FAST( hAffine.Init(options ) );
	CHECK_FAST( hAffine.Transform(inputImg) );

	inputImg.clear();
	CHECK_FAST( hAffine.Close() );

	return FAST_OK;
}

#endif // __RUN_COMPONENTS__