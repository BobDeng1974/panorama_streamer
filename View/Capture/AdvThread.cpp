

#include <iomanip>
#include <sstream>

#include <QFile>
#include <QDir>
#include "SharedImageBuffer.h"
#include "AdvThread.h"
#include "CaptureProp.h"
#include "D360Stitcher.h"
#include "D360Parser.h"
#include "QmlMainWindow.h"
#include "define.h"
#include <QDateTime>

extern QmlMainWindow* g_mainWindow;

AdvThread::AdvThread(SharedImageBuffer *sharedImageBuffer, QString fileName,
	int  width, int height, float capFPS) : sharedImageBuffer(sharedImageBuffer),
	m_Name("AdvThread"),
	m_fileName(fileName),
	m_grabbedFrame(ImageBufferData::NONE)
{
	//
	// Save passed parameters
	//
	m_width = width;
	m_height = height;

	//
	// Initialize variables(s)
	//
	doStop = false;
	doPause = false;
	fpsSum = 0;

	fps.clear();

	cap = NULL;

	statsData.averageFPS = 0;
	statsData.nFramesProcessed = 0;
	statsData.nAudioFrames = 0;

	m_isCanGrab = false;
	m_finished = false;
	m_deviceNumber = -1;
	m_captureFPS = capFPS;
}

AdvThread::~AdvThread()
{
	disconnect();
}

void AdvThread::run()
{
	m_grabbedFrame.clear();
	cap->start();
	cap->reset(m_grabbedFrame);

	int intervalms = 1000 / m_captureFPS;
	float fps = m_captureFPS; // full speed
	bool bIsFirstFrame = true;
	t.start();
	int elapsedTime = 0;

	while (true)
	{
		if (QThread::currentThread()->isInterruptionRequested())
		{
			std::cout << "Got signal to terminate" << std::endl;
			doStop = true;
		}
		//
		// Stop thread if doStop = TRUE 
		//
		doStopMutex.lock();
		if (doStop)
		{
			std::cout << "Stop" << std::endl;
			doStop = false;
			doStopMutex.unlock();
			break;
		}
		doStopMutex.unlock();

		//
		// Pause thread if doPause = TRUE 
		//
		doPauseMutex.lock();
		if (doPause)
		{
			doPauseMutex.unlock();
			QThread::msleep(intervalms);
			continue;
		}
		doPauseMutex.unlock();
		//sharedImageBuffer->syncForVideoProcessing(statsData.nFramesProcessed);

		// Capture frame (if available)
		while (cap->getIncomingType() != D360::Capture::IncomingFrameType::Video)
		{
			if (!cap->grabFrame(m_grabbedFrame))
			{
				m_isCanGrab = false;
				break;
			}

			if (cap->getIncomingType() != D360::Capture::IncomingFrameType::Audio)
				cap->retrieveAudioFrame();
		}

		if (m_isCanGrab == false) {
			break;
		}

		if (cap->getIncomingType() == D360::Capture::IncomingFrameType::Video)
		{
			cap->retrieveFrame(0, m_grabbedFrame);
		}
		else {
			QThread::msleep(intervalms);
			continue;
		}

		statsData.nFramesProcessed++;

		std::shared_ptr< D360Sticher> stitcher = sharedImageBuffer->getStitcher();
		if (stitcher)
			stitcher->updateAdvFrameMat(m_grabbedFrame, statsData.nFramesProcessed);

		// Pause if first frame is captured
		if (bIsFirstFrame && statsData.nFramesProcessed >= 2) {
			playAndPause(true);
			bIsFirstFrame = false;

			sharedImageBuffer->getStitcher()->reStitch();
		}

		elapsedTime = t.elapsed();
		t.restart();

		if (elapsedTime < intervalms)
			QThread::msleep(intervalms - elapsedTime);
	}
	
	PANO_LOG("CaptureAdv thread running is finished...");
}

void AdvThread::finishedThread()
{
	if (m_isCanGrab == false) {
		PANO_DEVICE_N_WARN("Reading frame has been finished.");
		emit report(THREAD_TYPE_CAPTURE, "EOF", m_deviceNumber);
	}
	else
	{
		PANO_DLOG("Emit finished signal");
		emit finished(THREAD_TYPE_CAPTURE, "", m_deviceNumber);
	}
	finishWC.wakeAll();
	m_finished = true;
}

void AdvThread::startedThread()
{
	sharedImageBuffer->getStitcher()->reStitch();
}

bool AdvThread::connect()
{
	GlobalAnimSettings* gasettings = sharedImageBuffer->getGlobalAnimSettings();

	CaptureDShow* capture = new CaptureDShow;

	//if (!capture->open(m_deviceNumber, cameraName, gasettings->m_fps))
	if (!capture->open(m_deviceNumber, m_fileName, m_captureFPS, false))
	{
		std::cout << "Can't Open Camera" << std::endl;
		emit report(THREAD_TYPE_CAPTURE, "Can't Open advertisement video file!", m_deviceNumber);
		delete capture;
		return false;
	}

	//
	// Set resolution
	//
	if (m_width != -1)
		capture->setProperty(CV_CAP_PROP_FRAME_WIDTH, m_width);
	else
	{
		m_width = capture->getProperty(CV_CAP_PROP_FRAME_WIDTH);
	}
	if (m_height != -1)
	{
		capture->setProperty(CV_CAP_PROP_FRAME_HEIGHT, m_height);
	}
	else
	{
		m_height = capture->getProperty(CV_CAP_PROP_FRAME_HEIGHT);
	}

	PANO_DEVICE_LOG(QString("Video resolution (Width: %1, Height: %2)").ARGN(m_width).ARGN(m_height));

	cap = capture;
	m_isCanGrab = true;

	return true;
}


bool AdvThread::disconnect()
{
	//
	// Camera is connected
	//
	if (cap)
	{
		PANO_DEVICE_LOG("Disconnecting Camera...");
		//
		// Disconnect camera
		//
		delete cap;
		cap = 0;
		return true;
	}
	//
	// Camera is NOT connected
	//
	else
		return false;
}

void AdvThread::stop()
{
	QMutexLocker locker(&doStopMutex);
	doStop = true;
}

void AdvThread::playAndPause(bool isPause)
{
	QMutexLocker locker(&doPauseMutex);
	doPause = isPause;
}

bool AdvThread::isConnected()
{
	return !cap;
}

int AdvThread::getInputSourceWidth()
{
	return cap->getProperty(CV_CAP_PROP_FRAME_WIDTH);
}

int AdvThread::getInputSourceHeight()
{
	return cap->getProperty(CV_CAP_PROP_FRAME_HEIGHT);
}

void AdvThread::waitForFinish()
{
	finishMutex.lock();
	finishWC.wait(&finishMutex);
	finishMutex.unlock();
}

bool AdvThread::isFinished()
{
	return m_finished;
}