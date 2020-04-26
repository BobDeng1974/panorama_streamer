#pragma once

//
// Local
//
#include "CaptureThread.h"
#include "Structures.h"
#include "D360Stitcher.h"
#include "SharedImageBuffer.h"



class CameraModule : public QObject
{
	Q_OBJECT

public:
	explicit CameraModule(int deviceNumber, SharedImageBuffer *sharedImageBuffer, QObject* main = NULL);
	~CameraModule();

	void startThreads();

	virtual bool connectToCamera(int width, int height, float exp, float fps, int audioDevIndex = -1, int dupIndex = 0);
	void snapshot(bool isCalibrate = false);
	IAudioThread * getAudioThread();
	CaptureThread * getCaptureThread();
	bool isConnected() { return isCameraConnected; }

protected:

#ifndef REMOVE_PROCESSING_THREAD_IN_CAMERAVIEW	/* [Updated by B] */		
	ProcessingThread *processingThread;
#endif

	QObject* m_Main;
	CaptureThread    *captureThread;
	AudioThread		*audioThread;
	AudioInput		*mic;

	SharedImageBuffer *sharedImageBuffer;

	void stopCaptureThread();
	void stopProcessingThread();


	int m_deviceNumber;
	bool isCameraConnected;
	QString m_Name;

	QThread* captureThreadInstance;
	QThread* processingThreadInstance;

	void qquit();

signals:
	void newImageProcessingFlags(struct ImageProcessingFlags imageProcessingFlags);
};