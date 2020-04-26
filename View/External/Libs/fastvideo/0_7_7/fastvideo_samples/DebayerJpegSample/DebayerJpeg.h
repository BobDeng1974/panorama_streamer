/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __DEBAYER_JPEG_H__
#define __DEBAYER_JPEG_H__

#include "fastvideo_sdk.h"
#include <memory>
#include "FastAllocator.h"

#include "Image.h"
#include "DebayerJpegOptions.h"

class DebayerJpeg {
private:
	fastDebayerHandle_t hDebayer;
	fastJpegEncoderHandle_t hEncoder;

	fastImportFromHostHandle_t hHostToDeviceAdapter;

	fastDeviceSurfaceBufferHandle_t srcBuffer;
	fastDeviceSurfaceBufferHandle_t debayerBuffer;

	fastJfifInfo_t jfifInfo;

	unsigned maxWidth;
	unsigned maxHeight;

	bool info;
	bool folder;

	int quality;

public:
	DebayerJpeg(bool info) { this->info = info; jfifInfo.h_Bytestream = NULL; this->jfifInfo.exifSections = NULL; this->jfifInfo.exifSectionsCount = 0; };
	~DebayerJpeg(void) {};

	fastStatus_t Init(DebayerJpegOptions &options, void *lut);
	fastStatus_t Transform(std::list< Image<unsigned char, FastAllocator> > &image);
	fastStatus_t Close();
};

#endif
