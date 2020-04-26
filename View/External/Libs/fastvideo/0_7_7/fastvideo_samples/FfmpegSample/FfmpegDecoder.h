/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __FFMPEG_DECODER_H__
#define __FFMPEG_DECODER_H__

#include <list>
#include <memory>

#include "FastAllocator.h"

#include "fastvideo_sdk.h"
#include "Image.h"
#include "BaseOptions.h"
#include "ReaderFfmpeg.hpp"

class FfmpegDecoder
{
private:
	static const unsigned Channels = 3;
	static const fastJpegFormat_t SamplingFmt = JPEG_420;
	
	std::unique_ptr<unsigned char, FastAllocator> h_RestoredStream;

	fastJpegDecoderHandle_t hDecoder;
	fastExportToHostHandle_t hDeviceToHostAdapter;

	fastDeviceSurfaceBufferHandle_t dstBuffer;

	ReaderFfmpeg ffmpeg;
	
	unsigned width;
	unsigned height;
	unsigned pitch;

	int totalFrames;
	fastSurfaceFormat_t surfaceFmt;
	fastJfifInfo_t jfifInfo;

	char* outputFilePattern;

	bool info;

public:
	FfmpegDecoder(bool info) { this->info = info; jfifInfo.h_Bytestream = NULL; this->jfifInfo.exifSections = NULL; this->jfifInfo.exifSectionsCount = 0;  };
	~FfmpegDecoder(void) { };

	fastStatus_t Init(BaseOptions &options);
	fastStatus_t Decode();
	fastStatus_t Close();
};

#endif
