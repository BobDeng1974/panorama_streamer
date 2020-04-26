#ifndef __IMAGE_FILTER_OPTIONS__
#define __IMAGE_FILTER_OPTIONS__

#include "BaseOptions.h"

#include "fastvideo_sdk.h"

class ImageFilterOptions : public virtual BaseOptions {
public:
	static double DisabledSharpConst;

	unsigned RawWidth;
	unsigned RawHeight;
	
	unsigned BitsCount;

	double SharpBefore;
	double SharpAfter;

	ImageFilterOptions(void) { };
	~ImageFilterOptions(void) { };

	bool Parse(int argc, char *argv[]);
};

#endif // __IMAGE_FILTER_OPTIONS__