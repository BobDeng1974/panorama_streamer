/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __SPLITTER_ENCODER__
#define __SPLITTER_ENCODER__

#include <list>
#include <memory>

#include "FastAllocator.h"

#include "fastvideo_sdk.h"

#include "DebayerJpegOptions.h"
#include "Image.h"

class SplitterEncoder
{
private:
	fastImportFromHostHandle_t hImportFromHost;
	fastBayerSplitterHandle_t hBayerSplitter;
	fastJpegEncoderHandle_t hEncoder;

	fastDeviceSurfaceBufferHandle_t srcBuffer;
	fastDeviceSurfaceBufferHandle_t bayerSplitterBuffer;

	unsigned maxSrcWidth;
	unsigned maxSrcHeight;

	unsigned maxDstWidth;
	unsigned maxDstHeight;

	unsigned quality;
	fastJfifInfo_t jfifInfo;

	bool info;
	
public:
	SplitterEncoder(bool info) { 
		this->info = info;
		hImportFromHost = NULL;
		hBayerSplitter = NULL;
		hEncoder = NULL;

		jfifInfo.exifSections = NULL;
		jfifInfo.h_Bytestream = NULL;
	};
	~SplitterEncoder(void) { };

	fastStatus_t Init(DebayerJpegOptions &options);
	fastStatus_t Transform(std::list< Image<unsigned char, FastAllocator> > &inputImages);
	fastStatus_t Close();
};

#endif // __SPLITTER_ENCODER__
