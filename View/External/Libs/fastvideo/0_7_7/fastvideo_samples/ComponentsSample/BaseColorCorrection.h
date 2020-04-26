/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __BASE_COLOR_CORRECTION_H__
#define __BASE_COLOR_CORRECTION_H__

#include <memory>
#include <list>

#include "FastAllocator.h"
#include "fastvideo_sdk.h"

#include "Image.h"
#include "GrayscaleCorrectionOptions.h"

class BaseColorCorrection {
private:
	fastImageFiltersHandle_t hBaseColorCorrection;
	
	fastImportFromHostHandle_t hHostToDeviceAdapter;
	fastExportToHostHandle_t hDeviceToHostAdapter;

	fastDeviceSurfaceBufferHandle_t srcBuffer;
	fastDeviceSurfaceBufferHandle_t baseColorCorrectionBuffer;

	std::unique_ptr<unsigned char, FastAllocator> buffer;

	unsigned maxWidth;
	unsigned maxHeight;
	fastSurfaceFormat_t surfaceFmt;

	bool info;
	bool folder;

public:
	BaseColorCorrection(bool info) { this->info = info; };
	~BaseColorCorrection(void) {};

	fastStatus_t Init(GrayscaleCorrectionOptions &options);
	fastStatus_t Transform(std::list< Image<unsigned char, FastAllocator> > &image);
	fastStatus_t Close();
};

#endif // __BASE_COLOR_CORRECTION_H__
