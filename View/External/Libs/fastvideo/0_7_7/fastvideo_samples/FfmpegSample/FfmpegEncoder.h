/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __FFMPEG_H__
#define __FFMPEG_H__

#include <list>
#include <memory>

#include "FastAllocator.h"

#include "fastvideo_sdk.h"
#include "Image.h"
#include "EncoderOptions.h"
#include "WriterFfmpeg.hpp"

class FfmpegEncoder
{
private:
	std::unique_ptr<unsigned char, FastAllocator> h_Jpegstream;

	int JpegstreamSize;

	static const int JPEG_HEADER_SIZE = 1024;
	static const int FRAME_TIME = 2;

	fastJpegEncoderHandle_t hEncoder;
	fastImportFromHostHandle_t hHostToDeviceAdapter;

	fastDeviceSurfaceBufferHandle_t srcBuffer;

	fastJfifInfo_t jfifInfo;
	int Quality;

	fastJpegExifSection_t *exifSecitons;
	unsigned exifSecitonsCount;

	WriterFfmpeg ffmpeg;
	int frameRate;
	int framesCount;

	unsigned maxWidth;
	unsigned maxHeight;

	bool info;
	
public:
	FfmpegEncoder(bool info) { this->info = info; jfifInfo.h_Bytestream = NULL; this->jfifInfo.exifSections = NULL; this->jfifInfo.exifSectionsCount = 0; };
	~FfmpegEncoder(void) { };

	fastStatus_t Init(EncoderOptions &options);
	fastStatus_t Encode(std::list< Image<unsigned char, FastAllocator> > &inputImg);
	fastStatus_t Close();
};

#endif
