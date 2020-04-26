/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#ifndef __RUN_JPEG__
#define __RUN_JPEG__

#include <list>

#include "Encoder.h"
#include "Decoder.h"

#include "EncoderOptions.h"
#include "BaseOptions.h"

#include "Image.h"
#include "memory_interface.h"
#include "supported_files.hpp"
#include "FastAllocator.h"

static fastStatus_t JpegDecoder(unsigned char *data, unsigned dataSize, fastJfifInfo_t *jfifInfo) {
	return fastJfifLoadFromMemory(
			 data,
			 dataSize,

			 jfifInfo
		);
}

template<class T>
static fastStatus_t RunDecode(BaseOptions &options){
	Decoder< T > hDecoder(options.Info);
	std::list<  Data< unsigned char, FastAllocator > > inputImg;
	std::list< Image<			  T, FastAllocator > > outputImg;

	options.SurfaceFmt = BaseOptions::GetSurfaceFormatFromExtension(options.OutputPath);

	fastStatus_t ret;
	if ( options.IsFolder ) {
		CHECK_FAST( loadJpegImages( options.InputPath, inputImg, false ) );
		int idx = 0;
		for (typename std::list< Data< unsigned char, FastAllocator > >::iterator i = inputImg.begin(); i != inputImg.end(); i++, idx++) {
			i->outputFileName = generateOutputFileName(options.OutputPath, idx);
		}
	} else {
		ret = loadJpeg(std::string(options.InputPath), inputImg, false);
		if (ret == FAST_IO_ERROR) {
			fprintf(stderr, "Input image file %s has not been found!\n", options.InputPath);
			return ret;
		}
		if (ret != FAST_OK)
			return ret;

		(--inputImg.end())->outputFileName =  std::string(options.OutputPath);
	}

	fastJfifInfo_t jfifInfo;
	jfifInfo.h_Bytestream = NULL;
	jfifInfo.exifSections = NULL;
	jfifInfo.exifSectionsCount = 0;

	jfifInfo.bytestreamSize = (*inputImg.begin()).size;
	CHECK_FAST( fastMalloc( (void **)&jfifInfo.h_Bytestream, jfifInfo.bytestreamSize ) );
		
	CHECK_FAST( JpegDecoder( (*inputImg.begin()).data.get(), (*inputImg.begin()).size, &jfifInfo ) );

	if (jfifInfo.h_Bytestream != NULL)
		CHECK_FAST_DEALLOCATION( fastFree(jfifInfo.h_Bytestream) );

	for (int i = 0; i < jfifInfo.exifSectionsCount; i++) {
		free(jfifInfo.exifSections[i].exifData);
	}

	if ( jfifInfo.exifSections != NULL ) {
		free(jfifInfo.exifSections);
	}

	options.MaxHeight = options.MaxHeight == 0 ? jfifInfo.height : options.MaxHeight;
	options.MaxWidth = options.MaxWidth == 0 ? jfifInfo.width : options.MaxWidth;
	options.SurfaceFmt = jfifInfo.jpegFmt == JPEG_Y ? FAST_I8 : options.SurfaceFmt;

	CHECK_FAST( hDecoder.Init(options) );
	CHECK_FAST( hDecoder.Decode(inputImg, outputImg) );

	for (typename std::list< Image< T, FastAllocator > >::iterator i = outputImg.begin(); i != outputImg.end(); i++) {
		CHECK_FAST( saveToFile((char *)(*i).outputFileName.c_str(), (*i).data, (*i).surfaceFmt, (*i).h, (*i).w, (*i).wPitch, false) );
	}

	inputImg.clear();
	outputImg.clear();
	CHECK_FAST( hDecoder.Close() );

	return FAST_OK;
}

template<class T>
static fastStatus_t RunEncode(EncoderOptions &options) {
	Encoder< T > hEncoder(options.Info);
	std::list< Image< T, FastAllocator > > inputImg;

	fastStatus_t ret;

	if ( options.IsFolder ) {
		CHECK_FAST( loadImages(options.InputPath, options.OutputPath, inputImg, 0, 0, 0, false) );
	} else {
		Image< T, FastAllocator > img;

		ret = loadFile( std::string(options.InputPath), std::string(options.OutputPath), img, options.MaxHeight, options.MaxWidth, 8, false );
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

	for (typename std::list< Image< T, FastAllocator > >::iterator i = inputImg.begin(); i != inputImg.end(); i++ ) {
		// correct options if grayscale image
		if ( ((*i).surfaceFmt == FAST_I8) ) {
			(*i).samplingFmt = JPEG_Y;
			options.SamplingFmt = (*i).samplingFmt;
			options.SurfaceFmt = FAST_I8;
		} else {
			(*i).samplingFmt = options.SamplingFmt;
			options.SurfaceFmt = (*i).surfaceFmt;
		}
	}
	
	printf("Surface format: %s\n", surfaceAnalyze((*(inputImg.begin())).surfaceFmt));
	printf("Sampling format: %s\n", samplingAnalyze((*(inputImg.begin())).samplingFmt));
	printf("JPEG quality: %d%%\n", options.Quality);
	printf("Restart interval: %d\n", options.RestartInterval);

	CHECK_FAST( hEncoder.Init(options) );
	CHECK_FAST( hEncoder.Encode(inputImg) );

	inputImg.clear();
	CHECK_FAST( hEncoder.Close() );

	return FAST_OK;
}

#endif //__RUN_JPEG__