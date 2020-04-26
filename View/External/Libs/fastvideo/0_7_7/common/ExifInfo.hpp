#ifndef __EXIF_INFO__
#define __EXIF_INFO__

#include "fastvideo_sdk.h"

#define EXIF_CODE 0xFFE1

#define EXIF_NAME "splitted bayer\0"
#define EXIF_NAME_LENGTH 15

#pragma pack(1)
typedef struct {
	char name[EXIF_NAME_LENGTH];
	char bayerFmt;
	unsigned width;
	unsigned height;
} SplitterExif_t;
#pragma pack()

static fastJpegExifSection_t *GenerateSplitterExif( fastBayerPattern_t bayerFmt, unsigned width, unsigned height ) {
	fastJpegExifSection_t *exifSection = new fastJpegExifSection_t[1];
	exifSection->exifCode = EXIF_CODE;
	exifSection->exifLength = sizeof( SplitterExif_t );
	exifSection->exifData = new char[exifSection->exifLength];

	SplitterExif_t *exif = ( SplitterExif_t * )exifSection->exifData;
	strcpy( exif->name, EXIF_NAME );
	exif->bayerFmt = (char)bayerFmt;
	exif->width = width;
	exif->height = height;

	return exifSection;
}

static fastStatus_t UpdateSplitterExif( fastJpegExifSection_t *exifSection, unsigned width, unsigned height ) {
	if ( exifSection->exifCode == EXIF_CODE && exifSection->exifLength == sizeof( SplitterExif_t ) ) {
		SplitterExif_t *exif = ( SplitterExif_t * )exifSection->exifData;
		exif->width = width;
		exif->height = height;
		return FAST_OK;
	}

	return FAST_INVALID_VALUE;
}

static fastStatus_t ParseSplitterExif( fastJpegExifSection_t *exifSection, fastBayerPattern_t &pattern, unsigned &width, unsigned &height ) {
	if ( exifSection->exifCode == EXIF_CODE && exifSection->exifLength == sizeof( SplitterExif_t ) ) {
		if ( strcmp( exifSection->exifData, EXIF_NAME ) == 0 ) {
			SplitterExif_t *exif = ( SplitterExif_t * )exifSection->exifData;
			pattern = (fastBayerPattern_t) exif->bayerFmt;
			width = exif->width;
			height = exif->height;
			return FAST_OK;
		}
	}

	return FAST_INVALID_VALUE;
}

#endif // __EXIF_INFO__