/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#include <fstream>
#include <memory>

#include "bmp.hpp"
#include "memory_interface.h"
#include <stdint.h>

#pragma pack(push)
    #pragma pack(1)

    struct BMPHeader{
        uint16_t type;
        uint32_t size;
        uint16_t reserved1;
        uint16_t reserved2;
        uint32_t offset;
    };

    struct BMPInfoHeader{
        uint32_t size;
        int32_t  width;
        int32_t  height;
        uint16_t planes;
        uint16_t bitsPerPixel;
        uint32_t compression;
        uint32_t imageSize;
        int32_t  xPelsPerMeter;
        int32_t  yPelsPerMeter;
        uint32_t clrUsed;
        uint32_t clrImportant;
    };
#pragma pack(pop)



template<typename T> static inline T uSnapUp(T a, T b){
    return a + (b - a % b) % b;
}

template<typename T> static inline T uDivUp(T a, T b){
    return (a / b) + (a % b != 0);
}

template<typename T>
void Read(T *data, std::ifstream &fd, size_t count = 1){
    if(count > 0)
        fd.read(reinterpret_cast<char *>(data), count * sizeof(T));
}

template<typename T>
void Write(std::ofstream &fd, const T *data, size_t count = 1){
    if(count > 0)
        fd.write(reinterpret_cast<const char *>(data), count * sizeof(T));
}


template <class T, class Allocator>
fastStatus_t LoadBMP(
    std::unique_ptr<T, Allocator> &h_Surface,
    fastSurfaceFormat_t &surfaceFmt,
    unsigned              &surfaceHeight,
    unsigned              &surfaceWidth,
    unsigned              &surfacePitch8,
    const char *filename
){
	if ( sizeof(T) != sizeof(unsigned char) ) {
		return FAST_IO_ERROR;
	}

    BMPHeader hdr;
    BMPInfoHeader infoHdr;

    std::ifstream fd(filename, std::ios::in | std::ios::binary);
    fd.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);

    //Load header and derive basic parameters
    Read(&hdr,     fd);
    Read(&infoHdr, fd);

    const bool        isTrueColor = (infoHdr.bitsPerPixel >= 16);
    const unsigned paletteEntries = isTrueColor ? 
        0 :
        (
            (infoHdr.clrUsed != 0) ? infoHdr.clrUsed : (1U << infoHdr.bitsPerPixel)
        );

    //const unsigned auxEntries = isTrueColor ? infoHdr.clrUsed : 0;
    const unsigned   bmpPitch = uDivUp(infoHdr.width * infoHdr.bitsPerPixel, 32) * 4U;
    const bool    isTopBottom = (infoHdr.height < 0);

    //Check basic signatures
    {
        if(hdr.type != 0x4D42)
            return FAST_INVALID_FORMAT;

        if(infoHdr.size != sizeof(BMPInfoHeader))
            return FAST_INVALID_FORMAT;

        if(infoHdr.bitsPerPixel != 8 && infoHdr.bitsPerPixel != 24)
            return FAST_UNSUPPORTED_FORMAT;

        if(infoHdr.compression != 0)
            return FAST_UNSUPPORTED_FORMAT;

        if(infoHdr.width <= 0)
            return FAST_UNSUPPORTED_FORMAT;

        if(infoHdr.height == 0 || infoHdr.width == 0 || paletteEntries > 256)
            return FAST_INVALID_FORMAT;
    }

    //Check sizes and offsets to match
    {
        if(infoHdr.imageSize != 0 && infoHdr.imageSize < abs(infoHdr.height) * bmpPitch)
            return FAST_INVALID_FORMAT;

        if(hdr.offset < sizeof(BMPHeader) + sizeof(BMPInfoHeader) + paletteEntries * 4)
            return FAST_INVALID_FORMAT;

        if(hdr.size < sizeof(BMPHeader) + sizeof(BMPInfoHeader) + paletteEntries * 4 + abs(infoHdr.height) * bmpPitch)
            return FAST_INVALID_FORMAT;
    }
    unsigned char palette[256][4];

    //Is palette greyscale? (== all table components are equal)
    //Is palette identity (table[i][...] = i, no need to look up palette)?
    bool isGreyscale = !isTrueColor;
    bool  isIdentity = !isTrueColor;
    {
        Read(palette, fd, paletteEntries);

        for(unsigned i = 0; i < paletteEntries; i++){
            if(palette[i][1] != palette[i][0] || palette[i][2] != palette[i][0])
                isGreyscale = false;

            if(palette[i][0] != i || palette[i][1] != i || palette[i][2] != i)
                isIdentity = false;
        }
    }

    if(!(isTrueColor || isGreyscale))
        return FAST_UNSUPPORTED_FORMAT; //Palette color images unsupported

    //Init destination buffer
    const unsigned channelCount = isTrueColor ? 3           : 1;
    surfaceFmt                  = isTrueColor ? FAST_BGR8 : FAST_I8;

    {
        surfaceHeight = abs(infoHdr.height);
        surfaceWidth  = infoHdr.width;
        surfacePitch8 = uSnapUp(surfaceWidth * channelCount, FAST_ALIGNMENT);

		//fastStatus_t ret;
		//CHECK_FAST_ALLOCATION( fastMalloc((void **)(h_Surface), surfaceHeight * surfacePitch8 * sizeof(unsigned char)) );
		CHECK_FAST_ALLOCATION( h_Surface.reset( (T *)Allocator::template allocate<T>(surfaceHeight * surfacePitch8) ) );
    }

    //Offset to the specified bitmap start location (a gap may be present)
    fd.seekg(hdr.offset, std::ios::beg);

    for(unsigned iy = 0; iy < surfaceHeight; iy++){
        const unsigned y = isTopBottom ? iy : (surfaceHeight - 1 - iy);
        Read(h_Surface.get() + y * surfacePitch8, fd, bmpPitch);

        if(isGreyscale && !isIdentity)
             for(unsigned x = 0; x < surfaceWidth; x++)
                 h_Surface.get()[y * surfacePitch8 + x] = palette[(int)h_Surface.get()[y * surfacePitch8 + x]][0];
    }
	return FAST_OK;
}

template fastStatus_t LoadBMP< unsigned char, FastAllocator >(
    std::unique_ptr< unsigned char, FastAllocator > &h_Surface,

    fastSurfaceFormat_t &surfaceFmt,
    unsigned            &surfaceHeight,
    unsigned            &surfaceWidth,
    unsigned            &surfacePitch8,

    const char *filename
);

template fastStatus_t LoadBMP< float, FastAllocator >(
    std::unique_ptr< float, FastAllocator > &h_Surface,

    fastSurfaceFormat_t &surfaceFmt,
    unsigned            &surfaceHeight,
    unsigned            &surfaceWidth,
    unsigned            &surfacePitch8,

    const char *filename
);

template<typename T>
fastStatus_t StoreBMP(
    const char           *filename,
    const T				 *h_Surface,
    fastSurfaceFormat_t surfaceFmt,
    unsigned              surfaceHeight,
    unsigned              surfaceWidth,
    unsigned              surfacePitch8
){
    BMPHeader hdr;
    BMPInfoHeader infoHdr;

    std::ofstream fd(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    fd.exceptions(std::ios::failbit | std::ios::badbit);

    const unsigned channelCount = (surfaceFmt == FAST_I8) ? 1 : 3;

    const unsigned       bmpPitch = uSnapUp(surfaceWidth * channelCount, 4U);
    const unsigned paletteEntries = (channelCount > 1) ? 0 : 256;

    hdr.type      = 0x4D42;
    hdr.size      = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + paletteEntries * 4 + surfaceHeight * bmpPitch;
    hdr.reserved1 = 0;
    hdr.reserved2 = 0;
    hdr.offset    = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + paletteEntries * 4;
    Write(fd, &hdr);

    infoHdr.size          = sizeof(BMPInfoHeader);
    infoHdr.width         = surfaceWidth;
    infoHdr.height        = surfaceHeight;
    infoHdr.planes        = 1;
    infoHdr.bitsPerPixel  = (channelCount > 1) ? 24 : 8;
    infoHdr.compression   = 0;
    infoHdr.imageSize     = surfaceHeight * bmpPitch;
    infoHdr.xPelsPerMeter = 0;
    infoHdr.yPelsPerMeter = 0;
    infoHdr.clrUsed       = (channelCount > 1) ? 0 : 256;
    infoHdr.clrImportant  = (channelCount > 1) ? 0 : 256;
    Write(fd, &infoHdr);

    if(channelCount == 1){
        unsigned char palette[256][4];

        for(unsigned i = 0; i < 256; i++){
            palette[i][0] = i;
            palette[i][1] = i;
            palette[i][2] = i;
            palette[i][3] = 0;
        }

        Write(fd, palette, 256);
    }

    for(unsigned y = 0; y < surfaceHeight; y++)
        Write(fd, h_Surface + (surfaceHeight - 1 - y) * surfacePitch8, bmpPitch);
	return FAST_OK;
}

template fastStatus_t StoreBMP<unsigned char>(
    const char           *filename,
    const unsigned char	 *h_Surface,
    fastSurfaceFormat_t surfaceFmt,
    unsigned              surfaceHeight,
    unsigned              surfaceWidth,
    unsigned              surfacePitch8
);

template fastStatus_t StoreBMP<float>(
    const char           *filename,
    const float			 *h_Surface,
    fastSurfaceFormat_t surfaceFmt,
    unsigned              surfaceHeight,
    unsigned              surfaceWidth,
    unsigned              surfacePitch8
);


