/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef __BASE_FFMPEG__
#define __BASE_FFMPEG__

#include <string>

#include "fastvideo_sdk.h"

extern "C" {
#include "libavformat/avformat.h"
}

class BaseFfmpeg
{
protected:
	AVFormatContext *formatContext_;
	AVCodecContext  *codecContext_;
	AVOutputFormat  *outputFormat_;
	AVStream        *videoStream_;

	AVPacket packet_;

	int totalFrames_;

	BaseFfmpeg() {
		avcodec_register_all();
		av_register_all();
		av_log_set_level(AV_LOG_QUIET);

		totalFrames_  = 0;

		outputFormat_ = 0;
		formatContext_  = 0;
		codecContext_   = 0;
		videoStream_ = 0;
	};

	~BaseFfmpeg(void) { };
};

#endif // __BASE_FFMPEG__