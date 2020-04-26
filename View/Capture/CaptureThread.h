
#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H


#include <thread>

// Qt
#include <QPixmap>
#include <QtCore/QTime>
#include <QtCore/QThread>

// Local
#include "CaptureXimea.h"
#include "CaptureImageFile.h"
#include "CaptureDShow.h"

#include "SharedImageBuffer.h"
#include "Config.h"
#include "Structures.h"

#include "AudioThread.h"
#include "StreamProcess.h"

class ImageBuffer;

class CaptureThread : public IAudioThread
{
    Q_OBJECT

    public:
		enum CaptureType {
			CAPTUREFILE = 0,
			CAPTUREXIMEA = 1,
			CAPTUREDSHOW = 2,
			CAPTUREVIDEO = 3,
		};
        CaptureThread(SharedImageBuffer *sharedImageBuffer, int deviceNumber, CaptureType capType,  int width, int height );
		~CaptureThread();

        void	stop();
		void	playAndPause(bool isPause);
        bool	connect(int dupIndex = 0);
        bool	disconnect();
        bool	isConnected();
        int		getInputSourceWidth();
        int		getInputSourceHeight();

		virtual AudioInput * getMic();

		void setFrameRate( float fps )
		{
			m_captureFPS = fps;
		}

		void setExposure( float exp )
		{
			m_captureExp = exp;
		}

		void setGain( float gain )
		{
			m_captureGain = gain;
		}

		void setAcqMode( CaptureXimea::CaptureMode capMode )
		{

		}

		void snapshot(bool isCalibreate = false);

		void waitForFinish();

		bool isFinished();

    private:
        void updateFPS( int );

		QString m_Name;
        SharedImageBuffer *sharedImageBuffer;
        //VideoCapture cap;
		D360::Capture* cap;
		StreamProcess* m_streamer;

		CaptureType m_captureType;

        SharedImageBuffer::ImageDataPtr m_grabbedFrame;
        QTime t;
        QMutex doStopMutex;
		QMutex doPauseMutex;
        QQueue<int> fps;

		QMutex finishMutex;
		QWaitCondition finishWC;
		bool m_finished;
		bool m_isCanGrab;

        struct ThreadStatisticsData statsData;
        bool doStop;		
		bool doPause;
		bool doSnapshot;
		bool doCalibrate;
        int captureTime;
        int sampleNumber;
        int fpsSum;
        int m_deviceNumber;
        int width;
        int height;

		float m_captureExp;
		float m_captureGain;
		float m_captureFPS;

    protected:
        void run();

	public slots:
			void process();

    signals:
        void updateStatisticsInGUI( struct ThreadStatisticsData );
		void newFrame( const QPixmap &frame, int frameNum );
		void finished(int type, QString msg, int id);
		void started(int type, QString msg, int id);
		void report(int type, QString msg, int id);
		void snapshoted(int id);
};

#endif // CAPTURETHREAD_H
