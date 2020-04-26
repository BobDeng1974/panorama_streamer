#ifndef __RUN_RAW_BAYER_COMPRESSION__
#define __RUN_RAW_BAYER_COMPRESSION__

#include "SplitterEncoder.h"
#include "DecoderMerger.h"

#include "DebayerJpegOptions.h"

#include "memory_interface.h"
#include "supported_files.hpp"
#include "FastAllocator.h"
#include "ExifInfo.hpp"

static fastStatus_t DecodeJpeg(unsigned char *data, unsigned dataSize, fastJfifInfo_t *jfifInfo) {
	return fastJfifLoadFromMemory(
			 data,
			 dataSize,

			 jfifInfo
		);
}

fastStatus_t RunSplitterEncoder(DebayerJpegOptions options) {
	SplitterEncoder hSplitterEncoder(options.Info);
	fastStatus_t ret;

	std::list< Image< unsigned char, FastAllocator > > inputImg;

	if ( options.IsFolder ) {
		CHECK_FAST( loadImages( options.InputPath, options.OutputPath, inputImg, options.RawWidth, options.RawHeight, options.BitsPerPixel, false ) );
	} else {
		Image< unsigned char, FastAllocator > img;
		ret = loadFile( std::string(options.InputPath), std::string(options.OutputPath), img, options.RawHeight, options.RawWidth, options.BitsPerPixel, false );
		if (ret != FAST_OK) {
			if (ret == FAST_IO_ERROR) {
				fprintf(stderr, "Input image file %s has not been found!\n", options.InputPath);
			}
			return ret;
		}
		
		options.MaxHeight = options.MaxHeight == 0 ? img.h : options.MaxHeight;
		options.MaxWidth = options.MaxWidth == 0 ? img.w : options.MaxWidth;
		inputImg.push_back(img);
	}

	printf("Output surface format: %s\n", surfaceAnalyze(options.SurfaceFmt));

	CHECK_FAST( hSplitterEncoder.Init(options) );
	CHECK_FAST( hSplitterEncoder.Transform(inputImg) );

	inputImg.clear();

	CHECK_FAST( hSplitterEncoder.Close() );

	return FAST_OK;
}

fastStatus_t RunDecoderMerger(DebayerJpegOptions options) {
	DecoderMerger hDecoderMerger(options.Info);
	fastStatus_t ret;
	unsigned maxRestoredWidth = 0, maxRestoredHeight = 0;

	std::list<  Data< unsigned char, FastAllocator > > inputImg;

	if ( options.IsFolder ) {
		CHECK_FAST( loadJpegImages( options.InputPath, inputImg, false ) );
		int idx = 0;
		for (std::list< Data< unsigned char, FastAllocator > >::iterator i = inputImg.begin(); i != inputImg.end(); i++, idx++) {
			i->outputFileName = generateOutputFileName(options.OutputPath, idx);
		}
	} else {
		ret = loadJpeg(std::string(options.InputPath), inputImg, false);
		if (ret == FAST_IO_ERROR) {
			fprintf(stderr, "Input image file %s has not been found!\n", options.InputPath);
			return ret;
		}
		if (ret != FAST_OK)
			return ret;

		(--inputImg.end())->outputFileName =  std::string(options.OutputPath);
	}

	fastJfifInfo_t jfifInfo;
	jfifInfo.h_Bytestream = NULL;
	jfifInfo.exifSections = NULL;
	jfifInfo.exifSectionsCount = 0;

	jfifInfo.bytestreamSize = (*inputImg.begin()).size;
	CHECK_FAST( fastMalloc( (void **)&jfifInfo.h_Bytestream, jfifInfo.bytestreamSize ) );
		
	CHECK_FAST( DecodeJpeg( (*inputImg.begin()).data.get(), (*inputImg.begin()).size, &jfifInfo ) );

	if (jfifInfo.h_Bytestream != NULL)
		CHECK_FAST_DEALLOCATION( fastFree(jfifInfo.h_Bytestream) );

	if (jfifInfo.exifSectionsCount < 1) {
		fprintf(stderr, "Incorrect JPEG: EXIF sections was not found\n");
		return FAST_IO_ERROR;
	}

	if ( jfifInfo.exifSections != NULL ) {
		for (int i = 0; i < jfifInfo.exifSectionsCount; i++) {
			ParseSplitterExif( &jfifInfo.exifSections[i], options.BayerFormat, maxRestoredWidth, maxRestoredHeight );

			free(jfifInfo.exifSections[i].exifData);
		}

		free(jfifInfo.exifSections);
	}

	if ( maxRestoredWidth == 0 || maxRestoredHeight == 0 ) {
		fprintf(stderr, "Incorrect JPEG: debayer parameters in EXIF section was not found\n");
		return FAST_IO_ERROR;
	}

	options.MaxHeight = options.MaxHeight == 0 ? jfifInfo.height : options.MaxHeight;
	options.MaxWidth = options.MaxWidth == 0 ? jfifInfo.width : options.MaxWidth;
	options.SurfaceFmt = jfifInfo.jpegFmt == JPEG_Y ? FAST_I8 : options.SurfaceFmt;

	printf("Output surface format: %s\n", surfaceAnalyze(BaseOptions::GetSurfaceFormatFromExtension(options.OutputPath)));

	CHECK_FAST( hDecoderMerger.Init(options, maxRestoredWidth, maxRestoredHeight) );
	CHECK_FAST( hDecoderMerger.Transform(inputImg) );

	inputImg.clear();

	CHECK_FAST( hDecoderMerger.Close() );

	return FAST_OK;
}

#endif // __RUN_RAW_BAYER_COMPRESSION__
