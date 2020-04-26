/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __READER_FFMPEG__
#define __READER_FFMPEG__

#include "BaseFfmpeg.hpp"

class ReaderFfmpeg : public virtual BaseFfmpeg
{
private:
	int videoStreamIdx_;
	int width_;
	int height_;

public:
	ReaderFfmpeg()  { };
	~ReaderFfmpeg() { };

	fastStatus_t Initialize(
		std::string fileName
	);

	fastStatus_t GetNextFrame(
		unsigned char **buffer,
		unsigned *bufferSize
	);

	fastStatus_t Close();

	int GetTotalFrames() { return totalFrames_; };
	int GetWidth() { return width_; };
	int GetHeight() { return height_; };
};

#endif // __READER_FFMPEG__