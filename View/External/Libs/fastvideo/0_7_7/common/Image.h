/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "fastvideo_sdk.h"
#include <string>
#include <memory>
#include <utility>
#include <cstring>

template< class T, class Allocator >
class Image {
public:
	std::string inputFileName;
	std::string outputFileName;

	//void *data;
	std::unique_ptr<T, Allocator> data;
	unsigned w;
	unsigned h;
	unsigned wPitch;
	unsigned bitsPerPixel;
	
	fastSurfaceFormat_t surfaceFmt;
	fastJpegFormat_t samplingFmt;
	bool isRaw;

	Image() {
		w = h = wPitch = 0;
		bitsPerPixel = 8;
		isRaw = false;
	};

	Image(const Image &img) {
		inputFileName = img.inputFileName;
		outputFileName = img.outputFileName;
		w = img.w;
		h = img.h;
		wPitch = img.wPitch;
		bitsPerPixel = img.bitsPerPixel;
		surfaceFmt = img.surfaceFmt;
		samplingFmt = img.samplingFmt;

		unsigned fullSize = wPitch * h;
		isRaw = img.isRaw;

		try {
			data.reset( (T *)Allocator::template allocate<T>(fullSize) );
		} catch (std::bad_alloc& ba) {
			fprintf(stderr, "Memory allocation failed: %s\n", ba.what());
			return;
		}
		memcpy( data.get(), img.data.get(), fullSize * sizeof(T) );
		//data = std::move(img.data);
	};

	unsigned GetBytesPerPixel ()
	{
		return (bitsPerPixel+7)/8;
	}
};

template< class T, class Allocator >
class Data {
public:
	std::string inputFileName;
	std::string outputFileName;

	std::unique_ptr<T, Allocator> data;
	unsigned int size;
	
	bool encoded;
	float loadTimeMs;

	
	Data() { };

	Data(const Data &img) {
		inputFileName = img.inputFileName;
		outputFileName = img.outputFileName;
		size = img.size;
		encoded = img.encoded;
		loadTimeMs = img.loadTimeMs;

		try {
			data.reset( (T *)Allocator::template allocate<T>(size) );
		} catch (std::bad_alloc& ba) {
			fprintf(stderr, "Memory allocation failed: %s\n", ba.what());
			return;
		}
		memcpy( data.get(), img.data.get(), size * sizeof(T) );
		//data = std::move(img.data);
	};
};

#endif
