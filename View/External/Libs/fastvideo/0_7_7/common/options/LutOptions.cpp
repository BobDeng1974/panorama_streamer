/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include "LutOptions.h"
#include "ParametersParser.h"

#include <stdio.h>
#include <string>
#include <string.h>

bool LutOptions::Parse(int argc, char *argv[]) {
	if ( !BaseOptions::Parse(argc, argv) ) {
		return false;
	}
	
	Lut_R = Lut_G = Lut_B = NULL;
	if (!ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "lut_r", &Lut_R)) {
		fprintf(stderr, "lut_r parameter not found\n");
		return false;
	}

	if (!ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "lut_g", &Lut_G)) {
		fprintf(stderr, "lut_g parameter not found\n");
		return false;
	}

	if (!ParametersParser::GetCmdLineArgumentString(argc, (const char **)argv, "lut_b", &Lut_B)) {
		fprintf(stderr, "lut_b parameter not found\n");
		return false;
	}

	return true;
}
