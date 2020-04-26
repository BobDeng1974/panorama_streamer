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
	"DebayerSample project for Fastvideo Image SDK - Copyright 2013 Fastvideo\n" \
	"www.fastcompression.com\n" \
	"Compiled for Windows-7/8 [x86/x64]\n" \
	"\n" \
	"This software is prepared for non-commercial use only. It is free for personal and educational (including non-profit organization) use. Distribution of this software without any permission from Fastvideo is NOT allowed. NO warranty and responsibility is provided by the authors for the consequences of using it.\n" \
	"\n" \
	"Hardware Requirements\n" \
	" - NVIDIA GPU 400, 500, 600 or 700 series with Compute Capability >= 2.0, NVIDIA drivers 310.90 or later\n" \
	"For the latest NVIDIA drivers visit http://www.nvidia.com/Download/index.aspx\n" \
	"\n" \
	"The DebayerSample demonstrates demosaic functionality.\n"\
	"\n" \
	"Supported parameters\n" \
	" -i <file> - input file in PGM or RAW format.\n" \
	" -o <file> - output file or mask in BMP/PPM format.\n" \
	" -type <demosaic type> - demosaic type (DFPD, HQLI).\n"\
	" -pattern <pattern> - bayer pattern (RGGB, BGGR, GBRG, GRBG).\n"\
	" -d <device ID> - GPU device ID. Default is 0.\n" \
	" -info - time / performance output is on. Default is off.\n" \
	" -w <width> - input file width in pixels. RAW files only.\n" \
	" -h <height> - input file height. RAW files only.\n" \
	" -bits <bits count> - bits count per pixel (12 or 8 bits). RAW files only.\n" \
	" -black_shift <unsigned byte> - black shift values.\n."\
	" -wb_R <float> -wb_G <float> -wb_B <float> - white balance correction coefficients for R, G, B channels respectively.\n"\
	" -matrixA <file> - file with intensity correction values for each image pixel. File is in PFM format.\n"\
	" -matrixB <file> - file with black shift values for each image pixel. File is in PGM format.\n"\
	" -lut <file> - text file with LUT table for pixel transformation.\n" \
	" -if <folder + mask> - folder path and mask for input file. Extension should be PGM or RAW.\n" \
	" -maxWidth <unsigned int> - set maximum image width for multiple files processing.\n" \
	" -maxHeight <unsigned int> - set maximum image height for multiple files processing.\n" \
	"\n" \
	"Command line for Debayer:\n" \
	"DebayerSample.exe -i <input image> -o <output image> -p <pattern> -type <demosaic type> -d <device ID> -info \n"\
	"	-black_shift <unsigned byte> -wb_R <float> -wb_G <float> -wb_B <float> -matrixA <file> -matrixB <file>\n" \
	"	-lut <file> -w <width> -h <height> -bits <bits count>\n" \
	"\n" \
	"Example of command line for color restore:\n" \
	"DebayerSample.exe -i test.pgm -o final.bmp -pattern RGGB -d 0 -info\n"\
	 "\n" \
	"Example of command line for color restore with pixel transform:\n" \
	"DebayerSample.exe -i test.pgm -o final.bmp -pattern RGGB -black_shift 0.0 -wb_R 1.0 -wb_G 1.0 -wb_B 1.0 -matrixA intensity.pfm -matrixB noise.pgm\n"\
	 "\n" \
	"Example of command line for color restore of 12 bit RAW image:\n" \
	"DebayerSample.exe -i Calendar.1920x1080.RGGB.raw -w 1920 -h 1080 -bits 12 -o final.bmp -type DFPD -pattern RGGB -info\n";


#endif
