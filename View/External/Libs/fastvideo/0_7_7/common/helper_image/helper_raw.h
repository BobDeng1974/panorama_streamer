/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef HELPER_RAW_H
#define HELPER_RAW_H

#include <fstream>
#include <iostream>

#include "fastvideo_sdk.h"
#include "memory_interface.h"
#include "helper_common.h"

template <class T, class Allocator>
inline fastStatus_t
__loadRaw(const char *file, std::unique_ptr<T, Allocator> &data,
		   unsigned int width, unsigned int height, unsigned int channels,
		   unsigned bitsPerPixel) {
	if ( sizeof(T) != sizeof(unsigned char) ) {
		return FAST_IO_ERROR;
	}

	FILE *fp = NULL;

    if (FOPEN_FAIL(FOPEN(fp, file, "rb")))
        return FAST_IO_ERROR;

	const unsigned sizeInBits = width * height * channels * bitsPerPixel * sizeof(unsigned char);
	const unsigned sizeInBytes = _uSnapUp<unsigned>(sizeInBits, 8) / 8;

	//fastStatus_t ret;
	//CHECK_FAST_ALLOCATION( fastMalloc((void **)data, sizeInBytes) );
	CHECK_FAST_ALLOCATION( data.reset( (T *)Allocator::template allocate<unsigned char>(sizeInBytes) ) );

	if ( fread(data.get(), sizeof(unsigned char), sizeInBytes, fp) < sizeInBytes ) {
		return FAST_IO_ERROR;
	}

	fclose(fp);
	return FAST_OK;
}

template <class T>
inline fastStatus_t
__saveRaw(const char *file, T *data,
		   unsigned int width,  unsigned int height, unsigned int channels,
		   unsigned bitsPerPixel) {
	if ( sizeof(T) != sizeof(unsigned char) ) {
		return FAST_IO_ERROR;
	}

	FILE *fp = NULL;

    if (FOPEN_FAIL(FOPEN(fp, file, "wb+")))
        return FAST_IO_ERROR;

	const unsigned sizeInBits = width * channels * bitsPerPixel * sizeof(unsigned char);
	const unsigned sizeInBytes = _uSnapUp<unsigned>(sizeInBits, 8) / 8;

	for (int i = 0; i < height; i++) {
		if ( fwrite(&data[i * sizeInBytes], sizeof(unsigned char), sizeInBytes, fp) < sizeInBytes ) {
			return FAST_IO_ERROR;
		}
	}

	fclose(fp);
	return FAST_OK;
}

#endif
