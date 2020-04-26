/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include <stdio.h>
#include <string>

#ifndef __GNUC__
#include <io.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#endif

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "bmp.hpp"
#include "helper_pgm.h"
#include "helper_raw.h"
#include "supported_files.hpp"
#include "memory_interface.h"
#include "timing.hpp"

#include "FastAllocator.h"

bool checkFileExtension(const char *fileName, const char *etalon)
{
	bool success = true;
	int startPosition = strlen(fileName);
	while (fileName[startPosition] != '.' && startPosition > 0) startPosition--;

	for (unsigned i = 0; i < strlen(etalon)+1; i++)
		if (tolower(fileName[startPosition + i]) != etalon[i]) {
			success = false;
			break;
		}
	return success;
}

const char *surfaceAnalyze(fastSurfaceFormat_t surface) {
	if (surface == FAST_RGB8) {
		return "RGB";
	}
	else if (surface == FAST_BGR8) {
		return "BGR";
	}
	else if (surface == FAST_RGB16) {
		return "RGB (16 bits per pixel)";
	}
	else if (surface == FAST_CrCbY8) {
		return "CrCbY";
	}
	else if (surface == FAST_YCbCr8) {
		return "YCbCr";
	}
	else if  ( surface == FAST_I8 ) { 
		return "grayscale";
	}
	
	return "unknown";
}

const char *patternAnalyze(fastBayerPattern_t bayerPattern) {
	if (bayerPattern == FAST_BAYER_NONE) {
		return "grayscale";
	}
	else if (bayerPattern == FAST_BAYER_RGGB) {
		return "RGGB";
	}
	else if (bayerPattern == FAST_BAYER_BGGR) {
		return "BGGR";
	}
	else if (bayerPattern == FAST_BAYER_GBRG) {
		return "GBRG";
	}
	else if  ( bayerPattern == FAST_BAYER_GRBG ) { 
		return "GRBG";
	}

	return "unknown";
}

const char *debayerTypeAnalyze(fastDebayerType_t type) {
	if ( type == FAST_HQLI )
		return "HQLI";

	if ( type == FAST_DFPD )
		return "DFPD";

	return "unknown";
}

const char *samplingAnalyze(fastJpegFormat_t sampling) {
	if (sampling == JPEG_444) {
		return "YCbCr444";
	}
	else if (sampling == JPEG_422) {
		return "YCbCr422";
	}
	else if (sampling == JPEG_420) {
		return "YCbCr420";
	}
	else if (sampling == JPEG_Y) {
		return "grayscale";
	}

	return "unknown";
}

bool fileExist(const char *fileName)
{
#ifndef __GNUC__
	if (_access(fileName, 4) != -1)
		return true;
	return false;
#else
	struct stat buffer;
	return (stat (fileName, &buffer) == 0);
#endif
}

int fileSize(const char *fname) {
	std::ifstream in(fname, std::ifstream::in | std::ifstream::binary);
	in.seekg(0, std::ifstream::end);
	return (int )in.tellg();
}

std::string generateOutputFileName(const char *pattern, unsigned idx) {
	std::string outputFileName(pattern);
	std::string::size_type loc = outputFileName.find( "*", 0 );
	if( loc != std::string::npos ) {
		std::ostringstream convert;
		convert << idx;
		outputFileName.replace(loc, 1, convert.str());
	}
	return outputFileName;
}

template< typename T, class Allocator >
fastStatus_t loadFile(
	std::string imageFileName,
	std::string outputFileName,
	Image<T, Allocator>	   &inputImg,

	unsigned	rawHeight,
	unsigned	rawWidth,
	unsigned	rawBits,

	bool info
) {
	if (!fileExist( imageFileName.c_str() ) ) {
		fprintf(stderr, "Can not open input file\n");
		return FAST_IO_ERROR;
	}

	inputImg.inputFileName = imageFileName;
	inputImg.outputFileName = outputFileName;

	hostTimer_t timer = NULL;
	if (info) {
		timer = hostTimerCreate();
		hostTimerStart(timer);
	}

	fastStatus_t ret;
	if (checkFileExtension(imageFileName.c_str(), ".bmp") || checkFileExtension(imageFileName.c_str(), ".dib")) {
		inputImg.bitsPerPixel = 8;
		CHECK_FAST_OPEN_FILE( LoadBMP(inputImg.data, inputImg.surfaceFmt, inputImg.h, inputImg.w, inputImg.wPitch, imageFileName.c_str()) );
	}
	else if (checkFileExtension(imageFileName.c_str(), ".pgm")) {
		inputImg.bitsPerPixel = 8;
		CHECK_FAST_OPEN_FILE( __loadPGM( imageFileName.c_str(), inputImg.data, inputImg.w, inputImg.wPitch, FAST_ALIGNMENT, inputImg.h, inputImg.surfaceFmt ) );
	}
	else if (checkFileExtension(imageFileName.c_str(), ".pgm16")) {
		CHECK_FAST_OPEN_FILE( __loadPGM16( imageFileName.c_str(), inputImg.data, inputImg.w, inputImg.wPitch, FAST_ALIGNMENT, inputImg.h, inputImg.surfaceFmt ) );
		inputImg.bitsPerPixel = inputImg.surfaceFmt == FAST_I8 ? 8 : ( inputImg.surfaceFmt == FAST_I12 ? 12 : 16 );
	}
	else if (checkFileExtension(imageFileName.c_str(), ".ppm")) {
		inputImg.bitsPerPixel = 8;
		CHECK_FAST_OPEN_FILE( __loadPPM( imageFileName.c_str(), inputImg.data, inputImg.w, inputImg.wPitch, FAST_ALIGNMENT, inputImg.h, inputImg.surfaceFmt ) );
	}
	else if (checkFileExtension(imageFileName.c_str(), ".ppm16")) {
		CHECK_FAST_OPEN_FILE( __loadPPM16( imageFileName.c_str(), inputImg.data, inputImg.w, inputImg.wPitch, FAST_ALIGNMENT, inputImg.h, inputImg.surfaceFmt ) );
		if ( inputImg.surfaceFmt == FAST_RGB12 || inputImg.surfaceFmt == FAST_I12 ) {
			inputImg.bitsPerPixel = 12;
		} else if ( inputImg.surfaceFmt == FAST_RGB16 || inputImg.surfaceFmt == FAST_I16 ) {
			inputImg.bitsPerPixel = 16;
		}
	}
	else if (checkFileExtension(imageFileName.c_str(), ".raw")) {
		inputImg.w = rawWidth;
		inputImg.wPitch = ( rawBits * rawWidth ) / 8;
		inputImg.h = rawHeight;
		inputImg.bitsPerPixel = rawBits;
		inputImg.surfaceFmt = rawBits == 12 ? FAST_I12 : ( rawBits == 16 ? FAST_I16 : FAST_I8 ) ;
		inputImg.isRaw = true;
		CHECK_FAST_OPEN_FILE( __loadRaw(imageFileName.c_str(), inputImg.data, inputImg.w, inputImg.h, 1, inputImg.bitsPerPixel) );
	} else {
		if (info) hostTimerDestroy(timer);
		fprintf(stderr, "Input file has unappropriate extension\n");
		return FAST_UNSUPPORTED_FORMAT;
	}

	if (info) {
		double totalTime = hostTimerEnd(timer);
		printf("File read time: %.2f ms\n\n", totalTime * 1000.0);
		hostTimerDestroy(timer);
	}

	return FAST_OK;
}

template fastStatus_t loadFile< unsigned char, FastAllocator >(
	std::string imageFileName,
	std::string outputFileName,
	Image<unsigned char, FastAllocator > &inputImg,

	unsigned	rawHeight,
	unsigned	rawWidth,
	unsigned	rawBits,

	bool info
);

template fastStatus_t loadFile< float, FastAllocator >(
	std::string imageFileName,
	std::string outputFileName,
	Image<float, FastAllocator > &inputImg,

	unsigned	rawHeight,
	unsigned	rawWidth,
	unsigned	rawBits,

	bool info
);

template< typename T, class Allocator >
fastStatus_t saveToFile(
    char    *imageFileName,
    std::unique_ptr<T, Allocator> &h_Surface,
    fastSurfaceFormat_t surfaceFmt,
    unsigned            surfaceHeight,
    unsigned            surfaceWidth,
    unsigned            surfacePitch8,
	bool info
) {
	bool isPgmSupported = surfaceFmt == FAST_I8;
	bool isBmp = checkFileExtension(imageFileName, ".bmp");
	bool isPgm = checkFileExtension(imageFileName, ".pgm");
	bool isPpm = checkFileExtension(imageFileName, ".ppm");
	bool isPpm16 = checkFileExtension(imageFileName, ".ppm16");
	bool isPgm16 = checkFileExtension(imageFileName, ".pgm16");

	hostTimer_t timer = NULL;
	double totalTime = 0.;
	if (info) timer = hostTimerCreate();

	fastStatus_t ret;
	if (isBmp) {
		if (info) hostTimerStart(timer);
		CHECK_FAST_SAVE_FILE( StoreBMP(imageFileName, h_Surface.get(), surfaceFmt, surfaceHeight, surfaceWidth, surfacePitch8) );
		if (info) {
			totalTime = hostTimerEnd(timer);
		}
	}
	else if ((isPgm || isPpm) && isPgmSupported) {
		if (isPpm) {
			fprintf(stderr, "Unappropriate output file format. Set to PGM\n");
			int pos = strlen(imageFileName);
			while (imageFileName[pos] != 'p') pos--;
			imageFileName[pos] = 'g';
		}
		if (info) hostTimerStart(timer);
		CHECK_FAST_SAVE_FILE( __savePGM(imageFileName, h_Surface, surfaceWidth, surfacePitch8, surfaceHeight) );
		if (info) {
			totalTime = hostTimerEnd(timer);
		}
	}
	else if ((isPgm || isPpm) && !isPgmSupported) {
		if (isPgm) {
			fprintf(stderr, "Unappropriate output file format. Set to PPM\n");
			int pos = strlen(imageFileName);
			while (imageFileName[pos] != 'g') pos--;
			imageFileName[pos] = 'p';
		}
		if (info) hostTimerStart(timer);
		CHECK_FAST_SAVE_FILE( __savePPM( imageFileName, h_Surface, surfaceWidth, surfacePitch8, surfaceHeight, surfaceFmt != FAST_I8 ? 3 : 1 ) );
		if (info) {
			totalTime = hostTimerEnd(timer);
		}
	} else if (isPpm16) {
		if (info) hostTimerStart(timer);

		CHECK_FAST_SAVE_FILE( __savePPM16( imageFileName, h_Surface, surfaceFmt, surfaceWidth, surfacePitch8, surfaceHeight, 3) );
		
		if (info) {
			totalTime = hostTimerEnd(timer);
		}
	} else if (isPgm16) {
		if (info) hostTimerStart(timer);

		CHECK_FAST_SAVE_FILE( __savePGM16( imageFileName, h_Surface, surfaceFmt, surfaceWidth, surfacePitch8, surfaceHeight) );
		
		if (info) {
			totalTime = hostTimerEnd(timer);
		}
	} else {
		fprintf(stderr, "Input file has unappropriate extension\n");
		return FAST_UNSUPPORTED_FORMAT;
	}
	if (info) {
		printf("File write time: %.2f ms\n\n", totalTime * 1000.0);
		hostTimerDestroy(timer);
	}
	return FAST_OK;
}

template fastStatus_t saveToFile< unsigned char, FastAllocator >(
    char    *imageFileName,
    std::unique_ptr< unsigned char, FastAllocator > &h_Surface,
    fastSurfaceFormat_t surfaceFmt,
    unsigned            surfaceHeight,
    unsigned            surfaceWidth,
    unsigned            surfacePitch8,
	bool info
);

template fastStatus_t saveToFile< float, FastAllocator >(
    char    *imageFileName,
    std::unique_ptr< float, FastAllocator > &h_Surface,
    fastSurfaceFormat_t surfaceFmt,
    unsigned            surfaceHeight,
    unsigned            surfaceWidth,
    unsigned            surfacePitch8,
	bool info
);

template< typename T, class Allocator >
fastStatus_t loadJpeg(std::string inputFileName, std::list< Data<T, Allocator> > &inputImg, bool info) {
	hostTimer_t timer = NULL;
	double loadTime = 0.;

	std::ifstream input(inputFileName.c_str(), std::ifstream::binary);

	if ( input.is_open() ) {
		input.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);

		timer = hostTimerCreate();
		hostTimerStart(timer);

		input.seekg(0, input.end);
		std::streamoff length = input.tellg();
		input.seekg(0, input.beg);

		Data<T, Allocator> data;
		data.size = (int )length;
		data.inputFileName = inputFileName;
		data.encoded = false;

		CHECK_FAST_ALLOCATION( data.data.reset( (T *)Allocator::template allocate<T>((int )length) ) );
		//CHECK_FAST( fastMalloc((void **)&(data.data), (int )length) );

		input.read((char *)data.data.get(), length);
		input.close();

		loadTime = hostTimerEnd(timer);
		hostTimerDestroy(timer);
		data.loadTimeMs = loadTime * 1000.0;
		if (info) {
			printf("JFIF images read time = %.2f ms\n\n", loadTime * 1000.0);
		}
		inputImg.push_back(data);
	} else {
		return FAST_IO_ERROR;
	}

	return FAST_OK;
}

template fastStatus_t loadJpeg<unsigned char, FastAllocator>(std::string inputFileName, std::list< Data<unsigned char, FastAllocator> > &inputImg, bool info);

fastStatus_t getFileList(const char *folderName, std::list<std::string> &fileList) {
	fileList.clear();

	std::string strFolderPath(folderName);
	std::replace( strFolderPath.begin(), strFolderPath.end(), '\\', '/');
	unsigned pos = strFolderPath.rfind("/");
	if ( pos != std::string::npos ) {
		strFolderPath = strFolderPath.substr(0, pos + 1);
	}
	else
		strFolderPath.erase();

#ifndef __GNUC__
	std::string strPattern(folderName);
	WIN32_FIND_DATA FileInformation;			// File information

	HANDLE hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if( hFile != INVALID_HANDLE_VALUE ) {
		do {
			if( FileInformation.cFileName[0] != '.' ) {
				if( ! ( FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {
					fileList.push_back( strFolderPath + "/" + FileInformation.cFileName );
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		::FindClose(hFile);
	}

#else
	std::string inputExtension(folderName);
	pos = inputExtension.rfind(".");
	if ( pos != std::string::npos ) {
		inputExtension = inputExtension.substr(pos);
	} else {
		return FAST_OK;
	}

	struct dirent *dir;
	DIR *d = opendir(strFolderPath.c_str());

	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if ( dir->d_type == DT_REG ) {
				std::string fileExtension(dir->d_name);
				pos = fileExtension.rfind(".");
				if ( pos != std::string::npos ) {
					fileExtension = fileExtension.substr(pos);
				}
				if ( inputExtension == fileExtension ) {
					fileList.push_back( strFolderPath + "/" + std::string(dir->d_name) );
				}
			}
		}
		closedir(d);
	}
#endif

	return FAST_OK;
}

template< typename T, class Allocator >
fastStatus_t loadJpegImages(char *folderName, std::list< Data<T, Allocator> > &inputImg, bool info) {
	std::list<std::string> files;

	hostTimer_t timer = NULL;
	double loadTime = 0.;

	fastStatus_t ret;
	CHECK_FAST( getFileList( folderName, files ) );
	if ( files.size() == 0 ) {
		fprintf(stderr, "No input files found\n");
		return FAST_IO_ERROR;
	}

	if (info) {
		timer = hostTimerCreate();
		hostTimerStart(timer);
	}
	
	for (std::list<std::string>::iterator i = files.begin(); i != files.end(); i++) {
		loadJpeg(*i, inputImg, false);
	}

	if (info) {
		loadTime = hostTimerEnd(timer);
		printf("JFIF images read time = %.2f ms\n\n", loadTime * 1000.0);
		hostTimerDestroy(timer);
		
	}

	return FAST_OK;
}

template fastStatus_t loadJpegImages<unsigned char, FastAllocator> (char *folderName, std::list< Data<unsigned char, FastAllocator> > &inputImg, bool info);

template< typename T, class Allocator >
fastStatus_t saveJpegImages(char *folderName, std::list< Data<T, Allocator> > images, bool info) {
	hostTimer_t timer = NULL;
	double loadTime = 0.;

	if (info) {
		timer = hostTimerCreate();
		hostTimerStart(timer);
	}

	int idx = 0;
	for (typename std::list< Data<T, Allocator> >::iterator i = images.begin(); i != images.end(); i++) {
		std::string str(folderName);
		if ( images.size() != 1 ) {
			std::string::size_type loc = str.find( "*", 0 );
			if( loc != std::string::npos ) {
				std::ostringstream convert;
				convert << idx;
				str.replace(loc, 1, convert.str());
			}
		}

		std::ofstream output(str.c_str(), std::ofstream::binary);

		if ( output.is_open() ) {
			output.write((char *)(*i).data, (*i).size);
			output.close();
			idx++;
		}
	}

	if (info) {
		loadTime = hostTimerEnd(timer);
		printf("JFIF images write time = %.2f ms\n\n", loadTime * 1000.0);
		hostTimerDestroy(timer);
	}

	return FAST_OK;
}

template< typename T, class Allocator >
fastStatus_t loadImages(
	char *folderName,
	const char *outputPattern,
	std::list< Image<T, Allocator> > &inputImg,
	unsigned widthDefault,
	unsigned heightDefault,
	unsigned bitsDefault,
	bool info
) {
	std::list<std::string> files;
	hostTimer_t timer = NULL;
	double loadTime = 0.;

	fastStatus_t ret;
	CHECK_FAST( getFileList( folderName, files ) );

	if ( files.size() == 0 ) {
		fprintf(stderr, "No input files found\n");
		return FAST_IO_ERROR;
	}

	if (info) {
		timer = hostTimerCreate();
		hostTimerStart(timer);
	}
	
	unsigned idx = 0;
	for (std::list<std::string>::iterator i = files.begin(); i != files.end(); i++) {
		Image<T, Allocator> img;
		CHECK_FAST( loadFile( *i, generateOutputFileName(outputPattern, idx), img, heightDefault, widthDefault, bitsDefault, info ) );
		inputImg.push_back(img);
		idx++;
	}

	if (info) {
		loadTime = hostTimerEnd(timer);
		printf("PGM images read time = %.2f ms\n\n", loadTime * 1000.0);
		hostTimerDestroy(timer);
	}

	files.clear();
	return FAST_OK;
}

template fastStatus_t loadImages< unsigned char, FastAllocator >(
	char *folderName,
	const char *outputPattern,
	std::list< Image< unsigned char, FastAllocator > > &inputImg,
	unsigned widthDefault,
	unsigned heightDefault,
	unsigned bitsDefault,
	bool info
);

template fastStatus_t loadImages< float, FastAllocator >(
	char *folderName,
	const char *outputPattern,
	std::list< Image< float, FastAllocator > > &inputImg,
	unsigned widthDefault,
	unsigned heightDefault,
	unsigned bitsDefault,
	bool info
);

fastStatus_t getImageSize(const char *fileName, unsigned &width, unsigned &height) {
	fastStatus_t ret;
	CHECK_FAST( getFileParameters(fileName, width, height) );
	return FAST_OK;
}

fastStatus_t getFirstImageSize(const char *folderName, unsigned &width, unsigned &height) {
	std::list<std::string> files;
	fastStatus_t ret;
	CHECK_FAST( getFileList( folderName, files ) );

	if ( files.size() == 0 ) {
		fprintf(stderr, "No input files found\n");
		return FAST_IO_ERROR;
	}

	CHECK_FAST( getImageSize( (*files.begin()).c_str(), width, height ) );

	files.clear();
	return FAST_OK;
}
