/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include "EncoderOptions.h"
#include "ParametersParser.h"

#include <stdio.h>

fastJpegFormat_t EncoderOptions::ParseSubsamplingFmt(const int samplingFmtCode){
	fastJpegFormat_t samplingFmt = JPEG_444;

    if(samplingFmtCode == 444)
        samplingFmt = JPEG_444;
    else if(samplingFmtCode == 422)
        samplingFmt = JPEG_422;
    else if(samplingFmtCode == 420)
        samplingFmt = JPEG_420;
    else{
        fprintf(stderr, "Bad subsampling format, set to default...\n");
    }

	return samplingFmt;
}

bool EncoderOptions::Parse(int argc, char *argv[]) {
	if ( !GrayscaleCorrectionOptions::Parse(argc, argv) ) {
		return false;
	}

	if ( Help ) {
		return true;
	}

	Quality = ParametersParser::GetCmdLineArgumentInt(argc, (const char **)argv, "q");
	if (Quality <= 0 || Quality > 100) Quality = 75;

	RestartInterval = ParametersParser::GetCmdLineArgumentInt(argc, (const char **)argv, "r");
	if (RestartInterval <= 0) RestartInterval = 16;

	unsigned sub = ParametersParser::GetCmdLineArgumentInt(argc, (const char **)argv, "s");
	if (sub <= 0) sub = 444;

	SamplingFmt = ParseSubsamplingFmt(sub);

	FFMPEG = ParametersParser::CheckCmdLineFlag(argc, (const char **)argv, "ffmpeg");
	FFMPEG_REVERSE = ParametersParser::CheckCmdLineFlag(argc, (const char **)argv, "ffmpeg_r");

	return true;
}
