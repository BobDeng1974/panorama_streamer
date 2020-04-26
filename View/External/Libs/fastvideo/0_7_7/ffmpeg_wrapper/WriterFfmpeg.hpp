/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __WRITER_FFMPEG__
#define __WRITER_FFMPEG__

#include "BaseFfmpeg.hpp"

class WriterFfmpeg : public virtual BaseFfmpeg
{
private:
	AVPixelFormat GetPixelFormat(fastJpegFormat_t samplingFmt);

public:
	WriterFfmpeg()  { };
	~WriterFfmpeg() { };

	fastStatus_t Initialize(
		fastJpegFormat_t samplingFmt,

		std::string fileName,
		int imageWidth,
		int imageHeight,
		int frameRate
	);

	fastStatus_t WriteFrame(
		unsigned char *buffer,
		int bufferSize
	);

	fastStatus_t Close();
};

#endif // __WRITER_FFMPEG__