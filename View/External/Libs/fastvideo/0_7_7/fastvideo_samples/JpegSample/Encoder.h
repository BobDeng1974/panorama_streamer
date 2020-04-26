/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <list>
#include <memory>

#include "FastAllocator.h"

#include "fastvideo_sdk.h"
#include "Image.h"
#include "EncoderOptions.h"

template< class T>
class Encoder
{
private:
	fastJpegEncoderHandle_t hEncoder;
	fastImportFromHostHandle_t hImportFromHost;
	
	fastDeviceSurfaceBufferHandle_t srcBuffer;

	fastJfifInfo_t jfifInfo;
	int Quality;

	unsigned maxWidth;
	unsigned maxHeight;

	bool info;
	bool folder;

public:
	Encoder(bool info) { this->info = info; hEncoder = NULL; hImportFromHost = NULL; jfifInfo.h_Bytestream = NULL; this->jfifInfo.exifSections = NULL; this->jfifInfo.exifSectionsCount = 0; };
	~Encoder(void) { };

	fastStatus_t Init(EncoderOptions &options);
	fastStatus_t Encode(std::list< Image<T, FastAllocator> > &inputImg);
	fastStatus_t SetMatrix(float *matrixA, unsigned char *matrixB);
	fastStatus_t Close();
};

#endif
