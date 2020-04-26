
#ifndef AdvThread_H
#define AdvThread_H


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

#include "StreamProcess.h"

class ImageBuffer;

class AdvThread : public QThread
{
    Q_OBJECT
public:
	enum CaptureType {
		CAPTUREFILE = 0,
		CAPTUREXIMEA = 1,
		CAPTUREDSHOW = 2,
		CAPTUREVIDEO = 3,
	};
    AdvThread(SharedImageBuffer *sharedImageBuffer, QString fileName, int width, int height, float fps);
	~AdvThread();

    void	stop();
	void	playAndPause(bool isPause);
    bool	connect();
    bool	disconnect();
    bool	isConnected();
    int		getInputSourceWidth();
    int		getInputSourceHeight();

	void waitForFinish();
	bool isFinished();

private:
	QString m_Name;
	QString m_fileName;
    SharedImageBuffer *sharedImageBuffer;
	D360::Capture* cap;

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
    int fpsSum;
    int m_deviceNumber;
    int m_width;
    int m_height;
	float m_captureFPS;

protected:
    void run();

public slots:
	void finishedThread();
	void startedThread();

signals:
	void newFrame( const QPixmap &frame, int frameNum );
	void finished(int type, QString msg, int id);
	void started(int type, QString msg, int id);
	void report(int type, QString msg, int id);
};

#endif // AdvThread_H
