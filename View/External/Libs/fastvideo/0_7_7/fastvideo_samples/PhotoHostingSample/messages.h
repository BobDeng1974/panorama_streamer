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
	"PhotoHostingSample project for Fastvideo Image SDK - Copyright 2013 Fastvideo\n" \
	"www.fastcompression.com\n" \
	"Compiled for Windows-7/8 [x86/x64]\n" \
	"\n" \
	"This software is prepared for non-commercial use only. It is free for personal and educational (including non-profit organization) use. Distribution of this software without any permission from Fastvideo is NOT allowed. NO warranty and responsibility is provided by the authors for the consequences of using it.\n" \
	"\n" \
	"Hardware Requirements\n" \
	" - NVIDIA GPU 400, 500, 600 or 700 series with Compute Capability >= 2.0, NVIDIA drivers 310.90 or later\n" \
	"For the latest NVIDIA drivers visit http://www.nvidia.com/Download/index.aspx\n" \
	"\n" \
	"The PhotoHostingSample demonstrates JPEG image resize process for single file and folder.\n"
	"\n" \
	"Supported parameters\n" \
	" -i <file> - input file in JPG format.\n" \
	" -o <file> - output file name in JPG format for single file. Or output folder path with mask for multiple files.\n" \
	" -outputWidth <new width> - new image width in pixels. Default is the same as the input.\n" \
	" -q <unsigned int> - JPEG encoder quality.\n" \
	" -d <device ID> - GPU device ID. Default is 0.\n" \
	" -s <subsampling> - subsampling 444, 422 or 420 for color images. Default is 444.\n" \
	" -r <restart interval> - number of MCUs between restart markers. Default is 16.\n" \
	" -info - time / performance output is on. Default is off.\n" \
	" -if <folder + mask> - folder path and mask for input file. Extension should be JPG.\n" \
	" -maxWidth <unsigned int> - set maximum image width for multiple files processing.\n" \
	" -maxHeight <unsigned int> - set maximum image height for multiple files processing.\n" \
	" -sharp_before <sigma> - sharpen correction before resize algorithm enabled. Default is off.\n" \
	" -sharp_after <sigma> - sharpen correction after resize algorithm enabled. Default is off.\n" \
	"\n" \
	"Command line for PhotoHostingSample:\n" \
	"PhotoHostingSample.exe -i <input image> -o <output image> -outputWidth <new width> -q <unsigned int> -s <subsampling> -r <restart interval> -d <device ID> -info\n"\
	"\n" \
	"Example of command line for PhotoHostingSample:\n" \
	"PhotoHostingSample.exe -i test2048.jpg -i final1024.jpg -q 85 -outputWidth 1024 -q 90 -r 4 -s 444 -info\n"\
	"\n" \
	"Example of command line for PhotoHostingSample with folder processing:\n" \
	"PhotoHostingSample.exe -if ./Images/*.jpg -o ./*.512.jpg -outputWidth 512 -maxWidth 2048 -maxHeight 1565 -q 90 -r 4 -s 444 -info";
#endif
