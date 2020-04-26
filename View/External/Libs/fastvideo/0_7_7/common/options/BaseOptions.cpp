/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include "BaseOptions.h"
#include "ParametersParser.h"

#include <stdio.h>
#include <string>
#include <string.h>

bool BaseOptions::CheckFileExtension(const char *fileName, const char *etalon) {
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

fastSurfaceFormat_t BaseOptions::GetSurfaceFormatFromExtension(const char* fname) {
	if (CheckFileExtension(fname, ".bmp") || CheckFileExtension(fname, ".dib") || CheckFileExtension(fname, ".jpg") || CheckFileExtension(fname, ".jpeg"))
		return FAST_BGR8;
	if (CheckFileExtension(fname, ".ppm"))
		return FAST_RGB8;
	if (CheckFileExtension(fname, ".ppm16"))
		return FAST_RGB16;
	if (CheckFileExtension(fname, ".pgm16"))
		return FAST_I16;
	return FAST_I8;
}

bool BaseOptions::Parse(int argc, char *argv[]) {
	Help = ParametersParser::CheckCmdLineFlag(argc, (const char **)argv, "help") || argc == 1;
	if ( Help ) {
		return true;
	}

	InputPath = OutputPath = NULL;
	if (!ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "i", &InputPath)) {
		if (!ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "if", &InputPath)) {
			fprintf(stderr, "Input path parameter is not found\n");
			return false;
		}
	}

	if (!ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "o", &OutputPath)) {
		fprintf(stderr, "Output file parameter is not found\n");
		return false;
	}

	IsFolder  = ParametersParser::CheckCmdLineFlag(argc, (const char **)argv, "if");
	MaxWidth  = ParametersParser::GetCmdLineArgumentInt(argc, (const char **)argv, "maxWidth");
	MaxHeight = ParametersParser::GetCmdLineArgumentInt(argc, (const char **)argv, "maxHeight");

	if ( IsFolder ) {
		if ( MaxWidth <= 0 || MaxHeight <= 0 ) {
			fprintf(stderr, "maxWidth or maxHeight option was not set\n");
			return false;
		}
	}

	RawWidth     = ParametersParser::GetCmdLineArgumentInt( argc, (const char **)argv, "w" );
	RawHeight    = ParametersParser::GetCmdLineArgumentInt( argc, (const char **)argv, "h" );
	BitsPerPixel = ParametersParser::GetCmdLineArgumentInt( argc, (const char **)argv, "bits" );
	if ( BitsPerPixel != 8 && BitsPerPixel != 12 ) {
		BitsPerPixel = 8;
	}

	ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "lut", &Lut);

	DeviceId = ParametersParser::GetCmdLineArgumentInt(argc, (const char **)argv, "d");
	SurfaceFmt = GetSurfaceFormatFromExtension(InputPath);

	Info = ParametersParser::CheckCmdLineFlag(argc, (const char **)argv, "info");

	return true;
}
