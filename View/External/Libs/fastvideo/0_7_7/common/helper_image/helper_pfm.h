/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef HELPER_PFM_H
#define HELPER_PFM_H

#include <fstream>
#include <iostream>

#include "fastvideo_sdk.h"
#include "memory_interface.h"
#include "helper_common.h"

template<class Allocator>
inline fastStatus_t
__loadPitchPFM(const char *file, std::unique_ptr<float, Allocator> &data,
		unsigned int &width, unsigned int &widthPitch, unsigned int pitchAlignment,
		unsigned int &height, unsigned &channels
) {
	FILE *fp = fopen(file, "rb");
	
	if ( fp == NULL ) {
		return FAST_IO_ERROR;
	}

	// check header
	char header[2] = { 0 };
	if (   fread(header, sizeof(char), 2, fp) < 2 ||
		 ( header[0] != 'P' && header[1] != '7' ) ) {
		return FAST_IO_ERROR;
	}

	unsigned int fileParameters[3];
	if ( fread(fileParameters, sizeof(unsigned int), 3, fp) < 3 ) {
		return FAST_IO_ERROR;
	}

	width = fileParameters[0];
	height = fileParameters[1];
	channels = fileParameters[2];
	widthPitch = _uSnapUp<unsigned>(width * channels, pitchAlignment);

	CHECK_FAST_ALLOCATION( data.reset( (float *)Allocator::template allocate<float>(height * widthPitch) ) );
	//CHECK_FAST_ALLOCATION( fastMalloc((void **)data, height * widthPitch * sizeof(float)) );
	
	// read and close file
	for (unsigned y = 0; y < height; y++) {
		if ( fread(&data.get()[y * widthPitch], sizeof(float), width * channels, fp) == 0 ) {
			return FAST_IO_ERROR;
		}
	}

	fclose(fp);
	return FAST_OK;
}

inline fastStatus_t
__savePitchPFM(const char *file, float *data, unsigned int width, unsigned int widthPitch, unsigned int height, unsigned int channels) {
	FILE *fp = fopen(file, "wb+");

	if ( fp == NULL ) {
		return FAST_IO_ERROR;
	}

	// check header
	char header[3] = "P7";
	if ( fwrite(header, sizeof(char), 2, fp) < 2 ) {
		return FAST_IO_ERROR;
	}

	fwrite(&width, sizeof(unsigned int), 1, fp);
	fwrite(&height, sizeof(unsigned int), 1, fp);
	fwrite(&channels, sizeof(unsigned int), 1, fp);
	
	// read and close file
	for (unsigned y = 0; y < height; y++) {
		if ( fwrite(&data[y * widthPitch], sizeof(float), width * channels, fp) == 0 ) {
			return FAST_IO_ERROR;
		}
	}

	fclose(fp);
	return FAST_OK;
}

#endif
