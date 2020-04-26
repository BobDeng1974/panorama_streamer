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

#include "fastvideo_sdk.h"
#include "RunCamera.hpp"

#include "messages.h"

#include "FfmpegOptions.h"
#include "ParametersParser.h"
#include "FilesParameterChecker.hpp"

int helpPrint() {
	printf("%s", mainHelp);
	return 0;
}

int main(int argc, char *argv[]){
	FfmpegOptions options;
	
	if ( !options.Parse(argc, argv) ) {
		helpPrint ();
		return -1;
	}

	if ( options.Help ) {
		helpPrint ();
		return 0;
	}
	
	fastStatus_t ret = FAST_OK;

	switch(FilesParameterChecker::Validate(
				options.InputPath,  FilesParameterChecker::FAST_RAW_GRAY,
				options.OutputPath, FilesParameterChecker::FAST_AVI
		   ) ) {
		case FilesParameterChecker::FAST_OK:
			ret = RunCamera(argc, argv, options);
			break;
		case FilesParameterChecker::FAST_INPUT_ERROR:
			fprintf(stderr, "Input file has unappropriate format.\nShould be bmp or pgm.\n");
			break;
		case FilesParameterChecker::FAST_OUTPUT_ERROR:
			fprintf(stderr, "Output file has unappropriate format.\nFor encoding output file should be in avi format.\n");
			break;
		case FilesParameterChecker::FAST_BOTH_ERROR:
			fprintf(stderr, "Input and output file has unappropriate format.\nShould be bmp, pgm or jpg.\n");
			break;
	}

    return ret == FAST_OK ? 0 : -1;
}
