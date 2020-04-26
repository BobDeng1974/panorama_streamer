/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include "DebayerOptions.h"
#include "ParametersParser.h"

#include <stdio.h>
#include <string>
#include <string.h>

fastBayerPattern_t DebayerOptions::GetBayerPatternFromString(const char *pattern) {
	fastBayerPattern_t ret = FAST_BAYER_RGGB;

	if ( pattern != NULL)
	{
		if (strcmp(pattern, "GRBG") == 0)
			ret = FAST_BAYER_GRBG;
		else if (strcmp(pattern, "BGGR") == 0)
			ret = FAST_BAYER_BGGR;
		else if (strcmp(pattern, "GBRG") == 0)
			ret = FAST_BAYER_GBRG;
		else if (strcmp(pattern, "RGGB") == 0)
			ret = FAST_BAYER_RGGB;
		else
		{
			fprintf(stderr, "Pattern %s was not recognized.\nSet to RGGB (8 bits per pixel)\n", pattern);
			ret = FAST_BAYER_RGGB;
		}
	}

	return ret;
}

fastDebayerType_t DebayerOptions::GetBayerAlgorithmType(const char *pattern) {
	fastDebayerType_t ret = FAST_DFPD;

	if ( pattern != NULL ) {
		if ( strcmp(pattern, "DFPD" ) == 0 ) 
			ret = FAST_DFPD;
		else if (strcmp(pattern, "HQLI" ) == 0 ) 
			ret = FAST_HQLI;
		else {
			fprintf(stderr, "Pattern %s was not recognized.\nSet to DFPD\n", pattern);
			ret = FAST_DFPD;
		}
	}

	return ret;
}

bool DebayerOptions::Parse(int argc, char *argv[]) {
	
	IsRaw = false;
	Is2bytesOutput = false;

	if ( !GrayscaleCorrectionOptions::Parse(argc, argv) ) {
		return false;
	}

	if ( Help ) {
		return true;
	}

	fastSurfaceFormat_t outputFormat =  GetSurfaceFormatFromExtension(OutputPath);
	if (outputFormat==FAST_RGB16)
		Is2bytesOutput = true;

	char *tmp = NULL;
	ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "type", &tmp);
	BayerType = GetBayerAlgorithmType(tmp);

	ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "pattern", &tmp);
	BayerFormat = GetBayerPatternFromString(tmp);
	ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "lut", &LutDebayer);

	return true;
}
