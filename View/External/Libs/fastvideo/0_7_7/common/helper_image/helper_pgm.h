/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef HELPER_PGM_H
#define HELPER_PGM_H

#include <memory>
#include <fstream>
#include <iostream>
#include <cmath>

#include "fastvideo_sdk.h"
#include "memory_interface.h"
#include "helper_common.h"
#include "string.h"

inline fastStatus_t
getFileParameters(const char *file, unsigned &width, unsigned &height) {
	FILE *fp = NULL;

	if (FOPEN_FAIL(FOPEN(fp, file, "rb")))
		return FAST_IO_ERROR;

	unsigned int startPosition = 0;
	unsigned channels;

	// check header
	char header[PGMHeaderSize] = { 0 };

	while( header[startPosition] == 0 ) {
		startPosition = 0;
		if (fgets(header, PGMHeaderSize, fp) == NULL)
			return FAST_IO_ERROR;

		while(isspace(header[startPosition])) startPosition++;
	}

	if (strncmp(&header[startPosition], "P5", 2) == 0) {
		channels = 1;
	} else if (strncmp(&header[startPosition], "P6", 2) == 0) {
		channels = 3;
	} else {
		channels = 0;
		return FAST_IO_ERROR;
	}

	// parse header, read maxval, width and height
	unsigned int maxval = 0;
	unsigned int i = 0;
	unsigned int readsCount = 0;
	
	if ( ( i = SSCANF(&header[startPosition + 2], "%u %u %u", &width, &height, &maxval) ) == EOF )
		i = 0;

	while (i < 3) {
		if (fgets(header, PGMHeaderSize, fp) == NULL)
			return FAST_IO_ERROR;

		if (header[0] == '#')
			continue;

		if (i == 0) {
			if ( ( readsCount = SSCANF(header, "%u %u %u", &width, &height, &maxval) ) != EOF )
				i += readsCount;
		} else if (i == 1) {
			if ( ( readsCount = SSCANF(header, "%u %u", &height, &maxval) ) != (int)EOF )
				i += readsCount;
		} else if (i == 2) {
			if ( ( readsCount = SSCANF(header, "%u", &maxval) ) != (int)EOF )
				i += readsCount;
		}
	}

	fclose(fp);

	return FAST_OK;
}

template<typename T, class Allocator>
inline fastStatus_t
loadPPM(const char *file, std::unique_ptr<T, Allocator> &data, unsigned int &width, unsigned &wPitch, unsigned pitchAlignment, unsigned int &height, int &bitsPerPixel, unsigned &channels) {
	FILE *fp = NULL;

	if (FOPEN_FAIL(FOPEN(fp, file, "rb")))
		return FAST_IO_ERROR;

	unsigned int startPosition = 0;

	// check header
	char header[PGMHeaderSize] = { 0 };

	while( header[startPosition] == 0 ) {
		startPosition = 0;
		if (fgets(header, PGMHeaderSize, fp) == NULL)
			return FAST_IO_ERROR;

		while(isspace(header[startPosition])) startPosition++;
	}

	int strOffset = 2;
	if (strncmp(&header[startPosition], "P5", 2) == 0) {
		channels = 1;
	} else if (strncmp(&header[startPosition], "P6", 2) == 0) {
		channels = 3;
	} else if ( strncmp(&header[startPosition], "P15", 3) == 0 ) {
		channels = 1;
		strOffset = 3;
	} else if ( strncmp(&header[startPosition], "P16", 3) == 0 ) {
		channels = 3;
		strOffset = 3;
	} else {
		channels = 0;
		return FAST_IO_ERROR;
	}

	// parse header, read maxval, width and height
	unsigned int maxval = 0;
	unsigned int i = 0;
	int readsCount = 0;
	
	if ( ( i = SSCANF(&header[startPosition + strOffset], "%u %u %u", &width, &height, &maxval) ) == EOF )
		i = 0;

	while (i < 3) {
		if (fgets(header, PGMHeaderSize, fp) == NULL)
			return FAST_IO_ERROR;

		if (header[0] == '#')
			continue;

		if (i == 0) {
			if ( ( readsCount = SSCANF(header, "%u %u %u", &width, &height, &maxval) ) != EOF )
				i += readsCount;
		} else if (i == 1) {
			if ( ( readsCount = SSCANF(header, "%u %u", &height, &maxval) ) != EOF )
				i += readsCount;
		} else if (i == 2) {
			if ( ( readsCount = SSCANF(header, "%u", &maxval) ) != EOF )
				i += readsCount;
		}
	}
	bitsPerPixel = int(log(maxval + 1) / log(2));
	const int intPerPixel = _uSnapUp<unsigned>(bitsPerPixel, 8) / 8;

	wPitch = channels * _uSnapUp<unsigned>(width * intPerPixel, pitchAlignment);

	CHECK_FAST_ALLOCATION( data.reset( (T *)Allocator::template allocate<T>(wPitch * height) ) );

	for (unsigned i = 0; i < height; i++) {
		if (fread(&data.get()[i * wPitch], sizeof(unsigned char), width * intPerPixel * channels, fp) == 0)
			return FAST_IO_ERROR;
	}
	
	fclose(fp);
	return FAST_OK;
}

template <class T, class Allocator>
inline fastStatus_t
__loadPPM(const char *file, std::unique_ptr<T, Allocator> &data, unsigned int &w, unsigned int &wPitch, unsigned pitchAlignment, unsigned int &h, fastSurfaceFormat_t &surfaceFmt) {
	fastStatus_t ret;
	unsigned int channels;
	int bitsPerPixel;
	CHECK_FAST( loadPPM(file, data, w, wPitch, pitchAlignment, h, bitsPerPixel, channels) );
	surfaceFmt = channels == 3 ? FAST_RGB8 : FAST_I8;
    return FAST_OK;
}

template <class T, class Allocator>
inline fastStatus_t
__loadPGM(const char *file, std::unique_ptr<T, Allocator> &data, unsigned int &w, unsigned int &wPitch, unsigned pitchAlignment, unsigned int &h, fastSurfaceFormat_t &surfaceFmt) {
	unsigned int channels;

	fastStatus_t ret;
	if ( sizeof(T) == sizeof(unsigned char) ) {
		int bitsPerPixel;
		CHECK_FAST( loadPPM(file, data, w, wPitch, pitchAlignment, h, bitsPerPixel, channels) );
	} else {
		return FAST_IO_ERROR;
	}
	surfaceFmt = channels == 3 ? FAST_RGB8 : FAST_I8;
	
	return FAST_OK;
}

inline fastStatus_t
savePPM(const char *file, unsigned char *data, const unsigned w, const unsigned wPitch, const unsigned h, const int bitsPerPixel, const unsigned int channels) {
	assert(NULL != data);
	assert(w > 0);
	assert(h > 0);

	std::fstream fh(file, std::fstream::out | std::fstream::binary);
	if (fh.bad())
		return FAST_IO_ERROR;

	if (channels == 1) {
		if ( bitsPerPixel == 8 ) {
		fh << "P5\n";
		} else {
			fh << "P15\n";
		}
	} else if (channels == 3) {
		if ( bitsPerPixel == 8 ) {
		fh << "P6\n";
		} else {
			fh << "P16\n";
		}
	} else
		return FAST_IO_ERROR;

	fh << w << "\n" << h << "\n" << ( ( 1 << bitsPerPixel ) - 1 ) << std::endl;
	const int intPerPixel = _uSnapUp<unsigned>(bitsPerPixel, 8) / 8;

	for (unsigned int y = 0; y < h && fh.good(); y++)
		fh.write(reinterpret_cast<const char *>(&data[y * wPitch]), w * channels * intPerPixel);

	fh.flush();
	if (fh.bad())
		return FAST_IO_ERROR;

	fh.close();
	return FAST_OK;
}

template <class T, class Allocator>
inline fastStatus_t
__savePGM(const char *file, std::unique_ptr<T, Allocator> &data, unsigned int w, unsigned wPitch, unsigned int h) {
	if ( sizeof(T) == sizeof(unsigned char) ) {
		return savePPM(file, (unsigned char *)data.get(), w, wPitch, h, 8, 1);
	}

	return FAST_IO_ERROR;
}

template <class T, class Allocator>
inline fastStatus_t
__savePPM(const char *file, std::unique_ptr<T, Allocator> &data, unsigned int w, unsigned wPitch, unsigned int h, unsigned int channels) {
	fastStatus_t ret;
	if ( sizeof(T) == sizeof(float) ) {
		std::unique_ptr<unsigned char, Allocator> idata;

		unsigned int size = wPitch * h;
		CHECK_FAST_ALLOCATION( idata.reset( (unsigned char *)Allocator::template allocate<unsigned char>(size) ) );

		std::transform(data.get(), data.get() + size, idata.get(), ConverterToUByte<T>());

		CHECK_FAST( savePPM(file, idata.get(), w, wPitch, h, 8, channels) );
	} else {
		CHECK_FAST( savePPM(file, (unsigned char *)data.get(), w, wPitch, h, 8, channels) );
	}
	return FAST_OK;
}

template <class T, class Allocator>
inline fastStatus_t
__loadPPM16(const char *file, std::unique_ptr<T, Allocator> &data, unsigned int &w, unsigned int &wPitch, unsigned pitchAlignment, unsigned int &h, fastSurfaceFormat_t &surfaceFmt) {
	fastStatus_t ret;
	unsigned int channels;

	if ( sizeof(T) == sizeof(unsigned char) ) {
		int bitsPerPixel;
		CHECK_FAST( loadPPM(file, data, w, wPitch, pitchAlignment, h, bitsPerPixel, channels) );
		if ( bitsPerPixel == 12 ) {
			surfaceFmt = channels == 3 ? FAST_RGB12 : FAST_I12;
		} else if ( bitsPerPixel == 16 ) {
			surfaceFmt = channels == 3 ? FAST_RGB16 : FAST_I16;
		} else if ( bitsPerPixel == 8 ) {
			surfaceFmt = channels == 3 ? FAST_RGB8 : FAST_I8;
		}
	} else {
		return FAST_IO_ERROR;
	}

    return FAST_OK;
	}

template <class T, class Allocator>
inline fastStatus_t
__loadPGM16(const char *file, std::unique_ptr<T, Allocator> &data, unsigned int &width, unsigned int &wPitch, unsigned pitchAlignment, unsigned int &height, fastSurfaceFormat_t &surfaceFmt) {
	fastStatus_t ret;
	unsigned int channels;

	if ( sizeof(T) == sizeof(unsigned char) ) {
		int bitsPerPixel = 0;
		CHECK_FAST( loadPPM(file, data, width, wPitch, pitchAlignment, height, bitsPerPixel, channels) );
		if ( bitsPerPixel == 12 ) {
			surfaceFmt = channels == 3 ? FAST_RGB12 : FAST_I12;
		} else if ( bitsPerPixel == 16 ) {
			surfaceFmt = channels == 3 ? FAST_RGB16 : FAST_I16;
		} else if ( bitsPerPixel == 8 ) {
			surfaceFmt = channels == 3 ? FAST_RGB8 : FAST_I8;
		}
	} else {
		return FAST_IO_ERROR;
	}

	return FAST_OK;
}

template <class T, class Allocator>
inline fastStatus_t
__savePGM16(const char *file, std::unique_ptr<T, Allocator> &data, fastSurfaceFormat_t surfaceFmt, unsigned int w, unsigned wPitch, unsigned int h) {
	fastStatus_t ret;
	if ( surfaceFmt == FAST_I12 || surfaceFmt == FAST_RGB12 ) {
		CHECK_FAST( savePPM(file, (unsigned char *)data.get(), w, wPitch, h, 12, 1) );
	} else if ( surfaceFmt == FAST_I16 || surfaceFmt == FAST_RGB16 ) {
		CHECK_FAST( savePPM(file, (unsigned char *)data.get(), w, wPitch, h, 16, 1) );
	}
	return ret;
}

template <class T, class Allocator>
inline fastStatus_t
__savePPM16(const char *file, std::unique_ptr<T, Allocator> &data, fastSurfaceFormat_t surfaceFmt, unsigned int w, unsigned wPitch, unsigned int h, unsigned int channels) {
	fastStatus_t ret;
	if ( surfaceFmt == FAST_I12 || surfaceFmt == FAST_RGB12 ) {
		CHECK_FAST( savePPM(file, (unsigned char *)data.get(), w, wPitch, h, 12, channels) );
	} else if ( surfaceFmt == FAST_I16 || surfaceFmt == FAST_RGB16 ) {
		CHECK_FAST( savePPM(file, (unsigned char *)data.get(), w, wPitch, h, 16, channels) );
	}
	return ret;
}


#endif
