/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __DEBAYER_FFMPEG_H__
#define __DEBAYER_FFMPEG_H__

#include "fastvideo_sdk.h"

#include "Image.h"
#include "FastAllocator.h"
#include "FfmpegOptions.h"
#include "WriterFfmpeg.hpp"

class DebayerFfmpeg
{
private:

	static const int JPEG_HEADER_SIZE = 1024;
	
	fastImportFromHostHandle_t hImportFromHost;
	fastImageFiltersHandle_t hMad;
	fastImageFiltersHandle_t hColorCorrection;
	fastDebayerHandle_t hDebayer;
	fastImageFiltersHandle_t hLut;
	fastJpegEncoderHandle_t hEncoder;
	fastExportToDeviceHandle_t hExportToDevice;
	
	fastDeviceSurfaceBufferHandle_t srcBuffer;
	fastDeviceSurfaceBufferHandle_t madBuffer;
	fastDeviceSurfaceBufferHandle_t colorCorrectionBuffer;
	fastDeviceSurfaceBufferHandle_t debayerBuffer;
	fastDeviceSurfaceBufferHandle_t lutBuffer;

	unsigned maxWidth;
	unsigned maxPitch;
	unsigned maxHeight;
	unsigned bufferSize;
	fastJfifInfo_t jfifInfo;

	unsigned char *d_buffer;
	unsigned char *jpegImage;

	WriterFfmpeg ffmpeg;
	int frameRate;
	int framesCount;

	bool info;

	int quality;
	fastSurfaceFormat_t surfaceFmt;

public:
	DebayerFfmpeg(bool info) { this->info = info; hMad = NULL; jfifInfo.h_Bytestream = NULL; this->jfifInfo.exifSections = NULL; this->jfifInfo.exifSectionsCount = 0; };
	~DebayerFfmpeg(void) { };

	fastStatus_t Init(FfmpegOptions &options, std::unique_ptr<unsigned char, FastAllocator> &lut, float *matrixA, unsigned char *matrixB);
	fastStatus_t StoreFrame(Image<unsigned char, FastAllocator> &image);
	fastStatus_t Close();

	void *GetDevicePtr();
};

#endif	// __DEBAYER_FFMPEG_H__
