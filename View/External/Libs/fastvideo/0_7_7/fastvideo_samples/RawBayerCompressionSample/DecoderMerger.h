/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __DECODER_MERGER__
#define __DECODER_MERGER__

#include <list>
#include <memory>

#include "FastAllocator.h"

#include "fastvideo_sdk.h"

#include "DebayerJpegOptions.h"
#include "Image.h"

class DecoderMerger
{
private:
	fastSurfaceFormat_t surfaceFmt;

	fastJpegDecoderHandle_t hDecoder;
	fastBayerMergerHandle_t hBayerMerger;
	fastDebayerHandle_t hDebayer;
	fastExportToHostHandle_t hExportToHost;

	fastDeviceSurfaceBufferHandle_t decoderBuffer;
	fastDeviceSurfaceBufferHandle_t bayerMergerBuffer;
	fastDeviceSurfaceBufferHandle_t debayerBuffer;

	unsigned maxSrcWidth;
	unsigned maxSrcHeight;

	fastJfifInfo_t jfifInfo;
	std::unique_ptr<unsigned char, FastAllocator> h_Result;

	bool info;
	
public:
	DecoderMerger(bool info) { 
		this->info = info;
		hBayerMerger = NULL;
		hDebayer = NULL;
		hDecoder = NULL;
		hExportToHost = NULL;

		jfifInfo.exifSections = NULL;
		jfifInfo.exifSectionsCount = 0;
		jfifInfo.h_Bytestream = NULL;
	};
	~DecoderMerger(void) { };

	fastStatus_t Init(DebayerJpegOptions &options, unsigned maxRestoredWidth, unsigned maxRestoredHeight);
	fastStatus_t Transform(std::list< Data<unsigned char, FastAllocator> > &inputImages);
	fastStatus_t Close();
};

#endif // __DECODER_MERGER__
