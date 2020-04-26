/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __DEBAYER_H__
#define __DEBAYER_H__

#include <list>
#include <memory>

#include "FastAllocator.h"

#include "fastvideo_sdk.h"
#include "Image.h"
#include "DebayerOptions.h"

class Debayer
{
private:
	fastDebayerHandle_t hDebayer;
	fastImageFiltersHandle_t hLut12to8;
	fastImageFiltersHandle_t hMad;
	fastImageFiltersHandle_t hColorCorrection;
	fastRawUnpackerHandle_t hRawUnpacker;

	fastImportFromHostHandle_t hHostToDeviceAdapter;

	fastExportToHostHandle_t hDeviceToHostAdapter;

	fastDeviceSurfaceBufferHandle_t srcBuffer;
	fastDeviceSurfaceBufferHandle_t lut12Buffer;
	fastDeviceSurfaceBufferHandle_t debayerBuffer;
	fastDeviceSurfaceBufferHandle_t madBuffer;
	fastDeviceSurfaceBufferHandle_t colorCorrectionBuffer;
	fastDeviceSurfaceBufferHandle_t dstBuffer;

	unsigned maxWidth;
	unsigned maxHeight;
	unsigned bitsPerPixel;
	unsigned bytesPerPixel;

	bool info;
	bool folder;

	std::unique_ptr<unsigned char, FastAllocator> h_Result;

	fastSurfaceFormat_t surfaceFmt;
	bool isRaw;
	bool is2bytesOutput;

public:
	Debayer(bool info)	{ this->info = info; hLut12to8 = NULL; hDebayer = NULL; hRawUnpacker = NULL; hMad = NULL; hColorCorrection = NULL; hHostToDeviceAdapter = NULL;  
	h_Result = NULL;};
	~Debayer(void) { };

	fastStatus_t Init(DebayerOptions &options, void *lut, float *matrixA, unsigned char *matrixB);
	fastStatus_t Transform(std::list< Image<unsigned char, FastAllocator> > &image);
	fastStatus_t Close();
};

#endif
