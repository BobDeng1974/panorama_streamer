#include "ImageFilterOptions.h"
#include "ParametersParser.h"

#include <stdio.h>
#include <string>
#include <string.h>

double ImageFilterOptions::DisabledSharpConst = -1.0;

bool ImageFilterOptions::Parse(int argc, char *argv[]) {
	if ( !BaseOptions::Parse(argc, argv) ) {
		return false;
	}

	if ( Help ) {
		return true;
	}

	RawWidth     = ParametersParser::GetCmdLineArgumentInt( argc, (const char **)argv, "w" );
	RawHeight    = ParametersParser::GetCmdLineArgumentInt( argc, (const char **)argv, "h" );
	BitsCount = ParametersParser::GetCmdLineArgumentInt( argc, (const char **)argv, "bits" );
	if ( BitsCount != 8 && BitsCount != 12 ) {
		BitsCount = 8;
	}

	SharpBefore = DisabledSharpConst;
	if ( ParametersParser::CheckCmdLineFlag(argc, (const char **)argv, "sharp_before") ) {
		SharpBefore = ParametersParser::GetCmdLineArgumentFloat(argc, (const char **)argv, "sharp_before", DisabledSharpConst);
		if ( SharpBefore < 0. ) {
			fprintf(stderr, "Incorrect sharp_before = %.3f. Set to 1\n", SharpBefore);
			SharpBefore = 1.;
		}
	}
	
	SharpAfter = DisabledSharpConst;
	if ( ParametersParser::CheckCmdLineFlag(argc, (const char **)argv, "sharp_after") ) {
		SharpAfter  = ParametersParser::GetCmdLineArgumentFloat(argc, (const char **)argv, "sharp_after", DisabledSharpConst);
		if ( SharpAfter < 0. ) {
			fprintf(stderr, "Incorrect sharp_after = %.3f. Set to 1\n", SharpAfter);
			SharpAfter = 1.;
		}
	}

	return true;
}
