/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include "ResizerOptions.h"
#include "ParametersParser.h"

#include <stdio.h>

bool ResizerOptions::Parse(int argc, char *argv[]) {
	if ( !EncoderOptions::Parse(argc, argv) ) {
		return false;
	}

	if ( !ImageFilterOptions::Parse(argc, argv) ) {
		return false;
	}

	if ( !CropOptions::Parse(argc, argv) ) {
		return false;
	}

	if ( Help ) {
		return true;
	}

	OutputWidth = ParametersParser::GetCmdLineArgumentInt(argc, (const char **)argv, "outputWidth");
	if ( OutputWidth < MIN_SCALED_SIZE ) {
		fprintf(stderr, "Unsupported output image width - %d. Minimum width is %d\n", OutputWidth, MIN_SCALED_SIZE);
		return false;
	}

	ShiftX = 0.0f;
	ShiftY = 0.0f;

	MaxScaleFactor = ParametersParser::GetCmdLineArgumentFloat(argc, (const char **)argv, "maxScale");

	if  ( IsFolder ) {
		if ( MaxScaleFactor <= SCALE_FACTOR_MIN || MaxScaleFactor > SCALE_FACTOR_MAX ) {
			fprintf(stderr, "Incorrect image scale factor (%.3f). Supported range of scales is (%d,%d]\n", MaxScaleFactor, ResizerOptions::SCALE_FACTOR_MIN, ResizerOptions::SCALE_FACTOR_MAX);
			return false;
		}
	}

	return true;
}
