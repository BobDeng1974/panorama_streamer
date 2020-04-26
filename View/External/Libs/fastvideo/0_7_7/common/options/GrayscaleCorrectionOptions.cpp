/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include "GrayscaleCorrectionOptions.h"
#include "ParametersParser.h"

#include <stdio.h>

bool GrayscaleCorrectionOptions::Parse(int argc, char *argv[]) {
	if ( !BaseOptions::Parse(argc, argv) ) {
		return false;
	}

	if ( Help ) {
		return true;
	}

	BaseColorCorrectionEnabled = false;
	if ( ParametersParser::CheckCmdLineFlag( argc, (const char **)argv, "colorCorrection" ) ) {
		BaseColorCorrectionEnabled = true;
		
		char *tmp = NULL;
		ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "colorCorrection", &tmp);
		
		if ( sscanf(tmp,
					"%*c%f%*c%f%*c%f%*c%f%*c%f%*c%f%*c%f%*c%f%*c%f%*c%f%*c%f%*c%f%*c",
					&BaseColorCorrection[ 0],
					&BaseColorCorrection[ 1],
					&BaseColorCorrection[ 2],
					&BaseColorCorrection[ 3],
					&BaseColorCorrection[ 4],
					&BaseColorCorrection[ 5],
					&BaseColorCorrection[ 6],
					&BaseColorCorrection[ 7],
					&BaseColorCorrection[ 8],
					&BaseColorCorrection[ 9],
					&BaseColorCorrection[10],
					&BaseColorCorrection[11]
			 ) < 12 ) {
				fprintf(stderr, "Incorrect -colorCorrection option (-colorCorrection %s)\n", tmp);
				BaseColorCorrectionEnabled = false;
		}
	}
	
	ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "matrixA", &MatrixA);
	ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "matrixB", &MatrixB);

	return true;
}
