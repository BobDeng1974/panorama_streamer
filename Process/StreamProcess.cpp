#include "StreamProcess.h"
#include "define.h"
#include "Config.h"
#include "TLogger.h"

#include <QQmlApplicationEngine>

extern QThread* g_mainThread;

StreamProcess::StreamProcess(SharedImageBuffer *sharedImageBuffer, bool toFile, QObject* main) 
:streamingThreadInstance(0),
m_isFile(toFile)
,m_Name("StreamProcess")
, m_Main(NULL)
{
	if (main)		setMain(main);
	//
	// Initialize variables(s)
	//
	doExit = false;
	doPause = false;
	sampleNumber = 0;
	fpsSum = 0;

	m_isOpened = false;
	m_audioInputCount = 0;
	m_audioProcessedCount = 0;
	m_videoInputCount = 0;
	m_videoProcessedCount = 0;
	m_audioFrame = 0;

	fps.clear();

	statsData.averageFPS = 0;
	statsData.nFramesProcessed = 0;

	qRegisterMetaType<MatBufferPtr>("MatBufferPtr");
	qRegisterMetaType<RawImagePtr>("RawImagePtr");

	m_sharedImageBuffer = sharedImageBuffer;
	m_audioChannelCount = 0;
}

StreamProcess::~StreamProcess()
{
	//stopStreamThread();
}

void StreamProcess::run()
{
	int frameNum = 0;

	t.start();
	int delay = 1000 / m_sharedImageBuffer->getGlobalAnimSettings()->m_fps;
	int continueCount = 0;
	int continueDelay = 1;
	captureTime = 0;

	while (1)
	{
		if (QThread::currentThread()->isInterruptionRequested())
		{
			std::cout << "Got signal to terminate" << std::endl;
			doExit = true;
		}
		//
		// Stop thread if doExit = TRUE 
		//
		doExitMutex.lock();
		if (doExit)
		{
			std::cout << "Stop" << std::endl;
			doExit = false;
			doExitMutex.unlock();
			break;
		}
		doExitMutex.unlock();	

		//
		// Pause thread if doPause = TRUE 
		//
		doPauseMutex.lock();
		if (doPause)
		{
			doPauseMutex.unlock();
			Sleep(delay);
			continue;
		}
		doPauseMutex.unlock();

		//Sleep(1);

		// Start timer (used to calculate capture rate)
		//t.start();

		audioFrameMutex.lock();
		QMap<int, void*> audioFrames = m_audioReadyFrames;
		audioFrameMutex.unlock();
		if (m_audioInputCount > m_audioProcessedCount)
		{
			if (m_audioInputCount > m_audioProcessedCount + 1)
			{
				std::cout << "a"; // audio frame skipped
			}
			m_audioProcessedCount = m_audioInputCount;
			//audioFrameMutex.lock();
			if (m_sharedImageBuffer->getGlobalAnimSettings()->m_captureType == D360::Capture::CaptureDomain::CAPTURE_DSHOW)
				m_Stream.StoreAudioFrame(audioFrames.first());
			else
				m_Stream.StoreAudioFrame(audioFrames);
			//audioFrameMutex.unlock();
			m_sharedImageBuffer->wakeForAudioProcessing(m_audioProcessedCount - 1);
		}

		// Streaming for exporting the AUDIO to the file
		m_Stream.StreamAudio();

		videoFrameMutex.lock();
		if (m_videoInputCount <= m_videoProcessedCount || m_Panorama == NULL)
		{
			videoFrameMutex.unlock();
			continueCount++;
			QThread::msleep(continueDelay);
			continue;
		}
		else if (m_videoInputCount > m_videoProcessedCount + 1)
		{
			std::cout << " "; // video frame skipped
		}
		m_videoProcessedCount = m_videoInputCount;

		if (captureTime + continueDelay * continueCount < delay)
			QThread::msleep(delay - captureTime - continueDelay * continueCount);

		continueCount = 0;

		// Capture frame (if available)
		// Streaming for exporting the VIDEO to the file
		m_Stream.StreamFrame(m_Panorama, statsData.nFramesProcessed, m_width, m_height);
		videoFrameMutex.unlock();
		std::cout << "-"; // video frame processed

		statsData.nFramesProcessed++;
		
		//std::cout << "Grabbed Frame " << m_grabbedFrame << " - data " << std::endl;
		//
		// Save capture time
		//
		captureTime = t.elapsed();
		//PANO_LOG(QString("Streaming time: %1, %2").ARGN(captureTime).arg(CUR_TIME));
		t.restart();

		//
		// Update statistics
		//
		updateFPS(captureTime);


		//
		// Inform GUI of updated statistics
		//
		if (statsData.nFramesProcessed % 10 == 0)
		{
			emit updateStatisticsInGUI(statsData);
		}
		if (statsData.nFramesProcessed % 30 == 0)
			;// printf("Streaming %d\n", statsData.nFramesProcessed);
	}

	disconnect();

	//delete m_grabbedFrame.mImageData;

	qDebug() << "Stopping streaming thread...";
}

void StreamProcess::process()
{
	m_finished = false;
	emit started(THREAD_TYPE_STREAM, "", -1);
	run();
	std::cout << "Streaming Thread - Emit finished signal" << std::endl;
	emit finished(THREAD_TYPE_STREAM, "", -1);
	finishWC.wakeAll();
	streamingThreadInstance = NULL;
	m_finished = true;

	this->moveToThread(g_mainThread);
}


bool StreamProcess::disconnect()
{
	m_Stream.Close();

	return true;
}

void StreamProcess::updateFPS(int timeElapsed)
{
	//
	// Add instantaneous FPS value to queue
	//
	if (timeElapsed > 0)
	{
		fps.enqueue((int)1000 / timeElapsed);
		//
		// Increment sample number
		//
		sampleNumber++;
	}
	statsData.instantFPS = (1000.0 / timeElapsed);
	//
	// Maximum size of queue is DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH
	//
	if (fps.size() > STREAMING_FPS_STAT_QUEUE_LENGTH)
		fps.dequeue();

	//
	// Update FPS value every DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH samples
	//
	if ((fps.size() == STREAMING_FPS_STAT_QUEUE_LENGTH) && (sampleNumber == STREAMING_FPS_STAT_QUEUE_LENGTH))
	{
		//
		// Empty queue and store sum
		//
		while (!fps.empty())
			fpsSum += fps.dequeue();
		//
		// Calculate average FPS
		//
		statsData.averageFPS = fpsSum / STREAMING_FPS_STAT_QUEUE_LENGTH;
		//statsData.averageFPS = cap->getProperty( CV_CAP_PROP_FPS );
		//std::cout << statsData.averageFPS << std::endl;
		//
		// Reset sum
		//
		fpsSum = 0;
		//
		// Reset sample number
		//
		sampleNumber = 0;
	}
}



void StreamProcess::stopStreamThread()
{
	if (!streamingThreadInstance)
		return;
	PANO_LOG("About to stop Stream thread...");
	doExitMutex.lock();
	doExit = true;
	doExitMutex.unlock();

	if (!this->isFinished())
		this->waitForFinish();
	
	PANO_LOG("Stream thread successfully stopped.");
}

void StreamProcess::waitForFinish()
{
	finishMutex.lock();
	finishWC.wait(&finishMutex);
	finishMutex.unlock();
}


bool StreamProcess::initialize(bool toFile, QString outFileName, int width, int height, int fps, int channelCount, 
	AVSampleFormat sampleFmt, int sampleRate, int audioLag, int videoCodec, int audioCodec, int crf)
{
	m_Panorama = NULL;
	m_isOpened = false;
	m_audioInputCount = 0;
	m_audioProcessedCount = 0;
	m_videoInputCount = 0;
	m_videoProcessedCount = 0;
	m_audioFrame = 0;

	m_width = width;
	m_height = height;
	if (outFileName.isEmpty())
	{
		return false;
	}
	int channels = channelCount;
	if (m_sharedImageBuffer->getGlobalAnimSettings()->m_captureType == D360::Capture::CaptureDomain::CAPTURE_DSHOW)
		channels = 2;
	int ret = m_Stream.Initialize(outFileName.toUtf8().constData(), width, height, fps, channels, sampleFmt, sampleRate, audioLag, toFile, videoCodec, audioCodec, crf);
	if (ret >= 0)
	{
		if (m_sharedImageBuffer->getGlobalAnimSettings()->m_captureType == D360::Capture::CaptureDomain::CAPTURE_DSHOW)
			m_Stream.setCaptureType(D360::Capture::CaptureDomain::CAPTURE_DSHOW);

		// Get expected audio channel count for output video
		m_audioChannelCount = m_sharedImageBuffer->getGlobalAnimSettings()->getAudioChannelCount();

		streamingThreadInstance = new QThread();
		this->moveToThread(streamingThreadInstance);
		connect(streamingThreadInstance, SIGNAL(started()), this, SLOT(process()));
		//connect( processingThread, SIGNAL(finished()), processingThreadInstance, SLOT(quit()));
		connect(streamingThreadInstance, SIGNAL(finished()), this, SLOT(deleteLater()));
		connect(this, SIGNAL(finished(int, QString, int)), streamingThreadInstance, SLOT(deleteLater()));
		if (m_Main) {
			connect(this, SIGNAL(finished(int, QString, int)), m_Main, SLOT(finishedThread(int, QString, int)));
			connect(this, SIGNAL(started(int, QString, int)), m_Main, SLOT(startedThread(int, QString, int)));
		}

		m_isOpened = true;

		streamingThreadInstance->start();

		return true;
	}

	return false;
}

void StreamProcess::streamPanorama(unsigned char* panorama)
{
	videoFrameMutex.lock();
	m_Panorama = panorama;
	m_videoInputCount++;
	videoFrameMutex.unlock();
}

void StreamProcess::streamAudio(int devNum, void* audioFrame)
{
	audioFrameMutex.lock();
	//m_audioFrame = audioFrame;
	m_audioFrames[devNum] = audioFrame;

	CameraInput::InputAudioChannelType audioType = m_sharedImageBuffer->getGlobalAnimSettings()->cameraSettingsList()[devNum].audioType;
	
	if (audioType == CameraInput::NoAudio) {
		audioFrameMutex.unlock();
		return; // Error: received audio frame from disabled device
	}
	
	if (m_audioFrames.size() == m_audioChannelCount)
	{
		m_audioReadyFrames = m_audioFrames;
		m_audioFrames.clear();
		m_audioInputCount++;
	}
	audioFrameMutex.unlock();
}

bool StreamProcess::isFinished()
{
	return m_finished;
}

void StreamProcess::playAndPause(bool isPause)
{
	QMutexLocker locker(&doPauseMutex);
	doPause = isPause;
}