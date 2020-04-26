/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __MESSAGES_H__
#define __MESSAGES_H__

static const char *mainHelp =
	"JpegSample project for Fastvideo Image SDK - Copyright 2013 Fastvideo\n" \
	"www.fastcompression.com\n" \
	"Compiled for Windows-7/8 [x86/x64]\n" \
	"\n" \
	"This software is prepared for non-commercial use only. It is free for personal and educational (including non-profit organization) use. Distribution of this software without any permission from Fastvideo is NOT allowed. NO warranty and responsibility is provided by the authors for the consequences of using it.\n" \
	"\n" \
	"Hardware Requirements\n" \
	" - NVIDIA GPU 400, 500, 600 or 700 series with Compute Capability >= 2.0, NVIDIA drivers 310.90 or later\n" \
	"For the latest NVIDIA drivers visit http://www.nvidia.com/Download/index.aspx\n" \
	"\n" \
	"The JpegSample demonstrates JPEG decoder and encoder functionality.\n"\
	"\n" \
	"Supported parameters\n" \
	" -i <file> - input file in BMP/PGM/PPM format for encoding, JPG for decoding\n" \
	" -o <file> - output file name or mask in JPG format for encoding, BMP/PGM/PPM for decoding\n" \
	" -q <quality> - quality setting according to standard. Default is 75.\n" \
	" -s <subsampling> - subsampling 444, 422 or 420 for color images. Default is 444.\n" \
	" -r <restart interval> - number of MCUs between restart markers. Default is 32.\n" \
	" -d <device ID> - GPU device ID. Default is 0.\n" \
	" -info - time / performance output is on. Default is off.\n" \
	" -float - set encoder or decoder operation mode to float.\n" \
	" -black_shift <unsigned byte> - black shift values.\n."\
	" -matrixA <file> - file with intensity correction values for each image pixel. File is in PFM format.\n"\
	" -matrixB <file> - file with black shift values for each image pixel. File is in PGM format.\n"\
	" -if <folder + mask> - folder path and mask for input file. BMP/PGM/PPM format for encoding and JPG for decoding\n" \
	" -maxWidth <unsigned int> - set maximum image width for multiple files processing.\n" \
	" -maxHeight <unsigned int> - set maximum image height for multiple files processing.\n" \
	"\n" \
	"Command line for JpegSample:\n" \
	"JpegSample.exe -i <input image> -o <output image> -s <subsampling> -q <quality> -r <restart interval> -d <device ID> -info\n" \
	"    -black_shift <unsigned byte> -matrixA <file> -matrixB <file>\n" \
	"\n" \
	"Example of command line for JPEG encoding:\n" \
	"JpegSample.exe -i test.bmp -o final.jpg -s 444 -q 75 -r 32 -d 0 -info\n" \
	"\n" \
	"Example of command line for JPEG decoding:\n" \
	"JpegSample.exe -i test.jpg -o final.bmp -d 0 -info\n"
	"\n" \
	"Example of grayscale image encoding with pixel correction:\n" \
	"JpegSample.exe -i test.pgm -o final.jpg -s 444 -q 75 -r 32 -black_shift 0.0 -matrixA intensity.pfm -matrixB noise.pgm\n";


#endif
