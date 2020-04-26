/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __FILES_HPP__
#define __FILES_HPP__

#include <list>
#include "Image.h"

#include "FastAllocator.h"

template< typename T, class Allocator >
fastStatus_t loadFile(
	std::string imageFileName,
	std::string outputFileName,
	Image<T, Allocator>	   &inputImg,

	unsigned	rawHeight,
	unsigned	rawWidth,
	unsigned	rawBits,

	bool info
);

template< typename T, class Allocator >
fastStatus_t loadImages(
	char *folderName,
	const char *outputPattern,
	std::list< Image<T, Allocator> > &inputImg,
	unsigned widthDefault,
	unsigned heightDefault,
	unsigned bitsDefault,
	bool info
);

template< typename T, class Allocator >
fastStatus_t saveToFile(
    char		  *imageFileName,
    std::unique_ptr<T, Allocator> &h_Surface,
    fastSurfaceFormat_t surfaceFmt,
    unsigned            surfaceHeight,
    unsigned            surfaceWidth,
    unsigned            surfacePitch8,
	bool info
);

bool checkFileExtension(const char *fileName, const char *etalon);
const char *surfaceAnalyze(fastSurfaceFormat_t surface);
const char *patternAnalyze(fastBayerPattern_t bayerPattern);
const char *samplingAnalyze(fastJpegFormat_t sampling);
const char *debayerTypeAnalyze(fastDebayerType_t type);

bool fileExist(const char *fileName);
int fileSize(const char *fname);

template< typename T, class Allocator >
fastStatus_t loadJpeg(std::string inputFileName, std::list< Data<T, Allocator> > &inputImg, bool info);
template< typename T, class Allocator >
fastStatus_t loadJpegImages(char *folderName, std::list< Data<T, Allocator> > &inputImg, bool info);
template< typename T, class Allocator >
fastStatus_t saveJpegImages(char *folderName, std::list< Data<T, Allocator> > &images, bool info);

std::string generateOutputFileName(const char *pattern, unsigned idx);

fastStatus_t getImageSize(const char *fileName, unsigned &width, unsigned &height);
fastStatus_t getFirstImageSize(const char *folderName, unsigned &width, unsigned &height);

#endif
