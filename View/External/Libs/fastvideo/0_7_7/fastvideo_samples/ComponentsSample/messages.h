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
	"ComponentsSample project for Fastvideo Image SDK - Copyright 2014 Fastvideo\n" \
	"www.fastcompression.com\n" \
	"Compiled for Windows-7/8 [x86/x64]\n" \
	"\n" \
	"This software is prepared for non-commercial use only. It is free for personal and educational (including non-profit organization) use. Distribution of this software without any permission from Fastvideo is NOT allowed. NO warranty and responsibility is provided by the authors for the consequences of using it.\n" \
	"\n" \
	"Hardware Requirements\n" \
	" - NVIDIA GPU 400, 500, 600 or 700 series with Compute Capability >= 2.0, NVIDIA drivers 310.90 or later\n" \
	"For the latest NVIDIA drivers visit http://www.nvidia.com/Download/index.aspx\n" \
	"\n" \
	"The ComponentsSample demonstrates base color correction and lut for color image.\n"
	"\n" \
	"Supported parameters\n" \
	" -i <file> - input file in PPM formats\n" \
	" -o <file> - output file in PPM format\n" \
	" -d <device ID> - GPU device ID. Default is 0.\n" \
	" -info - time / performance output is on. Default is off.\n" \
	" -maxWidth <unsigned int> - set maximum image width for multiple files processing.\n" \
	" -maxHeight <unsigned int> - set maximum image height for multiple files processing.\n" \
	" -baseColorCorrection - enable base color correction.\n" \
	" -colorCorrection {} - base color correction coefficients.\n" \
	" -lut8c - enable LUT for color image.\n" \
	" -lut_r <LUT file> - LUT file for R channel.\n."\
	" -lut_g <LUT file> - LUT file for G channel.\n"\
	" -lut_b <LUT file> - LUT file for B channel.\n"\
	" -affine <affine transform> - affine transform (can be flip, flop, rotate180, rotate90left, rotate90right).\n"\
	" -if <folder + mask> - folder path and mask for input file. Extension should be PGM or RAW.\n" \
	
	"\n" \
	"Command line for ComponentsSample:\n" \
	"ComponentsSample.exe -i <input image> -o <output image> -baseColorCorrection -colorCorrection {} -d <device ID> -info\n" \
	"\n" \
	"Example of command line for base color correction:\n" \
	"ComponentsSample.exe -i test.ppm -o final.ppm -baseColorCorrection -colorCorrection {1,0,0,0,0,1,0,0,0,0,1,0} -info\n"\
	 "\n" \
	"Example of command line for LUT:\n" \
	"ComponentsSample.exe -i test.RGGB.raw -lut8c -lut_r lutR.txt -lut_g lutG.txt -lut_b lutB.txt -info\n" \
	"\n" \
	"Example of command line for affine transform:\n" \
	"ComponentsSample.exe -i test.ppm -o final.ppm -affine flip -info\n";


#endif
