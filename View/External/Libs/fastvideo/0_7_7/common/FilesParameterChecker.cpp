/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include <string.h>
#include <cctype>

#include "FilesParameterChecker.hpp"

bool FilesParameterChecker::CheckFileExtension(const char *fileName, const char *etalon) {
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

bool FilesParameterChecker::IsRaw(const char *fileName) {
	return
		(CheckFileExtension(fileName, ".bmp")) ||
		(CheckFileExtension(fileName, ".ppm")) ||
		(CheckFileExtension(fileName, ".pgm"));
}

bool FilesParameterChecker::IsBmp(const char *fileName) {
	return
		(CheckFileExtension(fileName, ".bmp"));
}

bool FilesParameterChecker::IsRawColor(const char *fileName) {
	return
		(CheckFileExtension(fileName, ".bmp")) ||
		(CheckFileExtension(fileName, ".ppm")) ||
		(CheckFileExtension(fileName, ".ppm16"));

}

bool FilesParameterChecker::IsRawGray(const char *fileName) {
	return
		(CheckFileExtension(fileName, ".bmp")) ||
		(CheckFileExtension(fileName, ".pgm")) ||
		(CheckFileExtension(fileName, ".pgm16")) ||
		(CheckFileExtension(fileName, ".raw"));
}

bool FilesParameterChecker::IsJpeg(const char *fileName) {
	return
		(CheckFileExtension(fileName, ".jpg")) ||
		(CheckFileExtension(fileName, ".jpeg")) ||
		(CheckFileExtension(fileName, ".jfif"));
}

bool FilesParameterChecker::IsAvi(const char *fileName) {
	return CheckFileExtension(fileName, ".avi");
}

bool FilesParameterChecker::IsValid(const char *fileName, const fastFileType_t fileType) {
	switch( fileType ) {
		case FAST_RAW:
			return IsRaw(fileName);
		case FAST_BMP:
			return IsBmp(fileName);
		case FAST_RAW_COLOR:
			return IsRawColor(fileName);
		case FAST_RAW_GRAY:
			return IsRawGray(fileName);
		case FAST_JPEG:
			return IsJpeg(fileName);
		case FAST_AVI:
			return IsAvi(fileName);
	}

	return false;
}

FilesParameterChecker::fastFileStatus_t FilesParameterChecker::Validate(
	const char * inputFile, fastFileType_t  inputFileType,
	const char *outputFile, fastFileType_t outputFileType
) {
	bool  inputStatus = IsValid(  inputFile,  inputFileType );
	bool outputStatus = IsValid( outputFile, outputFileType );

	if ( inputStatus && outputStatus )
		return FAST_OK;

	if ( !inputStatus && outputStatus )
		return FAST_INPUT_ERROR;

	if ( inputStatus && !outputStatus )
		return FAST_OUTPUT_ERROR;

	return FAST_BOTH_ERROR;
}