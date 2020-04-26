/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __FAST_H__
#define __FAST_H__

#include <stdlib.h>

#ifdef __cplusplus
    extern "C" {
#endif

#ifndef __GNUC__

#ifdef FAST_EXPORTS
	#define DLL __declspec(dllexport) __cdecl
#else
	#define DLL
#endif

#else

#define DLL

#endif



////////////////////////////////////////////////////////////////////////////////
// Basic data types
////////////////////////////////////////////////////////////////////////////////
#define FAST_ALIGNMENT 4U
#define FAST_SCALE_FACTOR_MAX 40U
#define FAST_SCALE_FACTOR_MIN 1U
#define FAST_MIN_SCALED_SIZE 32U

typedef enum{
    FAST_OK,                           //No error
	FAST_INVALID_DEVICE,               //Device not exists or Non NVIDIA device or no CUDA-capable devices present
	FAST_INCOMPATIBLE_DEVICE,          //CUDA-capable devices present but it's compute compatibility below 2.0
    FAST_INSUFFICIENT_DEVICE_MEMORY,   //Insufficient device memory
    FAST_INSUFFICIENT_HOST_MEMORY,     //Insufficient host memory
    FAST_INVALID_HANDLE,               //Invalid JPEG encoder / decoder or Debayer handle
    FAST_INVALID_VALUE,                //Invalid API enum value
	FAST_UNAPPLICABLE_OPERATION,	   //This operation cannot be applied for current type of data
    FAST_INVALID_SIZE,                 //Invalid image dimensions
    FAST_UNALIGNED_DATA,               //Buffer base pointers or pitch not properly aligned
    FAST_INVALID_TABLE,                //Invalid quantization / Huffman table
    FAST_BITSTREAM_CORRUPT,            //Decoding error
    FAST_EXECUTION_FAILURE,            //Device execution failure (TDR watchdog?)
    FAST_INTERNAL_ERROR,               //Internal error

    FAST_IO_ERROR,                     //Failed to open/access file
    FAST_INVALID_FORMAT,               //Invalid file format
    FAST_UNSUPPORTED_FORMAT,           //File format is unsupported by the current version of FAST

	FAST_UNKNOWN_ERROR                 //Unrecognized error
} fastStatus_t;

typedef enum{
    FAST_I8,
	FAST_I12,
	FAST_I16,

    FAST_RGB8,
    FAST_BGR8,
	FAST_RGB12,
	FAST_RGB16,

    FAST_CrCbY8,
    FAST_YCbCr8
} fastSurfaceFormat_t;

typedef enum{
    JPEG_Y, JPEG_444, JPEG_422, JPEG_420
} fastJpegFormat_t;

typedef enum{
	FAST_BAYER_NONE,
	FAST_BAYER_RGGB,
	FAST_BAYER_BGGR,
	FAST_BAYER_GBRG,
	FAST_BAYER_GRBG,
} fastBayerPattern_t;

typedef enum{
	FAST_DFPD, FAST_HQLI
} fastDebayerType_t;

typedef enum{
	FAST_XIMEA12
} fastRawFormat_t;

typedef enum{
	FAST_GAUSSIAN_SHARPEN = 1,
	FAST_LUT_8_8,
	FAST_LUT_8_8_C,
	FAST_LUT_12_8,
	FAST_LUT_12_12,
	FAST_MAD,
	FAST_BASE_COLOR_CORRECTION,
	FAST_COLUMN_FILTER
} fastImageFilterType_t;

typedef enum{
	FAST_FLIP,
	FAST_FLOP,
	FAST_ROTATION180,
	FAST_ROTATION90LEFT,
	FAST_ROTATION90RIGHT
} fastAffineType_t;

typedef enum{
	FAST_LANCZOS = 1
} fastResizeType_t;


typedef enum{
    FAST_DEMO, FAST_RELEASE
} fastLicenseType_t;


typedef struct {
	fastLicenseType_t licenseType;
	unsigned short sdkLicenseVersion;
	unsigned short dongleLicenseVersion;
	char dongleName[56];
	char dongleId[8];
} fastLicenseInfo_t;

////////////////////////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////////////////////////
typedef struct fastEncoderHandleStruct *fastJpegEncoderHandle_t;
typedef struct fastDecoderHandleStruct *fastJpegDecoderHandle_t;
typedef struct fastDebayerHandleStruct *fastDebayerHandle_t;
typedef struct fastResizerHandleStruct *fastResizerHandle_t;
typedef struct fastImageFiltersHandleStruct *fastImageFiltersHandle_t;
typedef struct fastCropHandleStruct *fastCropHandle_t;
typedef struct fastGpuTimerStruct *fastGpuTimerHandle_t;
typedef struct fastExportToHostStruct *fastExportToHostHandle_t;
typedef struct fastExportToDeviceStruct *fastExportToDeviceHandle_t;
typedef struct fastImportFromHostStruct *fastImportFromHostHandle_t;
typedef struct fastImportFromDeviceStruct *fastImportFromDeviceHandle_t;
typedef struct fastAffineHandleStruct *fastAffineHandle_t;
typedef struct fastRawUnpackerHandleStruct *fastRawUnpackerHandle_t;
typedef struct fastBayerMergerHandleStruct *fastBayerMergerHandle_t;
typedef struct fastBayerSplitterHandleStruct *fastBayerSplitterHandle_t;

typedef struct fastDeviceSurfaceBufferStruct *fastDeviceSurfaceBufferHandle_t;

typedef struct{
    unsigned char data[64];
} fastQuantTable_t;

typedef struct{
    fastQuantTable_t table[4];
} fastJpegQuantState_t;

typedef struct{
    unsigned char bucket[16];
    unsigned char alphabet[256];
} fastHuffmanTable_t;

typedef struct{
    fastHuffmanTable_t table[2][2];
} fastJpegHuffmanState_t;

typedef struct{
    unsigned   quantTableMask;
    unsigned huffmanTableMask[2];
    unsigned  scanChannelMask;
    unsigned    scanGroupMask;
} fastJpegScanStruct_t;

typedef struct {
   unsigned short exifCode;
   char *exifData;
   int exifLength;
} fastJpegExifSection_t;

typedef struct {
	fastJpegFormat_t jpegFmt;

	unsigned char *h_Bytestream;
	unsigned bytestreamSize;

	unsigned height;
	unsigned width;

	fastJpegExifSection_t *exifSections;
	unsigned exifSectionsCount;

	fastJpegQuantState_t quantState;
	fastJpegHuffmanState_t huffmanState;
	fastJpegScanStruct_t scanMap;
	unsigned restartInterval;
} fastJfifInfo_t;

typedef struct{
	unsigned char lut[256];
} fastLut_8_8_t;

typedef struct{
	unsigned char lut_R[256];
	unsigned char lut_G[256];
	unsigned char lut_B[256];
} fastLut_8_8_C_t;

typedef struct{
	unsigned char lut[4096];
} fastLut_12_8_t;

typedef struct{
	unsigned short lut[4096];
} fastLut_12_12_t;

typedef struct{
	unsigned char *blackShiftMatrix;
	float *correctionMatrix;
} fastMad_t;

typedef struct{
	float matrix[12];
	fastBayerPattern_t bayerPattern;
} fastBaseColorCorrection_t;

typedef struct{
	double sigma;
} fastGaussianFilter_t;

typedef struct {
	int columnOffset;
} fastColumnFilter_t;

///////////////////////////////////////////////////////////////////////////////
// Core encoding/decoding calls
///////////////////////////////////////////////////////////////////////////////
/*! \brief Set GPU device to work with.

	Set GPU device to work with.
	If device is not found or it has not enough compute compatibility
	version, function will return FAST_INVALID_DEVICE in status.

	\param [in] affinity  affinity mask
*/
extern fastStatus_t DLL fastInit(unsigned affinity, bool openGlMode);

/*! \brief Allocate page-locked memory on CPU.

	Allocate page-locked memory on CPU.
	Page-locked memory cannot be moved from RAM to swap file. It
	increases PCI-Express I/O speed of GPU over conventional memory.

	\param [out] buffer  pointer to allocated memory
	\param [in] size  size of allocated memory in bytes
*/
extern fastStatus_t DLL fastMalloc(void **buffer, size_t size);

/*! \brief Free page-locked memory.

	Free page-locked memory.
	It is not necessary to call fastFree for every pointer because all
	device memory will be freed after encoder or decoder will be destroyed.

	\param [in] buffer  pointer to allocated memory
*/
extern fastStatus_t DLL fastFree(void *buffer);

/*! \brief Print to stdout information about detected GPU on PC and its properties.

	Print to stdout information about detected GPU on PC and its properties.
*/
extern fastStatus_t DLL fastPrintMemo();


/*! \brief Create Timer and returns associated handle.
	
	Create Timer and returns associated handle.
	Allocates necessary buffers in GPU memory. 
	In case GPU does not have enough free memory 
	returns FAST_INSUFFICIENT_DEVICE_MEMORY. 

	\param [out] handle pointer to created Timer handle. 
*/
extern fastStatus_t DLL fastGpuTimerCreate(
    fastGpuTimerHandle_t *handle);

/*! \brief Insert start event into GPU stream.

	Insert start event into GPU stream.
	
	\param [in] handle Timer handle pointer.
*/
extern fastStatus_t DLL fastGpuTimerStart(
    fastGpuTimerHandle_t handle);

/*! \brief Insert stop event into GPU stream.

	Insert stop event into GPU stream.
	
	\param [in] handle Timer handle pointer.
*/
extern fastStatus_t DLL fastGpuTimerStop(
    fastGpuTimerHandle_t handle);

/*! \brief Synchronize CPU thread with stop event and calculate time elapsed between start and stop events.

	Synchronize CPU thread with stop event 
	and calculate time elapsed between start and stop events.

	\param [in] handle Timer handle pointer.
	\param [out] elapsed time elapsed.
*/
extern fastStatus_t DLL fastGpuTimerGetTime(
	fastGpuTimerHandle_t handle,
	float *elapsed);

/*! \brief Destroy Timer handle.

	Destroy Timer handle.
	\param [in] handle Timer handle pointer.
*/
extern fastStatus_t DLL fastGpuTimerDestroy(
    fastGpuTimerHandle_t handle);


///////////////////////////////////////////////////////////////////////////////
// JPEG encoding calls
///////////////////////////////////////////////////////////////////////////////
/*! \brief Create JPEG Encoder and returns associated handle.

	Create JPEG Encoder and returns associated handle.
	Function fastCreateEncoderHandle allocates all necessary buffers
	in GPU memory. So in case when GPU does not have enough free memory
	fastCreateEncoderHandle returns FAST_INSUFFICIENT_DEVICE_MEMORY.
	Maximum dimensions of the image are set to encoder during creation. 
	If transformation result exceeds the maximum value error status FAST_INVALID_SIZE will be returned.  

	\param [out] handle  pointer to created JPEG encoder 
	\param [in] surfaceFmt  type of surface (uncompressed image). Surface is input for encoder.
	\param [in] maxSurfaceHeight  maximum image height in pixels
	\param [in] maxSurfaceWidth  maximum image width in pixels
	\param [in] maxSurfacePitch4  maximum width of the surface row in bytes aligned by 4 (int) 
*/
extern fastStatus_t DLL fastJpegEncoderCreate(
    fastJpegEncoderHandle_t *handle,

    fastSurfaceFormat_t surfaceFmt,

    unsigned maxWidth,
    unsigned maxHeight,

	fastDeviceSurfaceBufferHandle_t srcBuffer
);

/*! \brief Get GPU memory size requested for JPEG Encoder.

	Get GPU memory size requested for JPEG Encoder.
	
	\param [in] handle  pointer to JPEG encoder
	\param [out] requestedGpuSizeInBytes  requested GPU memory size in bytes
*/
extern fastStatus_t DLL fastJpegEncoderGetAllocatedGpuMemorySize(
    fastJpegEncoderHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

/*! \brief Encode surface to JPEG.

	Encode surface to JPEG.
	Buffer for JPEG bytestream has to be allocated before call. Its
	recommended size is surfaceHeight* surfacePitch8. Real JPEG bytestream
	size is calculated during compression and put to bytestreamSize. If size
	of h_Bytestream is not enough, procedure returns FAST_INTERNAL_ERROR
	in status. JPEG tables also have to be created before procedure call

	\param [in] handle  pointer to JPEG encoder
	\param [out] h_Bytestream  pointer to buffer for output JPEG bytestream
	\param [out] bytestreamSize  pointer to real size of JPEG bytestream in bytes
	\param [out] quantTableState  pointers to JPEG tables involved in JPEG serialization. Populated during encoding.
	\param [out] huffmanTableStat  pointers to JPEG tables involved in JPEG serialization. Populated during encoding.
	\param [out] scanMap  pointers to JPEG tables involved in JPEG serialization. Populated during encoding.
	\param [in] quality  adjusts output JPEG file size and quality. Quality is an integer value from 1 to 100 where 100 means best quality and maximum image size. 
	\param [in] restartInterval  number of MCUs (Minimum Coded Unit) processed as an independent sequence within a scan
	\param [in] h_Surface  pointer to input image buffer. This pointer can be null when JPEG encoder is created by fastJpegEncoderCreateLinked and use external device buffer for input image.
	\param [in] surfaceHeight height of the image in pixels.
	\param [in] surfaceWidth width of the image in pixels.
	\param [in] surfacePitch4 width of the surface row in bytes aligned by 4 (int).
	\param [in] samplingFmt type of output sampling for JPEG.
*/
extern fastStatus_t DLL fastJpegEncode(
    fastJpegEncoderHandle_t handle,

	unsigned quality,

	fastJfifInfo_t *jfifInfo
);

/*! \brief Destroy JPEG encoder.

	Destroy JPEG encoder.
	Procedure resets device context and frees all device memory. All
	pointers to device memory become invalid after procedure call.

	\param [in] handle  pointer to JPEG encoder 
*/
extern fastStatus_t DLL fastJpegEncoderDestroy(fastJpegEncoderHandle_t handle);

///////////////////////////////////////////////////////////////////////////////
// JPEG decoding calls
///////////////////////////////////////////////////////////////////////////////
/*! \brief Create JPEG Decoder and return associated handle.

	Create JPEG Decoder and return associated handle.
	Function fastCreateDecoderHandle allocates all necessary buffers in GPU memory.
	So in case when GPU does not have enough free memory fastCreateDecoderHandle
	returns FAST_INSUFFICIENT_DEVICE_MEMORY. Maximum dimensions of the image 
	are set to decoder during creation. Thus if  transformation result exceeds 
	the maximum value error status FAST_INVALID_SIZE will be returned.

	\param [out] handle  pointer to created JPEG decoder 
	\param [in] surfaceFmt  type of surface (uncompressed image). Surface is output for decoder
	\param [in] maxSurfaceHeight  maximum image height in pixels
	\param [in] maxSurfaceWidth  maximum image width in pixels
	\param [in] maxSurfacePitch4  maximum width of the surface row in bytes aligned by 4 (int)
*/
extern fastStatus_t DLL fastJpegDecoderCreate(
    fastJpegDecoderHandle_t *handle,

    fastSurfaceFormat_t surfaceFmt,

    unsigned maxWidth,
    unsigned maxHeight,

	fastDeviceSurfaceBufferHandle_t *dstBuffer
);

/*! \brief Get GPU memory size requested for JPEG Decoder.

	Get GPU memory size requested for JPEG Decoder.
	
	\param [in] handle  pointer to JPEG decoder
	\param [out] requestedGpuSizeInBytes  requested GPU memory size in bytes
*/
extern fastStatus_t DLL fastJpegDecoderGetAllocatedGpuMemorySize(
    fastJpegDecoderHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

/*! \brief Decode JPEG to surface.

	Decode JPEG to surface.
	In case of using output surface buffer, it has to be allocated before call.
	Its size is surfaceHeight* surfacePitch4.

	\param [in] handle  pointer to JPEG decoder 
	\param [out] h_Surface  pointer to output surface buffer. This pointer can be null when decoder output buffer is obtained with fastJpegDecoderGetOutputBuffer. In this case function simply doesnÕt copy output device buffer CPU surface.  
	\param [in] surfaceHeight height of the image in pixels
	\param [in] surfaceWidth width of the image in pixels
	\param [in] surfacePitch4 width of the surface row in bytes aligned by 4 (int)
	\param [in] samplingFmt type of output sampling for JPEG
	\param [in] h_Bytestream  pointer to buffer for input JPEG bytestream
	\param [in] bytestreamSize  size of JPEG bytestream in bytes
	\param [in] quantTableState  pointers to JPEG tables. Tables are populated during JPEG loading
	\param [in] huffmanTableStat  pointers to JPEG tables. Tables are populated during JPEG loading
	\param [in] scanMap  pointers to JPEG tables. Tables are populated during JPEG loading
	\param [in] restartInterval  number of MCUs (Minimum Coded Unit) processed as an independent sequence within a scan. Value is calculated during JPEG loading
*/
extern fastStatus_t DLL fastJpegDecode(
    fastJpegDecoderHandle_t handle,

	fastJfifInfo_t *jfifInfo
);

/*! \brief Destroy JPEG decoder.

	Destroy JPEG decoder.
	Procedure resets device context and frees all device memory. All
	pointers to device memory become invalid after procedure call.
	If output surface buffer is obtained by	fastJpegDecoderGetOutputBuffer
	it becomes invalid after this procedure call.

	\param [in] handle  pointer to JPEG decoder 
*/
extern fastStatus_t DLL fastJpegDecoderDestroy(fastJpegDecoderHandle_t handle);

///////////////////////////////////////////////////////////////////////////////
// Debayer calls
///////////////////////////////////////////////////////////////////////////////
/*! \brief Create Debayer and return associated handle.

	Create Debayer and return associated handle.
	Function fastDebayerCreate allocates all necessary buffers
	in GPU memory. So in case when GPU does not have enough free
	memory fastDebayerCreate returns FAST_INSUFFICIENT_DEVICE_MEMORY.
	Maximum dimensions of the image are set to Debayer during creation. 
	Thus if transformation result exceeds the maximum value error status 
	FAST_INVALID_SIZE will be returned.

	\param [out] handle  pointer to created Debayer handle 
	\param [in] debayerType  debayer algorithm
	\param [in] bayerFmt  bayer pattern
	\param [in] surfaceFmt  define a format of output pixel (8-bit or 16-bit)
	\param [in] maxSurfaceHeight maximum image height in pixels.
	\param [in] maxSurfaceWidth maximum image width in pixels.
*/
extern fastStatus_t DLL fastDebayerCreate(
	fastDebayerHandle_t	*handle,

	fastDebayerType_t	debayerType,
	fastBayerPattern_t	bayerFmt,
	fastSurfaceFormat_t	surfaceFmt,

	unsigned maxWidth,
	unsigned maxHeight,

	fastDeviceSurfaceBufferHandle_t  srcBuffer,
	fastDeviceSurfaceBufferHandle_t *dstBuffer
);

/*! \brief Get GPU memory size requested for Debayer.

	Get GPU memory size requested for Debayer.
	
	\param [in] handle  pointer to Debayer
	\param [out] requestedGpuSizeInBytes  requested GPU memory size in bytes
*/
extern fastStatus_t DLL fastDebayerGetAllocatedGpuMemorySize(
    fastDebayerHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

/*! \brief Restore image color.

	Restore image color.
	The Buffer for restored image has to be allocated before procedure
	call. It's recommended to allocate 3 * pitchOutput * height bytes
	for output buffer, where pitchOutput >= width; width and height are
	values in pixels. Parameter pitchInput must be equal or greater than
	input image width. If allocated buffer size is not enough, procedure
	will fail with segmentation fault. FAST_EXECUTION_FAILURE will be
	return in status if the system couldn't to copy data to GPU.

	\param [in] handle  Debayer handle 
	\param [in] input  pointer to input image buffer
	\param [in] pitchInput  width of input image buffer in bytes
	\param [out] output  pointer to restored image
	\param [in] pitchOutput  width of output image buffer in bytes
*/
extern fastStatus_t DLL fastDebayerTransform(
 	fastDebayerHandle_t	handle,

	unsigned width,
    unsigned height
);

/*! \brief Destroy Debayer handle.

	Destroy Debayer handle.
	Procedure frees all device memory.

	\param [in] handle  pointer to Debayer handle
*/
extern fastStatus_t DLL fastDebayerDestroy(fastDebayerHandle_t handle);

///////////////////////////////////////////////////////////////////////////////
// Resizer calls
///////////////////////////////////////////////////////////////////////////////
extern fastStatus_t DLL fastResizerCreate(
	fastResizerHandle_t *handle,
	fastSurfaceFormat_t surfaceFmt,

	unsigned maxSrcWidth,
	unsigned maxSrcHeight,

	unsigned maxDstWidth,
	unsigned maxDstHeight,

	double maxScaleFactor,

	float shiftX,
	float shiftY,

	fastDeviceSurfaceBufferHandle_t  srcBuffer,
	fastDeviceSurfaceBufferHandle_t *dstBuffer
);

/*! \brief Resize the image according to scaleFactor value.

	Image is resized according to scaleFactor and put into dstBuffer 
	(see getResizerOutputBuffer). Output dimensions of the resized image 
	can differ from the realWidth*scaleFactor and realHeight*scaleFactor 
	depending on the resize algorithm therefore output buffer dimensions 
	outputHeight and outputWidth should be returned.	

	\param [in] handle Resizer handle.
	\param [in] resizeType
	\param [in] width input image width in pixels.
	\param [in] height input image height in pixels.
	\param [in] resizedWidth width of the output buffer.
	\param [out] resizedHeight height of the output buffer.
*/
extern fastStatus_t DLL fastResizerTransform(
    fastResizerHandle_t	handle,
	fastResizeType_t resizeType,

 	unsigned width,
	unsigned height,

	unsigned resizedWidth,
	unsigned *resizedHeight
);

/*! \brief Calculate requested GPU memory for Resizer.

	Calculate requested GPU memory for Resizer. 
	Returns requested memory size in bytes for Resizer. 

	\param [in] handle Resizer handle.
	\param [out] requestedGpuSizeInBytes memory size in bytes.
*/
extern fastStatus_t DLL fastResizerGetAllocatedGpuMemorySize(
    fastResizerHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

/*! \brief Destroy Resizer handle.

	Destroy Resizer handle.
	Procedure frees all device memory.
	
	\param [in] handle pointer to Resizer handle.
*/
extern fastStatus_t DLL fastResizerDestroy(fastResizerHandle_t handle);

///////////////////////////////////////////////////////////////////////////////
// Image Filters calls
///////////////////////////////////////////////////////////////////////////////
extern fastStatus_t DLL fastImageFilterCreate(
    fastImageFiltersHandle_t  *handle,
	fastSurfaceFormat_t		   surfaceFmt,

	fastImageFilterType_t filterType,
	void *staticFilterParameters,

	unsigned maxWidth,
	unsigned maxHeight,

	fastDeviceSurfaceBufferHandle_t  srcBuffer,
	fastDeviceSurfaceBufferHandle_t *dstBuffer
);

extern fastStatus_t DLL fastImageFiltersGetAllocatedGpuMemorySize(
    fastImageFiltersHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

extern fastStatus_t DLL fastImageFiltersTransform(
 	fastImageFiltersHandle_t	handle,
	void *filterParameters,

 	unsigned width,
	unsigned height
);

extern fastStatus_t DLL fastImageFiltersDestroy(fastImageFiltersHandle_t handle);

////////////////////////////////////////////////////////////////////////////////
// Crop calls
////////////////////////////////////////////////////////////////////////////////
extern fastStatus_t DLL fastCropCreate(
	fastCropHandle_t *handle,
	fastSurfaceFormat_t surfaceFmt,
	
	unsigned maxSrcWidth,
	unsigned maxSrcHeight,

	unsigned maxDstWidth,
	unsigned maxDstHeight,

	fastDeviceSurfaceBufferHandle_t  srcBuffer,
	fastDeviceSurfaceBufferHandle_t *dstBuffer
);

extern fastStatus_t DLL fastCropGetAllocatedGpuMemorySize(
    fastCropHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

extern fastStatus_t DLL fastCropTransform(
    fastCropHandle_t	handle,
	
  	unsigned width,
	unsigned height,

	unsigned leftTopCoordsX,
	unsigned leftTopCoordsY,
	unsigned croppedWidth,
	unsigned croppedHeight
);

extern fastStatus_t DLL fastCropDestroy(fastCropHandle_t handle);

////////////////////////////////////////////////////////////////////////////////
// Raw Unpacker calls
////////////////////////////////////////////////////////////////////////////////
extern fastStatus_t DLL fastRawUnpackerCreate(
	fastRawUnpackerHandle_t	*handle,

	fastRawFormat_t	rawFmt,
	fastSurfaceFormat_t	surfaceFmt,

	unsigned maxWidth,
	unsigned maxHeight,

	fastDeviceSurfaceBufferHandle_t *dstBuffer
);

extern fastStatus_t DLL fastRawUnpackerGetAllocatedGpuMemorySize(
    fastRawUnpackerHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

extern fastStatus_t DLL fastRawUnpackerDecode(
 	fastRawUnpackerHandle_t	handle,

	void* h_src,
	unsigned width,
    unsigned height
);

extern fastStatus_t DLL fastRawUnpackerDestroy(fastRawUnpackerHandle_t handle);

///////////////////////////////////////////////////////////////////////////////
// BayerMerger calls
///////////////////////////////////////////////////////////////////////////////
extern fastStatus_t DLL fastBayerMergerCreate(
	fastBayerMergerHandle_t *handle,
	fastSurfaceFormat_t surfaceFmt,
	
	unsigned maxDstWidth,
	unsigned maxDstHeight,

	fastDeviceSurfaceBufferHandle_t  srcBuffer,
	fastDeviceSurfaceBufferHandle_t *dstBuffer
);

extern fastStatus_t DLL fastBayerMergerGetAllocatedGpuMemorySize(
    fastBayerMergerHandle_t handle,

	unsigned *requestedGpuSizeInBytes
);

extern fastStatus_t DLL fastBayerMergerMerge(
    fastBayerMergerHandle_t handle,
	
  	unsigned width,
	unsigned height
);

extern fastStatus_t DLL fastBayerMergerDestroy(fastBayerMergerHandle_t handle);

///////////////////////////////////////////////////////////////////////////////
// BayerSplitter calls
///////////////////////////////////////////////////////////////////////////////
extern fastStatus_t DLL fastBayerSplitterCreate(
	fastBayerSplitterHandle_t *handle,
	fastSurfaceFormat_t surfaceFmt,
	
	unsigned maxSrcWidth,
	unsigned maxSrcHeight,

	unsigned *maxDstWidth,
	unsigned *maxDstHeight,

	fastDeviceSurfaceBufferHandle_t  srcBuffer,
	fastDeviceSurfaceBufferHandle_t *dstBuffer
);

extern fastStatus_t DLL fastBayerSplitterGetAllocatedGpuMemorySize(
    fastBayerSplitterHandle_t handle,

	unsigned *requestedGpuSizeInBytes
);

extern fastStatus_t DLL fastBayerSplitterSplit(
    fastBayerSplitterHandle_t handle,

 	unsigned srcWidth,
	unsigned srcHeight,

	unsigned *dstWidth,
	unsigned *dstHeight
);

extern fastStatus_t DLL fastBayerSplitterDestroy(fastBayerSplitterHandle_t handle);

///////////////////////////////////////////////////////////////////////////////
// ImportFromHost calls
///////////////////////////////////////////////////////////////////////////////
extern fastStatus_t DLL fastImportFromHostCreate(
	fastImportFromHostHandle_t *handle,
	fastSurfaceFormat_t surfaceFmt,

	unsigned maxWidth,
	unsigned maxHeight,

	fastDeviceSurfaceBufferHandle_t *dstBuffer
);

extern fastStatus_t DLL fastImportFromHostCopy(
	fastImportFromHostHandle_t handle,

	void* h_src,
	unsigned width,
	unsigned pitch,
	unsigned height
);

extern fastStatus_t DLL fastImportFromHostGetAllocatedGpuMemorySize(
    fastImportFromHostHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

extern fastStatus_t DLL fastImportFromHostDestroy(fastImportFromHostHandle_t handle);

///////////////////////////////////////////////////////////////////////////////
// ExportToHost interface
///////////////////////////////////////////////////////////////////////////////
fastStatus_t DLL fastExportToHostCreate(
	fastExportToHostHandle_t *handle,
	fastSurfaceFormat_t surfaceFmt,

	fastDeviceSurfaceBufferHandle_t srcBuffer
);

fastStatus_t DLL fastExportToHostCopy(
	fastExportToHostHandle_t handle,
	void* h_dst,

	unsigned width,
	unsigned pitch,
	unsigned height
);

fastStatus_t DLL fastExportToHostGetAllocatedGpuMemorySize(
	fastExportToHostHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

fastStatus_t DLL fastExportToHostDestroy(fastExportToHostHandle_t handle);

///////////////////////////////////////////////////////////////////////////////
// ImportFromDevice calls
///////////////////////////////////////////////////////////////////////////////
extern fastStatus_t DLL fastImportFromDeviceCreate(
	fastImportFromDeviceHandle_t *handle,
	fastSurfaceFormat_t surfaceFmt,

	unsigned maxWidth,
	unsigned maxHeight,

	fastDeviceSurfaceBufferHandle_t *dstBuffer
);

extern fastStatus_t DLL fastImportFromDeviceCopy(
	fastImportFromDeviceHandle_t handle,

	void* d_src,
	unsigned width,
	unsigned pitch,
	unsigned height
);

extern fastStatus_t DLL fastImportFromDeviceGetAllocatedGpuMemorySize(
    fastImportFromDeviceHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

extern fastStatus_t DLL fastImportFromDeviceDestroy(fastImportFromDeviceHandle_t handle);

///////////////////////////////////////////////////////////////////////////////
// ExportToDevice interface
///////////////////////////////////////////////////////////////////////////////
fastStatus_t DLL fastExportToDeviceCreate(
	fastExportToDeviceHandle_t *handle,
	fastSurfaceFormat_t surfaceFmt,

	fastDeviceSurfaceBufferHandle_t srcBuffer
);

fastStatus_t DLL fastExportToDeviceCopy(
	fastExportToDeviceHandle_t handle,
	void* d_dst,

	unsigned width,
	unsigned pitch,
	unsigned height
);

fastStatus_t DLL fastExportToDeviceGetAllocatedGpuMemorySize(
	fastExportToDeviceHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

fastStatus_t DLL fastExportToDeviceDestroy(fastExportToDeviceHandle_t handle);

///////////////////////////////////////////////////////////////////////////////
// Affine calls
///////////////////////////////////////////////////////////////////////////////
extern fastStatus_t DLL fastAffineCreate(
    fastAffineHandle_t  *handle,
	fastSurfaceFormat_t		   surfaceFmt,

	fastAffineType_t affineType,

	unsigned maxWidth,
	unsigned maxHeight,

	fastDeviceSurfaceBufferHandle_t  srcBuffer,
	fastDeviceSurfaceBufferHandle_t *dstBuffer
);

extern fastStatus_t DLL fastAffineGetAllocatedGpuMemorySize(
    fastAffineHandle_t	handle,

	unsigned *requestedGpuSizeInBytes
);

extern fastStatus_t DLL fastAffineTransform(
 	fastAffineHandle_t	handle,

 	unsigned width,
	unsigned height
);

extern fastStatus_t DLL fastAffineDestroy(fastAffineHandle_t handle);

////////////////////////////////////////////////////////////////////////////////
// File handling
////////////////////////////////////////////////////////////////////////////////
/*! \brief Serialize JPEG bytestream to file.

	Serialize JPEG bytestream to file.

	\param [in] filename  path to JPEG file
	\param [in] h_Bytestream  pointer to storing JPEG bytestream
	\param [in] bytestreamSize  size of JPEG bytestream in bytes
	\param [in] quantTableState  pointers to JPEG tables
	\param [in] huffmanTableStat  pointers to JPEG tables
	\param [in] scanMap  pointers to JPEG tables
	\param [in] surfaceHeight  height of JPEG image in pixels
	\param [in] surfaceWidth  width of JPEG image in pixels
	\param [in] samplingFmt  type of sampling of JPEG image
	\param [in] restartInterval  restart interval for JPEG image
*/
extern fastStatus_t DLL fastJfifStoreToFile(
    const char *filename,
    fastJfifInfo_t *jfifInfo
);

/*! \brief Serialize JPEG bytestream to buffer in memory.

	Serialize JPEG bytestream to buffer in memory.
	Buffer for serialization has to be allocated before call. Its
	size is surfaceHeight* surfacePitch4

	\param [in] h_Bytestream  pointer to storing JPEG bytestream
	\param [in] bytestreamSize  size of JPEG bytestream in bytes
	\param [out] outputStream  pointer to buffer for serialization JPEG format. Buffer is allocated by customer application.
	\param [inout] outputStreamSize  size of buffer for serialization JPEG format in bytes
	\param [in] quantTableState  pointers to JPEG tables
	\param [in] huffmanTableStat  pointers to JPEG tables
	\param [in] scanMap  pointers to JPEG tables
	\param [in] surfaceHeight  height of JPEG image in pixels
	\param [in] surfaceWidth  width of JPEG image in pixels
	\param [in] samplingFmt  type of sampling of JPEG image
	\param [in] restartInterval  restart interval for JPEG image
*/
extern fastStatus_t DLL fastJfifStoreToMemory(
    unsigned char *outputStream,
	unsigned *outputStreamSize,

    fastJfifInfo_t *jfifInfo
);

/*! \brief Load JPEG image from disk to memory.

	Load JPEG image from disk to memory.
	If JPEG file is not found, procedure returns FAST_UNSUPPORTED_FORMAT.
	If some errors occur during file parsing, function returns
	FAST_INVALID_FORMAT and put to stderr description of error. If function
	cannot allocate memory on host for JPEG bytestream it returns
	FAST_INSUFFICIENT_HOST_MEMORY.

	\param [in] filename  path to JPEG file
	\param [out] h_Bytestream  pointer to buffer for loading JPEG bytestream. Buffer is allocated by the procedure
	\param [out] bytestreamSize  pointer to size of JPEG bytestream in bytes
	\param [out] quantTableState  pointers to JPEG tables
	\param [out] huffmanTableStat  pointers to JPEG tables
	\param [out] scanMap  pointers to JPEG tables
	\param [out] surfaceHeight  height of JPEG image in pixels
	\param [out] surfaceWidth  width of JPEG image in pixels
	\param [out] samplingFmt  type of sampling of JPEG image
	\param [out] restartInterval  restart interval for JPEG image
*/
extern fastStatus_t DLL fastJfifLoadFromFile(
    const char *filename,

    fastJfifInfo_t *jfifInfo
);

/*! \brief Load JPEG image from buffer in memory.

	Load JPEG image from buffer in memory.
	If some errors occur during parsing file, function returns FAST_INVALID_FORMAT
	and put to stderr description of error. If function cannot allocate memory on
	host for JPEG bytestream it returns FAST_INSUFFICIENT_HOST_MEMORY.

	\param [in] inputStream  path to JPEG file
	
	\param [in] inputStream  pointer to buffer with JPEG file 
	\param [in] inputStreamSize  size of buffer with JPEG file in byte
	\param [out] h_Bytestream  pointer to buffer for loading JPEG bytestream. Buffer is allocated by the procedure
	\param [out] bytestreamSize  pointer to size of JPEG bytestream in bytes
	\param [out] quantTableState  pointers to JPEG tables
	\param [out] huffmanTableStat  pointers to JPEG tables
	\param [out] scanMap  pointers to JPEG tables
	\param [out] surfaceHeight  height of JPEG image in pixels
	\param [out] surfaceWidth  width of JPEG image in pixels
	\param [out] samplingFmt  type of sampling of JPEG image
	\param [out] restartInterval  restart interval for JPEG image
*/
extern fastStatus_t DLL fastJfifLoadFromMemory(
 	const unsigned char	*inputStream,
	unsigned inputStreamSize,

 	fastJfifInfo_t *jfifInfo
);

////////////////////////////////////////////////////////////////////////////////
// License Info
////////////////////////////////////////////////////////////////////////////////

extern fastStatus_t DLL fastLicenseInfo(
    fastLicenseInfo_t *licenseInfo
);

#ifdef __cplusplus
}
#endif



#endif
