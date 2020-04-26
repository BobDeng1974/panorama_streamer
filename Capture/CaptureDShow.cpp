#include "CaptureDShow.h"
#include <QDebug>
#include "define.h"
#include "CaptureProp.h"

void CaptureDShow::init()
{
	m_cDeviceName = "Camera";
	m_aDeviceName = "";

	m_pVideoCodecCtx = 0;
	m_pVideoFrame = 0;
	m_pAudioFrame = 0;
	m_pTransferAudioFrame = 0;
	m_pFrameRGB = 0;
	m_pFrameBuffer = 0;
	img_convert_ctx = 0;

	m_pAudioCodecCtx = 0;

	m_nWidth = 0;
	m_nHeight = 0;
	m_isAudioDisabled = false;
	m_isLeft = true;
	m_isRight = true;

	av_register_all();
	avdevice_register_all();
	avcodec_register_all();
	m_pFormatCtx = avformat_alloc_context();

	return;
}

static int open_codec_context(int *stream_idx,
	AVFormatContext *fmt_ctx, enum AVMediaType type)
{
	int ret, stream_index;
	AVStream *st;
	AVCodecContext *dec_ctx = NULL;
	AVCodec *dec = NULL;
	AVDictionary *opts = NULL;

	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not find %s stream from device\n",
			av_get_media_type_string(type));
		return ret;
	}
	else {
		stream_index = ret;
		st = fmt_ctx->streams[stream_index];

		/* find decoder for the stream */
		dec_ctx = st->codec;
		dec = avcodec_find_decoder(dec_ctx->codec_id);
		if (!dec) {
			fprintf(stderr, "Failed to find %s codec\n",
				av_get_media_type_string(type));
			return AVERROR(EINVAL);
		}

		/* Init the decoders, with or without reference counting */
		//av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
		if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
			fprintf(stderr, "Failed to open %s codec\n",
				av_get_media_type_string(type));
			return ret;
		}
		*stream_idx = stream_index;
	}

	return 0;
}

bool CaptureDShow::open(int index, QString name, float fps, bool isDShow, int dupIndex)
{
	m_nCameraIndex = index;
	m_fps = fps;
	// ffmpeg -list_devices true -f dshow -i dummy
	//ffmpeg -f dshow -list_options true -i video = "USB2.0 Camera"
	QString cDeviceName;
	QString fullDName;
	AVInputFormat *fmt = NULL;
	if (isDShow)
	{
		fullDName = cDeviceName.prepend("video=") + ":" + name;
		fmt = av_find_input_format("dshow");
	}

	else
		fullDName = name;
	PANO_LOG("Path of DirectShow device: " + fullDName);

	int ret, i;
	m_nVideoStream = -1;
	m_nAudioStream = -1;

	AVDictionary        *tOptions = NULL;
	char fpsString[10];
	sprintf(fpsString, "%d", (int)fps);
	av_dict_set(&tOptions, "framerate", fpsString, 0);
	//av_dict_set(&tOptions, "rtbufsize", "100000000", 0);
	av_dict_set(&tOptions, "video_device_number", QString::number(dupIndex).toLocal8Bit().data(), 0);
	//framerate

	if (avformat_open_input(&m_pFormatCtx, fullDName.toUtf8().data(), fmt, &tOptions) < 0)
	{
		fprintf(stderr, "Could not open input file '%s'", fullDName.toUtf8().data());
		return false;
	}
	if ((ret = avformat_find_stream_info(m_pFormatCtx, 0)) < 0)
	{
		fprintf(stderr, "Failed to retrieve input stream information");
		return false;
	}

	if (open_codec_context(&m_nVideoStream, m_pFormatCtx, AVMEDIA_TYPE_VIDEO) >= 0)
	{
		m_pVideoCodecCtx = m_pFormatCtx->streams[m_nVideoStream]->codec;

		m_nWidth = m_pVideoCodecCtx->width;
		m_nHeight = m_pVideoCodecCtx->height;

		m_pVideoFrame = av_frame_alloc();
		m_pAudioFrame = av_frame_alloc();
		m_pFrameRGB = av_frame_alloc();
		m_pTransferAudioFrame = av_frame_alloc();

		int num_bytes = avpicture_get_size(AV_PIX_FMT_RGB24, m_nWidth, m_nHeight);
		m_pFrameBuffer = (uint8_t *)av_malloc(num_bytes);
		avpicture_fill((AVPicture*)m_pFrameRGB, m_pFrameBuffer, AV_PIX_FMT_RGB24, m_nWidth, m_nHeight);

		img_convert_ctx = sws_getCachedContext(NULL, m_pVideoCodecCtx->width, m_pVideoCodecCtx->height, m_pVideoCodecCtx->pix_fmt, m_pVideoCodecCtx->width, m_pVideoCodecCtx->height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}
	else
		return false;

	if (open_codec_context(&m_nAudioStream, m_pFormatCtx, AVMEDIA_TYPE_AUDIO) >= 0)
	{
		m_pAudioCodecCtx = m_pFormatCtx->streams[m_nAudioStream]->codec;
	}

	av_log_set_level(AV_LOG_QUIET);
	//av_log_set_level(AV_LOG_DEBUG);
	av_dump_format(m_pFormatCtx, 0, fullDName.toUtf8().data(), 0);

	//av_log_set_level(AV_LOG_QUIET);

	return true;
}

int CaptureDShow::decode_packet(AVPacket pkt, int *got_frame)
{
	int ret = 0;
	int decoded = pkt.size;

	*got_frame = 0;

	if (pkt.stream_index == m_nVideoStream) {
		av_frame_unref(m_pVideoFrame);
		/* decode video frame */
		ret = avcodec_decode_video2(m_pVideoCodecCtx, m_pVideoFrame, got_frame, &pkt);
		if (ret < 0) {
			//fprintf(stderr, "Error decoding video frame (%s)\n", av_err2str(ret));
			return ret;
		}

		if (*got_frame) {
			m_incomingType = D360::Capture::Video;
		}
	}
	else if (pkt.stream_index == m_nAudioStream) {
		/* decode audio frame */
		ret = avcodec_decode_audio4(m_pAudioCodecCtx, m_pAudioFrame, got_frame, &pkt);
		if (ret < 0) {
			//fprintf(stderr, "Error decoding audio frame (%s)\n", av_err2str(ret));
			return ret;
		}
		/* Some audio decoders decode only part of the packet, and have to be
		* called again with the remainder of the packet data.
		* Sample: fate-suite/lossless-audio/luckynight-partial.shn
		* Also, some decoders might over-read the packet. */
		decoded = FFMIN(ret, pkt.size);

		if (*got_frame && m_isAudioDisabled == false) {
			m_incomingType = D360::Capture::Audio;
		}
	}

	return decoded;
}

bool CaptureDShow::grabFrame(ImageBufferData& frame)
{
	int res;
	int frameFinished = 0;
	AVPacket pkt;
	pkt.size = 0;

	int ret = 0;
	int got_frame = 0;

	do
	{
		if (pkt.size == 0)
		{
			pkt.stream_index = -1;
			ret = av_read_frame(m_pFormatCtx, &pkt);
		}

		if (pkt.size == 0) // file read complete
		{
			m_incomingType = D360::Capture::None;
			frameFinished = 1;
			break;
		}

		got_frame = 0;
		ret = decode_packet(pkt, &got_frame);
		if (ret < 0)
			break;
		pkt.data += ret;
		pkt.size -= ret;
		if (pkt.size == 0)
		{
			av_free_packet(&pkt);
		}
	} while (got_frame == 0);

	return frameFinished != 1;
}

bool CaptureDShow::retrieveFrame(int channel, ImageBufferData& frame)
{
	if (m_incomingType == IncomingFrameType::Video)
	{
		if (m_pVideoCodecCtx->pix_fmt == AV_PIX_FMT_YUV420P || m_pVideoCodecCtx->pix_fmt == AV_PIX_FMT_YUVJ420P)
		{
			// yuv420 --> color buffer

			// ==== width ====
			//	 Y		   Y
			//   Y		   Y
			//   U         V

			AVPicture* pict = ((AVPicture*)m_pVideoFrame);

			frame.mFormat = ImageBufferData::YUV420;

			frame.mImageY.buffer = pict->data[0];
			frame.mImageY.width = m_pVideoFrame->width;
			frame.mImageY.height = m_pVideoFrame->height;
			frame.mImageY.stride = pict->linesize[0];

			frame.mImageU.buffer = pict->data[1];
			frame.mImageU.width = m_pVideoFrame->width / 2;
			frame.mImageU.height = m_pVideoFrame->height / 2;
			frame.mImageU.stride = pict->linesize[1];

			frame.mImageV.buffer = pict->data[2];
			frame.mImageV.width = m_pVideoFrame->width / 2;
			frame.mImageV.height = m_pVideoFrame->height / 2;
			frame.mImageV.stride = pict->linesize[2];
		}
		else if (m_pVideoCodecCtx->pix_fmt == AV_PIX_FMT_YUYV422)
		{
			// yuv422 --> color buffer

			// ==== width ====
			//	 Y		   Y
			//   Y		   Y
			//   U         V
			//   U		   V

			AVPicture* pict = ((AVPicture*)m_pVideoFrame);

			frame.mFormat = ImageBufferData::YUV422;

			frame.mImageY.buffer = pict->data[0];
			frame.mImageY.width = m_pVideoFrame->width;
			frame.mImageY.height = m_pVideoFrame->height;
			frame.mImageY.stride = pict->linesize[0];
			frame.mImageU = AlignedImage();
			frame.mImageV = AlignedImage();
		}
		else
		{
			sws_scale(img_convert_ctx, ((AVPicture*)m_pVideoFrame)->data, ((AVPicture*)m_pVideoFrame)->linesize, 0, m_pVideoCodecCtx->height, ((AVPicture *)m_pFrameRGB)->data, ((AVPicture *)m_pFrameRGB)->linesize);
			//QImage snapshotImage(m_pFrameRGB->data[0], m_pVideoFrame->width, m_pVideoFrame->height, QImage::Format::Format_RGB888);
			frame.mFormat = ImageBufferData::RGB888;
			frame.mImageY.buffer = m_pFrameRGB->data[0];
			frame.mImageY.width = m_pVideoFrame->width;
			frame.mImageY.height = m_pVideoFrame->height;
			frame.mImageY.stride = m_pVideoFrame->width * 3;
			frame.mImageU = AlignedImage();
			frame.mImageV = AlignedImage();
		}

		m_incomingType = IncomingFrameType::None;
#if 0	// Original snapshot function
		if (isSnapshot) // screen capture
		{
			sws_scale(img_convert_ctx, ((AVPicture*)m_pVideoFrame)->data, ((AVPicture*)m_pVideoFrame)->linesize, 0, m_pVideoCodecCtx->height, ((AVPicture *)m_pFrameRGB)->data, ((AVPicture *)m_pFrameRGB)->linesize);
			QImage snapshotImage(m_pFrameRGB->data[0], m_pVideoFrame->width, m_pVideoFrame->height, QImage::Format::Format_RGB888);

			snapshotImage.save(m_snapshot + QString("%1.png").arg(m_nCameraIndex));

			isSnapshot = false;
		}
#endif
		return true;
	}

	return false;
}

ImageBufferData CaptureDShow::convertToRGB888(ImageBufferData& image)
{
	SwsContext* convert_ctx = NULL;
	AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;
	const uint8_t* inData[8] = {NULL};
	int inLineSize[8] = {0};

	switch (image.mFormat)
	{
	case ImageBufferData::YUV420: 
		pix_fmt = AV_PIX_FMT_YUV420P; 
		inData[0] = image.mImageY.buffer;
		inLineSize[0] = image.mImageY.stride;
		inData[1] = image.mImageU.buffer;
		inLineSize[1] = image.mImageU.stride;
		inData[2] = image.mImageV.buffer;
		inLineSize[2] = image.mImageV.stride;
		break;
	case ImageBufferData::YUV422: 
		pix_fmt = AV_PIX_FMT_YUV422P; 
		inData[0] = image.mImageY.buffer;
		inLineSize[0] = image.mImageY.stride;
		break;
	default:	// ImageBufferData::RBB888;
		return image;
	}
	
	AVFrame* frameRGB = av_frame_alloc();
	int num_bytes = avpicture_get_size(AV_PIX_FMT_RGB24, image.mImageY.width, image.mImageY.height);

	uint8_t* frameBuffer = (uint8_t *)av_malloc(num_bytes);
	avpicture_fill((AVPicture*)frameRGB, frameBuffer, AV_PIX_FMT_RGB24, image.mImageY.width, image.mImageY.height);
	convert_ctx = sws_getCachedContext(NULL, image.mImageY.width, image.mImageY.height,
		pix_fmt, image.mImageY.width, image.mImageY.height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);

	sws_scale(convert_ctx, inData, inLineSize, 0, image.mImageY.height, frameRGB->data, frameRGB->linesize);

	ImageBufferData outImage(ImageBufferData::RGB888);
	outImage.mImageY.buffer = frameRGB->data[0];
	outImage.mImageY.width = image.mImageY.width;
	outImage.mImageY.height = image.mImageY.height;
	outImage.mImageY.stride = frameRGB->linesize[0];

	sws_freeContext(convert_ctx);
	convert_ctx = 0;
	av_frame_free(&frameRGB);

	return outImage;
}

void* CaptureDShow::retrieveAudioFrame()
{
	if (m_incomingType != IncomingFrameType::Audio)		return NULL;

	//m_pTransferAudioFrame = m_pAudioFrame;
	m_pTransferAudioFrame->format = m_pAudioFrame->format;
	m_pTransferAudioFrame->sample_rate = m_pAudioFrame->sample_rate;
	m_pTransferAudioFrame->nb_samples = m_pAudioFrame->nb_samples;
	if (m_pAudioFrame->channels == 0)
		m_pTransferAudioFrame->channels = av_get_channel_layout_nb_channels(m_pAudioFrame->channel_layout);
	m_pTransferAudioFrame->channels = m_isLeft & m_isRight ? m_pTransferAudioFrame->channels : 1;
	m_pTransferAudioFrame->channel_layout = av_get_default_channel_layout(m_pTransferAudioFrame->channels);

	av_frame_get_buffer(m_pTransferAudioFrame, 0);
	if (m_isLeft & m_isRight) {
		memcpy(m_pTransferAudioFrame->extended_data[0], m_pAudioFrame->extended_data[0], m_pAudioFrame->nb_samples * av_get_bytes_per_sample(m_pAudioCodecCtx->sample_fmt));
		memcpy(m_pTransferAudioFrame->extended_data[1], m_pAudioFrame->extended_data[1], m_pAudioFrame->nb_samples * av_get_bytes_per_sample(m_pAudioCodecCtx->sample_fmt));
	}
	else if (m_isLeft)
		memcpy(m_pTransferAudioFrame->extended_data[0], m_pAudioFrame->extended_data[0], m_pAudioFrame->nb_samples * av_get_bytes_per_sample(m_pAudioCodecCtx->sample_fmt));
	else if (m_isRight)
		memcpy(m_pTransferAudioFrame->extended_data[0], m_pAudioFrame->extended_data[1], m_pAudioFrame->nb_samples * av_get_bytes_per_sample(m_pAudioCodecCtx->sample_fmt));

	m_incomingType = IncomingFrameType::None;

	return m_pTransferAudioFrame;
}

void CaptureDShow::close()
{
	//avcodec_close(m_pCodecCtx);
	if (m_pVideoFrame) av_frame_free(&m_pVideoFrame);
	if (m_pAudioFrame) av_frame_free(&m_pAudioFrame);
	//if (m_pFrameRGB) av_frame_free(&m_pFrameRGB);
	//if (m_pFrameBuffer)	av_freep(m_pFrameBuffer);
	if (m_pTransferAudioFrame) av_frame_free(&m_pTransferAudioFrame);
	if (img_convert_ctx)
	{
		sws_freeContext(img_convert_ctx);
		img_convert_ctx = 0;
	}
	if (m_pFormatCtx)
	{
		avformat_close_input(&m_pFormatCtx);
	}
}

double CaptureDShow::getProperty(int property_id)
{
	if (m_pVideoCodecCtx == NULL)
		return 0;

	int ival = 0;
	float fval = 0;

	switch (property_id)
	{
		// OCV parameters
	case CV_CAP_PROP_FRAME_WIDTH: return m_pVideoCodecCtx->width;
	case CV_CAP_PROP_FRAME_HEIGHT: return m_pVideoCodecCtx->height;
	}

	return 0.0f;
}

// -1:Disabled, 0:Mixed, 1:Left, 2:Right
void CaptureDShow::setAudio(CameraInput::InputAudioChannelType audioType)
{
	if (audioType == CameraInput::NoAudio) {	// Disabled
		m_isAudioDisabled = true;
		m_isLeft = false;	
		m_isRight = false;
		return;
	}

	if (audioType == CameraInput::MixedChannel) {	// Mixed
		m_isLeft = true;
		m_isRight = false;
	}
	else if (audioType == CameraInput::LeftChannel) {	// Left
		m_isLeft = true;
		m_isRight = false;
	}
	else if (audioType == CameraInput::RightChannel) {	// Right
		m_isLeft = false;
		m_isRight = true;
	}
}