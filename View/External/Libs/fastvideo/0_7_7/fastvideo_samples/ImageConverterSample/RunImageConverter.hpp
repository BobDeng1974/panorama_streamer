/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __RUN_CONVERTERS__
#define __RUN_CONVERTERS__

#include "supported_files.hpp"
#include "helper_pgm.h"
#include "helper_raw.h"
#include "ConverterOptions.h"

#include "memory_interface.h"

fastStatus_t convertor16to8(char *inputFile, char *outputFile, unsigned shift) {
	Image<unsigned char, FastAllocator> inputImg;
	Image<unsigned char, FastAllocator> outputImg;

	printf("Input file: %s\n", inputFile);
	printf("Output file: %s\n", outputFile);
	printf("Shift option: %d\n", shift);

	fastStatus_t ret;
	CHECK_FAST_OPEN_FILE( loadFile( std::string(inputFile), std::string(""), inputImg, 0, 0, 0, false) );
	unsigned width = unsigned(inputImg.w);
	unsigned height = inputImg.h;
	unsigned channels = 1;

	
	if (inputImg.surfaceFmt == FAST_RGB16 || inputImg.surfaceFmt == FAST_RGB12) {
		channels = 3;
	}

	CHECK_FAST_ALLOCATION( outputImg.data.reset( (unsigned char *)FastAllocator::allocate<unsigned char>(width * channels * height) ) );

	unsigned char *src = inputImg.data.get();
	unsigned char *dest = outputImg.data.get();

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width * channels; j++) {
			unsigned val = src[i * inputImg.wPitch + j * 2] + ( src[i * inputImg.wPitch + j * 2 + 1] << 8 );
			dest[i * width * channels + j] = ( val >> shift ) & 0xFF;
		}
	}

	CHECK_FAST_SAVE_FILE( savePPM(outputFile, outputImg.data.get(), width, width * channels, height, 8, channels) );
	printf("Convertion complete\n");

	return FAST_OK;
}

fastStatus_t convertor8to16(char *inputFile, char *outputFile, unsigned int bitsPerPixel, unsigned shift) {
	Image<unsigned char, FastAllocator> inputImg;
	Image<unsigned short, FastAllocator> outputImg;

	printf("Input file: %s\n", inputFile);
	printf("Bits per pixel: %d\n", bitsPerPixel);
	printf("Output file: %s\n", outputFile);
	printf("Shift option: %d\n", shift);

	fastStatus_t ret;
	CHECK_FAST_OPEN_FILE( loadFile( std::string(inputFile), std::string(""), inputImg, 0, 0, 0, false) );
	unsigned channels = 1;
	
	if (inputImg.surfaceFmt == FAST_RGB8 || inputImg.surfaceFmt == FAST_BGR8) {
		channels = 3;
	}

	CHECK_FAST_ALLOCATION( outputImg.data.reset( (unsigned short *)FastAllocator::allocate<unsigned short>(inputImg.w * channels * inputImg.h) ) );

	unsigned char  *src  =  inputImg.data.get();
	unsigned short *dest = outputImg.data.get();

	for (int i = 0; i < inputImg.h; i++) {
		for (int j = 0; j < inputImg.w *channels; j++) {
			dest[i * inputImg.w * channels + j] = src[i * inputImg.wPitch + j ] << shift;
		}
	}

	CHECK_FAST_SAVE_FILE( savePPM(outputFile, (unsigned char *)outputImg.data.get(), inputImg.w, inputImg.w * channels * sizeof(unsigned short), inputImg.h, bitsPerPixel, channels) );
	printf("Convertion complete\n");

	return FAST_OK;
}

fastStatus_t convertorRawToPgm16(char *inputFile, char *outputFile, unsigned w, unsigned h, unsigned bitsPerPixel) {
	Image<unsigned char, FastAllocator> inputImg;
	
	printf("Input file: %s (%d x %d pixels)\n", inputFile, w, h, bitsPerPixel);
	printf("Bits per pixel: %d\n", bitsPerPixel);
	printf("Output file: %s\n", outputFile);

	fastStatus_t ret;
	CHECK_FAST_OPEN_FILE( loadFile( std::string(inputFile), std::string(""), inputImg, h, w, bitsPerPixel, false) );
	Image<unsigned char, FastAllocator> outputImg;
	outputImg.data.reset(FastAllocator::allocate<unsigned char>(inputImg.h*inputImg.w*sizeof(unsigned short)));
	outputImg.w = inputImg.w;
	outputImg.h = inputImg.h;
	outputImg.bitsPerPixel = inputImg.bitsPerPixel;
	outputImg.wPitch = inputImg.w*sizeof(unsigned short);

	unsigned char* _rawData = (unsigned char*)inputImg.data.get();
	unsigned short* _outputData = (unsigned short*)outputImg.data.get();


	int packedPos = 0;
	for (int i=0;i<outputImg.h;i++)
		for (int j=0; j<outputImg.w;j+=2)
		{
			 _outputData[i*outputImg.wPitch/sizeof(unsigned short) + j] = _rawData[packedPos] + ((_rawData[packedPos + 1] &0x0F)<<8); 
			 _outputData[i*outputImg.wPitch/sizeof(unsigned short) + j + 1] = (_rawData[packedPos+2]<<4) + ((_rawData[packedPos + 1] &0xF0)>>4); 
			packedPos+=3;
		}


	CHECK_FAST_SAVE_FILE( savePPM(outputFile, outputImg.data.get(), outputImg.w, outputImg.wPitch, outputImg.h, bitsPerPixel, 1) );
	printf("Convertion complete\n");

	return FAST_OK;
}

fastStatus_t convertorPgm16ToRaw(char *inputFile, char *outputFile) {
	Image<unsigned char, FastAllocator> inputImg;

	printf("Input file: %s\n", inputFile);
	printf("Output file: %s\n", outputFile);

	fastStatus_t ret;
	CHECK_FAST_OPEN_FILE( loadFile( std::string(inputFile), std::string(""), inputImg, 0, 0, 0, false) );
	if (inputImg.w % 2 != 0) {
		fprintf(stderr,"Image should have even width\n");
		return FAST_INVALID_SIZE;
	}

	std::unique_ptr<unsigned char, FastAllocator> rawData;

	const unsigned sizeInBits = inputImg.w * 1 * inputImg.bitsPerPixel * sizeof(unsigned char)*inputImg.h;
	const unsigned sizeInBytes = ( sizeInBits + 7 ) / 8;

	rawData.reset(FastAllocator::allocate<unsigned char>(sizeInBytes) );
	unsigned char* _rawData = (unsigned char*)rawData.get();

	unsigned short*  _inputImg = (unsigned short*)inputImg.data.get();
	int packedPos = 0;
	for (int i=0;i<inputImg.h;i++)
		for (int j=0; j<inputImg.w;j+=2)
		{
			_rawData[packedPos] = _inputImg[i*inputImg.wPitch/sizeof(unsigned short) + j]&0xFF;
			_rawData[packedPos + 1] = (_inputImg[i*inputImg.wPitch/sizeof(unsigned short) + j]>>8) +
				((_inputImg[i*inputImg.wPitch/sizeof(unsigned short) + j + 1]<<4)&0xFF);
			_rawData[packedPos + 2] = (_inputImg[i*inputImg.wPitch/sizeof(unsigned short) + j + 1]>>4);
			packedPos+=3;
		}

	CHECK_FAST_SAVE_FILE( __saveRaw(outputFile, _rawData, inputImg.w, inputImg.h, 1, inputImg.bitsPerPixel) );
	printf("Convertion complete\n");

	return FAST_OK;
}

fastStatus_t RunConvertion(ConverterOptions options) {
	if ( checkFileExtension(options.InputPath, ".raw") ) {
		if ( checkFileExtension(options.OutputPath, ".pgm16") ) {
			return convertorRawToPgm16( options.InputPath, options.OutputPath, options.RawWidth, options.RawHeight, options.BitsPerPixel );
		}
	} else if ( checkFileExtension(options.InputPath, ".pgm16") || checkFileExtension(options.InputPath, ".ppm16") ) {
		if ( checkFileExtension(options.OutputPath, ".pgm") || checkFileExtension(options.OutputPath, ".ppm") ) {
			return convertor16to8( options.InputPath, options.OutputPath, options.Shift );
		}

		if ( checkFileExtension(options.InputPath, ".pgm16") && checkFileExtension(options.OutputPath, ".raw") ) {
			return convertorPgm16ToRaw( options.InputPath, options.OutputPath );
		}
	} else if ( checkFileExtension(options.InputPath, ".pgm") || checkFileExtension(options.InputPath, ".ppm") ) {
		if ( checkFileExtension(options.OutputPath, ".pgm16") || checkFileExtension(options.OutputPath, ".ppm16") ) {
			return convertor8to16( options.InputPath, options.OutputPath, options.BitsPerPixel, options.Shift );
		}
	}

	fprintf( stderr, "Incorrect input parameters. Supported conversions: RAW <-> PGM16, PPM(PGM) <-> PPM16(PGM16)\n");
	return FAST_INVALID_VALUE;
}

#endif // __RUN_CONVERTERS__
 