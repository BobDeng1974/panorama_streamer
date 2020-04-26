#pragma once


#include <QImage>


#include <iostream>

#include "ImageBuffer.h"

namespace D360
{

	class Capture
	{
	public:

		Capture()
		{
			isSnapshot = false;
			init();
		}
		virtual ~Capture() { close(); }

		virtual bool   open(int index);
		virtual bool   start()
		{
			return true;
		}
		virtual void   close();
		virtual double getProperty(int);
		virtual bool   setProperty(int, double);
		virtual void   reset(ImageBufferData& frame)
		{

		}

		virtual bool grabFrame(ImageBufferData& frame);
		virtual bool retrieveFrame(int, ImageBufferData& frame);
		virtual void* retrieveAudioFrame();

		void setSnapshotPath(QString path)
		{
			m_snapshot = path;
		}
		void snapshot() { isSnapshot = true; }

		void setCurFrame(float curFrame)
		{
			m_curFrame = curFrame;
		}

		float getCurFrame()
		{
			return m_curFrame;
		}

		enum CaptureDomain
		{
			CAPTURE_XIMEA = 0,
			CAPTURE_DSHOW = 1,
			CAPTURE_VIDEO = 2,
			CAPTURE_FILE  = 3,
			CAPTURE_OPENCV = 4
		};
		virtual int getCaptureDomain()
		{
			return m_captureDomain;
		}
		virtual int msToWait()
		{
			return 0;
		}

		enum IncomingFrameType
		{
			None,
			Video,
			Audio
		};

		virtual IncomingFrameType getIncomingType()
		{
			return m_incomingType;
		}

		virtual ImageBufferData convertToRGB888(ImageBufferData& image) {
			return image;
		}

	protected:
		QString m_snapshot;
		CaptureDomain m_captureDomain;
		IncomingFrameType m_incomingType;

		float m_curFrame;

		void init();
		void errMsg(const char* msg, int errNum);

		int  getBpp();

		bool isSnapshot;
	};

}