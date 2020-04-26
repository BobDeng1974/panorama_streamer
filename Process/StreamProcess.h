#ifndef STREAMPROCESS_H
#define STREAMPROCESS_H

#include <QObject>
#include <QTime>
#include <QThread>
#include <QMutex>
#include <QQueue>

#include <Structures.h>
#include <Buffer.h>

#include "StreamFfmpeg.hpp"
#include "SharedImageBuffer.h"

class StreamProcess : public QObject
{
	Q_OBJECT

public:
	StreamProcess(SharedImageBuffer* sharedImageBuffer, bool toFile, QObject* parent = NULL);
	~StreamProcess();

	bool	initialize(bool toFile, QString outFileName, int width, int height, int fps, 
		int channelCount = 0, AVSampleFormat sampleFmt = AV_SAMPLE_FMT_S16, 
		int sampleRate = 0, int audioLag = 0, 
		int videoCodec = (int)AV_CODEC_ID_H264, int audioCodec = (int)AV_CODEC_ID_AAC, int crf = 23);
	void	stopStreamThread();
	void	waitForFinish();
	bool	isFinished();
	bool	disconnect();

	void	playAndPause(bool isPause);
	bool	isOpened() { return m_isOpened; }

private:
	void updateFPS(int);
	void setMain(QObject* main) { m_Main = main; }

	QString m_Name;
	QTime t;
	QMutex doExitMutex;
	QMutex finishMutex;
	QWaitCondition finishWC;
	bool m_finished;

	QQueue<int> fps;

	struct ThreadStatisticsData statsData;
	bool m_isOpened;
	bool doExit;
	int captureTime;
	int sampleNumber;
	int fpsSum;

	QThread* streamingThreadInstance;

	int m_audioInputCount;
	int m_videoInputCount;
	int m_audioProcessedCount;
	int m_videoProcessedCount;
	StreamFfmpeg m_Stream;
	//RawImagePtr m_Panorama;
	void * m_audioFrame;
	QMap<int, void*>	m_audioFrames;
	QMap<int, void*>	m_audioReadyFrames;
	int m_audioChannelCount;
	unsigned char* m_Panorama;
	int m_width;
	int m_height;

	bool m_isFile;
	SharedImageBuffer* m_sharedImageBuffer;
	QMutex videoFrameMutex;
	QMutex audioFrameMutex;		

	QMutex doPauseMutex;
	bool doPause;

	QObject* m_Main;

protected:
	void run();

	public slots:
	void process();
	void streamPanorama(unsigned char* panorama);
	void streamAudio(int devNum, void* audioFrame);

signals:
	void updateStatisticsInGUI(struct ThreadStatisticsData);
	//void newFrame(const QPixmap &frame, int frameNum);
	void finished(int type, QString msg, int id);
	void started(int type, QString msg, int id);
};

#endif // STREAMPROCESS_H