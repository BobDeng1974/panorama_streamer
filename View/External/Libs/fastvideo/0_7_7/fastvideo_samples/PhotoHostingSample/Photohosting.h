/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __PHOTOHOSTING_H__
#define __PHOTOHOSTING_H__

#include <list>
#include <memory>

#include "FastAllocator.h"

#include "fastvideo_sdk.h"
#include "ResizerOptions.h"
#include "Image.h"

class Photohosting
{
private:
	std::unique_ptr<unsigned char, FastAllocator> h_ResizedJpegStream;
	unsigned resizedJpegStreamSize;

	fastResizerHandle_t hResizer;
	fastJpegDecoderHandle_t hDecoder;
	fastJpegEncoderHandle_t hEncoder;
	fastImageFiltersHandle_t hImageFilterAfter;
	fastImageFiltersHandle_t hImageFilterBefore;
	fastCropHandle_t hCrop;

	fastDeviceSurfaceBufferHandle_t d_decoderBuffer;
	fastDeviceSurfaceBufferHandle_t d_resizerBuffer;
	fastDeviceSurfaceBufferHandle_t d_imageFilterBufferAfter;
	fastDeviceSurfaceBufferHandle_t d_imageFilterBufferBefore;
	fastDeviceSurfaceBufferHandle_t d_cropBuffer;

	ResizerOptions options;
	fastJfifInfo_t jfifInfo;

	unsigned channelCount;

	bool info;

public:
	Photohosting(bool info) { this->info = info; jfifInfo.h_Bytestream = NULL; this->jfifInfo.exifSections = NULL; this->jfifInfo.exifSectionsCount = 0; };
	~Photohosting(void) { };

	fastStatus_t Init(ResizerOptions &options);
	fastStatus_t Resize(std::list< Data< unsigned char, FastAllocator > > &inputImages, const char *outputPattern);
	fastStatus_t Close();
};

#endif
