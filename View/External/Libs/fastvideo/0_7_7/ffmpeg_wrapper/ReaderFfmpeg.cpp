#include "ReaderFfmpeg.hpp"

fastStatus_t ReaderFfmpeg::Initialize(
	std::string fileName
) {
	if( avformat_open_input(&formatContext_, fileName.c_str(), NULL, NULL) != 0 ) {
		return FAST_INVALID_VALUE;
	}

	if( avformat_find_stream_info( formatContext_, NULL ) < 0 ) {
		return FAST_INVALID_VALUE;
	}

	videoStreamIdx_ = -1;
	for(size_t i=0; i < formatContext_->nb_streams; i++) {
		if(formatContext_->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStreamIdx_ = int(i);
			break;
		}
	}
	if( videoStreamIdx_ == -1 ) {
		return FAST_INVALID_VALUE;
	}

	AVStream *stream = formatContext_->streams[videoStreamIdx_];
	codecContext_ = stream->codec;
   
	width_ = codecContext_->width;
	height_ = codecContext_->height;

	AVCodec *codec = avcodec_find_decoder(codecContext_->codec_id);
	if( codec == NULL ) {
		return FAST_INVALID_VALUE;
	}

	if( avcodec_open2(codecContext_, codec, NULL)<0 ) {
		return FAST_INVALID_VALUE;
	}

	totalFrames_ = ( stream->nb_frames > 0 ) ? stream->nb_frames : -1;

	return FAST_OK;
}

fastStatus_t ReaderFfmpeg::Close() {
	if( codecContext_ ) {
		avcodec_close(codecContext_);
		codecContext_ = 0;
	}

	if( formatContext_ ) {
		avformat_close_input(&formatContext_);
		formatContext_ = 0;
	}

	return FAST_OK;
}

fastStatus_t ReaderFfmpeg::GetNextFrame(
	unsigned char **buffer,
	unsigned *bufferSize
) {
	if(formatContext_ == 0)
		return FAST_INVALID_VALUE;

	if ( av_read_frame(formatContext_, &packet_) < 0 )
		return FAST_INVALID_VALUE;

	*buffer = packet_.data;
	*bufferSize = packet_.size;

	return FAST_OK;
}