#include "WriterFfmpeg.hpp"

AVPixelFormat WriterFfmpeg::GetPixelFormat(fastJpegFormat_t samplingFmt) {
	if ( samplingFmt == JPEG_420 )
		return AV_PIX_FMT_YUVJ420P;
	if ( samplingFmt == JPEG_422 )
		return AV_PIX_FMT_YUVJ422P;
	return AV_PIX_FMT_YUVJ444P;
}

fastStatus_t WriterFfmpeg::Initialize(
	fastJpegFormat_t samplingFmt,

	std::string fileName,
	int imageWidth,
	int imageHeight,
	int frameRate
) {
	std::string smallFileName = std::string(fileName);
	unsigned found = smallFileName.find_last_of("/\\");
	if ( found != std::string::npos ) {
		smallFileName = smallFileName.substr(found+1);
	}

	avformat_alloc_output_context2(&formatContext_, NULL, NULL, smallFileName.c_str());
	if( !formatContext_ ) {
		return FAST_INVALID_VALUE;
	}

	outputFormat_ = formatContext_->oformat;
	sprintf(formatContext_->filename, "%s", smallFileName.c_str());

	AVCodec *codec = 0;
	if(outputFormat_->video_codec != AV_CODEC_ID_NONE) {
		AVCodecID codec_id = AV_CODEC_ID_MJPEG;
 		codec = avcodec_find_encoder(codec_id);
 		if( !codec ) {
 			return FAST_INVALID_VALUE;
 		}

		videoStream_ = avformat_new_stream(formatContext_, codec);
		if( !videoStream_ ) {
			return FAST_INVALID_VALUE;
		}

		videoStream_->id = formatContext_->nb_streams-1;
		codecContext_ = videoStream_->codec;

		avcodec_get_context_defaults3(codecContext_, codec);
		codecContext_->codec_id = codec_id;

		codecContext_->bit_rate = 400000;

		codecContext_->qmin = 1;
		codecContext_->qmax = 1;

		codecContext_->width  = (imageWidth  / 32) * 32;
		codecContext_->height = (imageHeight / 16) * 16;

		codecContext_->time_base.den = frameRate;
		codecContext_->time_base.num = 1;
		codecContext_->gop_size      = 12;

		codecContext_->pix_fmt = GetPixelFormat(samplingFmt);

		if (codecContext_->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
			codecContext_->max_b_frames = 2;
		}
		if (codecContext_->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
			codecContext_->mb_decision = 2;
		}
   
		if (formatContext_->oformat->flags & AVFMT_GLOBALHEADER)
			codecContext_->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

	if (avcodec_open2(codecContext_, codec, NULL) < 0) {
		return FAST_INVALID_VALUE;
	}

	if (!(outputFormat_->flags & AVFMT_NOFILE)) {
		if (avio_open(&formatContext_->pb, fileName.c_str(), AVIO_FLAG_WRITE) < 0) {
			return FAST_INVALID_VALUE;
		}
	}

	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];
 		
	time (&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "%d-%m-%Y %I:%M:%S", timeinfo);

	av_dict_set(&formatContext_->metadata, "creation_time", buffer, 0);
	av_dict_set(&formatContext_->metadata, "date", buffer, 0);
	av_dict_set(&formatContext_->metadata, "filename", smallFileName.c_str(), 0);

	if ( avformat_write_header(formatContext_, 0) != 0 ) {
		return FAST_INVALID_VALUE;
	}

	return FAST_OK;
};

fastStatus_t WriterFfmpeg::Close() {
	int got_output = 1;
	while( got_output != 0 ) {
		if( avcodec_encode_video2(codecContext_, &packet_, NULL, &got_output) < 0 )
			break;
		if( got_output ) {
			if( av_interleaved_write_frame(formatContext_, &packet_) < 0 ) {
				av_free_packet(&packet_);
				break;
			}
			av_free_packet(&packet_);
		}
	}

	if( totalFrames_ != 0 )
		av_write_trailer(formatContext_);

	if( videoStream_ )
		avcodec_close(videoStream_->codec);

	for(unsigned int i = 0; i < formatContext_->nb_streams; i++) {
		av_freep(&formatContext_->streams[i]->codec);
		av_freep(&formatContext_->streams[i]);
	}

	if (!(outputFormat_->flags & AVFMT_NOFILE)) {
		avio_close(formatContext_->pb);
	}

	if( formatContext_ != 0 ) {
		av_free(formatContext_);
		formatContext_ = 0;
	}

	return FAST_OK;
}

fastStatus_t WriterFfmpeg::WriteFrame(
	unsigned char *buffer,
	int bufferSize
) {
	if( buffer == NULL || bufferSize <= 0 || codecContext_->codec_id != AV_CODEC_ID_MJPEG )
		return FAST_INVALID_VALUE;
    
	av_init_packet(&packet_);
	if (codecContext_->coded_frame->pts != AV_NOPTS_VALUE)
		packet_.pts = av_rescale_q(codecContext_->coded_frame->pts, codecContext_->time_base, videoStream_->time_base);

	codecContext_->coded_frame->key_frame = true;

	packet_.flags |= AV_PKT_FLAG_KEY;
	packet_.stream_index = videoStream_->index;
         
	packet_.data = buffer;
	packet_.size = bufferSize;
         
	if( av_interleaved_write_frame(formatContext_, &packet_) != 0 ) {
		return FAST_INVALID_VALUE;
	}

	av_free_packet(&packet_);
	totalFrames_++;
	return FAST_OK;
}
