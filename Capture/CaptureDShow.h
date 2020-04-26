#pragma once

#include <memory>

#include <QImage>

#include "Capture.h"
#include "CaptureAudio.h"
#include "Structures.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/mem.h>
#include <libswscale/swscale.h>
}

#include <iostream>

/**********************************************************************************/

class CaptureDShow : public D360::Capture, public AudioInput
{
public:
	CaptureDShow()
	{
		m_captureDomain = D360::Capture::CAPTURE_DSHOW;
		init();
		m_Name = "CaptureDShow";
	}
	virtual ~CaptureDShow()
	{
		close();
	}

	bool				open(int index, QString name, float fps, bool isDShow, int dupIndex = 0);
	virtual void		close();
	virtual double		getProperty(int);
	virtual bool		grabFrame(ImageBufferData& frame);
	virtual bool		retrieveFrame(int channel, ImageBufferData& frame);
	virtual void*		retrieveAudioFrame();
	void				setAudio(CameraInput::InputAudioChannelType audioType);
	virtual ImageBufferData convertToRGB888(ImageBufferData& image);

private:
	void init();
	int decode_packet(AVPacket pkt, int *got_frame);

	int m_nCameraIndex;

	AVCodecContext* m_pVideoCodecCtx;
	AVFormatContext* m_pFormatCtx;
	int m_nVideoStream;
	int m_nAudioStream;
	SwsContext * img_convert_ctx;

	AVFrame* m_pVideoFrame;
	AVFrame* m_pFrameRGB;
	uint8_t* m_pFrameBuffer;

	QString m_cDeviceName;
	QString m_aDeviceName;

	int m_nWidth;
	int m_nHeight;

	float m_fps;
	bool m_isAudioDisabled;
	bool m_isLeft;
	bool m_isRight;
	QString m_Name;
};
